
#include <math.h>
#include <float.h>
#include <sys/time.h>

#include <stdio.h>
#include <string>
#include <vector>
#include <cstddef>
#include <array>
#include <map>

#include "window.hpp"
#include <GLFW/glfw3.h>

#include "event.hpp"
#include "ui.hpp"

#include "alikins.hpp"
#include "enharmonic.hpp"
#include "cv_utils.hpp"
#include "specificValueWidgets.hpp"


struct SpecificValue : Module
{
    enum ParamIds
    {
        VALUE1_PARAM,
        OCTAVE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        VALUE1_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        VALUE1_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };


    SpecificValue() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(VALUE1_PARAM, -10.0f, 10.0f, 0.0f, "The voltage", " V");
    }

    void process(const ProcessArgs &args) override;

    float volt_value;
    float hz_value;
    float lfo_hz_value;
    float cents_value;
    float lfo_bpm_value;

};

void SpecificValue::process(const ProcessArgs &args)
{
    if (inputs[VALUE1_INPUT].isConnected()) {
        params[VALUE1_PARAM].setValue(inputs[VALUE1_INPUT].getVoltage());
    }
    volt_value = params[VALUE1_PARAM].getValue();
    outputs[VALUE1_OUTPUT].setVoltage(volt_value);
}

enum AdjustKey
{
    UP,
    DOWN,
    INITIAL
};

struct FloatField : TextField
{
    ParamWidget *paramWidget;
    SpecificValue *module;

    FloatField(SpecificValue *module);

    void step() override {
		TextField::step();
	}

    void setParamWidget(ParamWidget *paramWidget) {
		this->paramWidget = paramWidget;
		if (paramWidget->paramQuantity) {
			text = paramWidget->paramQuantity->getDisplayValueString();
        }
        TextField::selectAll();
	}

    void onAction(const event::Action &e) override;
    void onChange(const event::Change &e) override;

    void onSelectKey(const event::SelectKey &e) override;
    void onDoubleClick(const event::DoubleClick &e) override;

    virtual void handleKeyPress(AdjustKey key, bool shift_pressed, bool mod_pressed);

    virtual void increment(float delta);

    float textToVolts(std::string field_text);
    std::string voltsToText(float param_volts);

    float minValue = -10.0f;
    float maxValue = 10.0f;

    double prev_mouse_up_time = 0.0;

    std::string orig_string;

    bool y_dragging = false;

    float INC = 1.0f;
    float SHIFT_INC = 10.1f;
    float MOD_INC = 0.1f;

};

FloatField::FloatField(SpecificValue *_module)
{
    module = _module;

    INC = 0.01f;
    SHIFT_INC = 0.1f;
    MOD_INC = 0.001f;
}

float FloatField::textToVolts(std::string field_text) {
    return atof(field_text.c_str());
}

std::string FloatField::voltsToText(float param_volts){
    return string::f("%#.4g", param_volts);
}

void FloatField::onChange(const event::Change &e) {
    // debug("FloatField onChange  text=%s param=%f", text.c_str(),
    //    module->params[SpecificValue::VALUE1_PARAM].getValue());
    if (!module) {
        return;
    }

    if (this != APP->event->selectedWidget)
    {
        std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].getValue());
        setText(new_text);
    }
}

void FloatField::onAction(const event::Action &e)
{
    TextField::onAction(e);
    float volts = textToVolts(text);

    if (!module)
        return;

    module->params[SpecificValue::VALUE1_PARAM].setValue(volts);
    e.consume(this);
}

void FloatField::onDoubleClick(const event::DoubleClick &e) {
    selectAll();
}

void FloatField::increment(float delta) {
    float field_value = atof(text.c_str());
    field_value += delta;

    field_value = clampSafe(field_value, minValue, maxValue);
    text = voltsToText(field_value);
    // debug("orig_string: %s text: %s", orig_string.c_str(), text.c_str());
}

void FloatField::handleKeyPress(AdjustKey adjustKey, bool shift_pressed, bool mod_pressed) {
    float inc = shift_pressed ? SHIFT_INC : INC;
    // mod "wins" if shift and mod pressed
    inc = mod_pressed ? MOD_INC : inc;
    inc = adjustKey == AdjustKey::UP ? inc : -inc;

    increment(inc);

    event::Action e;
    onAction(e);
}

void FloatField::onSelectKey(const event::SelectKey &e) {
    // debug("e.key: %d", e.key);

    if (!e.getTarget())
        TextField::onSelectKey(e);

    bool shift_pressed = ((APP->window->getMods() & RACK_MOD_MASK) == GLFW_MOD_SHIFT);
    bool mod_pressed = ((APP->window->getMods() & RACK_MOD_MASK) == GLFW_MOD_ALT);

    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
        switch (e.key) {
            case GLFW_KEY_UP: {
                handleKeyPress(AdjustKey::UP, shift_pressed, mod_pressed);
                e.consume(this);
            } break;
            case GLFW_KEY_DOWN: {
                handleKeyPress(AdjustKey::DOWN, shift_pressed, mod_pressed);
                e.consume(this);
            } break;
        }
        e.consume(this);
    }

}

struct HZFloatField : FloatField
{
    SpecificValue *module;

    HZFloatField(SpecificValue *_module) ;

    void onChange(const event::Change &e) override;
    void onAction(const event::Action &e) override;

    void increment(float delta) override;

    float textToVolts(std::string field_text);
    std::string voltsToText(float param_volts);
};

HZFloatField::HZFloatField(SpecificValue *_module) : FloatField(_module)
{
    module = _module;
    INC = 1.0f;
    SHIFT_INC = 10.0f;
    MOD_INC = 0.1f;

    minValue = cv_to_freq(-10.0f);
    maxValue = cv_to_freq(10.0f);
}

float HZFloatField::textToVolts(std::string field_text) {
    float freq = strtof(text.c_str(), NULL);
    return freq_to_cv(freq);
}

std::string HZFloatField::voltsToText(float param_volts){
    float freq = cv_to_freq(param_volts);
    // std::string new_text = string::f("%0.*g", freq < 100 ? 3 : 2, freq);
    std::string new_text = string::f("%#.*g", freq < 100 ? 6: 7, freq);

    return new_text;
}

void HZFloatField::increment(float delta){
    float field_value = atof(text.c_str());
    field_value += delta;
    field_value = clampSafe(field_value, minValue, maxValue);

    text = string::f("%#.*g", field_value < 100 ? 6: 7, field_value);
}

void HZFloatField::onChange(const event::Change &e) {
    if (!module)
        return;

    if (this != APP->event->selectedWidget)
    {
        std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].getValue());
        setText(new_text);
    }
}

void HZFloatField::onAction(const event::Action &e) {
    TextField::onAction(e);

    float volts = textToVolts(text);

    if (!module)
        return;

    module->params[SpecificValue::VALUE1_PARAM].setValue(volts);
}

struct LFOHzFloatField : FloatField {
    SpecificValue *module;

    LFOHzFloatField(SpecificValue *_module);

    void onAction(const event::Action &e) override;
    void onChange(const event::Change &e) override;

    void increment(float delta) override;

    float textToVolts(std::string field_text);
    std::string voltsToText(float param_volts);
};

LFOHzFloatField::LFOHzFloatField(SpecificValue *_module) : FloatField(_module)
{
    module = _module;
    INC = 0.01f;
    SHIFT_INC = 0.1f;
    MOD_INC = 0.001f;

    minValue = lfo_cv_to_freq(-10.0f);
    maxValue = lfo_cv_to_freq(10.0f);
}

float LFOHzFloatField::textToVolts(std::string field_text) {
    float freq_hz = strtof(text.c_str(), NULL);
    return lfo_freq_to_cv(freq_hz);
}

std::string LFOHzFloatField::voltsToText(float param_volts) {
    float lfo_freq_hz = lfo_cv_to_freq(param_volts);
    // std::string new_text = string::f("%0.*f", lfo_freq_hz < 100 ? 4 : 3, lfo_freq_hz);
    std::string new_text = string::f("%#0.*g", lfo_freq_hz < 100 ? 5 : 6, lfo_freq_hz);
    return new_text;
}

void LFOHzFloatField::increment(float delta) {
    float field_value = atof(text.c_str());
    field_value += delta;

    field_value = clampSafe(field_value, minValue, maxValue);
    text = string::f("%#0.*g", 6, field_value);
}

void LFOHzFloatField::onChange(const event::Change &e) {
    if (!module)
        return;

    if (this != APP->event->selectedWidget)
    {
        std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].getValue());
        setText(new_text);
    }
}

void LFOHzFloatField::onAction(const event::Action &e)
{
    TextField::onAction(e);
    float volts = textToVolts(text);

    if (!module)
        return;

    module->params[SpecificValue::VALUE1_PARAM].setValue(volts);
}

struct LFOBpmFloatField : FloatField {
    SpecificValue *module;

    LFOBpmFloatField(SpecificValue *_module);

    void onAction(const event::Action &e) override;
    void onChange(const event::Change &e) override;

    void increment(float delta) override;
    float textToVolts(std::string field_text);
    std::string voltsToText(float param_volts);
};

LFOBpmFloatField::LFOBpmFloatField(SpecificValue *_module) : FloatField(_module)
{
    module = _module;
    INC = 1.0f;
    SHIFT_INC = 10.0f;
    MOD_INC = 0.1f;

    minValue = lfo_cv_to_freq(-10.0f)* 60.0f;
    maxValue = lfo_cv_to_freq(10.0f) * 60.0f;
}

float LFOBpmFloatField::textToVolts(std::string field_text) {
    float lfo_bpm = strtof(text.c_str(), NULL);
    float lfo_hz = lfo_bpm / 60.0f;
    return lfo_freq_to_cv(lfo_hz);
}

std::string LFOBpmFloatField::voltsToText(float param_volts){
    float lfo_freq_hz = lfo_cv_to_freq(param_volts);
    float lfo_bpm = lfo_freq_hz * 60.0f;
    std::string new_text = string::f("%.6g", lfo_bpm);
    return new_text;
}

void LFOBpmFloatField::increment(float delta) {
    float field_value = atof(text.c_str());
    field_value += delta;

    field_value = clampSafe(field_value, minValue, maxValue);
    text = string::f("%.6g", field_value);
}

void LFOBpmFloatField::onChange(const event::Change &e) {
    if (!module)
        return;

    if (this != APP->event->selectedWidget)
    {
        std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].getValue());
        setText(new_text);
    }
}

void LFOBpmFloatField::onAction(const event::Action &e)
{
    TextField::onAction(e);
    float volts = textToVolts(text);

    if (!module)
        return;

    module->params[SpecificValue::VALUE1_PARAM].setValue(volts);
}

struct CentsField : FloatField {
    SpecificValue *module;

    CentsField(SpecificValue *_module);
    void onChange(const event::Change &e) override;
    void onAction(const event::Action &e) override;

    void increment(float delta) override;
};

CentsField::CentsField(SpecificValue *_module) : FloatField(_module) {
    module = _module;
    INC = 1.0f;
    SHIFT_INC = 10.0f;
    MOD_INC = 0.1f;

    minValue = -50.0f;
    maxValue = 50.0f;
}

void CentsField::increment(float delta) {
    float field_value = atof(text.c_str());
    field_value += delta;

    field_value = clampSafe(field_value, minValue, maxValue);
    // debug("field_value1: %f", field_value);
    field_value = chop(field_value, 0.01f);
    // debug("field_value2: %f", field_value);
    text = string::f("%0.2f", field_value);
}

void CentsField::onChange(const event::Change &e) {
    if (!module)
        return;

    float cents = volts_to_note_cents(module->params[SpecificValue::VALUE1_PARAM].getValue());
    cents = chop(cents, 0.01f);
    std::string new_text = string::f("%0.2f", cents);
    setText(new_text);
}

void CentsField::onAction(const event::Action &e) {

    TextField::onAction(e);
    double cents = strtod(text.c_str(), NULL);

    // figure what to tweak the current volts
    double cent_volt = 1.0 / 12.0 / 100.0;
    double delta_volt = cents * cent_volt;

    if (!module)
        return;

    double nearest_note_voltage = volts_of_nearest_note(module->params[SpecificValue::VALUE1_PARAM].getValue());

    module->params[SpecificValue::VALUE1_PARAM].setValue((float) (nearest_note_voltage + delta_volt));

}

struct NoteNameField : TextField {
    ParamWidget *paramWidget;
    SpecificValue *module;

    NoteNameField(SpecificValue *_module);

    void step() override {
		TextField::step();
	}

    void setParamWidget(ParamWidget *paramWidget) {
		this->paramWidget = paramWidget;
		if (paramWidget->paramQuantity) {
			text = paramWidget->paramQuantity->getDisplayValueString();
        }
        TextField::selectAll();
	}

    float minValue = -10.0f;
    float maxValue = 10.0f;

    float orig_value;

    void onChange(const event::Change &e) override;
    void onAction(const event::Action &e) override;
    void onSelectKey(const event::SelectKey &e) override;
    void onDoubleClick(const event::DoubleClick &e) override;

    void increment(float delta);
    void handleKeyPress(AdjustKey key, bool shift_pressed, bool mod_pressed);

    bool y_dragging = false;

    double prev_mouse_up_time = 0.0;

    float INC = 1.0f;
    float SHIFT_INC = 12.0f;
    float MOD_INC = 0.01f;

};

NoteNameField::NoteNameField(SpecificValue *_module)
{
    module = _module;
}

void NoteNameField::increment(float delta) {
    if (!module)
        return;

    float field_value = module->params[SpecificValue::VALUE1_PARAM].getValue();
    field_value += (float) delta * 1.0 / 12.0;

    field_value = clampSafe(field_value, minValue, maxValue);
    field_value = chop(field_value, 0.001f);
    paramWidget->paramQuantity->setValue(field_value);
}

void NoteNameField::handleKeyPress(AdjustKey adjustKey, bool shift_pressed, bool mod_pressed) {
    //inc by oct for shift, and 1 cent for mod
    float inc = shift_pressed ? SHIFT_INC : INC;
    inc = mod_pressed ? MOD_INC : inc;
    inc = adjustKey == AdjustKey::UP ? inc : -inc;

    increment(inc);
}

void NoteNameField::onSelectKey(const event::SelectKey &e) {
    if (!e.getTarget()) {
        TextField::onSelectKey(e);
    }

    bool shift_pressed = ((APP->window->getMods() & RACK_MOD_MASK) == GLFW_MOD_SHIFT);
    bool mod_pressed = ((APP->window->getMods() & RACK_MOD_MASK) == GLFW_MOD_ALT);

    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
        switch (e.key) {
            case GLFW_KEY_UP: {
                handleKeyPress(AdjustKey::UP, shift_pressed, mod_pressed );
                e.consume(this);
            } break;
            case GLFW_KEY_DOWN: {
                handleKeyPress(AdjustKey::DOWN, shift_pressed, mod_pressed);
                e.consume(this);
            } break;
        }
        e.consume(this);
    }
}

void NoteNameField::onChange(const event::Change &e) {
    if (!module)
        return;

    float cv_volts = module->params[SpecificValue::VALUE1_PARAM].getValue();
    int octave = volts_to_octave(cv_volts);
    int note_number = volts_to_note(cv_volts);

    // DEBUG("cv_volts: %f, octave: %d, note_number: %d, can: %s",
    //        cv_volts, octave, note_number, note_name_vec[note_number].c_str());

    std::string new_text = string::f("%s%d", note_name_vec[note_number].c_str(), octave);

    setText(new_text);
}

void NoteNameField::onAction(const event::Action &e) {
    TextField::onAction(e);

    if (!module)
        return;

    // canoicalize the entered note name into a canonical note id
    // (ie, c4 -> C4, Db3 -> C#3, etc)
    // then look that up in enharmonic_name_map to voltage
    NoteOct *noteOct = parseNote(text);

    auto enharm_search = enharmonic_name_map.find(noteOct->name);
    if (enharm_search == enharmonic_name_map.end())
    {
        // debug("%s was  NOT A VALID note name", noteOct->name.c_str());
        return;
    }

    std::string can_note_name = enharmonic_name_map[noteOct->name];

    std::string can_note_id = string::f("%s%s", can_note_name.c_str(), noteOct->octave.c_str());

    /*
    debug("text: %s", text.c_str());
    debug("note_name: %s", noteOct->name.c_str());
    debug("can_note_name: %s", can_note_name.c_str());
    debug("note_name_flag: %s", noteOct->flag.c_str());
    debug("note_oct: %s", noteOct->octave.c_str());
    debug("can_note_id: %s", can_note_id.c_str());
    */

    // search for can_note_id in map to find volts value

    auto search = note_name_to_volts_map.find(can_note_id);
    if(search != note_name_to_volts_map.end()) {
        paramWidget->paramQuantity->setValue((float) note_name_to_volts_map[can_note_id]);
        return;
    }
    else {
        // TODO: change the text color to indicate bogus name?
        // debug("%s was  NOT A VALID CANONICAL NOTE ID", can_note_id.c_str());
        return;
    }
    e.consume(this);
}

void NoteNameField::onDoubleClick(const event::DoubleClick &e) {
    selectAll();
}

struct SpecificValueWidget : ModuleWidget
{

    void step() override;
    void onChange(const event::Change &e) override;

    float prev_volts = 0.0f;
    float prev_input = 0.0f;

    FloatField *volts_field;
    HZFloatField *hz_field;
    LFOHzFloatField *lfo_hz_field;
    NoteNameField *note_name_field;
    CentsField *cents_field;
    LFOBpmFloatField *lfo_bpm_field;

    SpecificValueWidget(SpecificValue *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SpecificValue.svg")));

        // TODO: widget with these children?
        float y_baseline = 45.0f;

        Vec volt_field_size = Vec(70.0f, 22.0f);
        Vec hz_field_size = Vec(70.0f, 22.0f);
        Vec lfo_hz_field_size = Vec(70.0f, 22.0f);

        float x_pos = 10.0f;

        y_baseline = 38.0f;

        volts_field = new FloatField(module);
        volts_field->box.pos = Vec(x_pos, y_baseline);
        volts_field->box.size = volt_field_size;

        addChild(volts_field);

        y_baseline = 78.0f;

        float h_pos = x_pos;
        hz_field = new HZFloatField(module);
        hz_field->box.pos = Vec(x_pos, y_baseline);
        hz_field->box.size = hz_field_size;

        addChild(hz_field);

        y_baseline = 120.0f;

        lfo_hz_field = new LFOHzFloatField(module);
        lfo_hz_field->box.pos = Vec(h_pos, y_baseline);
        lfo_hz_field->box.size = lfo_hz_field_size;

        addChild(lfo_hz_field);

        y_baseline += lfo_hz_field->box.size.y;
        y_baseline += 5.0f;
        y_baseline += 12.0f;

        lfo_bpm_field = new LFOBpmFloatField(module);
        lfo_bpm_field->box.pos = Vec(x_pos, y_baseline);
        lfo_bpm_field->box.size = Vec(70.0f, 22.0f);

        addChild(lfo_bpm_field);

        y_baseline += lfo_bpm_field->box.size.y;
        y_baseline += 20.0f;

        note_name_field = new NoteNameField(module);
        note_name_field->box.pos = Vec(x_pos, y_baseline);
        note_name_field->box.size = Vec(70.0f, 22.0f);

        addChild(note_name_field);

        y_baseline += note_name_field->box.size.y;
        y_baseline += 5.0f;

        cents_field = new CentsField(module);
        cents_field->box.pos = Vec(x_pos, y_baseline);
        cents_field->box.size = Vec(55.0f, 22.0f);

        addChild(cents_field);

        y_baseline += cents_field->box.size.y;
        y_baseline += 5.0f;

        float middle = box.size.x / 2.0f;
        float in_port_x = 15.0f;

        y_baseline += 12.0f;

        PJ301MPort *value_in_port = createInput<PJ301MPort>(
            Vec(in_port_x, y_baseline),
            module,
            SpecificValue::VALUE1_INPUT);

        value_in_port->box.pos = Vec(2.0f, y_baseline);

        inputs.push_back(value_in_port);
        addChild(value_in_port);

        float out_port_x = middle + 24.0f;

        PJ301MPort *value_out_port = createOutput<PJ301MPort>(
            Vec(out_port_x, y_baseline),
            module,
            SpecificValue::VALUE1_OUTPUT);

        outputs.push_back(value_out_port);
        value_out_port->box.pos = Vec(box.size.x - value_out_port->box.size.x - 2.0f, y_baseline);

        addChild(value_out_port);

        y_baseline += value_out_port->box.size.y;
        y_baseline += 16.0f;

        PurpleTrimpot *trimpot = createParam<PurpleTrimpot>(
            Vec(middle - 24.0f, y_baseline + 4.5f),
            module,
            SpecificValue::VALUE1_PARAM);

        addParam(trimpot);

        volts_field->setParamWidget(trimpot);
        note_name_field->setParamWidget(trimpot);

        // fire off an event to refresh all the widgets
        event::Change e;
        onChange(e);

    }
};

void SpecificValueWidget::step() {
    ModuleWidget::step();

    if (!module)
        return;

    float param_value = module->params[SpecificValue::VALUE1_PARAM].getValue();
    float input_value = module->params[SpecificValue::VALUE1_INPUT].getValue();


    if (prev_volts != param_value ||
        prev_input != input_value) {

            // debug("SpVWidget step - emitting EventChange / onChange prev_volts=%f param=%f",
            //     prev_volts, module->params[SpecificValue::VALUE1_PARAM].getValue());
            prev_volts = param_value;
            prev_input = input_value;

            // ignore Nan/Inf and dont emit change
            if (std::isfinite(param_value) && std::isfinite(input_value)) {
                event::Change e;
                onChange(e);
            }
    }
}

void SpecificValueWidget::onChange(const event::Change &e) {
    ModuleWidget::onChange(e);

    if (!module)
        return;

    volts_field->onChange(e);
    hz_field->onChange(e);
    lfo_hz_field->onChange(e);
    note_name_field->onChange(e);
    cents_field->onChange(e);
    lfo_bpm_field->onChange(e);
}

Model *modelSpecificValue = createModel<SpecificValue, SpecificValueWidget>("SpecificValue");

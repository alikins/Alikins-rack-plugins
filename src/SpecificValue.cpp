#include <stdio.h>
#include <string>
#include <vector>
#include <cstddef>
#include <array>
#include <map>
#include <unordered_map>
#include <math.h>
#include <float.h>

#include "window.hpp"
#include <GLFW/glfw3.h>


#include "alikins.hpp"
#include "ui.hpp"
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


    SpecificValue() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    float volt_value;
    float hz_value;
    float lfo_hz_value;
    float cents_value;
    float lfo_bpm_value;

};

void SpecificValue::step()
{
    if (inputs[VALUE1_INPUT].active) {
        params[VALUE1_PARAM].value = inputs[VALUE1_INPUT].value;
    }
    volt_value = params[VALUE1_PARAM].value;
    outputs[VALUE1_OUTPUT].value = volt_value;
}

enum AdjustKey
{
    UP,
    DOWN,
    INITIAL
};

struct FloatField : TextField
{
    SpecificValue *module;

    FloatField(SpecificValue *module);

    void onAction(EventAction &e) override;
    void onChange(EventChange &e) override;
    void onKey(EventKey &e) override;

    virtual void increment(float delta);

    float textToVolts(std::string field_text);
    std::string voltsToText(float param_volts);

    virtual void handleKey(AdjustKey key, bool shift_pressed, bool mod_pressed);

    float INC = 1.0f;
    float SHIFT_INC = 0.1f;
    float MOD_INC = 0.001f;
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
    return stringf("%0.3f", param_volts);
}

void FloatField::onChange(EventChange &e) {
    // debug("FloatField onChange  text=%s param=%f", text.c_str(),
    //    module->params[SpecificValue::VALUE1_PARAM].value);
    std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].value);
    setText(new_text);
}

void FloatField::onAction(EventAction &e)
{
    TextField::onAction(e);
    float volts = textToVolts(text);
    module->params[SpecificValue::VALUE1_PARAM].value = volts;
}

void FloatField::increment(float delta) {
    // debug("inc delta: %f", delta);
    float field_value = atof(text.c_str());
    field_value += delta;
    text = voltsToText(field_value);
    // debug("new text: %s", text.c_str());
}

void FloatField::handleKey(AdjustKey adjustKey, bool shift_pressed, bool mod_pressed) {
    // debug("INC %f shift_pressed: %d mod_pressed: %d", INC, shift_pressed, mod_pressed);
    float inc = shift_pressed ? SHIFT_INC : INC;
    // mod "wins" if shift and mod pressed
    inc = mod_pressed ? MOD_INC : inc;
    inc = adjustKey == AdjustKey::UP ? inc : -inc;

    // debug("inc: %f", inc);
    increment(inc);

    EventAction e;
    onAction(e);
}

void FloatField::onKey(EventKey &e) {
    // debug("e.key: %d", e.key);
    bool shift_pressed = windowIsShiftPressed();
    bool mod_pressed = windowIsModPressed();

	switch (e.key) {
		case GLFW_KEY_UP: {
			e.consumed = true;
            handleKey(AdjustKey::UP, shift_pressed, mod_pressed);
		} break;
		case GLFW_KEY_DOWN: {
			e.consumed = true;
            handleKey(AdjustKey::DOWN, shift_pressed, mod_pressed);
		} break;
	}

	if (!e.consumed) {
		TextField::onKey(e);
	}
}

struct HZFloatField : FloatField
{
    SpecificValue *module;

    HZFloatField(SpecificValue *_module) ;
    void onChange(EventChange &e) override;
    void onAction(EventAction &e) override;

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
}

float HZFloatField::textToVolts(std::string field_text) {
    float freq = strtof(text.c_str(), NULL);
    return freq_to_cv(freq);
}

std::string HZFloatField::voltsToText(float param_volts){
    float freq = cv_to_freq(param_volts);
    std::string new_text = stringf("%0.*f", freq < 100 ? 4 : 3, freq);
    return new_text;
}

void HZFloatField::increment(float delta){
    // debug("HZ incr delta=%f", delta);
    float field_value = atof(text.c_str());
    field_value += delta;
    text = stringf("%0.*f", field_value < 100 ? 4 : 3, field_value);
}

void HZFloatField::onChange(EventChange &e) {
     if (this != gFocusedWidget)
     {
         std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].value);
         setText(new_text);
     }
}

void HZFloatField::onAction(EventAction &e) {
    TextField::onAction(e);

    float volts = textToVolts(text);

    module->params[SpecificValue::VALUE1_PARAM].value = volts;
}

struct LFOHzFloatField : FloatField {
    SpecificValue *module;

    LFOHzFloatField(SpecificValue *_module);
    void onAction(EventAction &e) override;
    void onChange(EventChange &e) override;

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
}

float LFOHzFloatField::textToVolts(std::string field_text) {
    float freq_hz = strtof(text.c_str(), NULL);
    return lfo_freq_to_cv(freq_hz);
}

std::string LFOHzFloatField::voltsToText(float param_volts) {
    float lfo_freq_hz = lfo_cv_to_freq(param_volts);
    std::string new_text = stringf("%0.*f", lfo_freq_hz < 100 ? 4 : 3, lfo_freq_hz);
    return new_text;
}

void LFOHzFloatField::increment(float delta) {
    float field_value = atof(text.c_str());
    field_value += delta;
    text = stringf("%0.*f", field_value < 100 ? 4 : 3, field_value);
}

void LFOHzFloatField::onChange(EventChange &e) {
    if (this != gFocusedWidget)
    {
        std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].value);
        setText(new_text);
    }
}

void LFOHzFloatField::onAction(EventAction &e)
{
    TextField::onAction(e);
    float volts = textToVolts(text);
    module->params[SpecificValue::VALUE1_PARAM].value = volts;
}

struct LFOBpmFloatField : FloatField {
    SpecificValue *module;

    LFOBpmFloatField(SpecificValue *_module);
    void onAction(EventAction &e) override;
    void onChange(EventChange &e) override;

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
}

float LFOBpmFloatField::textToVolts(std::string field_text) {
    float lfo_bpm = strtof(text.c_str(), NULL);
    float lfo_hz = lfo_bpm / 60.0f;
    return lfo_freq_to_cv(lfo_hz);
}

std::string LFOBpmFloatField::voltsToText(float param_volts){
    float lfo_freq_hz = lfo_cv_to_freq(param_volts);
    float lfo_bpm = lfo_freq_hz * 60.0f;
    std::string new_text = stringf("%0.*f", lfo_bpm < 100 ? 4 : 3, lfo_bpm);
    return new_text;
}

void LFOBpmFloatField::increment(float delta) {
    float field_value = atof(text.c_str());
    field_value += delta;
    text = stringf("%0.*f", field_value < 100 ? 4 : 3, field_value);
}

void LFOBpmFloatField::onChange(EventChange &e) {
     if (this != gFocusedWidget)
     {
         std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].value);
         setText(new_text);
     }
}

void LFOBpmFloatField::onAction(EventAction &e)
{
    TextField::onAction(e);
    float volts = textToVolts(text);
    module->params[SpecificValue::VALUE1_PARAM].value = volts;
}

struct CentsField : FloatField {
    SpecificValue *module;

    CentsField(SpecificValue *_module);
    void onChange(EventChange &e) override;
    void onAction(EventAction &e) override;

    void increment(float delta) override;
};

CentsField::CentsField(SpecificValue *_module) : FloatField(_module) {
    module = _module;
    INC = 0.1f;
    SHIFT_INC = 1.0f;
    MOD_INC = 0.01f;
}

void CentsField::increment(float delta) {
    float field_value = atof(text.c_str());
    field_value += delta;
    text = stringf("% 0.2f", field_value < 100 ? 4 : 3, field_value);
}

void CentsField::onChange(EventChange &e) {
    float cents = volts_to_note_cents(module->params[SpecificValue::VALUE1_PARAM].value);
    std::string new_text = stringf("% 0.2f", cents);
    setText(new_text);
}

void CentsField::onAction(EventAction &e) {

    TextField::onAction(e);
    float cents = strtof(text.c_str(), NULL);

    // figure what to tweak the current volts
    float cent_volt = 1.0f / 12.0f / 100.0f;
    float delta_volt = cents * cent_volt;
    float nearest_note_voltage = volts_of_nearest_note(module->params[SpecificValue::VALUE1_PARAM].value);

    module->params[SpecificValue::VALUE1_PARAM].value = nearest_note_voltage + delta_volt;

}

struct NoteNameField : TextField {
    SpecificValue *module;

    NoteNameField(SpecificValue *_module);
    void onChange(EventChange &e) override;
    void onAction(EventAction &e) override;
    void onKey(EventKey &e) override;

    void increment(float delta);
    void handleKey(AdjustKey key, bool shift_pressed, bool mod_pressed);
};

NoteNameField::NoteNameField(SpecificValue *_module)
{
    module = _module;
}

void NoteNameField::increment(float delta) {
    module->params[SpecificValue::VALUE1_PARAM].value += delta * 1.0f / 12.0f;
}

void NoteNameField::handleKey(AdjustKey adjustKey, bool shift_pressed, bool mod_pressed) {
    //inc by oct for shift, and 1 cent for mod
    float inc = shift_pressed ? 12.0f : 1.0f;
    inc = mod_pressed ? 0.01f : inc;
    inc = adjustKey == AdjustKey::UP ? inc : -inc;

    increment(inc);

    EventChange e;
    onChange(e);
}

void NoteNameField::onKey(EventKey &e) {
    bool shift_pressed = windowIsShiftPressed();
    bool mod_pressed = windowIsModPressed();

	switch (e.key) {
		case GLFW_KEY_UP: {
			e.consumed = true;
            handleKey(AdjustKey::UP, shift_pressed, mod_pressed );
		} break;
		case GLFW_KEY_DOWN: {
			e.consumed = true;
            handleKey(AdjustKey::DOWN, shift_pressed, mod_pressed);
		} break;
	}

	if (!e.consumed) {
		TextField::onKey(e);
	}
}

void NoteNameField::onChange(EventChange &e) {
    float cv_volts = module->params[SpecificValue::VALUE1_PARAM].value;
    int octave = volts_to_octave(cv_volts);
    int note_number = volts_to_note(cv_volts);

    std::string new_text = stringf("%s%d", note_name_vec[note_number].c_str(), octave);

    setText(new_text);
}

void NoteNameField::onAction(EventAction &e) {
    TextField::onAction(e);

    // canoicalize the entered note name into a canonical note id
    // (ie, c4 -> C4, Db3 -> C#3, etc)
    // then look that up in enharmonic_name_map to voltage
    NoteOct *noteOct = parseNote(text);

    auto enharm_search = enharmonic_name_map.find(noteOct->name);
    if (enharm_search == enharmonic_name_map.end())
    {
        debug("%s was  NOT A VALID note name", noteOct->name.c_str());
        return;
    }

    std::string can_note_name = enharmonic_name_map[noteOct->name];

    std::string can_note_id = stringf("%s%s", can_note_name.c_str(), noteOct->octave.c_str());

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
        module->params[SpecificValue::VALUE1_PARAM].value = note_name_to_volts_map[can_note_id];
        return;
    }
    else {
        // TODO: change the text color to indicate bogus name?
        debug("%s was  NOT A VALID CANONICAL NOTE ID", can_note_id.c_str());
        return;
    }
}

struct SpecificValueWidget : ModuleWidget
{
    SpecificValueWidget(SpecificValue *module);

    void step() override;
    void onChange(EventChange &e) override;

    float prev_volts = 0.0f;
    float prev_input = 0.0f;

    FloatField *volts_field;
    HZFloatField *hz_field;
    LFOHzFloatField *lfo_hz_field;
    NoteNameField *note_name_field;
    CentsField *cents_field;
    LFOBpmFloatField *lfo_bpm_field;
};

SpecificValueWidget::SpecificValueWidget(SpecificValue *module) : ModuleWidget(module)
{
    setPanel(SVG::load(assetPlugin(plugin, "res/SpecificValue.svg")));

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

    Port *value_in_port = Port::create<PJ301MPort>(
        Vec(in_port_x, y_baseline),
        Port::INPUT,
        module,
        SpecificValue::VALUE1_INPUT);

    value_in_port->box.pos = Vec(2.0f, y_baseline);

    inputs.push_back(value_in_port);
    addChild(value_in_port);

    float out_port_x = middle + 24.0f;

    Port *value_out_port = Port::create<PJ301MPort>(
        Vec(out_port_x, y_baseline),
        Port::OUTPUT,
        module,
        SpecificValue::VALUE1_OUTPUT);

    outputs.push_back(value_out_port);
    value_out_port->box.pos = Vec(box.size.x - value_out_port->box.size.x - 2.0f, y_baseline);

    addChild(value_out_port);

    y_baseline += value_out_port->box.size.y;
    y_baseline += 16.0f;

    PurpleTrimpot *trimpot = ParamWidget::create<PurpleTrimpot>(
        Vec(middle - 24.0f, y_baseline + 4.5f),
        module,
        SpecificValue::VALUE1_PARAM,
        -10.0f, 10.0f, 0.0f);

    params.push_back(trimpot);
    addChild(trimpot);

    // fire off an event to refresh all the widgets
    EventChange e;
    onChange(e);
}

void SpecificValueWidget::step() {
    ModuleWidget::step();

    if (prev_volts != module->params[SpecificValue::VALUE1_PARAM].value ||
        prev_input != module->params[SpecificValue::VALUE1_INPUT].value) {
            // debug("SpVWidget step - emitting EventChange / onChange prev_volts=%f param=%f",
            //     prev_volts, module->params[SpecificValue::VALUE1_PARAM].value);
            prev_volts = module->params[SpecificValue::VALUE1_PARAM].value;
            prev_input = module->params[SpecificValue::VALUE1_INPUT].value;
            EventChange e;
		    onChange(e);
    }
}

void SpecificValueWidget::onChange(EventChange &e) {
    ModuleWidget::onChange(e);
    volts_field->onChange(e);
    hz_field->onChange(e);
    lfo_hz_field->onChange(e);
    note_name_field->onChange(e);
    cents_field->onChange(e);
    lfo_bpm_field->onChange(e);
}

Model *modelSpecificValue = Model::create<SpecificValue, SpecificValueWidget>(
    "Alikins", "SpecificValue", "Specific Value", UTILITY_TAG);

#include <stdio.h>
#include <string>
#include <vector>
#include <cstddef>
#include <array>
#include <map>
#include <math.h>
#include <float.h>
#include <sys/time.h>

#include "window.hpp"
#include <GLFW/glfw3.h>

#include "alikins.hpp"
#include "ui.hpp"
#include "event.hpp"
#include "enharmonic.hpp"
#include "cv_utils.hpp"
#include "specificValueWidgets.hpp"

#define DOUBLE_CLICK_SECS 0.5

// From KISS FFT,
// https://github.com/mborgerding/kissfft/blob/master/test/testcpp.cc
// BSD-3-Clause
static inline
double curtime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec*.000001;
}

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
    SpecificValue *module;

    FloatField(SpecificValue *module);
    void onAction(const event::Action &e) override;
    void onChange(const event::Change &e) override;
    void onHoverKey(const event::HoverKey &e) override;

	// void onMouseMove(ventMouseMove &e) override;
    // void onMouseUp(EventMouseUp &e) override;

    // void onDragMove(const event::DragMove &e) override;
    void onDragEnd(const event::DragEnd &e) override;

    // TODO: was used to implement 'esc' undo behavior,
    //       need to figure out how to do that in v1
    // void onFocus(EventFocus &e) override;

    virtual void handleKey(AdjustKey key, bool shift_pressed, bool mod_pressed);

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
    std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].getValue());
    setText(new_text);
}

void FloatField::onAction(const event::Action &e)
{
    TextField::onAction(e);
    float volts = textToVolts(text);

    module->params[SpecificValue::VALUE1_PARAM].setValue(volts);
}

/*
void FloatField::onFocus(EventFocus &e) {
    orig_string = text;
    TextField::onFocus(e);
}
*/

/*
void FloatField::onMouseUp(EventMouseUp &e) {
    double new_mouse_up_time = curtime();
    double delta = new_mouse_up_time - prev_mouse_up_time;
    prev_mouse_up_time = new_mouse_up_time;

    if (delta <= DOUBLE_CLICK_SECS) {
        // Select 'all' text in the field
        selection = 0;
		cursor = text.size();
    }
}
*/

/*
void FloatField::onMouseMove(EventMouseMove &e) {
	if (this == gDraggedWidget) {
        // debug("FloatField::onMouseMove y_dragging: %d", y_dragging);
        if (e.mouseRel.x != 0.0f && !y_dragging)  {
            TextField::onMouseMove(e);
            return;
        }
    }
}
*/

/*
void FloatField::onDragMove(const event::DragMove &e)
{
    // wait until we are moving and can tell if up/down or left/right before locking the cursor

    // no vertical cursor movement, dont do anything. In particular, not
    // locking the cursor so text selection keeps working.
    if (e.mouseDelta.y == 0.0f || fabs(e.mouseDelta.x) >= 1.0f) {
        APP->
        if (this == gDraggedWidget) {
            return;
        }
    }
    // debug("FloatField::onDragMove doing SOMETHING, start y_dragging");
    y_dragging = true;

    // lock the
    windowCursorLock();

    bool shift_pressed = windowIsShiftPressed();
    bool mod_pressed = windowIsModPressed();

    float inc = shift_pressed ? SHIFT_INC : INC;
    // mod "wins" if shift and mod pressed
    inc = mod_pressed ? MOD_INC : inc;

    float delta = inc * -e.mouseRel.y;

    increment(delta);

    // we change the text in the field, trigger onAction to update the param
    EventAction ae;
    onAction(ae);
}
*/

void FloatField::onDragEnd(const event::DragEnd &e) {
    // mouse key released, stop dragging and release the cursor lock
    y_dragging = false;
    windowCursorUnlock();
}

void FloatField::increment(float delta) {
    float field_value = atof(text.c_str());
    field_value += delta;

    field_value = clamp2(field_value, minValue, maxValue);
    text = voltsToText(field_value);
    // debug("orig_string: %s text: %s", orig_string.c_str(), text.c_str());
}

void FloatField::handleKey(AdjustKey adjustKey, bool shift_pressed, bool mod_pressed) {
    float inc = shift_pressed ? SHIFT_INC : INC;
    // mod "wins" if shift and mod pressed
    inc = mod_pressed ? MOD_INC : inc;
    inc = adjustKey == AdjustKey::UP ? inc : -inc;

    increment(inc);

    event::Action e;
    onAction(e);
}

void FloatField::onHoverKey(const event::HoverKey &e) {
    // debug("e.key: %d", e.key);
    // bool shift_pressed = windowIsShiftPressed();
    // bool mod_pressed = windowIsModPressed();

    bool shift_pressed = ((APP->window->getMods() & RACK_MOD_MASK) == GLFW_MOD_SHIFT);
    bool mod_pressed = ((APP->window->getMods() & RACK_MOD_MASK) == GLFW_MOD_ALT);
    // e.consume(this);
    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
        switch (e.key) {
            case GLFW_KEY_UP: {
                e.consume(this);
                handleKey(AdjustKey::UP, shift_pressed, mod_pressed);
            } break;
            case GLFW_KEY_DOWN: {
                e.consume(this);
                handleKey(AdjustKey::DOWN, shift_pressed, mod_pressed);
            } break;
            case GLFW_KEY_ESCAPE: {
                e.consume(this);
                // debug("escape key pressed, orig_string: %s", orig_string.c_str());
                text = orig_string;
                event::Action ea;
                onAction(ea);
            } break;
        }
    }

	if (!e.isConsumed()) {
		TextField::onHoverKey(e);
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
    if (this != gFocusedWidget)
    {
        std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].getValue());
        setText(new_text);
    }
}

void HZFloatField::onAction(const event::Action &e) {
    TextField::onAction(e);

    float volts = textToVolts(text);

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
    if (this != gFocusedWidget)
    {
        std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].getValue());
        setText(new_text);
    }
}

void LFOHzFloatField::onAction(const event::Action &e)
{
    TextField::onAction(e);
    float volts = textToVolts(text);
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
    if (this != gFocusedWidget)
    {
        std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].getValue());
        setText(new_text);
    }
}

void LFOBpmFloatField::onAction(const event::Action &e)
{
    TextField::onAction(e);
    float volts = textToVolts(text);
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
    double nearest_note_voltage = volts_of_nearest_note(module->params[SpecificValue::VALUE1_PARAM].getValue());

    module->params[SpecificValue::VALUE1_PARAM].setValue((float) (nearest_note_voltage + delta_volt));

}

struct NoteNameField : TextField {
    SpecificValue *module;

    NoteNameField(SpecificValue *_module);

    float minValue = -10.0f;
    float maxValue = 10.0f;

    float orig_value;

    void onChange(const event::Change &e) override;
    void onAction(const event::Action &e) override;
    void onHoverKey(const event::HoverKey &e) override;

	void onMouseMove(EventMouseMove &e) override;
    void onMouseUp(EventMouseUp &e) override;

    // void onDragMove(const event::DragMove &e) override;
    void onDragEnd(const event::DragEnd &e) override;

    // void onFocus(EventFocus &e) override;

    void increment(float delta);
    void handleKey(AdjustKey key, bool shift_pressed, bool mod_pressed);

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
    float field_value = module->params[SpecificValue::VALUE1_PARAM].getValue();
    field_value += (float) delta * 1.0 / 12.0;

    field_value = clampSafe(field_value, minValue, maxValue);
    field_value = chop(field_value, 0.001f);
    module->params[SpecificValue::VALUE1_PARAM].setValue(field_value);
}

void NoteNameField::handleKey(AdjustKey adjustKey, bool shift_pressed, bool mod_pressed) {
    //inc by oct for shift, and 1 cent for mod
    float inc = shift_pressed ? SHIFT_INC : INC;
    inc = mod_pressed ? MOD_INC : inc;
    inc = adjustKey == AdjustKey::UP ? inc : -inc;

    increment(inc);

    event::Change e;
    onChange(e);
}

void NoteNameField::onHoverKey(const event::HoverKey &e) {
    bool shift_pressed = windowIsShiftPressed();
    bool mod_pressed = windowIsModPressed();

    if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
        switch (e.key) {
            case GLFW_KEY_UP: {
                e.consumed = true;
                handleKey(AdjustKey::UP, shift_pressed, mod_pressed );
            } break;
            case GLFW_KEY_DOWN: {
                e.consumed = true;
                handleKey(AdjustKey::DOWN, shift_pressed, mod_pressed);
            } break;
            case GLFW_KEY_ESCAPE: {
                e.consumed = true;
                // debug("escape key pressed, orig_value: %0.5f", orig_value);
                module->params[SpecificValue::VALUE1_PARAM].getValue() = orig_value;
                EventChange ec;
                onChange(ec);
            } break;
        }
    }

	if (!e.consumed) {
		TextField::onHoverKey(e);
	}
}

/*
void NoteNameField::onFocus(EventFocus &e) {
    orig_value = module->params[SpecificValue::VALUE1_PARAM].getValue();
    // debug("onFocus orig_value: %0.5f", orig_value);
    TextField::onFocus(e);
}
*/

void NoteNameField::onChange(const event::Change &e) {
    float cv_volts = module->params[SpecificValue::VALUE1_PARAM].getValue();
    int octave = volts_to_octave(cv_volts);
    int note_number = volts_to_note(cv_volts);
    /* debug("cv_volts: %f, octave: %d, note_number: %d, can: %s",
     cv_volts, octave, note_number, note_name_vec[note_number].c_str()); */
    std::string new_text = string::f("%s%d", note_name_vec[note_number].c_str(), octave);

    setText(new_text);
}

void NoteNameField::onAction(const event::Action &e) {
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
        // float f = (float) orig;
        // module->params[SpecificValue::VALUE1_PARAM].getValue() = note_name_to_volts_map[can_note_id];
        module->params[SpecificValue::VALUE1_PARAM].setValue((float) note_name_to_volts_map[can_note_id]);

        return;
    }
    else {
        // TODO: change the text color to indicate bogus name?
        debug("%s was  NOT A VALID CANONICAL NOTE ID", can_note_id.c_str());
        return;
    }
}

void NoteNameField::onMouseMove(EventMouseMove &e) {
	if (this == gDraggedWidget) {
        if (e.mouseRel.x != 0.0f && !y_dragging)  {
            TextField::onMouseMove(e);
            return;
        }
    }
}

// FIXME: refactor to share this and other bits better
// with FloatField and friends
void NoteNameField::onMouseUp(EventMouseUp &e) {
    double new_mouse_up_time = curtime();
    double delta = new_mouse_up_time - prev_mouse_up_time;
    prev_mouse_up_time = new_mouse_up_time;

    if (delta <= DOUBLE_CLICK_SECS) {
        // Select 'all' text in the field
        selection = 0;
		cursor = text.size();
    }
}

/*
void NoteNameField::onDragMove(const event::DragMove &e)
{
    // TextField::onDragMove(e);

    if (e.mouseDelta.y == 0.0f || fabs(e.mouseDelta.x) >= 1.0f) {
        if (this == gDraggedWidget) {
            return;
        }
    }

    y_dragging = true;

    windowCursorLock();

    bool shift_pressed = windowIsShiftPressed();
    bool mod_pressed = windowIsModPressed();

    float inc = shift_pressed ? SHIFT_INC : INC;
    inc = mod_pressed ? MOD_INC : inc;

    float delta = inc * -e.mouseRel.y;


    //debug("v: %0.5f, dy: %0.5f, delta: %0.5f",
    //    module->params[SpecificValue::VALUE1_PARAM].getValue(),
    //    e.mouseRel.y,
    //    delta);


    increment(delta);

    EventChange ce;
    onChange(ce);
}
*/

void NoteNameField::onDragEnd(const event::DragEnd &e) {
    windowCursorUnlock();
    y_dragging = false;
}


struct SpecificValueWidget : ModuleWidget
{
    SpecificValueWidget(SpecificValue *module);

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
};

SpecificValueWidget::SpecificValueWidget(SpecificValue *module) : ModuleWidget(module)
{
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

    Port *value_in_port = createInput<PJ301MPort>(
        Vec(in_port_x, y_baseline),
        module,
        SpecificValue::VALUE1_INPUT);

    value_in_port->box.pos = Vec(2.0f, y_baseline);

    inputs.push_back(value_in_port);
    addChild(value_in_port);

    float out_port_x = middle + 24.0f;

    Port *value_out_port = createOutput<PJ301MPort>(
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

    if (prev_volts != module->params[SpecificValue::VALUE1_PARAM].getValue() ||
        prev_input != module->params[SpecificValue::VALUE1_INPUT].getValue()) {
            // debug("SpVWidget step - emitting EventChange / onChange prev_volts=%f param=%f",
            //     prev_volts, module->params[SpecificValue::VALUE1_PARAM].getValue());
            prev_volts = module->params[SpecificValue::VALUE1_PARAM].getValue();
            prev_input = module->params[SpecificValue::VALUE1_INPUT].getValue();
            EventChange e;
		    onChange(e);
    }
}

void SpecificValueWidget::onChange(const event::Change &e) {
    ModuleWidget::onChange(e);
    volts_field->onChange(e);
    hz_field->onChange(e);
    lfo_hz_field->onChange(e);
    note_name_field->onChange(e);
    cents_field->onChange(e);
    lfo_bpm_field->onChange(e);
}

Model *modelSpecificValue = createModel<SpecificValue, SpecificValueWidget>("SpecificValue");

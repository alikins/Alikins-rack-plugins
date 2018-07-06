#include <stdio.h>
#include <string>
#include <vector>
#include <cstddef>
#include <array>
#include <map>
#include <unordered_map>
#include <math.h>
#include <float.h>

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

std::unordered_map<std::string, float> note_name_to_volts_map = gen_note_name_map();
std::unordered_map<std::string, std::string> enharmonic_name_map = gen_enharmonic_name_map();

void SpecificValue::step()
{

    if (inputs[VALUE1_INPUT].active) {
        params[VALUE1_PARAM].value = inputs[VALUE1_INPUT].value;
    }
    volt_value = params[VALUE1_PARAM].value;
    outputs[VALUE1_OUTPUT].value = volt_value;
}

struct FloatField : TextField
{
    float value;
    SpecificValue *module;

    FloatField(SpecificValue *_module);

    void onAction(EventAction &e) override;
    void onChange(EventChange &e) override;

    float textToVolts(std::string field_text);
    std::string voltsToText(float param_volts);
};

FloatField::FloatField(SpecificValue *_module)
{
    module = _module;
    value = module->params[SpecificValue::VALUE1_PARAM].value;
    text = voltsToText(value);
}

// TODO: this is really data stuff, so could be in type/struct/class for the data
//       (volt, freq/hz, period/seconds, note_name)
//       and instanced and provided to a generic ValueField widget
//       that has-a data type converter thingy
float FloatField::textToVolts(std::string field_text) {
    return atof(field_text.c_str());
}

std::string FloatField::voltsToText(float param_volts){
    return stringf("%0.3f", param_volts);
}

void FloatField::onChange(EventChange &e) {
    //debug("FloatField onChange  text=%s param=%f", text.c_str(),
    // module->params[SpecificValue::VALUE1_PARAM].value);

     if (this != gFocusedWidget) {
        std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].value);
        setText(new_text);
     }
}

void FloatField::onAction(EventAction &e)
{
    //debug("FloatField onAction text=%s", text.c_str());

    //update text first?
    TextField::onAction(e);

    float volts = textToVolts(text);

    //debug("FloatField setting volts=%f text=%s", volts, text.c_str());
    module->params[SpecificValue::VALUE1_PARAM].value = volts;

    //debug("FloatField onAction2 text=%s volts=%f module->volt_values=%f",
    //      text.c_str(), volts, module->volt_value);

}


struct HZFloatField : TextField
{
    float value;
    SpecificValue *module;

    HZFloatField(SpecificValue *_module);
    void onChange(EventChange &e) override;
    void onAction(EventAction &e) override;

    float textToVolts(std::string field_text);
    std::string voltsToText(float param_volts);
};

HZFloatField::HZFloatField(SpecificValue *_module)
{
    module = _module;
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

void HZFloatField::onChange(EventChange &e) {
    //debug("HZFloatField onChange  text=%s param=%f", text.c_str(),
    // module->params[SpecificValue::VALUE1_PARAM].value);

     //TextField::onChange(e);

     if (this != gFocusedWidget)
     {
         std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].value);
         setText(new_text);
     }
}

void HZFloatField::onAction(EventAction &e)
{
    //debug("HZFloatField onAction text=%s", text.c_str());

    //update text first?
    TextField::onAction(e);

    float volts = textToVolts(text);

    //debug("HZ FloatField onAction about to set VALUE*_PARAM to volts: %f", volts);
    module->params[SpecificValue::VALUE1_PARAM].value = volts;
}

struct LFOHzFloatField : TextField {
    float value;
    SpecificValue *module;

    LFOHzFloatField(SpecificValue *_module);
    void onAction(EventAction &e) override;
    void onChange(EventChange &e) override;

    float textToVolts(std::string field_text);
    std::string voltsToText(float param_volts);
};

LFOHzFloatField::LFOHzFloatField(SpecificValue *_module)
{
    module = _module;
}

float LFOHzFloatField::textToVolts(std::string field_text) {
    float freq_hz = strtof(text.c_str(), NULL);
    // float freq_hz = lfo_bpm
    return lfo_freq_to_cv(freq_hz);
}

std::string LFOHzFloatField::voltsToText(float param_volts){
    float lfo_freq_hz = lfo_cv_to_freq(param_volts);
    // float lfo_bpm = freq_hz * 60.0f;
    std::string new_text = stringf("%0.*f", lfo_freq_hz < 100 ? 4 : 3, lfo_freq_hz);
    return new_text;
}

void LFOHzFloatField::onChange(EventChange &e) {
    // debug("LFOHzFloatField onChange  text=%s param=%f", text.c_str(),
    //      module->params[SpecificValue::VALUE1_PARAM].value);

     //TextField::onChange(e);

     if (this != gFocusedWidget)
     {
         std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].value);
         setText(new_text);
     }
}

void LFOHzFloatField::onAction(EventAction &e)
{
    // debug("LFOHzFloatField onAction text=%s", text.c_str());

    //update text first?
    TextField::onAction(e);

    float volts = textToVolts(text);

    //debug("LFOhZ FloatField onAction about to set VALUE*_PARAM to volts: %f", volts);
    module->params[SpecificValue::VALUE1_PARAM].value = volts;
}

struct LFOBpmFloatField : TextField {
    float value;
    SpecificValue *module;

    LFOBpmFloatField(SpecificValue *_module);
    void onAction(EventAction &e) override;
    void onChange(EventChange &e) override;

    float textToVolts(std::string field_text);
    std::string voltsToText(float param_volts);
};

LFOBpmFloatField::LFOBpmFloatField(SpecificValue *_module)
{
    module = _module;
}

float LFOBpmFloatField::textToVolts(std::string field_text) {
    float lfo_bpm = strtof(text.c_str(), NULL);
    // float freq_hz = lfo_bpm
    float lfo_hz = lfo_bpm / 60.0f;
    return lfo_freq_to_cv(lfo_hz);
}

std::string LFOBpmFloatField::voltsToText(float param_volts){
    float lfo_freq_hz = lfo_cv_to_freq(param_volts);
    float lfo_bpm = lfo_freq_hz * 60.0f;
    std::string new_text = stringf("%0.*f", lfo_bpm < 100 ? 4 : 3, lfo_bpm);
    return new_text;
}

void LFOBpmFloatField::onChange(EventChange &e) {
    // debug("LFOHzFloatField onChange  text=%s param=%f", text.c_str(),
    //      module->params[SpecificValue::VALUE1_PARAM].value);

     //TextField::onChange(e);

     if (this != gFocusedWidget)
     {
         std::string new_text = voltsToText(module->params[SpecificValue::VALUE1_PARAM].value);
         setText(new_text);
     }
}

void LFOBpmFloatField::onAction(EventAction &e)
{
    // debug("LFOHzFloatField onAction text=%s", text.c_str());

    //update text first?
    TextField::onAction(e);

    float volts = textToVolts(text);

    //debug("LFOhZ FloatField onAction about to set VALUE*_PARAM to volts: %f", volts);
    module->params[SpecificValue::VALUE1_PARAM].value = volts;
}

struct CentsField : TextField {
    float value;
    SpecificValue *module;

    CentsField(SpecificValue *_module);
    void onChange(EventChange &e) override;
    void onAction(EventAction &e) override;

};

CentsField::CentsField(SpecificValue *_module) {
    module = _module;
}

void CentsField::onChange(EventChange &e) {
    // debug("CentsField onChange");
    float cents = volts_to_note_cents(module->params[SpecificValue::VALUE1_PARAM].value);

    // debug("CentsField onChange cents: %f", cents);
    if (this != gFocusedWidget || fabs(cents) >= 0.50f)
    {
        float cents = volts_to_note_cents(module->params[SpecificValue::VALUE1_PARAM].value);
        std::string new_text = stringf("% 0.2f", cents);
        setText(new_text);
    }
}

void CentsField::onAction(EventAction &e) {

    TextField::onAction(e);
    float cents = strtof(text.c_str(), NULL);

    // figure what to tweak the current volts
    float cent_volt = 1.0f / 12.0f / 100.0f;
    float delta_volt = cents * cent_volt;
    float nearest_note_voltage = volts_of_nearest_note(module->params[SpecificValue::VALUE1_PARAM].value);
    //debug("volts: %f nearest_volts: %f",
    //  module->params[SpecificValue::VALUE1_PARAM].value, nearest_note_voltage);
    //debug("delta_volt: %+f nearest_note_voltage+delta_volt: %f", delta_volt, nearest_note_voltage,
    //    nearest_note_voltage + delta_volt);
    module->params[SpecificValue::VALUE1_PARAM].value = nearest_note_voltage + delta_volt;

}

struct NoteNameField : TextField {
    float value;
    SpecificValue *module;

    NoteNameField(SpecificValue *_module);
    void onChange(EventChange &e) override;
    void onAction(EventAction &e) override;
};

NoteNameField::NoteNameField(SpecificValue *_module)
{
    module = _module;
}

void NoteNameField::onChange(EventChange &e) {
    //debug("NoteNameField onChange  text=%s param=%f", text.c_str(),
    // module->params[SpecificValue::VALUE1_PARAM].value);

     //TextField::onChange(e);

     if (this != gFocusedWidget)
     {
        float cv_volts = module->params[SpecificValue::VALUE1_PARAM].value;
        int octave = volts_to_octave(cv_volts);
        int note_number = volts_to_note(cv_volts);
        // debug("vc_volts: %f, octave=%d, note_number=%d, note=%d", cv_volts, octave, note_number, note_name_vec[note_number].c_str());
        // float semi_cents = volts_to_note_and_cents(cv_volts, module->params[SpecificValue::OCTAVE_PARAM].value);
        // note_info = volts_to_note_info(cv_volts, module->params[SpecificValue::OCTAVE_PARAM].value);
        // TODO: modf for oct/fract part, need to get +/- cents from chromatic notes

        std::string new_text = stringf("%s%d", note_name_vec[note_number].c_str(), octave);
        // debug("foo %f bar %f", )
        setText(new_text);
     }

}


void NoteNameField::onAction(EventAction &e) {
    TextField::onAction(e);

    // split into 'stuff before any int or -' and a number like string
    // ie C#11 -> C# 11,  A-4 -> A 4
    std::size_t note_flag_found_loc = text.find_last_of("#♯b♭");

    std::string note_flag = "";
    if(note_flag_found_loc!=std::string::npos){
        note_flag = text[note_flag_found_loc];
    }

    std::size_t found = text.find_first_of("-0123456789");

    // if no oct number, assume it is oct 4
    std::string note_name = text;
    std::string note_oct = "4";

    if(found != std::string::npos){

        note_name = text.substr(0, found);
        note_oct = text.substr(found, text.length());
    }

    auto enharm_search = enharmonic_name_map.find(note_name);
    if (enharm_search == enharmonic_name_map.end())
    {
        debug("%s was  NOT A VALID note name", note_name.c_str());
        return;
    }

    std::string can_note_name = enharmonic_name_map[note_name];

    std::string can_note_id = stringf("%s%s", can_note_name.c_str(), note_oct.c_str());


    debug("text: %s", text.c_str());
    debug("note_name: %s", note_name.c_str());
    debug("can_note_name: %s", can_note_name.c_str());
    debug("note_name_flag: %s", note_flag.c_str());
    debug("note_oct: %s", note_oct.c_str());
    debug("can_note_id: %s", can_note_id.c_str());


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
    // debug("adding field %d", i);

    y_baseline = 38.0f;

    volts_field = new FloatField(module);
    volts_field->box.pos = Vec(x_pos, y_baseline);
    volts_field->box.size = volt_field_size;
    volts_field->value = module->params[SpecificValue::VALUE1_PARAM].value;
    addChild(volts_field);

    y_baseline = 78.0f;

    float h_pos = x_pos;
    hz_field = new HZFloatField(module);
    hz_field->box.pos = Vec(x_pos, y_baseline);
    hz_field->box.size = hz_field_size;
    hz_field->value = module->hz_value;
    addChild(hz_field);

    y_baseline = 120.0f;

    lfo_hz_field = new LFOHzFloatField(module);
    lfo_hz_field->box.pos = Vec(h_pos, y_baseline);
    lfo_hz_field->box.size = lfo_hz_field_size;
    lfo_hz_field->value = module->lfo_hz_value;

    addChild(lfo_hz_field);

    y_baseline += lfo_hz_field->box.size.y;
    y_baseline += 5.0f;
    y_baseline += 12.0f;

    lfo_bpm_field = new LFOBpmFloatField(module);
    lfo_bpm_field->box.pos = Vec(x_pos, y_baseline);
    lfo_bpm_field->box.size = Vec(70.0f, 22.0f);
    lfo_bpm_field->value = module->lfo_bpm_value;
    addChild(lfo_bpm_field);

    y_baseline += lfo_bpm_field->box.size.y;
    y_baseline += 20.0f;

    // y_baseline = 180.0f;

    note_name_field = new NoteNameField(module);
    note_name_field->box.pos = Vec(x_pos, y_baseline);
    note_name_field->box.size = Vec(70.0f, 22.0f);
    note_name_field->value = module->volt_value;
    addChild(note_name_field);

    y_baseline += note_name_field->box.size.y;
    y_baseline += 5.0f;
    // y_baseline += 20.0f;

    cents_field = new CentsField(module);
    cents_field->box.pos = Vec(x_pos, y_baseline);
    cents_field->box.size = Vec(55.0f, 22.0f);
    cents_field->value = module->cents_value;
    addChild(cents_field);

    y_baseline += cents_field->box.size.y;
    y_baseline += 5.0f;

    // y_baseline += period_field->box.size.y;
    //y_baseline += 20.0f;

    float middle = box.size.x / 2.0f;
    float in_port_x = 15.0f;

    // y_baseline += 24.0f + 12.0f;
    y_baseline += 12.0f;
    // y_baseline += lfo_hz_field->box.size.y;

    Port *value_in_port = Port::create<PJ301MPort>(
        Vec(in_port_x, y_baseline),
        Port::INPUT,
        module,
        SpecificValue::VALUE1_INPUT);
    //value_in_port->box.pos = Vec(middle - value_in_port->box.size.x / 2, y_baseline);
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

    //debug(" trimpot: dv: %f v: %f p.value: %f", trimpot->defaultValue, trimpot->value,
    //    module->params[SpecificValue::VALUE1_PARAM].value);
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
    // debug("SpvWidget onChange");
    ModuleWidget::onChange(e);
    volts_field->onChange(e);
    hz_field->onChange(e);
    lfo_hz_field->onChange(e);
    note_name_field->onChange(e);
    cents_field->onChange(e);
    lfo_bpm_field->onChange(e);
}

Model *modelSpecificValue = Model::create<SpecificValue, SpecificValueWidget>(
    "Alikins", "SpecificValue", "Specific Values", UTILITY_TAG);

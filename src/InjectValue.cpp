#include "alikins.hpp"
#include <math.h>
#include "ui.hpp"
#include "window.hpp"
#include "dsp/digital.hpp"

#include "ParamFloatField.hpp"


struct InjectValue : Module
{
    enum ParamIds
    {
        INJECT_ENABLED_PARAM,
        INPUT_VOLTAGE_RANGE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        VALUE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds
    {
        DEBUG1_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    enum HoverEnabled
    {
        OFF,
        WITH_SHIFT,
        ALWAYS
    };

    InjectValue() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

    float param_value = 0.0f;
    float input_param_value = 0.0f;

    HoverEnabled enabled = WITH_SHIFT;
    VoltageRange inputRange = MINUS_PLUS_FIVE;

    float minInputVoltage = -5.0f;
    float maxInputVoltage = 5.0f;

};

void InjectValue::step()
{
    if (inputs[VALUE_INPUT].active) {
        input_param_value = inputs[VALUE_INPUT].value;
        param_value = input_param_value;
    }
}


json_t* InjectValue::toJson() {
    json_t *rootJ = json_object();

    json_t *inputRangeJ = json_object();
    // float v = (isfinite(minInputVoltage) && isfinite(maxValue)) ? value : 0.f;
	json_object_set_new(inputRangeJ, "minInputVoltage", json_real(minInputVoltage));
    json_object_set_new(inputRangeJ, "maxInputVoltage", json_real(maxInputVoltage));
    json_object_set_new(rootJ, "inputRange", inputRangeJ);

    return rootJ;
}

void InjectValue::fromJson(json_t *rootJ) {
    json_t *inputRangeJ = json_object_get(rootJ, "inputRange");
    if (inputRangeJ) {
        json_t *minVoltageJ = json_object_get(inputRangeJ, "minInputVoltage");
        json_t *maxVoltageJ = json_object_get(inputRangeJ, "maxInputVoltage");

        minInputVoltage = json_number_value(minVoltageJ);
        maxInputVoltage = json_number_value(maxVoltageJ);
    }
}


struct InjectValueWidget : ModuleWidget
{
    InjectValueWidget(InjectValue *module);

    void step() override;
    void onChange(EventChange &e) override;

    // TODO: enum/params/ui for input range
    float min_input = -5.0f;
    float max_input = 5.0f;

    ParamWidget *enableInjectSwitch;
    ParamWidget *inputVoltageSwitch;

    ParamFloatField *param_value_field;
    TextField *min_field;
    TextField *max_field;
    TextField *default_field;
    TextField *widget_type_field;

};

InjectValueWidget::InjectValueWidget(InjectValue *module) : ModuleWidget(module)
{
    setPanel(SVG::load(assetPlugin(plugin, "res/InjectValue.svg")));

    float y_baseline = 45.0f;

    Vec text_field_size = Vec(70.0f, 22.0f);

    float x_pos = 10.0f;

    y_baseline = 38.0f;

    param_value_field = new ParamFloatField(module);
    param_value_field->box.pos = Vec(x_pos, y_baseline);
    param_value_field->box.size = text_field_size;

    addChild(param_value_field);

    y_baseline = 78.0f;
    min_field = new TextField();
    min_field->box.pos = Vec(x_pos, y_baseline);
    min_field->box.size = text_field_size;

    addChild(min_field);

    y_baseline = 118.0f;
    max_field = new TextField();
    max_field->box.pos = Vec(x_pos, y_baseline);
    max_field->box.size = text_field_size;

    addChild(max_field);

    y_baseline = 158.0f;
    default_field = new TextField();
    default_field->box.pos = Vec(x_pos, y_baseline);
    default_field->box.size = text_field_size;

    addChild(default_field);

    y_baseline = 198.0f;
    widget_type_field = new TextField();
    widget_type_field->box.pos = Vec(x_pos, y_baseline);
    widget_type_field->box.size = text_field_size;

    addChild(widget_type_field);

    float middle = box.size.x / 2.0f;
    // float out_port_x = middle;
    float out_port_x = 60.0f;

    y_baseline = box.size.y - 65.0f;

    /*
    ParamWidget *some_param = Component::create<ParamWidget>(Vec(60, 150), module);
    some_param->paramId = paramId;
	some_param->setLimits(minValue, maxValue);
	some_param->setDefaultValue(defaultValue);
    */

    enableInjectSwitch = ParamWidget::create<CKSSThree>(Vec(10, box.size.y - 120.0f), module,
        InjectValue::INJECT_ENABLED_PARAM, 0.0f, 2.0f, 0.0f);

    addParam(enableInjectSwitch);

    inputVoltageSwitch = ParamWidget::create<CKSSThree>(Vec(60, box.size.y - 120.0f ), module,
        InjectValue::INPUT_VOLTAGE_RANGE_PARAM, 0.0f, 2.0f, 0.0f);

    addParam(inputVoltageSwitch);


    inputVoltageSwitch = CKSSThree()
    Port *value_in_port = Port::create<PJ301MPort>(
        Vec(20, y_baseline),
        Port::INPUT,
        module,
        InjectValue::VALUE_INPUT);

    inputs.push_back(value_in_port);
    value_in_port->box.pos = Vec(5, y_baseline);
    addChild(value_in_port);


    Port *value_out_port = Port::create<PJ301MPort>(
        Vec(out_port_x, y_baseline),
        Port::OUTPUT,
        module,
        InjectValue::DEBUG1_OUTPUT);

    outputs.push_back(value_out_port);
    value_out_port->box.pos = Vec(middle - value_out_port->box.size.x/2, y_baseline);

    addChild(value_out_port);

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

    // fire off an event to refresh all the widgets
    EventChange e;
    onChange(e);
}

void InjectValueWidget::step() {


    bool shift_pressed = windowIsShiftPressed();

    if (!gHoveredWidget) {
        return;
    }

    if (module->params[InjectValue::INJECT_ENABLED_PARAM].value == InjectValue::OFF) {
        return;
    }

    if (module->params[InjectValue::INJECT_ENABLED_PARAM].value == InjectValue::WITH_SHIFT &&!shift_pressed) {
        return;
    }

    // TODO/FIXME: I assume there is a better way to check type?
    ParamWidget *pwidget = dynamic_cast<ParamWidget *>(gHoveredWidget);

    // Don't inject values into the switch that turns inject on/off
    if (pwidget && (pwidget != enableInjectSwitch))
    {

        InjectValue *injectValueModule = dynamic_cast<InjectValue *>(module);

        // rescale the input CV (-10/+10V) to whatever the range of the param widget is
        float scaled_value = rescale(module->inputs[InjectValue::VALUE_INPUT].value, injectValueModule->minInputVoltage, injectValueModule->maxInputVoltage, pwidget->minValue, pwidget->maxValue);

        // debug("input: %f scaled_value: %f", module->inputs[InjectValue::VALUE_INPUT].value, scaled_value);

        // ParamWidgets are-a QuantityWidget, so change it's value
        pwidget->setValue(scaled_value);

        // force a step of the param widget to get it to 'animate'
        pwidget->step();

        // EventChange e;
        // pwidget->onChange(e);

        // Show the value that will be injected
        // TODO: show the original input value and scaled output?
        param_value_field->setValue(scaled_value);

        min_field->setText(stringf("%#.4g", pwidget->minValue));
        max_field->setText(stringf("%#.4g", pwidget->maxValue));
        default_field->setText(stringf("%#.4g", pwidget->defaultValue));
        widget_type_field->setText("Param");

        //EventChange e;
        //onChange(e);
        // TODO:
    }
    ModuleWidget::step();

}

void InjectValueWidget::onChange(EventChange &e) {
    ModuleWidget::onChange(e);
    param_value_field->onChange(e);
}

Model *modelInjectValue = Model::create<InjectValue, InjectValueWidget>(
    "Alikins", "InjectValue", "Inject Value - inject value into param under cursor", UTILITY_TAG, CONTROLLER_TAG);

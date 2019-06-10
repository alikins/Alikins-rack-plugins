#include "alikins.hpp"
#include <math.h>
#include "ui.hpp"
#include "window.hpp"

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

    enum InjectEnabled
    {
        OFF,
        WITH_SHIFT,
        ALWAYS
    };

    InjectValue() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(InjectValue::INPUT_VOLTAGE_RANGE_PARAM, 0.0f, 2.0f, 0.0f, "Input Voltage Range");
        configParam(InjectValue::INJECT_ENABLED_PARAM, 0.0f, 2.0f, 0.0f, "Enable Inject");
    }

    void process(const ProcessArgs &args) override;

    float param_value = 0.0f;
    float input_param_value = 0.0f;

    InjectEnabled enabled = WITH_SHIFT;
    VoltageRange inputRange = MINUS_PLUS_FIVE;
};

void InjectValue::process(const ProcessArgs &args)
{
    enabled = (InjectEnabled) clamp((int) round(params[INJECT_ENABLED_PARAM].getValue()), 0, 2);

    inputRange  = (VoltageRange) clamp((int) round(params[INPUT_VOLTAGE_RANGE_PARAM].getValue()), 0, 2);

    if (!inputs[VALUE_INPUT].isConnected()) {
        return;
    }

    param_value = inputs[VALUE_INPUT].getVoltage();
}

struct InjectValueWidget : ModuleWidget
{

    void step() override;
    void onChange(const event::Change &e) override;

    // TODO: enum/params/ui for input range

    ParamWidget *enableInjectSwitch;

    ParamFloatField *param_value_field;
    TextField *min_field;
    TextField *max_field;
    TextField *default_field;
    TextField *widget_type_field;

    InjectValueWidget(InjectValue *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/InjectValue.svg")));

        float y_baseline = 45.0f;

        Vec text_field_size = Vec(70.0f, 22.0f);

        float x_pos = 10.0f;

        y_baseline = 38.0f;

        param_value_field = new ParamFloatField(module);
        param_value_field->box.pos = Vec(x_pos, y_baseline);
        param_value_field->box.size = text_field_size;
        param_value_field->setValue(module->param_value);

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

        y_baseline = box.size.y - 128.0f;

        addParam(createParam<CKSSThree>(Vec(5.0f, y_baseline ), module, InjectValue::INPUT_VOLTAGE_RANGE_PARAM));

        addInput(createInput<PJ301MPort>(
            Vec(60.0f, y_baseline - 2.0),
            module,
            InjectValue::VALUE_INPUT));

        y_baseline = box.size.y - 65.0f;

        enableInjectSwitch = createParam<CKSSThree>(Vec(5, box.size.y - 62.0f), module, InjectValue::INJECT_ENABLED_PARAM);

        addParam(enableInjectSwitch);

        addChild(createWidget<ScrewSilver>(Vec(0.0f, 0.0f)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
        addChild(createWidget<ScrewSilver>(Vec(0.0f, 365.0f)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

        // fire off an event to refresh all the widgets
        event::Change e;
        onChange(e);
    }
};


void InjectValueWidget::step() {
    InjectValue *injectValueModule = dynamic_cast<InjectValue *>(module);

    if (!injectValueModule) {
        return;
    }

    if (!APP->event->hoveredWidget) {
        return;
    }

    // TODO/FIXME: I assume there is a better way to check type?
    ParamWidget *pwidget = dynamic_cast<ParamWidget *>(APP->event->hoveredWidget);

    if (!pwidget) {
        min_field->setText("");
        max_field->setText("");
        default_field->setText("");
        widget_type_field->setText("unknown");

        ModuleWidget::step();
        return;
    }

    // float input = module->inputs[InjectValue::VALUE_INPUT].getVoltage();
    float input_value = injectValueModule->param_value;

    // clamp the input to withing input voltage range before scaling it
    float clamped_input = clamp(input_value,
                                voltage_min[injectValueModule->inputRange],
                                voltage_max[injectValueModule->inputRange]);

    // rescale the input CV to whatever the range of the param widget is
    float scaled_value = rescale(clamped_input,
                                 voltage_min[injectValueModule->inputRange],
                                 voltage_max[injectValueModule->inputRange],
                                 pwidget->paramQuantity->getMinValue(),
                                 pwidget->paramQuantity->getMaxValue());

    /*
        debug("input_value: %f (in_min: %f, in_max:%f) clamped_in: %f out_min: %f, out_max: %f) scaled_value: %f",
            input_value,
            voltage_min[injectValueModule->inputRange],
            voltage_max[injectValueModule->inputRange],
            clamped_input,
            pwidget->minValue,
            pwidget->maxValue,
            scaled_value);
        */

    // Show the value that will be injected
    // TODO: show the original input value and scaled output?

    bool shift_pressed = ((APP->window->getMods() & RACK_MOD_MASK) == GLFW_MOD_SHIFT);

    if (!injectValueModule->enabled || (injectValueModule->enabled == InjectValue::WITH_SHIFT && !shift_pressed))
    {
        return;
    }

    param_value_field->setValue(scaled_value);

    min_field->setText(string::f("%#.4g", pwidget->paramQuantity->getMinValue()));
    max_field->setText(string::f("%#.4g", pwidget->paramQuantity->getMaxValue()));
    default_field->setText(string::f("%#.4g", pwidget->paramQuantity->getDefaultValue()));
    widget_type_field->setText("Param");

    // ParamWidgets are-a QuantityWidget, so change it's value
    // but don't inject values into the switch that turns inject on/off
    if (pwidget != enableInjectSwitch)
    {

        // TODO: would be useful to have a light to indicate when values are being injected
        pwidget->paramQuantity->setValue(scaled_value);

        // force a step of the param widget to get it to 'animate'
        pwidget->step();

    }

    ModuleWidget::step();
}

void InjectValueWidget::onChange(const event::Change &e) {
    ModuleWidget::onChange(e);
    param_value_field->onChange(e);
}

Model *modelInjectValue = createModel<InjectValue, InjectValueWidget>("InjectValue");

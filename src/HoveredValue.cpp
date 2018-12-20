#include "alikins.hpp"
#include "ParamFloatField.hpp"

#include <math.h>
#include "ui.hpp"
#include "window.hpp"
#include "dsp/digital.hpp"


struct HoveredValue : Module
{
    enum ParamIds
    {
        HOVERED_PARAM_VALUE_PARAM,
        HOVER_ENABLED_PARAM,
        OUTPUT_RANGE_PARAM,
        HOVERED_SCALED_PARAM_VALUE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        NUM_INPUTS
    };
    enum OutputIds
    {
        PARAM_VALUE_OUTPUT,
        SCALED_PARAM_VALUE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };

    enum HoverEnabled {OFF, WITH_SHIFT, ALWAYS};

    HoveredValue() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    HoverEnabled enabled = WITH_SHIFT;

    VoltageRange outputRange = MINUS_PLUS_FIVE;

    SchmittTrigger hoverArmedTrigger;

};

void HoveredValue::step()
{

    outputs[PARAM_VALUE_OUTPUT].value = params[HOVERED_PARAM_VALUE_PARAM].value;
    outputs[SCALED_PARAM_VALUE_OUTPUT].value = params[HOVERED_SCALED_PARAM_VALUE_PARAM].value;

}


struct HoveredValueWidget : ModuleWidget
{
    HoveredValueWidget(HoveredValue *module);

    void step() override;
    void onChange(EventChange &e) override;

    ParamWidget *enableHoverSwitch;
    ParamWidget *outputRangeSwitch;

    ParamFloatField *param_value_field;
    TextField *min_field;
    TextField *max_field;
    TextField *default_field;
    TextField *widget_type_field;
};

HoveredValueWidget::HoveredValueWidget(HoveredValue *module) : ModuleWidget(module)
{
    setPanel(SVG::load(assetPlugin(plugin, "res/HoveredValue.svg")));

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

    // Scaled output and scaled output range
    y_baseline = y_baseline + 25.0f;

    outputRangeSwitch = ParamWidget::create<CKSSThree>(Vec(5, y_baseline ), module,
        HoveredValue::OUTPUT_RANGE_PARAM, 0.0f, 2.0f, 0.0f);

    addParam(outputRangeSwitch);

    // Scaled output port
    Port *scaled_value_out_port = Port::create<PJ301MPort>(
        Vec(60.0f, y_baseline),
        Port::OUTPUT,
        module,
        HoveredValue::SCALED_PARAM_VALUE_OUTPUT);

    outputs.push_back(scaled_value_out_port);

    addChild(scaled_value_out_port);

    // enabled/disable switch
    y_baseline = box.size.y - 65.0f;

    enableHoverSwitch = ParamWidget::create<CKSSThree>(Vec(5, box.size.y - 62.0f), module,
        HoveredValue::HOVER_ENABLED_PARAM, 0.0f, 2.0f, 0.0f);

    addParam(enableHoverSwitch);

    Port *raw_value_out_port = Port::create<PJ301MPort>(
        Vec(60.0f, box.size.y - 67.0f),
        Port::OUTPUT,
        module,
        HoveredValue::PARAM_VALUE_OUTPUT);

    outputs.push_back(raw_value_out_port);

    addChild(raw_value_out_port);

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

    // fire off an event to refresh all the widgets
    EventChange e;
    onChange(e);
}

void HoveredValueWidget::step() {
    ModuleWidget::step();

    bool shift_pressed = windowIsShiftPressed();

    if (!gHoveredWidget) {
        return;
    }

    if (module->params[HoveredValue::HOVER_ENABLED_PARAM].value == HoveredValue::OFF) {
        return;
    }

    if (module->params[HoveredValue::HOVER_ENABLED_PARAM].value == HoveredValue::WITH_SHIFT &&!shift_pressed) {
        return;
    }

    VoltageRange outputRange = (VoltageRange) round(module->params[HoveredValue::OUTPUT_RANGE_PARAM].value);

    // TODO/FIXME: I assume there is a better way to check type?
    ParamWidget *pwidget = dynamic_cast<ParamWidget *>(gHoveredWidget);
    if (pwidget)
    {
        param_value_field->setValue(pwidget->value);

        // TODO: option for selecting the output range? (uni/bi/original)
        // TODO: outputs for 'raw/original' value and scaled
        // TODO: show value of original and scaled?
        // int output_index = round(params[OUTPUT_RANGE_PARAM].value);
        // float min_out = voltage_min[output_index];
        float tapped_value = pwidget->value;

        float scaled_value = rescale(pwidget->value, pwidget->minValue, pwidget->maxValue,
                                     voltage_min[outputRange],
                                     voltage_max[outputRange]);

        engineSetParam(module, HoveredValue::HOVERED_PARAM_VALUE_PARAM, tapped_value);
        engineSetParam(module, HoveredValue::HOVERED_SCALED_PARAM_VALUE_PARAM, scaled_value);

        min_field->setText(stringf("%#.4g", pwidget->minValue));
        max_field->setText(stringf("%#.4g", pwidget->maxValue));
        default_field->setText(stringf("%#.4g", pwidget->defaultValue));
        widget_type_field->setText("Param");

        // TODO: if we use type name detection stuff (cxxabi/typeinfo/etc) we could possibly
        //       also show the name of the hovered widget as a hint on mystery meat params
        // TODO: anyway to get the orig literal name of an enum value (ie, LFO_VC_OUTPUT etc)
        //       at runtime? might also be hint
    }

    Port *port = dynamic_cast<Port *>(gHoveredWidget);
    if (port)
    {
        float tapped_value = 0.0f;
        if (port->type == port->INPUT)
        {
            tapped_value = port->module->inputs[port->portId].value;
            param_value_field->setValue(tapped_value);
            widget_type_field->setText("Input");
        }
        if (port->type == port->OUTPUT)
        {
            tapped_value = port->module->outputs[port->portId].value;
            param_value_field->setValue(tapped_value);
            widget_type_field->setText("Output");
        }

        engineSetParam(module, HoveredValue::HOVERED_PARAM_VALUE_PARAM, tapped_value);

        // inputs/outputs dont have variable min/max, so just use the -10/+10 and
        // 0 for the default to get the point across.
        min_field->setText(stringf("%#.4g", -10.0f));
        max_field->setText(stringf("%#.4g", 10.0f));
        default_field->setText(stringf("%#.4g", 0.0f));
    }

    // TODO: if a WireWidget, can we figure out it's in/out and current value? That would be cool,
    //       though it doesn't look like WireWidgets are ever hovered (or gHoveredWidget never
    //       seems to be a WireWidget).
}

void HoveredValueWidget::onChange(EventChange &e) {
    ModuleWidget::onChange(e);
    param_value_field->onChange(e);
}

Model *modelHoveredValue = Model::create<HoveredValue, HoveredValueWidget>(
    "Alikins", "HoveredValue", "Hovered Value - get value under cursor", UTILITY_TAG, CONTROLLER_TAG);

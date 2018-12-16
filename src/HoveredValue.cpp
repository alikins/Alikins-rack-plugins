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

// #include "specificValueWidgets.hpp"

struct HoveredValue : Module
{
    enum ParamIds
    {
        HOVERED_PARAM_VALUE_PARAM,
        NUM_PARAMS
    };
    enum InputIds
    {
        NUM_INPUTS
    };
    enum OutputIds
    {
        PARAM_VALUE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds
    {
        NUM_LIGHTS
    };


    HoveredValue() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    float param_value;

};

void HoveredValue::step()
{
    outputs[PARAM_VALUE_OUTPUT].value = param_value;
}

enum AdjustKey
{
    UP,
    DOWN,
    INITIAL
};

// TODO/FIXME: This is more or less adhoc TextField mixed with QuantityWidget
//             just inherit from both?
struct ParamFloatField : TextField
{
    HoveredValue *module;
    float hovered_value;

    ParamFloatField(HoveredValue *module);

    void setValue(float value);
    void onChange(EventChange &e) override;

    std::string paramValueToText(float param_value);

};

ParamFloatField::ParamFloatField(HoveredValue *_module)
{
    module = _module;
}

std::string ParamFloatField::paramValueToText(float param_value) {
    return stringf("%#.4g", param_value);
}

void ParamFloatField::setValue(float value) {
    this->hovered_value = value;
    this->module->param_value = value;
    EventChange e;
    onChange(e);
}

void ParamFloatField::onChange(EventChange &e) {
    std::string new_text = paramValueToText(hovered_value);
    setText(new_text);
}

struct HoveredValueWidget : ModuleWidget
{
    HoveredValueWidget(HoveredValue *module);

    void step() override;
    void onChange(EventChange &e) override;

    ParamFloatField *param_value_field;
    TextField *min_field;
    TextField *max_field;
    TextField *default_field;
    TextField *widget_type_field;
};

HoveredValueWidget::HoveredValueWidget(HoveredValue *module) : ModuleWidget(module)
{
    setPanel(SVG::load(assetPlugin(plugin, "res/HoveredValue.svg")));

    // TODO: widget with these children?
    float y_baseline = 45.0f;

    Vec param_value_field_size = Vec(70.0f, 22.0f);

    float x_pos = 10.0f;

    y_baseline = 38.0f;

    param_value_field = new ParamFloatField(module);
    param_value_field->box.pos = Vec(x_pos, y_baseline);
    param_value_field->box.size = param_value_field_size;

    addChild(param_value_field);

    Vec min_field_size = Vec(70.0f, 22.0f);

    y_baseline = 78.0f;
    min_field = new TextField();
    min_field->box.pos = Vec(x_pos, y_baseline);
    min_field->box.size = min_field_size;

    addChild(min_field);

    y_baseline = 118.0f;
    max_field = new TextField();
    max_field->box.pos = Vec(x_pos, y_baseline);
    max_field->box.size = min_field_size;

    addChild(max_field);

    y_baseline = 158.0f;
    default_field = new TextField();
    default_field->box.pos = Vec(x_pos, y_baseline);
    default_field->box.size = min_field_size;

    addChild(default_field);

    y_baseline = 198.0f;
    widget_type_field = new TextField();
    widget_type_field->box.pos = Vec(x_pos, y_baseline);
    widget_type_field->box.size = min_field_size;

    addChild(widget_type_field);

    float middle = box.size.x / 2.0f;
    float out_port_x = middle;

    y_baseline = box.size.y - 65.0f;
    Port *value_out_port = Port::create<PJ301MPort>(
        Vec(out_port_x, y_baseline),
        Port::OUTPUT,
        module,
        HoveredValue::PARAM_VALUE_OUTPUT);

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

void HoveredValueWidget::step() {
    ModuleWidget::step();

    if (gHoveredWidget) {

        // TODO/FIXME: I assume there is a better way to check type?
        ParamWidget *pwidget = dynamic_cast<ParamWidget*>(gHoveredWidget);
        if (pwidget) {
            param_value_field->setValue(pwidget->value);
            min_field->setText(stringf("%#.4g", pwidget->minValue));
            max_field->setText(stringf("%#.4g", pwidget->maxValue));
            default_field->setText(stringf("%#.4g", pwidget->defaultValue));
            widget_type_field->setText("Param");
        }

        Port *port = dynamic_cast<Port*>(gHoveredWidget);
        if (port) {
            if (port->type == port->INPUT) {
                param_value_field->setValue(port->module->inputs[port->portId].value);
                widget_type_field->setText("Input");
            }
            if (port->type == port->OUTPUT) {
                param_value_field->setValue(port->module->outputs[port->portId].value);
                widget_type_field->setText("Output");
            }

            // inputs/outputs dont have variable min/max, so just use the -10/+10 and
            // 0 for the default to get the point across.
            min_field->setText(stringf("%#.4g", -10.0f));
            max_field->setText(stringf("%#.4g", 10.0f));
            default_field->setText(stringf("%#.4g", 0.0f));

        }

        // TODO: if a WireWidget, can we figure out it's in/out and current value? That would be cool,
        //       though it doesn't look like WireWidgets are ever hovered (or gHoveredWidget never // //       seems to be a WireWidget).

    }

}

void HoveredValueWidget::onChange(EventChange &e) {
    ModuleWidget::onChange(e);
    param_value_field->onChange(e);
}

Model *modelHoveredValue = Model::create<HoveredValue, HoveredValueWidget>(
    "Alikins", "HoveredValue", "Show Value Of Hovered Widget", UTILITY_TAG);

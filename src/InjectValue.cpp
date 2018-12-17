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
        HOVER_ENABLED_PARAM,
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

    enum HoverEnabled {OFF, WITH_SHIFT, ALWAYS};

    InjectValue() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    float param_value = 0.0f;
    float input_param_value = 0.0f;

    HoverEnabled enabled = WITH_SHIFT;

    // SchmittTrigger hoverArmedTrigger;

};

void InjectValue::step()
{
    if (inputs[VALUE_INPUT].active) {
        input_param_value = inputs[VALUE_INPUT].value;
        param_value = input_param_value;
    }
}


struct InjectValueWidget : ModuleWidget
{
    InjectValueWidget(InjectValue *module);

    void step() override;
    void onChange(EventChange &e) override;

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

    addParam(ParamWidget::create<CKSSThree>(Vec(19, box.size.y - 120.0f), module,
                                       InjectValue::HOVER_ENABLED_PARAM, 0.0f, 2.0f, 0.0f));

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
    ModuleWidget::step();

    bool shift_pressed = windowIsShiftPressed();

    if (!gHoveredWidget) {
        return;
    }

    if (module->params[InjectValue::HOVER_ENABLED_PARAM].value == InjectValue::OFF) {
        return;
    }

    if (module->params[InjectValue::HOVER_ENABLED_PARAM].value == InjectValue::WITH_SHIFT &&!shift_pressed) {
        return;
    }

    // TODO/FIXME: I assume there is a better way to check type?
    ParamWidget *pwidget = dynamic_cast<ParamWidget *>(gHoveredWidget);
    if (pwidget)
    {
        engineSetParam(pwidget->module, pwidget->paramId, module->inputs[InjectValue::VALUE_INPUT].value);
        // pwidget->setValue();
        //EventChange e;
        // onChange(e);

        //param_value_field->setValue(pwidget->value);
        param_value_field->setValue(module->inputs[InjectValue::VALUE_INPUT].value);

        min_field->setText(stringf("%#.4g", pwidget->minValue));
        max_field->setText(stringf("%#.4g", pwidget->maxValue));
        default_field->setText(stringf("%#.4g", pwidget->defaultValue));
        widget_type_field->setText("Param");
    }

    Port *port = dynamic_cast<Port *>(gHoveredWidget);
    if (port)
    {
        // port->module->inputs[port->portId].value = module->inputs[InjectValue::VALUE_INPUT].value;

        if (port->type == port->INPUT)
        {
            param_value_field->setValue(port->module->inputs[port->portId].value);
            widget_type_field->setText("Input");
        }
        if (port->type == port->OUTPUT)
        {
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
    //       though it doesn't look like WireWidgets are ever hovered (or gHoveredWidget never
    //       seems to be a WireWidget).
}

void InjectValueWidget::onChange(EventChange &e) {
    ModuleWidget::onChange(e);
    param_value_field->onChange(e);
}

Model *modelInjectValue = Model::create<InjectValue, InjectValueWidget>(
    "Alikins", "InjectValue", "Inject Value - inject value into param under cursor", UTILITY_TAG, CONTROLLER_TAG);

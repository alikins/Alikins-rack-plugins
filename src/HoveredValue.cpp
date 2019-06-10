#include <math.h>

#include "alikins.hpp"
#include "ParamFloatField.hpp"

#include "ui.hpp"
#include "window.hpp"

#include <iostream>
#include <typeinfo>
#include <cxxabi.h>
#include "prettyprint.hpp"

/* Notes:

Most of this is implemented in HoveredValueWidget.step(). Each
time the gui thread steps the scene graph, .step() is checking
if gHoveredWidget is a ParamWidget or Port. It is more or less
polling.

Is there any way to get an event when any ParamWidget or Port is
hovered? (onMouseMove or onMouseEnter? but for other widgets). May
be better implemented as responding to hypothetical onHoverEnter
or onHoverLeave instead of polling.

*/

/* This module was inspired by the question and discussion at
    https://www.facebook.com/groups/vcvrack/permalink/286752278651590/

    "Hi Folks, Just wondering, is it possible to see control parameter values in VCVRack?"
*/
struct HoveredValue : Module {
    enum ParamIds {
        HOVERED_PARAM_VALUE_PARAM,
        HOVER_ENABLED_PARAM,
        OUTPUT_RANGE_PARAM,
        HOVERED_SCALED_PARAM_VALUE_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        PARAM_VALUE_OUTPUT,
        SCALED_PARAM_VALUE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    enum HoverEnabled {OFF, WITH_SHIFT, ALWAYS};

    HoveredValue() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(OUTPUT_RANGE_PARAM, 0.0f, 2.0f, 0.0f, "Output Range");
        configParam(HOVER_ENABLED_PARAM, 0.0f, 2.0f, 0.0f, "Enable Hover");
    }

    void process(const ProcessArgs &args) override;

    json_t* dataToJson() override;
    void dataFromJson(json_t *rootJ) override;

    HoverEnabled enabled = WITH_SHIFT;

    VoltageRange outputRange = MINUS_PLUS_FIVE;

    bool useTooltip = true;

};

void HoveredValue::process(const ProcessArgs &args) {
    outputs[PARAM_VALUE_OUTPUT].setVoltage(params[HOVERED_PARAM_VALUE_PARAM].getValue());
    outputs[SCALED_PARAM_VALUE_OUTPUT].setVoltage(params[HOVERED_SCALED_PARAM_VALUE_PARAM].getValue());
}

json_t* HoveredValue::dataToJson() {
    json_t *rootJ = json_object();
    json_object_set_new(rootJ, "useTooltip", json_boolean(useTooltip));
    return rootJ;
}

void HoveredValue::dataFromJson(json_t *rootJ) {
    json_t *useTooltipJ = json_object_get(rootJ, "useTooltip");
    if (useTooltipJ) {
        useTooltip = json_boolean_value(useTooltipJ);
    }
}

struct UseTooltipMenuItem : MenuItem {
    HoveredValue *module;

    void onAction(const event::Action &e) override {
        if (!module)
            return;

        if (module->useTooltip) {
            module->useTooltip = false;
        } else {
            module->useTooltip = true;
        }
    }

    void step() override {
        if (!module)
            return;

        rightText = CHECKMARK(module->useTooltip);
    }
};


struct HoveredValueWidget : ModuleWidget {
    // ParamWidget *enableHoverSwitch;
    // ParamWidget *outputRangeSwitch;

    ParamFloatField *param_value_field;
    TextField *min_field;
    TextField *max_field;
    TextField *default_field;
    TextField *widget_type_field;

    Tooltip *tooltip = NULL;

    void step() override;
    void onChange(const event::Change &e) override;

    void tooltipHide();
    void tooltipShow(std::string tooltipText, Widget *hoveredWidget);

    HoveredValueWidget(HoveredValue *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/HoveredValue.svg")));

        float y_baseline = 45.0f;

        Vec text_field_size = Vec(70.0f, 22.0f);

        float x_pos = 10.0f;

        y_baseline = 38.0f;

        tooltip = new Tooltip();

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
        // y_baseline = y_baseline + 25.0f;
        y_baseline = box.size.y - 128.0f;

        addParam(createParam<CKSSThree>(Vec(5, y_baseline), module, HoveredValue::OUTPUT_RANGE_PARAM));
        // addParam(outputRangeSwitch);

        // Scaled output port
        addOutput(createOutput<PJ301MPort>(
            Vec(60.0f, y_baseline - 2.0f),
            module,
            HoveredValue::SCALED_PARAM_VALUE_OUTPUT));

        // enabled/disable switch
        y_baseline = box.size.y - 65.0f;

        addParam(createParam<CKSSThree>(Vec(5, box.size.y - 62.0f), module, HoveredValue::HOVER_ENABLED_PARAM));

        // addParam(enableHoverSwitch);

        addOutput(createOutput<PJ301MPort>(
            Vec(60.0f, box.size.y - 67.0f),
            module,
            HoveredValue::PARAM_VALUE_OUTPUT));

        addChild(createWidget<ScrewSilver>(Vec(0.0f, 0.0f)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
        addChild(createWidget<ScrewSilver>(Vec(0.0f, 365.0f)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

        // fire off an event to refresh all the widgets
        event::Change e;
        onChange(e);

    }

    void appendContextMenu(Menu *menu) override {

        // Menu *menu = ModulecreateWidgetContextMenu();

        MenuLabel *spacerLabel = new MenuLabel();
        menu->addChild(spacerLabel);

        HoveredValue *hoveredValue = dynamic_cast<HoveredValue*>(module);
        assert(hoveredValue);

        UseTooltipMenuItem *useTooltipMenuItem =
            createMenuItem<UseTooltipMenuItem>("Show Tooltip",
                                                CHECKMARK(hoveredValue->useTooltip > 2.0f));

        useTooltipMenuItem->module = hoveredValue;
        menu->addChild(useTooltipMenuItem);

    }

};

void HoveredValueWidget::tooltipHide() {
    // assert?
    if (!tooltip) {
        // debug("tooltip was null");
        return;
    }

    if (!tooltip->parent) { return; }

    // debug("tooltipHide[res=%s]: tt->text:%s", reason.c_str(), tooltip->text.c_str());

    /*
     * if (!gScene) {
        // debug("gScene was null");
        return;
    }
    */

    if (tooltip->parent) {
        APP->scene->removeChild(tooltip);
    }

}


void HoveredValueWidget::tooltipShow(std::string tooltipText, Widget *hoveredWidget) {

    ModuleWidget *hoveredParent = hoveredWidget->getAncestorOfType<ModuleWidget>();
    if (!hoveredParent) {
        return;
    }

    if (!reinterpret_cast<HoveredValue *>(module)->useTooltip) {
        return;
    }

    // debug("hoveredParent pos (%f, %f)", hoveredParent->box.pos.x, hoveredParent->box.pos.y);

    Vec offsetFromHovered = Vec(20.0, -20.0f);
    Vec absHovered = hoveredParent->getAbsoluteOffset(hoveredWidget->box.pos);
    Vec hoveredOffset = absHovered.plus(offsetFromHovered);

    tooltip->box.pos = hoveredOffset;

    // debug("tooltip show: (%f, %f) pos: (%f, %f)", tooltip->box.pos.x, tooltip->box.pos.y,
    // hoveredWidget->box.pos.x, hoveredWidget->box.pos.y);

    tooltip->text = tooltipText;
    if (!tooltip->parent) {
        APP->scene->addChild(tooltip);
    }
}

void HoveredValueWidget::step() {
    /* TODO: would be useful to be more explicit about the state here,
        ie, states indicating if hovered widget is not injectable or not,
        and if we are actively injecting or not. And transitions for when
        we start hovering and stop hovering over a widget.
    */

    if (!module)
        return;

    float display_min = -5.0f;
    float display_max = 5.0f;
    float display_default = 0.0f;
    float display_value = 0.0f;

    std::string display_type = "";
    std::string display_label = "";
    std::string display_description = "";
    std::string display_unit = "";

    float raw_value = 0.0f;

    ModuleWidget::step();

    bool shift_pressed = ((APP->window->getMods() & RACK_MOD_MASK) == GLFW_MOD_SHIFT);

    if (!APP->event->hoveredWidget) {
        tooltipHide();
        return;
    }

    if (module->params[HoveredValue::HOVER_ENABLED_PARAM].getValue() == HoveredValue::OFF) {
        tooltipHide();
        return;
    }

    if (module->params[HoveredValue::HOVER_ENABLED_PARAM].getValue() == HoveredValue::WITH_SHIFT &&!shift_pressed) {
        tooltipHide();
        return;
    }


    VoltageRange outputRange = (VoltageRange) round(module->params[HoveredValue::OUTPUT_RANGE_PARAM].getValue());

    // ParamWidget *pwidget = dynamic_cast<ParamWidget *>(gHoveredWidget);
    ParamWidget *pwidget = dynamic_cast<ParamWidget *>(APP->event->hoveredWidget);

    if (pwidget) {
        // TODO: show value of original and scaled?

        raw_value = pwidget->paramQuantity->getValue();
        display_min = pwidget->paramQuantity->getMinValue();
        display_max = pwidget->paramQuantity->getMaxValue();
        display_default = pwidget->paramQuantity->getDefaultValue();
        display_label = pwidget->paramQuantity->getLabel();
        display_value = pwidget->paramQuantity->getDisplayValue();
        display_description = pwidget->paramQuantity->description;
        display_unit = pwidget->paramQuantity->getUnit();
        display_type = "Param";

        // TODO: if we use type name detection stuff (cxxabi/typeinfo/etc) we could possibly
        //       also show the name of the hovered widget as a hint on mystery meat params
        // TODO: anyway to get the orig literal name of an enum value (ie, LFO_VC_OUTPUT etc)
        //       at runtime? might also be hint

    }

    PortWidget *port_widget = dynamic_cast<PortWidget *>(APP->event->hoveredWidget);

    if (port_widget) {
        if (port_widget->type == PortWidget::OUTPUT) {
            raw_value = port_widget->module->outputs[port_widget->portId].getVoltage();
            display_type = "Output";
        }

        if (port_widget->type == PortWidget::INPUT) {
            raw_value = port_widget->module->inputs[port_widget->portId].getVoltage();
            display_type = "Input";
        }

        // inputs/outputs dont have variable min/max, so just use the -10/+10 and
        // 0 for the default to get the point across.
        display_min = -10.0f;
        display_max = 10.0f;
        display_default = 0.0f;
        display_unit = " V";
        display_label = string::f("%s port", display_type.c_str());
        display_value = raw_value;
    }

    if (!pwidget && !port_widget) {
        tooltipHide();
    } else {
        // TODO build fancier tool tip text
        // TODO maybe just draw a widget like a tooltip, would be cool to draw a pop up a mini 'scope'

        std::string param_string = string::f("%s: %#.4g%s\n%s\ndisplay: %f\nraw: %#.4g",
                display_label.c_str(),
                display_value,
                display_unit.c_str(),
                display_description.c_str(),
                display_value,
                raw_value);

        tooltipShow(param_string,
                APP->event->hoveredWidget);

    }

    float scaled_value = rescale(raw_value, display_min, display_max,
                                 voltage_min[outputRange],
                                 voltage_max[outputRange]);

    module->params[HoveredValue::HOVERED_PARAM_VALUE_PARAM].setValue(raw_value);
    module->params[HoveredValue::HOVERED_SCALED_PARAM_VALUE_PARAM].setValue(scaled_value);

    param_value_field->setValue(raw_value);
    min_field->setText(string::f("%#.4g", display_min));
    max_field->setText(string::f("%#.4g", display_max));
    default_field->setText(string::f("%#.4g", display_default));
    widget_type_field->setText(display_type);

    // TODO: if a WireWidget, can we figure out it's in/out and current value? That would be cool,
    //       though it doesn't look like WireWidgets are ever hovered (or gHoveredWidget never
    //       seems to be a WireWidget).
}

void HoveredValueWidget::onChange(const event::Change &e) {
    ModuleWidget::onChange(e);
    param_value_field->onChange(e);
}



Model *modelHoveredValue = createModel<HoveredValue, HoveredValueWidget>("HoveredValue");

#include "alikins.hpp"

struct ShiftPedal : Module {
    enum ParamIds {
        LEFT_SHIFT_PARAM,
        RIGHT_SHIFT_PARAM,
        LEFT_CTRL_PARAM,
        RIGHT_CTRL_PARAM,
        LEFT_ALT_PARAM,
        RIGHT_ALT_PARAM,
        LEFT_SUPER_PARAM,
        RIGHT_SUPER_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        LEFT_SHIFT_GATE_OUTPUT,
        RIGHT_SHIFT_GATE_OUTPUT,
        EITHER_SHIFT_GATE_OUTPUT,
        LEFT_CTRL_GATE_OUTPUT,
        RIGHT_CTRL_GATE_OUTPUT,
        EITHER_CTRL_GATE_OUTPUT,
        LEFT_ALT_GATE_OUTPUT,
        RIGHT_ALT_GATE_OUTPUT,
        EITHER_ALT_GATE_OUTPUT,
        LEFT_SUPER_GATE_OUTPUT,
        RIGHT_SUPER_GATE_OUTPUT,
        EITHER_SUPER_GATE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };


    ShiftPedal() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(LEFT_SHIFT_PARAM, 0.0f, 10.0f, 0.0f, "Left Shift is pressed");
        configParam(RIGHT_SHIFT_PARAM, 0.0f, 10.0f, 0.0f, "Right Shift is pressed");
        configParam(LEFT_CTRL_PARAM, 0.0f, 10.0f, 0.0f, "Left Ctrl key is pressed");
        configParam(RIGHT_CTRL_PARAM, 0.0f, 10.0f, 0.0f, "Right Ctrl key is pressed");
        configParam(LEFT_ALT_PARAM, 0.0f, 10.0f, 0.0f, "Left Alt key is pressed");
        configParam(RIGHT_ALT_PARAM, 0.0f, 10.0f, 0.0f, "Right Alt key is pressed");
        configParam(LEFT_SUPER_PARAM, 0.0f, 10.0f, 0.0f, "Left Super/Win/Command key is pressed");
        configParam(RIGHT_SUPER_PARAM, 0.0f, 10.0f, 0.0f, "Right Super/Win/Command key is pressed");
    }
	void process(const ProcessArgs &args) override;

    // TODO: should probably setup a pulse generator for the gate outs
};

void ShiftPedal::process(const ProcessArgs &args) {
    // TODO: should probably setup a pulse generator for the gate outs

    outputs[LEFT_SHIFT_GATE_OUTPUT].setVoltage(params[LEFT_SHIFT_PARAM].getValue());
    outputs[RIGHT_SHIFT_GATE_OUTPUT].setVoltage(params[RIGHT_SHIFT_PARAM].getValue());
    outputs[EITHER_SHIFT_GATE_OUTPUT].value = params[LEFT_SHIFT_PARAM].getValue() +
        params[RIGHT_SHIFT_PARAM].getValue() >= 10.0f ? 10.0f : 0.0f;

    outputs[LEFT_CTRL_GATE_OUTPUT].setVoltage(params[LEFT_CTRL_PARAM].getValue());
    outputs[RIGHT_CTRL_GATE_OUTPUT].setVoltage(params[RIGHT_CTRL_PARAM].getValue());
    outputs[EITHER_CTRL_GATE_OUTPUT].value = params[LEFT_CTRL_PARAM].getValue() +
        params[RIGHT_CTRL_PARAM].getValue() >= 10.0f ? 10.0f : 0.0f;

    outputs[LEFT_ALT_GATE_OUTPUT].setVoltage(params[LEFT_ALT_PARAM].getValue());
    outputs[RIGHT_ALT_GATE_OUTPUT].setVoltage(params[RIGHT_ALT_PARAM].getValue());
    outputs[EITHER_ALT_GATE_OUTPUT].value = params[LEFT_ALT_PARAM].getValue() +
        params[RIGHT_ALT_PARAM].getValue() >= 10.0f ? 10.0f : 0.0f;

    outputs[LEFT_SUPER_GATE_OUTPUT].setVoltage(params[LEFT_SUPER_PARAM].getValue());
    outputs[RIGHT_SUPER_GATE_OUTPUT].setVoltage(params[RIGHT_SUPER_PARAM].getValue());
    outputs[EITHER_SUPER_GATE_OUTPUT].value = params[LEFT_SUPER_PARAM].getValue() +
        params[RIGHT_SUPER_PARAM].getValue() >= 10.0f ? 10.0f : 0.0f;
}

struct ShiftSwitch : app::SvgSwitch {
    ShiftSwitch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ShiftIsOff.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ShiftIsOn.svg")));
        shadow->opacity = 0.0f;
    }
};

struct CtrlSwitch : SvgSwitch  {
    CtrlSwitch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CtrlIsOff.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/CtrlIsOn.svg")));
        shadow->opacity = 0.0f;
    }
};

struct AltSwitch : SvgSwitch {
    AltSwitch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/AltIsOff.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/AltIsOn.svg")));
        shadow->opacity = 0.0f;
    }
};

struct SuperSwitch : SvgSwitch  {
    SuperSwitch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SuperIsOff.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/SuperIsOn.svg")));
        shadow->opacity = 0.0f;
    }
};

struct PurplePort : SvgPort {
	PurplePort() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PurplePort.svg")));
	}
};

struct ShiftPedalWidget : ModuleWidget {
    ShiftPedalWidget(ShiftPedal *module) {

        setModule(module);
        box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ShiftPedal.svg")));

        // FIXME: change to #defines
        float buttonWidth = 30.0f;
        float buttonHeight = 55.5f;
        // float y_start = 25.0f;
        float y_start = 35.0f;
        float y_spacing = 1.5f;
        float y_row_spacing = 6.5f;
        float y_baseline = y_start;
        float port_x_start = 3.0f;
        float middle = box.size.x / 2.0f;

        // first row, shift
        leftShiftButtonSwitch = createParam<ShiftSwitch>(Vec(0.0f, y_baseline),
                module,
                ShiftPedal::LEFT_SHIFT_PARAM);
        addParam(leftShiftButtonSwitch);

        buttonHeight = leftShiftButtonSwitch->box.size.y;

        PurplePort *leftShiftButtonPort = createOutput<PurplePort>(Vec(port_x_start, y_baseline + buttonHeight + y_spacing),
                module,
                ShiftPedal::LEFT_SHIFT_GATE_OUTPUT);
        addOutput(leftShiftButtonPort);

        // float portHeight = 24.0f;
        // float portWidth = 24.0f;
        float portHeight = leftShiftButtonPort->box.size.y;
        float portWidth = leftShiftButtonPort->box.size.x;

        // Add the 'either' port

        addOutput(createOutput<PurplePort>(Vec(middle - (portWidth/2.0f), y_baseline + buttonHeight + y_spacing),
                                    module,
                                    ShiftPedal::EITHER_SHIFT_GATE_OUTPUT));

        rightShiftButtonSwitch = createParam<ShiftSwitch>(Vec(buttonWidth, y_start), module, ShiftPedal::RIGHT_SHIFT_PARAM);
        addParam(rightShiftButtonSwitch);

        addOutput(createOutput<PurplePort>(Vec(box.size.x - portWidth - port_x_start, y_start + buttonHeight + y_spacing),
                    module,
                    ShiftPedal::RIGHT_SHIFT_GATE_OUTPUT));

        // next row
        y_baseline = y_baseline + buttonHeight + y_spacing + portHeight + y_row_spacing;

        leftCtrlButtonSwitch = createParam<CtrlSwitch>(Vec(0.0f, y_baseline), module, ShiftPedal::LEFT_CTRL_PARAM);
        addParam(leftCtrlButtonSwitch);

        // update for this row, although ended up making all the buttons the same size for now
        buttonHeight = leftCtrlButtonSwitch->box.size.y;

        addOutput(createOutput<PurplePort>(Vec(port_x_start, y_baseline + buttonHeight + y_spacing),
                    module,
                    ShiftPedal::LEFT_CTRL_GATE_OUTPUT));

        //either
        addOutput(createOutput<PurplePort>(Vec(middle - (portWidth / 2.0f), y_baseline + buttonHeight + y_spacing),
                                     module,
                                     ShiftPedal::EITHER_CTRL_GATE_OUTPUT));

        rightCtrlButtonSwitch = createParam<CtrlSwitch>(Vec(buttonWidth, y_baseline), module, ShiftPedal::RIGHT_CTRL_PARAM);
        addParam(rightCtrlButtonSwitch);

        addOutput(createOutput<PurplePort>(Vec(box.size.x - portWidth - port_x_start,
            y_baseline + buttonHeight + y_spacing),
                    module,
                    ShiftPedal::RIGHT_CTRL_GATE_OUTPUT));

        // third row Alt
        y_baseline = y_baseline + buttonHeight + y_spacing + portHeight + y_row_spacing;

        leftAltButtonSwitch = createParam<AltSwitch>(Vec(0.0f, y_baseline), module, ShiftPedal::LEFT_ALT_PARAM);
        addParam(leftAltButtonSwitch);

        float altButtonHeight = leftAltButtonSwitch->box.size.y;

        addOutput(createOutput<PurplePort>(Vec(2.0f, y_baseline + altButtonHeight + y_spacing),
                    module,
                    ShiftPedal::LEFT_ALT_GATE_OUTPUT));

        //either
        addOutput(createOutput<PurplePort>(Vec(middle - (portWidth / 2.0f), y_baseline + altButtonHeight + y_spacing),
                                     module,
                                     ShiftPedal::EITHER_ALT_GATE_OUTPUT));

        rightAltButtonSwitch = createParam<AltSwitch>(Vec(buttonWidth, y_baseline), module, ShiftPedal::RIGHT_ALT_PARAM);
        addParam(rightAltButtonSwitch);

        addOutput(createOutput<PurplePort>(Vec(box.size.x - portWidth - port_x_start, y_baseline + altButtonHeight + y_spacing),
                    module,
                    ShiftPedal::RIGHT_ALT_GATE_OUTPUT));

        // fourth row, super
        y_baseline = y_baseline + altButtonHeight + y_spacing + portHeight + y_row_spacing;

        leftSuperButtonSwitch = createParam<SuperSwitch>(Vec(0.0f, y_baseline), module, ShiftPedal::LEFT_SUPER_PARAM);
        addParam(leftSuperButtonSwitch);

        float superButtonHeight = leftSuperButtonSwitch->box.size.y;

        addOutput(createOutput<PurplePort>(Vec(2.0f, y_baseline + superButtonHeight + y_spacing),
                    module,
                    ShiftPedal::LEFT_SUPER_GATE_OUTPUT));

        //either
        addOutput(createOutput<PurplePort>(Vec(middle - (portWidth / 2.0f), y_baseline + superButtonHeight + y_spacing),
                                     module,
                                     ShiftPedal::EITHER_SUPER_GATE_OUTPUT));

        rightSuperButtonSwitch = createParam<SuperSwitch>(Vec(buttonWidth, y_baseline), module, ShiftPedal::RIGHT_SUPER_PARAM);
        addParam(rightSuperButtonSwitch);

        addOutput(createOutput<PurplePort>(Vec(box.size.x - portWidth - port_x_start, y_baseline + superButtonHeight + y_spacing),
                    module,
                    ShiftPedal::RIGHT_SUPER_GATE_OUTPUT));

        addChild(createWidget<ScrewSilver>(Vec(0.0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x-15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(0.0f, 365.0f)));
    }

    ParamWidget *leftShiftButtonSwitch;
    ParamWidget *rightShiftButtonSwitch;
    ParamWidget *leftCtrlButtonSwitch;
    ParamWidget *rightCtrlButtonSwitch;
    ParamWidget *leftAltButtonSwitch;
    ParamWidget *rightAltButtonSwitch;
    ParamWidget *leftSuperButtonSwitch;
    ParamWidget *rightSuperButtonSwitch;

    void step() override;

};


void ShiftPedalWidget::step() {

    if (!module) {
        return;
    }

    // TODO: switch to new key event api
    getParam(ShiftPedal::LEFT_SHIFT_PARAM)->getParamQuantity()->setValue(glfwGetKey(APP->window->win,
                GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 10.0f : 0.0f);
    getParam(ShiftPedal::RIGHT_SHIFT_PARAM)->getParamQuantity()->setValue(glfwGetKey(APP->window->win,
                GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ? 10.0f : 0.0f);

    getParam(ShiftPedal::LEFT_CTRL_PARAM)->getParamQuantity()->setValue(glfwGetKey(APP->window->win,
                GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? 10.0f : 0.0f);
    getParam(ShiftPedal::RIGHT_CTRL_PARAM)->getParamQuantity()->setValue(glfwGetKey(APP->window->win,
                GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ? 10.0f : 0.0f);

    getParam(ShiftPedal::LEFT_ALT_PARAM)->getParamQuantity()->setValue(glfwGetKey(APP->window->win,
                GLFW_KEY_LEFT_ALT) == GLFW_PRESS ? 10.0f : 0.0f);
    getParam(ShiftPedal::RIGHT_ALT_PARAM)->getParamQuantity()->setValue(glfwGetKey(APP->window->win,
                GLFW_KEY_RIGHT_ALT) == GLFW_PRESS ? 10.0f : 0.0f);

    getParam(ShiftPedal::LEFT_SUPER_PARAM)->getParamQuantity()->setValue(glfwGetKey(APP->window->win,
                GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ? 10.0f : 0.0f);
    getParam(ShiftPedal::RIGHT_SUPER_PARAM)->getParamQuantity()->setValue(glfwGetKey(APP->window->win,
                GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS ? 10.0f : 0.0f);

    ModuleWidget::step();
}

Model *modelShiftPedal = createModel<ShiftPedal, ShiftPedalWidget>("ShiftPedal");

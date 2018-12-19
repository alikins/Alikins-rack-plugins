#include "alikins.hpp"

#include "dsp/digital.hpp"
#include "window.hpp"

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
        LEFT_CTRL_GATE_OUTPUT,
        RIGHT_CTRL_GATE_OUTPUT,
        LEFT_ALT_GATE_OUTPUT,
        RIGHT_ALT_GATE_OUTPUT,
        LEFT_SUPER_GATE_OUTPUT,
        RIGHT_SUPER_GATE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };


    ShiftPedal() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    // TODO: should probably setup a pulse generator for the gate outs
};

void ShiftPedal::step() {
    // TODO: should probably setup a pulse generator for the gate outs

    outputs[LEFT_SHIFT_GATE_OUTPUT].value = params[LEFT_SHIFT_PARAM].value;
    outputs[RIGHT_SHIFT_GATE_OUTPUT].value = params[RIGHT_SHIFT_PARAM].value;

    outputs[LEFT_CTRL_GATE_OUTPUT].value = params[LEFT_CTRL_PARAM].value;
    outputs[RIGHT_CTRL_GATE_OUTPUT].value = params[RIGHT_CTRL_PARAM].value;

    outputs[LEFT_ALT_GATE_OUTPUT].value = params[LEFT_ALT_PARAM].value;
    outputs[RIGHT_ALT_GATE_OUTPUT].value = params[RIGHT_ALT_PARAM].value;

    outputs[LEFT_SUPER_GATE_OUTPUT].value = params[LEFT_SUPER_PARAM].value;
    outputs[RIGHT_SUPER_GATE_OUTPUT].value = params[RIGHT_SUPER_PARAM].value;
}

bool windowIsCtrlPressed() {
	return glfwGetKey(gWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(gWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
}


bool windowIsLeftShiftPressed() {
	return glfwGetKey(gWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
}

bool windowIsRightShiftPressed() {
    return glfwGetKey(gWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
}

struct ShiftSwitch : SVGSwitch, ToggleSwitch {
    ShiftSwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/ShiftIsOff.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/ShiftIsOn.svg")));
    }
};

struct ModSwitch : SVGSwitch, ToggleSwitch {
    ModSwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/ModIsOff.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/ModIsOn.svg")));
    }
};

struct PurplePort : SVGPort {
	PurplePort() {
		setSVG(SVG::load(assetPlugin(plugin, "res/PurplePort.svg")));
	}
};

// FIXME: maybe? vector and an enum likely better
/*
struct Modifiers {
    bool left_shift = false;
    bool right_shift = false;
    bool shift = false;
    bool left_ctrl = false;
    bool right_strl = false;
    bool ctrl = false;
    bool left_alt = false;
    bool right_alt = false;
    bool alt = false;
    bool left_super = false;
    bool right_super = false;
    bool super = false;
};
*/

struct ShiftPedalWidget : ModuleWidget {
    ShiftPedalWidget(ShiftPedal *module);

    // ParamWidget *shiftButtonSwitch;
    ParamWidget *leftShiftButtonSwitch;
    ParamWidget *rightShiftButtonSwitch;
    ParamWidget *leftCtrlButtonSwitch;
    ParamWidget *rightCtrlButtonSwitch;
    ParamWidget *leftAltButtonSwitch;
    ParamWidget *rightAltButtonSwitch;
    ParamWidget *leftSuperButtonSwitch;
    ParamWidget *rightSuperButtonSwitch;

    void step() override;
    void onKey(EventKey &e) override;
    void onHoverKey(EventHoverKey &e) override;

    // left shift, right shift, left ctrl, right ctrl
    // std::vector<bool> modifiers = {false, false, false, false};
};


ShiftPedalWidget::ShiftPedalWidget(ShiftPedal *module) : ModuleWidget(module) {
    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/ShiftPedal.svg")));

    float buttonWidth = 30.0f;
    float shiftButtonHeight = 55.5f;
    float ctrlButtonHeight = 55.0f;
    float y_start = 15.0f;
    float y_spacing = 0.5f;
    float y_baseline = y_start;

    leftShiftButtonSwitch = ParamWidget::create<ShiftSwitch>(Vec(0.0f, y_baseline),
                module,
                ShiftPedal::LEFT_SHIFT_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(leftShiftButtonSwitch);

    debug("leftShiftButtonSwitch height %f", leftShiftButtonSwitch->box.size.y);
    shiftButtonHeight = leftShiftButtonSwitch->box.size.y;

    // Port *leftShiftButtonPort;
    Port *leftShiftButtonPort =
        Port::create<PurplePort>(Vec(2.0f, y_baseline + shiftButtonHeight + y_spacing),
                                Port::OUTPUT,
                                module,
                                ShiftPedal::LEFT_SHIFT_GATE_OUTPUT);
    // leftShiftButtonPort->box.pos.x =
    addOutput(leftShiftButtonPort);

    float portHeight = leftShiftButtonPort->box.size.y;
    debug("leftShiftButtonPort height: %f", portHeight);

    rightShiftButtonSwitch = ParamWidget::create<ShiftSwitch>(Vec(buttonWidth, y_start),
                module,
                ShiftPedal::RIGHT_SHIFT_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(rightShiftButtonSwitch);

    addOutput(Port::create<PurplePort>(Vec(buttonWidth + 2.0f, y_start + shiftButtonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::RIGHT_SHIFT_GATE_OUTPUT));

    // next row
    y_baseline = y_baseline + shiftButtonHeight + y_spacing + portHeight + y_spacing;

    leftCtrlButtonSwitch = ParamWidget::create<ModSwitch>(Vec(0.0f, y_baseline),
                module,
                ShiftPedal::LEFT_CTRL_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(leftCtrlButtonSwitch);

    ctrlButtonHeight = leftCtrlButtonSwitch->box.size.y;

    addOutput(Port::create<PurplePort>(Vec(2.0f, y_baseline + ctrlButtonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::LEFT_CTRL_GATE_OUTPUT));

    rightCtrlButtonSwitch = ParamWidget::create<ModSwitch>(Vec(buttonWidth, y_baseline),
                module,
                ShiftPedal::RIGHT_CTRL_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(rightCtrlButtonSwitch);

    addOutput(Port::create<PurplePort>(Vec(buttonWidth + 2.0f, y_baseline + ctrlButtonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::RIGHT_CTRL_GATE_OUTPUT));

    // third row Alt
    y_baseline = y_baseline + ctrlButtonHeight + y_spacing + portHeight + y_spacing;

    leftAltButtonSwitch = ParamWidget::create<ModSwitch>(Vec(0.0f, y_baseline),
                module,
                ShiftPedal::LEFT_ALT_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(leftAltButtonSwitch);

    float altButtonHeight = leftAltButtonSwitch->box.size.y;

    addOutput(Port::create<PurplePort>(Vec(2.0f, y_baseline + altButtonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::LEFT_ALT_GATE_OUTPUT));

    rightAltButtonSwitch = ParamWidget::create<ModSwitch>(Vec(buttonWidth, y_baseline),
                module,
                ShiftPedal::RIGHT_ALT_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(rightAltButtonSwitch);

    addOutput(Port::create<PurplePort>(Vec(buttonWidth + 2.0f, y_baseline + altButtonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::RIGHT_ALT_GATE_OUTPUT));

    // fourth row, super
    y_baseline = y_baseline + altButtonHeight + y_spacing + portHeight + y_spacing;

    leftSuperButtonSwitch = ParamWidget::create<ModSwitch>(Vec(0.0f, y_baseline),
                module,
                ShiftPedal::LEFT_SUPER_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(leftSuperButtonSwitch);

    float superButtonHeight = leftSuperButtonSwitch->box.size.y;

    addOutput(Port::create<PurplePort>(Vec(2.0f, y_baseline + superButtonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::LEFT_SUPER_GATE_OUTPUT));

    rightSuperButtonSwitch = ParamWidget::create<ModSwitch>(Vec(buttonWidth, y_baseline),
                module,
                ShiftPedal::RIGHT_SUPER_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(rightSuperButtonSwitch);

    addOutput(Port::create<PurplePort>(Vec(buttonWidth + 2.0f, y_baseline + superButtonHeight + y_spacing),
                Port::OUTPUT,
                module,
                ShiftPedal::RIGHT_SUPER_GATE_OUTPUT));

    addChild(Widget::create<ScrewSilver>(Vec(0.0, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-15, 365.0f)));

}

void ShiftPedalWidget::step() {
    // TODO: just return a list of bools for all of them
    // bool shift_pressed = windowIsShiftPressed();
    // bool mod_pressed = windowIsModPressed();
    // bool ctrl_pressed = windowIsCtrlPressed();
    // bool left_shift_pressed = windowIsLeftShiftPressed();
    // bool right_shift_pressed = windowIsRightShiftPressed();

    // shiftButtonSwitch->setValue(shift_pressed ? 10.0f : 0.0f);
    // modButtonSwitch->setValue(mod_pressed ? 10.0f : 0.0f);

    leftShiftButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 10.0f : 0.0f);
    rightShiftButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS ? 10.0f : 0.0f);

    leftCtrlButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ? 10.0f : 0.0f);
    rightCtrlButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS ? 10.0f : 0.0f);

    leftAltButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_LEFT_ALT) == GLFW_PRESS ? 10.0f : 0.0f);
    rightAltButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS ? 10.0f : 0.0f);

    leftSuperButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_LEFT_SUPER) == GLFW_PRESS ? 10.0f : 0.0f);
    rightSuperButtonSwitch->setValue(glfwGetKey(gWindow, GLFW_KEY_RIGHT_SUPER) == GLFW_PRESS ? 10.0f : 0.0f);

    /*
    if (left_shift_pressed) {
        debug("left shift pressed");
    }

    if (right_shift_pressed) {
        debug("right shift pressed");
    }

    if (ctrl_pressed) {
        debug("ctrl pressed");
    }
    */

    ModuleWidget::step();
}

void ShiftPedalWidget::onKey(EventKey &e) {
    debug("ShiftPedalWidget::onKey e.key=%d", e.key);
}

void ShiftPedalWidget::onHoverKey(EventHoverKey &e) {
    debug("ShiftPedalWidget::onHoverKey e.key=%d", e.key);
}

Model *modelShiftPedal = Model::create<ShiftPedal, ShiftPedalWidget>(
        "Alikins", "ShiftPedal", "Shift Pedal - Send gate when shift is pressed", UTILITY_TAG);

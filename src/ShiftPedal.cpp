#include "alikins.hpp"

#include "dsp/digital.hpp"
#include "window.hpp"

struct ShiftPedal : Module {
    enum ParamIds {
        SHIFT_BUTTON_PARAM,
        MOD_BUTTON_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        SHIFT_GATE_OUTPUT,
        MOD_GATE_OUTPUT,
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

    outputs[SHIFT_GATE_OUTPUT].value = params[SHIFT_BUTTON_PARAM].value;
    outputs[MOD_GATE_OUTPUT].value = params[MOD_BUTTON_PARAM].value;
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

struct ShiftPedalWidget : ModuleWidget {
    ShiftPedalWidget(ShiftPedal *module);

    ParamWidget *shiftButtonSwitch;
    ParamWidget *modButtonSwitch;

    void step() override;
    void onKey(EventKey &e) override;
    void onHoverKey(EventHoverKey &e) override;
};


ShiftPedalWidget::ShiftPedalWidget(ShiftPedal *module) : ModuleWidget(module) {

    box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/ShiftPedal.svg")));

    shiftButtonSwitch = ParamWidget::create<ShiftSwitch>(Vec(0.0f, 20.0f),
                module,
                ShiftPedal::SHIFT_BUTTON_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(shiftButtonSwitch);

    addOutput(Port::create<PJ301MPort>(Vec(2.0f, 80.0f),
                Port::OUTPUT,
                module,
                ShiftPedal::SHIFT_GATE_OUTPUT));

    modButtonSwitch = ParamWidget::create<ModSwitch>(Vec(0.0f, 135.0f),
                module,
                ShiftPedal::MOD_BUTTON_PARAM,
                0.0f, 10.0f, 0.0f);
    addParam(modButtonSwitch);

    addOutput(Port::create<PJ301MPort>(Vec(2.0f, 195.0f),
                Port::OUTPUT,
                module,
                ShiftPedal::MOD_GATE_OUTPUT));

    addChild(Widget::create<ScrewSilver>(Vec(0.0, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-15, 365.0f)));

}

void ShiftPedalWidget::step() {
    bool shift_pressed = windowIsShiftPressed();
    bool mod_pressed = windowIsModPressed();

    shiftButtonSwitch->setValue(shift_pressed ? 10.0f : 0.0f);
    modButtonSwitch->setValue(mod_pressed ? 10.0f : 0.0f);

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

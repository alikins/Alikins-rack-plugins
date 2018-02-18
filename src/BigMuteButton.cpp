#include "alikins.hpp"
#include <stdio.h>


struct BigMuteButton : Module {
    enum ParamIds {
        BIG_MUTE_BUTTON_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        LEFT_INPUT,
        RIGHT_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };


    BigMuteButton() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

};


void BigMuteButton::step() {
    if (params[BIG_MUTE_BUTTON_PARAM].value) {
        outputs[LEFT_OUTPUT].value = inputs[LEFT_INPUT].value;
        outputs[RIGHT_OUTPUT].value = inputs[RIGHT_INPUT].value;
    }

}

struct BigSwitch : SVGSwitch, ToggleSwitch {
    BigSwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/BigMuteButtonUnmute.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/BigMuteButtonMute.svg")));
    }
};

BigMuteButtonWidget::BigMuteButtonWidget() {
    BigMuteButton *module = new BigMuteButton();
    setModule(module);
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/BigMuteButton.svg")));
        addChild(panel);
    }

    addParam(createParam<BigSwitch>(Vec(0.0f, 0.0f), module, BigMuteButton::BIG_MUTE_BUTTON_PARAM, 0.0, 1.0, 0.0));

    addInput(createInput<PJ301MPort>(Vec(5.0f, 310.0f), module, BigMuteButton::LEFT_INPUT));
    addInput(createInput<PJ301MPort>(Vec(5.0f, 340.0f), module, BigMuteButton::RIGHT_INPUT));

    addOutput(createOutput<PJ301MPort>(Vec(50.0f, 310.0f), module, BigMuteButton::LEFT_OUTPUT));
    addOutput(createOutput<PJ301MPort>(Vec(50.0f, 340.0f), module, BigMuteButton::RIGHT_OUTPUT));

}

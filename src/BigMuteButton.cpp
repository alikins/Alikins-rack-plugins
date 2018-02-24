#include "alikins.hpp"


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
    } else {
        outputs[LEFT_OUTPUT].value = 0.0f;
        outputs[RIGHT_OUTPUT].value = 0.0f;
    }

}

struct BigSwitch : SVGSwitch, ToggleSwitch {
    BigSwitch() {
        addFrame(SVG::load(assetPlugin(plugin, "res/BigMuteButtonUnmute.svg")));
        addFrame(SVG::load(assetPlugin(plugin, "res/BigMuteButtonMute.svg")));
    }
};


struct BigMuteButtonWidget : ModuleWidget {
    BigMuteButtonWidget(BigMuteButton *module);
};


BigMuteButtonWidget::BigMuteButtonWidget(BigMuteButton *module) : ModuleWidget(module) {

    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/BigMuteButton.svg")));

    addParam(ParamWidget::create<BigSwitch>(Vec(0.0f, 0.0f),
                module,
                BigMuteButton::BIG_MUTE_BUTTON_PARAM,
                0.0, 1.0, 0.0));

    addInput(Port::create<PJ301MPort>(Vec(4.0f, 302.0f),
                Port::INPUT,
                module,
                BigMuteButton::LEFT_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(4.0f, 330.0f),
                Port::INPUT,
                module,
                BigMuteButton::RIGHT_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(60.0f, 302.0f),
                Port::OUTPUT,
                module,
                BigMuteButton::LEFT_OUTPUT));
    addOutput(Port::create<PJ301MPort>(Vec(60.0f, 330.0f),
                Port::OUTPUT,
                module,
                BigMuteButton::RIGHT_OUTPUT));

    addChild(Widget::create<ScrewSilver>(Vec(0.0, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-15, 0)));
    addChild(Widget::create<ScrewSilver>(Vec(30, 365)));

}

Model *modelBigMuteButton = Model::create<BigMuteButton, BigMuteButtonWidget>(
        "Alikins", "BigMuteButton", "Big Mute Button", UTILITY_TAG);

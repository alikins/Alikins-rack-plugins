#include "alikins.hpp"
#include <stdio.h>


struct BigMuteButton : Module {
    enum ParamIds {
        BIG_RED_BUTTON_PARAM,
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
        MUTE_LIGHT,
        NUM_LIGHTS
    };


    BigMuteButton() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    // For more advanced Module features, read Rack's engine.hpp header file
    // - toJson, fromJson: serialization of internal data
    // - onSampleRateChange: event triggered by a change of sample rate
    // - reset, randomize: implements special behavior when user clicks these from the context menu
};


void BigMuteButton::step() {
    if (params[BIG_RED_BUTTON_PARAM].value) {
        outputs[LEFT_OUTPUT].value = inputs[LEFT_INPUT].value
        outputs[RIGHT_OUTPUT].value = inputs[RIGHT_INPUT].value
        lights[MUTED_LIGHT].setBrightness(1.0);
    }

}


BigMuteButtonWidget::BigMuteButtonWidget() {
    BigMuteButton *module = new BigMuteButton();
    setModule(module);
    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);


    {
        SVGPanel *panel = new SVGPanel();
        panel->box.size = box.size;
        panel->setBackground(SVG::load(assetPlugin(plugin, "res/BigMuteButton.svg")));
        addChild(panel);
    }

    addParam(createParam<LEDButton>(Vec(0,0), module, BigMuteButton::BIG_REG_BUTTON_PARAM + i, 0.0, 1.0, 0.0));
    addChild(createLight<MediumLight<RedLight>>(Vec(0, 0), module, BigMuteButton::MUTED_LIGHT + i));

    addOutput(createOutput<PJ301MPort>(Vec(x_pos + 20 + light_radius, y_pos), module, BigMuteButton::BUTTON1_OUTPUT + i));

}

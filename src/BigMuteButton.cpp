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

    float gain_mult = 1.0f;

    enum FadeState {
        UNMUTED_STEADY,
        MUTED_STEADY,
        MUTED_FADE_DOWN,
        UNMUTED_FADE_UP,
        INITIAL
    };

    // FadeState state = UNMUTED_STEADY;
    FadeState state = INITIAL;
    dsp::SchmittTrigger muteOnTrigger;
    dsp::SchmittTrigger muteOffTrigger;

    float gmult2 = 1.0f;

    float crossfade_mix = 0.005f;

    BigMuteButton() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(BIG_MUTE_BUTTON_PARAM, 0.0f, 1.0f, 0.0f);
    }

	void process(const ProcessArgs &args) override;

    void onReset() override {
        state = UNMUTED_STEADY;
    }

};

void BigMuteButton::process(const ProcessArgs &args) {

    // INITIAL state, choose next state based on current value of BIG_MUTE_BUTTON_PARAM
    //  since BIG_MUTE_BUTTON_PARAM should be based on either the default, or for a saved
    // patch, the value saved to the params JSON.

    if (muteOnTrigger.process(params[BIG_MUTE_BUTTON_PARAM].getValue())) {
        // debug("MUTE ON");
        state = MUTED_FADE_DOWN;
        gmult2 = 1.0f;
    }

    if (muteOffTrigger.process(!params[BIG_MUTE_BUTTON_PARAM].getValue())) {
        // debug("MUTE OFF");
        state = UNMUTED_FADE_UP;
        gmult2 = 0.0f;
    }

    switch(state) {
        case INITIAL:
            state = (params[BIG_MUTE_BUTTON_PARAM].getValue() == 0.0f) ? UNMUTED_STEADY : MUTED_STEADY;
            break;
        case MUTED_STEADY:
            gmult2 = 0.0f;
            break;
        case UNMUTED_STEADY:
            gmult2 = 1.0f;
            break;
        case MUTED_FADE_DOWN:
            if (isNear(gmult2, 0.0f)) {
                state = MUTED_STEADY;
                // debug("faded  down crossfade to 0.0");
                gmult2 = 0.0f;
                break;
            }
            gmult2 = crossfade(gmult2, 0.0f, crossfade_mix);
            break;
        case UNMUTED_FADE_UP:
            if (isNear(gmult2, 1.0f)) {
                state = UNMUTED_STEADY;
                // debug("faded up crossfade to 1.0");
                gmult2 = 1.0f;
                break;
            }
            gmult2 = crossfade(gmult2, 1.0f, crossfade_mix);
            break;
    }

    gmult2 = clamp(gmult2, 0.0f, 1.0f);

    outputs[LEFT_OUTPUT].setVoltage(inputs[LEFT_INPUT].getVoltage() * gmult2);
    outputs[RIGHT_OUTPUT].setVoltage(inputs[RIGHT_INPUT].getVoltage() * gmult2);

    // debug("state: %d, gmult2: %f", state, gmult2);

    // TODO: to eliminate worse case DC thump, also apply a RC filter of some sort?
}

struct BigSwitch : SvgSwitch {
    BigSwitch() {
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BigMuteButtonMute.svg")));
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BigMuteButtonUnmute.svg")));
    }
};


struct BigMuteButtonWidget : ModuleWidget {
    BigMuteButtonWidget(BigMuteButton *module) {
        setModule(module);
        // box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BigMuteButton.svg")));

        addParam(createParam<BigSwitch>(Vec(0.0f, 0.0f),
                    module,
                    BigMuteButton::BIG_MUTE_BUTTON_PARAM));

        addInput(createInput<PJ301MPort>(Vec(4.0f, 302.0f),
                    module,
                    BigMuteButton::LEFT_INPUT));
        addInput(createInput<PJ301MPort>(Vec(4.0f, 330.0f),
                    module,
                    BigMuteButton::RIGHT_INPUT));

        addOutput(createOutput<PJ301MPort>(Vec(60.0f, 302.0f),
                    module,
                    BigMuteButton::LEFT_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(60.0f, 330.0f),
                    module,
                    BigMuteButton::RIGHT_OUTPUT));

        addChild(createWidget<ScrewSilver>(Vec(0.0, 0)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x-15, 0)));
        addChild(createWidget<ScrewSilver>(Vec(30, 365)));
    }
};

Model *modelBigMuteButton = createModel<BigMuteButton, BigMuteButtonWidget>("BigMuteButton");

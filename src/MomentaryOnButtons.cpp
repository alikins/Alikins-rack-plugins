#include <stdio.h>
#include "alikins.hpp"

struct MomentaryOnButtons : Module {
    enum ParamIds {
        ENUMS(BUTTON_PARAM, MOMENTARY_BUTTONS),
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(BUTTON_INPUT, MOMENTARY_BUTTONS),
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(BUTTON_OUTPUT, MOMENTARY_BUTTONS),
        NUM_OUTPUTS
    };
    enum LightIds {
        ENUMS(BLINK_LIGHT, MOMENTARY_BUTTONS),
        NUM_LIGHTS
    };

    MomentaryOnButtons() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        for (int i = 0; i < MOMENTARY_BUTTONS; i++) {
            configParam(BUTTON_PARAM + i,
                        0.0, 1.0, 0.0, string::f("Button %d", i+1));
        }
    }

    void process(const ProcessArgs &args) override;

};


void MomentaryOnButtons::process(const ProcessArgs &args) {

    for (int i = 0; i < MOMENTARY_BUTTONS; i++) {

        lights[BLINK_LIGHT + i].setBrightness(0.0);
        outputs[BUTTON_OUTPUT + i].setVoltage(0.0f);

        if (params[BUTTON_PARAM + i].getValue()) {
            outputs[BUTTON_OUTPUT + i].setVoltage(5.0f);
            lights[BLINK_LIGHT + i].setBrightness(1.0);
        }
    }
}

struct LightupButton : SvgSwitch {
	LightupButton() {
		momentary = true;
        addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LightupButtonDown.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/LightupButton.svg")));
        // shadow->opacity = 0.0f;
    };
};

struct MomentaryOnButtonsWidget : ModuleWidget {
    MomentaryOnButtonsWidget(MomentaryOnButtons *module) {
        setModule(module);
        box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        int x_offset = 8;
        int y_offset = 26;

        int x_start = 0;
        int y_start = 24;

        int x_pos = 0;
        int y_pos = 0;

        int light_size = 20;

        {
            SvgPanel *panel = new SvgPanel();
            panel->box.size = box.size;
            panel->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/MomentaryOnButtons.svg")));
            addChild(panel);
        }

        /*
           addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
           addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
           addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
           addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
           */

        for (int i = 0; i < MOMENTARY_BUTTONS; i++) {

            x_pos = x_start + x_offset;
            y_pos = y_start + (i * y_offset);

            addParam(createParam<LightupButton>(Vec(x_pos, y_pos + 3),
                module, MomentaryOnButtons::BUTTON_PARAM + i));

            //addChild(createLight<MediumLight<RedLight>>(Vec(x_pos + 5 + light_radius, y_pos + light_radius),
            //    module, MomentaryOnButtons::BLINK_LIGHT + i));

            addOutput(createOutput<PJ301MPort>(Vec(x_pos + light_size + 4, y_pos),
                module, MomentaryOnButtons::BUTTON_OUTPUT + i));
        }

    }
};


Model *modelMomentaryOnButtons = createModel<MomentaryOnButtons, MomentaryOnButtonsWidget>("MomentaryOnButtons");

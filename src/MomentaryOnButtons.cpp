#include <stdio.h>
#include "alikins.hpp"


struct MomentaryOnButtons : Module {
    enum ParamIds {
        BUTTON1_PARAM,
        BUTTON2_PARAM,
        BUTTON3_PARAM,
        BUTTON4_PARAM,
        BUTTON5_PARAM,
        BUTTON6_PARAM,
        BUTTON7_PARAM,
        BUTTON8_PARAM,
        BUTTON9_PARAM,
        BUTTON10_PARAM,
        BUTTON11_PARAM,
        BUTTON12_PARAM,
        BUTTON13_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        BUTTON1_INPUT,
        BUTTON2_INPUT,
        BUTTON3_INPUT,
        BUTTON4_INPUT,
        BUTTON5_INPUT,
        BUTTON6_INPUT,
        BUTTON7_INPUT,
        BUTTON8_INPUT,
        BUTTON9_INPUT,
        BUTTON10_INPUT,
        BUTTON11_INPUT,
        BUTTON12_INPUT,
        BUTTON13_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        BUTTON1_OUTPUT,
        BUTTON2_OUTPUT,
        BUTTON3_OUTPUT,
        BUTTON4_OUTPUT,
        BUTTON5_OUTPUT,
        BUTTON6_OUTPUT,
        BUTTON7_OUTPUT,
        BUTTON8_OUTPUT,
        BUTTON9_OUTPUT,
        BUTTON10_OUTPUT,
        BUTTON11_OUTPUT,
        BUTTON12_OUTPUT,
        BUTTON13_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        BLINK1_LIGHT,
        BLINK2_LIGHT,
        BLINK3_LIGHT,
        BLINK4_LIGHT,
        BLINK5_LIGHT,
        BLINK6_LIGHT,
        BLINK7_LIGHT,
        BLINK8_LIGHT,
        BLINK9_LIGHT,
        BLINK10_LIGHT,
        BLINK11_LIGHT,
        BLINK12_LIGHT,
        BLINK13_LIGHT,
        NUM_LIGHTS
    };


    MomentaryOnButtons() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void process(const ProcessArgs &args) override;

};


void MomentaryOnButtons::process(const ProcessArgs &args) {

    for (int i = 0; i < MOMENTARY_BUTTONS; i++) {

        lights[BLINK1_LIGHT + i].setBrightness(0.0);
        outputs[BUTTON1_OUTPUT + i].value = 0.0;

        if (params[BUTTON1_PARAM + i].value) {
            outputs[BUTTON1_OUTPUT + i].value = 5.0;
            lights[BLINK1_LIGHT + i].setBrightness(1.0);
        }
    }
}


struct MomentaryOnButtonsWidget : ModuleWidget {
    MomentaryOnButtonsWidget(MomentaryOnButtons *module);
};


MomentaryOnButtonsWidget::MomentaryOnButtonsWidget(MomentaryOnButtons *module) : ModuleWidget(module) {
    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    int x_offset = 0;
    int y_offset = 26;

    int x_start = 0;
    int y_start = 24;

    int x_pos = 0;
    int y_pos = 0;

    int light_radius = 7;

    {
        SVGPanel *panel = new SVGPanel();
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

        addParam(createParam<LEDButton>(Vec(x_pos + light_radius, y_pos + 3), module, MomentaryOnButtons::BUTTON1_PARAM + i, 0.0, 1.0, 0.0));
        addChild(createLight<MediumLight<RedLight>>(Vec(x_pos + 5 + light_radius, y_pos + light_radius), module, MomentaryOnButtons::BLINK1_LIGHT + i));

        addOutput(createOutput<PJ301MPort>(Vec(x_pos + 20 + light_radius, y_pos),
                  module,
                  MomentaryOnButtons::BUTTON1_OUTPUT + i));
    }
}

Model *modelMomentaryOnButtons = createModel<MomentaryOnButtons, MomentaryOnButtonsWidget>("MomentaryOnButtons");

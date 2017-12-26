#include "alikins.hpp"
#include <stdio.h>


struct MomentaryOnButtons : Module {
	enum ParamIds {
        BUTTON1_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
        BUTTON1_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
        BUTTON1_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		BLINK_LIGHT,
		NUM_LIGHTS
	};


	MomentaryOnButtons() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void MomentaryOnButtons::step() {

	// lights[BLINK_LIGHT].value = 0.0;
	lights[BLINK_LIGHT].setBrightness(0.0);
    outputs[BUTTON1_OUTPUT].value = 0.0;

    if (params[BUTTON1_PARAM].value)
        outputs[BUTTON1_OUTPUT].value = 1.0;
	    // lights[BLINK_LIGHT].value = 1.0;
	    lights[BLINK_LIGHT].setBrightness(1.0);
}


MomentaryOnButtonsWidget::MomentaryOnButtonsWidget() {
	MomentaryOnButtons *module = new MomentaryOnButtons();
	setModule(module);
	box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

	{
		SVGPanel *panel = new SVGPanel();
		panel->box.size = box.size;
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/MomentaryOnButtons.svg")));
		addChild(panel);
	}

    /*
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
*/


	addChild(createLight<MediumLight<RedLight>>(Vec(41, 59), module, MomentaryOnButtons::BLINK_LIGHT));

    addParam(createParam<LEDButton>(Vec(10, 110), module, MomentaryOnButtons::BUTTON1_PARAM, 0.0, 1.0, 0.0));
	addOutput(createOutput<PJ301MPort>(Vec(30, 110), module, MomentaryOnButtons::BUTTON1_OUTPUT));

}

#include "alikins.hpp"
#include <stdio.h>
#include "dsp/digital.hpp"
#include "util.hpp"

struct IdleSwitch : Module {
	enum ParamIds {
        BUTTON1_PARAM,
		TIME_PARAM,
        NUM_PARAMS
	};
	enum InputIds {
        INPUT_SOURCE_INPUT,
		HEARTBEAT_INPUT,
        NUM_INPUTS
	};
	enum OutputIds {
        IDLE_GATE_OUTPUT,
        TIME_OUTPUT,
        NUM_OUTPUTS
	};
	enum LightIds {
		IDLE_GATE_LIGHT,
		NUM_LIGHTS
	};

	SchmittTrigger _trigger;

    int frameCount = 0;

	IdleSwitch() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

	// For more advanced Module features, read Rack's engine.hpp header file
	// - toJson, fromJson: serialization of internal data
	// - onSampleRateChange: event triggered by a change of sample rate
	// - reset, randomize: implements special behavior when user clicks these from the context menu
};


void IdleSwitch::step() {
    float last = 0;
    float interval = 100;
    float theTime = 0.0;

    outputs[IDLE_GATE_OUTPUT].value = 0.0;
    lights[IDLE_GATE_LIGHT].setBrightness(0.0);

    theTime = params[TIME_PARAM].value;
    outputs[TIME_OUTPUT].value = params[TIME_PARAM].value;

    // Compute time
	// float deltaTime = powf(2.0, params[TIME_PARAM].value);
	float deltaTime = params[TIME_PARAM].value;
	int maxFrameCount = (int)ceilf(deltaTime * engineGetSampleRate());

    info("theTime: %f maxFrameCount: %d frameCount: %d", theTime, maxFrameCount, frameCount);
    if (inputs[INPUT_SOURCE_INPUT].active) {
        if (_trigger.process(inputs[INPUT_SOURCE_INPUT].value)) {
            // outputs[IDLE_GATE_OUTPUT].value = 10;
            // lights[IDLE_GATE_LIGHT].setBrightness(1.0);
            frameCount = 0;
        // info("maxFrameCount: %d frameCount: %d", maxFrameCount, frameCount);
        // info("theTime: %f maxFrameCount: %d frameCount: %d", theTime, maxFrameCount, frameCount);
        }
    }


    if (frameCount <= maxFrameCount) {
            outputs[IDLE_GATE_OUTPUT].value = 5.0;
            lights[IDLE_GATE_LIGHT].setBrightness(1.0);

    }

    frameCount++;
}


IdleSwitchWidget::IdleSwitchWidget() {
	IdleSwitch *module = new IdleSwitch();
	setModule(module);
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
		panel->setBackground(SVG::load(assetPlugin(plugin, "res/IdleSwitch.svg")));
		addChild(panel);
	}

    /*
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
	addChild(createScrew<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
	addChild(createScrew<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
*/

    y_pos = y_start + y_offset;

	addInput(createInput<PJ301MPort>(Vec(11, y_pos), module, IdleSwitch::INPUT_SOURCE_INPUT));

    x_pos = x_start + x_offset;
    y_pos = y_pos + y_offset + 5;

    addParam(createParam<RoundSmallBlackKnob>(Vec(20, y_pos), module, IdleSwitch::TIME_PARAM, 0.0, 8.0, 1.0));

    y_pos = y_pos + y_offset + 5;

    addOutput(createOutput<PJ301MPort>(Vec(x_pos + 20, y_pos), module, IdleSwitch::TIME_OUTPUT));

    y_pos = y_pos + y_offset + 5;

    addParam(createParam<LEDButton>(Vec(x_pos + light_radius, y_pos + 3), module, IdleSwitch::BUTTON1_PARAM, 0.0, 1.0, 0.0));

    addChild(createLight<MediumLight<RedLight>>(Vec(x_pos + 5 + light_radius, y_pos + light_radius), module, IdleSwitch::IDLE_GATE_LIGHT));

    addOutput(createOutput<PJ301MPort>(Vec(x_pos + 20 + light_radius, y_pos), module, IdleSwitch::IDLE_GATE_OUTPUT));


}

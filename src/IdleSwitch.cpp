#include "alikins.hpp"
#include <stdio.h>
#include "dsp/digital.hpp"
#include "util.hpp"

struct IdleSwitch : Module {
    enum ParamIds {
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
        // TIME_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        IDLE_GATE_LIGHT,
        NUM_LIGHTS
    };

    SchmittTrigger inputTrigger;
    SchmittTrigger heartbeatTrigger;

    int frameCount = 0;
    int maxFrameCount = 0;

    IdleSwitch() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    // For more advanced Module features, read Rack's engine.hpp header file
    // - toJson, fromJson: serialization of internal data
    // - onSampleRateChange: event triggered by a change of sample rate
    // - reset, randomize: implements special behavior when user clicks these from the context menu
};


void IdleSwitch::step() {
    outputs[IDLE_GATE_OUTPUT].value = 10.0;
    lights[IDLE_GATE_LIGHT].setBrightness(0.0);

    // TIME_PARAM is time between idle loop checks (the timeout) in seconds
    // theTime = params[TIME_PARAM].value;
    // mostly for debugging atm
    // outputs[TIME_OUTPUT].value = params[TIME_PARAM].value;

    float sampleRate = engineGetSampleRate();

    if (inputs[HEARTBEAT_INPUT].active) {
        if (heartbeatTrigger.process(inputs[HEARTBEAT_INPUT].value)) {
            // reset the timeout on heartbeat
            maxFrameCount = 0;
        }
        else {
            maxFrameCount++;
        }
    }
    else {
        // Compute time
        float deltaTime = params[TIME_PARAM].value;

        maxFrameCount = (int)ceilf(deltaTime * sampleRate);
    }

    // float delta = 1.0 / sampleRate;
    // float frameTime = frameCount * delta;
    // float maxFrameTime = maxFrameCount * delta;

    // info("theTime: %f maxFrameCount: %d frameCounttime: %f delta: %f", theTime, maxFrameTime, frameTime, delta);
    // info("maxFrameCount: %d frameCount: %d delta: %f", maxFrameCount, frameCount, delta);
    if (inputs[INPUT_SOURCE_INPUT].active) {
        if (inputTrigger.process(inputs[INPUT_SOURCE_INPUT].value)) {
            // outputs[IDLE_GATE_OUTPUT].value = 10;
            // lights[IDLE_GATE_LIGHT].setBrightness(1.0);
            frameCount = 0;

            // info("maxFrameCount: %d frameCount: %d", maxFrameCount, frameCount);
            // info("theTime: %f maxFrameCount: %d frameCount: %d", theTime, maxFrameCount, frameCount);
        }
        else {
            frameCount++;
        }
    }

    if (frameCount <= maxFrameCount) {
        outputs[IDLE_GATE_OUTPUT].value = 0.0;
        lights[IDLE_GATE_LIGHT].setBrightness(1.0);

    }

    // frameCount++;
}


IdleSwitchWidget::IdleSwitchWidget() {
    IdleSwitch *module = new IdleSwitch();
    setModule(module);
    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

    int x_offset = 5;
    int y_offset = 25;

    int x_start = 0 + x_offset;
    int y_start = 40;

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

    y_pos = y_start;
    x_pos = x_start;

    addInput(createInput<PJ301MPort>(Vec(x_pos, y_pos), module, IdleSwitch::INPUT_SOURCE_INPUT));

    x_pos = x_pos + 25;
    // y_pos = y_pos + y_offset + 5;

    addParam(createParam<RoundSmallBlackKnob>(Vec(x_pos, y_pos), module, IdleSwitch::TIME_PARAM, 0.0, 8.0, 1.0));

    // y_pos = y_pos + y_offset + 25;
    // x_pos = x_pos + 30;

    addOutput(createOutput<PJ301MPort>(Vec(x_pos + 30, y_pos), module, IdleSwitch::IDLE_GATE_OUTPUT));


    y_pos = y_pos + y_offset + 5;
    // x_pos = x_pos + 25;
    x_pos = x_start;

    // addParam(createParam<LEDButton>(Vec(x_pos, y_pos), module, IdleSwitch::BUTTON1_PARAM, 0.0, 1.0, 0.0));

    addChild(createLight<MediumLight<RedLight>>(Vec(x_pos + light_radius, y_pos + light_radius), module, IdleSwitch::IDLE_GATE_LIGHT));

    addInput(createInput<PJ301MPort>(Vec(x_pos + 25, y_pos), module, IdleSwitch::HEARTBEAT_INPUT));


    // addOutput(createOutput<PJ301MPort>(Vec(x_pos, y_pos), module, IdleSwitch::TIME_OUTPUT));
}

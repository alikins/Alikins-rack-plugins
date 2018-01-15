#include "alikins.hpp"
#include <stdio.h>
#include "dsp/digital.hpp"
#include "util.hpp"

/* IdleSwitch
 *
 * What:
 *
 * If no input events are seen at Input Source within the timeout period
 * emit a gate on Idle Gate Output that lasts until there are input events
 * again. Then reset the timeout period.
 *
 * Sort of metaphoricaly like an idle handler or timeout in event based
 * programming like GUI main loops.
 *
 * If no Heartbeat source is active, the timeout period is set by the value
 * of the Timeout param.
 *
 * If there is a Heartbeat source, when it gets an event, the timeout period
 * is reset. After a heart beat event, the Idle Gate Output will remain on until
 * an input event is seen at Input Source. When there is an input event, the Idle
 * Gate Output is turned off until the next Heartbeat.
 *
 * To use the eventloop/gui main loop analogy, a Heartbeat event is equilivent to
 * running an idle handler directly (or running a mainloop iteration with no non-idle
 * events pending).
 *
 * Why:
 *
 * Original intentional was to use in combo with a human player and midi/cv keyboard.
 * As long as the human is playing, the IdleSwitch output is 'off', but if they go
 * idle for some time period the output is turned on. For example, a patch may plain
 * loud drone when idle, but would turn the drone off or down when the human played
 * and then turn it back on when it stopped. Or maybe it could be used to start an
 * drum fill...
 *
 * The Heartbeat input allows this be synced to clock, beat, or sequence. In the drone
 * example above, the drone would then only come back in on a beat.
 *
 * And perhaps most importantly, it can be used to do almost random output and
 * make weird noises.
 */


/* TODO
 * - ext time in
 * - ext time out
 *   - is there a 'standard' for communicating lengths of time (like delay time)?
 * - idle start trigger
 * - idle end trigger
 * - switch for output to be high for idle or low for idle
 * - time display widget for timeout length
 * - Fine/Course params fors for timeout
 * - idle timeout countdown display for remaining time before timeout
 *   - gui 'progress' widget?
*/

/* FIXME
 * Heartbeat -> reset idle timeout? countdown?
 * timeout -> time before idle?
 * switch output -> idling? idle mode on ? sleeping? waiting?
*/

struct IdleSwitch : Module {
    enum ParamIds {
        TIME_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        INPUT_SOURCE_INPUT,
        HEARTBEAT_INPUT,
        TIME_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        IDLE_GATE_OUTPUT,
        TIME_OUTPUT,
        IDLE_START_OUTPUT,
        IDLE_END_OUTPUT,
        FRAME_COUNT_OUTPUT,
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

    float idleGateOutput;
    float idleGateLightBrightness;
    float deltaTime;
    float previousHeartbeatTime;;

    IdleSwitch() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    // For more advanced Module features, read Rack's engine.hpp header file
    // - toJson, fromJson: serialization of internal data
    // - onSampleRateChange: event triggered by a change of sample rate
    // - reset, randomize: implements special behavior when user clicks these from the context menu
};


void IdleSwitch::step() {

    // TIME_PARAM is time between idle loop checks (the timeout) in seconds
    // theTime = params[TIME_PARAM].value;
    // mostly for debugging atm
    // outputs[TIME_OUTPUT].value = params[TIME_PARAM].value;

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
        if (inputs[TIME_INPUT].active) {
            // deltaTime = inputs[TIME_INPUT].value;
	        //deltaTime = 1e-3 * powf(10.0 / 1e-3, clampf(inputs[TIME_INPUT].value / 10.0, 0.0, 1.0));
	        // deltaTime = 1e-3 * powf(1.0 / 1e-3, clampf(inputs[TIME_INPUT].value, 0.0, 10.0));
            // Not sure what should happen in the min delay time > 0 but less than the
	        deltaTime = clampf(inputs[TIME_INPUT].value, 0.001, 10.0);
        }
        else {
            deltaTime = params[TIME_PARAM].value;
        }
        maxFrameCount = (int)ceilf(deltaTime * sampleRate);
    }

    if (frameCount <= maxFrameCount) {
        idleGateOutput  = 0.0;
        idleGateLightBrightness = 0.0;
    }
    else {
        idleGateOutput = 10.0;
        idleGateLightBrightness = 1.0;
    }

    outputs[IDLE_GATE_OUTPUT].value = idleGateOutput;
    lights[IDLE_GATE_LIGHT].setBrightness(idleGateLightBrightness);

    outputs[TIME_OUTPUT].value = deltaTime;
    // frameCount++;
}

IdleSwitchWidget::IdleSwitchWidget() {
    IdleSwitch *module = new IdleSwitch();
    setModule(module);
    setPanel(SVG::load(assetPlugin(plugin, "res/IdleSwitch.svg")));

    addChild(createScrew<ScrewSilver>(Vec(15, 0)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 0)));
    addChild(createScrew<ScrewSilver>(Vec(15, 365)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 365)));

    addParam(createParam<Davies1900hBlackKnob>(Vec(38.86, 141.482), module, IdleSwitch::TIME_PARAM, 0.0, 10.0, 0.25));
    addInput(createInput<PJ301MPort>(Vec(10, 141.482), module, IdleSwitch::TIME_INPUT));
    addOutput(createOutput<PJ301MPort>(Vec(75, 141.482), module, IdleSwitch::TIME_OUTPUT));

    addInput(createInput<PJ301MPort>(Vec(42.378, 37.242), module, IdleSwitch::INPUT_SOURCE_INPUT));
    addInput(createInput<PJ301MPort>(Vec(43.304, 91.441), module, IdleSwitch::HEARTBEAT_INPUT));

    addOutput(createOutput<PJ301MPort>(Vec(42.25, 190.0), module, IdleSwitch::IDLE_GATE_OUTPUT));

    addChild(createLight<LargeLight<RedLight>>(Vec(48, 220.0), module, IdleSwitch::IDLE_GATE_LIGHT));
}

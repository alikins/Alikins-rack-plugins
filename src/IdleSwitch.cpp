#include <stdio.h>
#include <sstream>
#include <iomanip>

#include "alikins.hpp"
// #include "util.hpp"


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
 * The timeout period is set by the value
 * of the 'Time before idle' param.
 *
 * If there is a 'Reset idle' source, when it gets an event, the timeout period
 * is reset. After a reset event, the Idle Gate Output will remain on until
 * an input event is seen at Input Source. When there is an input event, the Idle
 * Gate Output is turned off until the expiration of the 'Time before idle' or
 * the next 'Reset idle'.
 *
 * To use the eventloop/gui main loop analogy, a 'Reset idle' event is equilivent to
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
 * The 'Reset idle' input allows this be kind of synced to a clock, beat, or sequence.
 * In the dronevexample above, the drone would then only come back in on a beat.
 *
 * And perhaps most importantly, it can be used to do almost random output and
 * make weird noises.
 */


/* TODO
 *   - is there a 'standard' for communicating lengths of time (like delay time)?
 * - idle start trigger
 * - idle end trigger
 * - switch for output to be high for idle or low for idle
 * - time display widget for timeout length
 * - Fine/Course params fors for timeout
 * - idle timeout countdown display for remaining time before timeout
 *   - gui 'progress' widget?
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
        PULSE_INPUT,
        SWITCHED_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        IDLE_GATE_OUTPUT,
        TIME_OUTPUT,
        IDLE_START_OUTPUT,
        IDLE_END_OUTPUT,
        FRAME_COUNT_OUTPUT,
        ON_WHEN_IDLE_OUTPUT,
        OFF_WHEN_IDLE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float idleTimeoutMS = 140.0f;
    float idleTimeLeftMS = 0.0f;

    dsp::SchmittTrigger inputTrigger;

    // FIXME: these names are confusing
    dsp::SchmittTrigger heartbeatTrigger;

    // clock mode stuff
    dsp::SchmittTrigger pulseTrigger;
    int pulseFrame = 0;
    bool waiting_for_pulse = false;
    bool pulse_mode = false;

    dsp::PulseGenerator idleStartPulse;
    dsp::PulseGenerator idleEndPulse;

    // FIXME: not really counts
    int frameCount = 0;
    int maxFrameCount = 0;

    float idleGateOutput = 0.0;

    float deltaTime = 0.0f;

    bool is_idle = false;

//    IdleSwitch() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    IdleSwitch() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(TIME_PARAM, 0.f, 10.f, 0.25f, "Time before idle", " ms", 0.0f, 1000.0f);
    }
    // void step() override;
	void process(const ProcessArgs &args) override;
};


void IdleSwitch::process(const ProcessArgs &args) {
    bool pulse_seen = false;
    bool time_exceeded = false;
    // pulse_mode = inputs[PULSE_INPUT].active;
    pulse_mode = inputs[PULSE_INPUT].isConnected();

    // float sampleRate = engineGetSampleRate();
    float sampleRate = args.sampleRate;
    // Compute the length of our idle time based on the knob + time cv
    // -or-
    // base it one the time since the last clock pulse
    if (pulse_mode) {
        if (inputTrigger.process(inputs[PULSE_INPUT].getVoltage())) {
            // keep track of which frame we got a pulse
            // FIXME: without a max time, frameCount can wrap?
            // update pulseFrame to point to current frame count
            pulseFrame = frameCount;

            waiting_for_pulse = true;
            pulse_seen = true;

        }

        deltaTime = fmax(frameCount - pulseFrame, 0) / sampleRate;
       // if we are waiting, maxframeCount is the time since last pulse and increasing
        maxFrameCount = frameCount;

    } else {
        deltaTime = params[TIME_PARAM].getValue();
        if (inputs[TIME_INPUT].isConnected()) {
            deltaTime += clamp(inputs[TIME_INPUT].getVoltage(), 0.0f, 10.0f);
        }

        // TODO: refactor into submethods if not subclass
        maxFrameCount = (int)ceilf(deltaTime * sampleRate);
    }

    idleTimeoutMS = std::round(deltaTime*1000);

    // debug("is_idle: %d pulse_mode: %d pulse_frame: %d frameCount: %d maxFrameCount: %d ", is_idle, pulse_mode, pulseFrame, frameCount, maxFrameCount);
    // debug("is_idle: %d pulse_mode: %d w_f_pulse: %d pulse_seen: %d pulseFrame: %d frameCount: %d deltaTime: %f",
    //        is_idle, pulse_mode, waiting_for_pulse, pulse_seen, pulseFrame, frameCount, deltaTime);

    if (inputs[HEARTBEAT_INPUT].isConnected() &&
          heartbeatTrigger.process(inputs[HEARTBEAT_INPUT].getVoltage())) {
            frameCount = 0;
    }

    // time_left_s is always 0 for pulse mode until we predict the future
    float frames_left = fmax(maxFrameCount - frameCount, 0);
    float time_left_s = frames_left / sampleRate;

    // TODO: simplify the start/end/gate on logic... really only a few states to check

    // the start of idle  (not idle -> idle trans)
    if ((frameCount > maxFrameCount) || (waiting_for_pulse && pulse_seen)) {
        time_exceeded = true;
        if (!is_idle) {
            idleStartPulse.trigger(0.01);
        }

    }

    // stay idle once we start until there is an input event
    is_idle = (is_idle || time_exceeded);

    if (is_idle) {
        idleGateOutput = 10.0;
        outputs[ON_WHEN_IDLE_OUTPUT].setVoltage(inputs[SWITCHED_INPUT].getVoltage());
        outputs[OFF_WHEN_IDLE_OUTPUT].setVoltage(0.0f);

    } else {
        idleGateOutput = 0.0;
        outputs[ON_WHEN_IDLE_OUTPUT].setVoltage(0.0f);
        outputs[OFF_WHEN_IDLE_OUTPUT].setVoltage(inputs[SWITCHED_INPUT].getVoltage());

        is_idle = false;

        // if we arent idle yet, the idleTimeLeft is changing and we need to update time remaining display
        // update idletimeLeftMS which drives the digit display widget
        idleTimeLeftMS = time_left_s*1000;
    }

    frameCount++;

    if (inputs[INPUT_SOURCE_INPUT].isConnected() &&
            inputTrigger.process(inputs[INPUT_SOURCE_INPUT].getVoltage())) {

        // only end idle if we are already idle (idle->not idle transition)
        if (is_idle) {
            idleEndPulse.trigger(0.01);
        }

        is_idle = false;

        waiting_for_pulse = false;
        frameCount = 0;
        pulseFrame = 0;
    }

    // once clock input works, could add an output to indicate how long between clock
    // If in pulse mode, deltaTime can be larger than 10s internal, but the max output
    // to "Time output" is 10V. ie, after 10s the "Time Output" stops increasing.
    outputs[TIME_OUTPUT].setVoltage(clamp(deltaTime, 0.0f, 10.0f));
    outputs[IDLE_GATE_OUTPUT].setVoltage(idleGateOutput);

    outputs[IDLE_START_OUTPUT].setVoltage(idleStartPulse.process(1.0/args.sampleRate) ? 10.0 : 0.0);
    outputs[IDLE_END_OUTPUT].setVoltage(idleEndPulse.process(1.0/args.sampleRate) ? 10.0 : 0.0);

}


//  From AS DelayPlus.cpp https://github.com/AScustomWorks/AS
struct IdleSwitchMsDisplayWidget : TransparentWidget {

  float *value = NULL;

  IdleSwitchMsDisplayWidget() {
  }

  void draw(NVGcontext *vg) override {
    std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/Segment7Standard.ttf"));

    if (!value) {
        return;
    }

    // text
    if (font) {
        nvgFontSize(vg, 18);
        nvgFontFaceId(vg, font->handle);
        nvgTextLetterSpacing(vg, 2.5);

        std::string to_display = "0.00";

        if (value) {
            to_display = string::f("%-4.f", *value);
        }

        Vec textPos = Vec(0.5f, 19.0f);

        NVGcolor textColor = nvgRGB(0x65, 0xf6, 0x78);
        nvgFillColor(vg, textColor);
        nvgText(vg, textPos.x, textPos.y, to_display.c_str(), NULL);
    }
  }
};

struct IdleSwitchWidget : ModuleWidget {
    IdleSwitchWidget(IdleSwitch *module) {
        setModule(module);

        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/IdleSwitch.svg")));

        float x_baseline = 10.0f;
        float y_baseline = 40.0f;
        // DEBUG("y_baseline3: %f", y_baseline);

        addChild(createWidget<ScrewSilver>(Vec(0.0f, 0.0f)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - RACK_GRID_WIDTH, 0.0f)));
        addChild(createWidget<ScrewSilver>(Vec(0.0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        addInput(createInput<PJ301MPort>(Vec(x_baseline, y_baseline),
            module, IdleSwitch::INPUT_SOURCE_INPUT));

        x_baseline += 37.5f;

        addInput(createInput<PJ301MPort>(Vec(x_baseline, y_baseline),
            module, IdleSwitch::HEARTBEAT_INPUT));

        x_baseline += 37.5f;

        addInput(createInput<PJ301MPort>(Vec(x_baseline, y_baseline),
            module, IdleSwitch::PULSE_INPUT));

        y_baseline  += 80.0f;
        // DEBUG("y_baseline5: %f", y_baseline);

        // idle time display
        // FIXME: handle large IdleTimeoutMs (> 99999ms) better
        IdleSwitchMsDisplayWidget *idle_time_display = new IdleSwitchMsDisplayWidget();
        idle_time_display->box.pos = Vec(20, y_baseline);
        idle_time_display->box.size = Vec(70, 24);

        if (module) {
            idle_time_display->value = &module->idleTimeoutMS;
        }
        addChild(idle_time_display);

        y_baseline += 30.0f;
        // DEBUG("y_baseline7: %f", y_baseline);

        addParam(createParam<Davies1900hBlackKnob>(Vec(38.86, y_baseline),
            module, IdleSwitch::TIME_PARAM));

        y_baseline += 5.0f;
        // DEBUG("y_baseline8: %f", y_baseline);

        addInput(createInput<PJ301MPort>(Vec(10, y_baseline),
            module, IdleSwitch::TIME_INPUT));

        addOutput(createOutput<PJ301MPort>(Vec(80, y_baseline),
            module, IdleSwitch::TIME_OUTPUT));

        y_baseline += 68.0f;
        // DEBUG("y_baseline9: %f", y_baseline);

        IdleSwitchMsDisplayWidget *time_remaining_display = new IdleSwitchMsDisplayWidget();
        time_remaining_display->box.pos = Vec(20, y_baseline);
        time_remaining_display->box.size = Vec(70, 24);

        if (module) {
            time_remaining_display->value = &module->idleTimeLeftMS;
        }
        addChild(time_remaining_display);

        y_baseline += 40.0;
        // DEBUG("y_baseline10: %f", y_baseline);

        addOutput(createOutput<PJ301MPort>(Vec(10, y_baseline),
            module, IdleSwitch::IDLE_START_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(47.5, y_baseline),
            module, IdleSwitch::IDLE_GATE_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(85, y_baseline),
            module, IdleSwitch::IDLE_END_OUTPUT));

        y_baseline += 52.0f;

        addInput(createInput<PJ301MPort>(Vec(10.0f, y_baseline),
            module, IdleSwitch::SWITCHED_INPUT));
        addOutput(createOutput<PJ301MPort>(Vec(47.5f, y_baseline),
            module, IdleSwitch::ON_WHEN_IDLE_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(85.0f, y_baseline),
            module, IdleSwitch::OFF_WHEN_IDLE_OUTPUT));

    }
};


Model *modelIdleSwitch = createModel<IdleSwitch, IdleSwitchWidget>("IdleSwitch");

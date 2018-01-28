#include <stdio.h>
#include <sstream>
#include <iomanip>

#include "alikins.hpp"
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

    int idleTimeoutMS = 140;
    int idleTimeLeftMS = 0;

    SchmittTrigger inputTrigger;

    // FIXME: these names are confusing
    SchmittTrigger heartbeatTrigger;

    // clock mode stuff
    SchmittTrigger pulseTrigger;
    int pulseFrame = 0;
    bool waiting_for_pulse = false;
    bool pulse_mode = false;

    // FIXME: not really counts
    int frameCount = 0;
    int maxFrameCount = 0;

    float idleGateOutput = 0.0;
    float idleGateLightBrightness = 0.0;

    float deltaTime = 0;

    bool is_idle = false;

    IdleSwitch() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};


void IdleSwitch::step() {
    bool pulse_seen = false;
    pulse_mode = inputs[PULSE_INPUT].active;

    float sampleRate = engineGetSampleRate();

    // Compute the length of our idle time based on the knob + time cv
    // -or-
    // base it one the time since the last clock pulse
    if (pulse_mode) {
        if (inputTrigger.process(inputs[PULSE_INPUT].value)) {
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
        deltaTime = params[TIME_PARAM].value;
        if (inputs[TIME_INPUT].active) {
            deltaTime += clampf(inputs[TIME_INPUT].value, 0.0, 10.0);
        }

        // TODO: refactor into submethods if not subclass
        maxFrameCount = (int)ceilf(deltaTime * sampleRate);
    }

    idleTimeoutMS = std::round(deltaTime*1000);

    // debug("is_idle: %d pulse_mode: %d pulse_frame: %d frameCount: %d maxFrameCount: %d ", is_idle, pulse_mode, pulseFrame, frameCount, maxFrameCount);
    // debug("is_idle: %d pulse_mode: %d w_f_pulse: %d pulse_seen: %d pulseFrame: %d frameCount: %d deltaTime: %f",
    //        is_idle, pulse_mode, waiting_for_pulse, pulse_seen, pulseFrame, frameCount, deltaTime);

    if (inputs[HEARTBEAT_INPUT].active &&
          heartbeatTrigger.process(inputs[HEARTBEAT_INPUT].value)) {
            frameCount = 0;
    }

    // time_left_s is always 0 for pulse mode until we predict the future
    float frames_left = fmax(maxFrameCount - frameCount, 0);
    float time_left_s = frames_left / sampleRate;

    is_idle = (is_idle || (frameCount > maxFrameCount) || (waiting_for_pulse && pulse_seen));

    if (is_idle) {
        idleGateOutput = 10.0;
        idleGateLightBrightness = 1.0;

    } else {
        idleGateOutput = 0.0;
        idleGateLightBrightness = 0.0;

        is_idle = false;

        // if we arent idle yet, the idleTimeLeft is changing and we need to update time remaining display
        // update idletimeLeftMS which drives the digit display widget
        idleTimeLeftMS = time_left_s*1000;
    }

    frameCount++;

    if (inputs[INPUT_SOURCE_INPUT].active &&
            inputTrigger.process(inputs[INPUT_SOURCE_INPUT].value)) {
        is_idle = false;

        waiting_for_pulse = false;
        frameCount = 0;
        pulseFrame = 0;
    }

    // once clock input works, could add an output to indicate how long between clock
    // If in pulse mode, deltaTime can be larger than 10s internal, but the max output
    // to "Time output" is 10V. ie, after 10s the "Time Output" stops increasing.
    outputs[TIME_OUTPUT].value = clampf(deltaTime, 0.0, 10.0);
    outputs[IDLE_GATE_OUTPUT].value = idleGateOutput;
    lights[IDLE_GATE_LIGHT].setBrightness(idleGateLightBrightness);
}


//  From AS DelayPlus.cpp https://github.com/AScustomWorks/AS
struct MsDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  MsDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  }

  void draw(NVGcontext *vg) override {
    // Background
    // these go to...
    NVGcolor backgroundColor = nvgRGB(0x11, 0x11, 0x11);

    NVGcolor borderColor = nvgRGB(0xff, 0xff, 0xff);

    nvgBeginPath(vg);

    nvgRoundedRect(vg, 0.0, 0.0, box.size.x, box.size.y, 5.0);
    nvgFillColor(vg, backgroundColor);
    nvgFill(vg);

    nvgStrokeWidth(vg, 3.0);
    nvgStrokeColor(vg, borderColor);

    nvgStroke(vg);

    // text
    nvgFontSize(vg, 18);
    nvgFontFaceId(vg, font->handle);
    nvgTextLetterSpacing(vg, 2.5);

    std::stringstream to_display;
    to_display << std::right  << std::setw(5) << *value;

    Vec textPos = Vec(0.5f, 19.0f);

    NVGcolor textColor = nvgRGB(0x65, 0xf6, 0x78);
    nvgFillColor(vg, textColor);
    nvgText(vg, textPos.x, textPos.y, to_display.str().c_str(), NULL);
  }
};


IdleSwitchWidget::IdleSwitchWidget() {
    IdleSwitch *module = new IdleSwitch();
    setModule(module);
    setPanel(SVG::load(assetPlugin(plugin, "res/IdleSwitch.svg")));

    addChild(createScrew<ScrewSilver>(Vec(5, 0)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 365)));

    addInput(createInput<PJ301MPort>(Vec(37, 30.0), module, IdleSwitch::INPUT_SOURCE_INPUT));
    addInput(createInput<PJ301MPort>(Vec(37, 70.0), module, IdleSwitch::HEARTBEAT_INPUT));
    addInput(createInput<PJ301MPort>(Vec(70, 70.0), module, IdleSwitch::PULSE_INPUT));

    // idle time display
    // FIXME: handle large IdleTimeoutMs (> 99999ms) better
    MsDisplayWidget *idle_time_display = new MsDisplayWidget();
    idle_time_display->box.pos = Vec(20, 130);
    idle_time_display->box.size = Vec(70, 24);
    idle_time_display->value = &module->idleTimeoutMS;
    addChild(idle_time_display);

    addInput(createInput<PJ301MPort>(Vec(10, 165.0), module, IdleSwitch::TIME_INPUT));
    addParam(createParam<Davies1900hBlackKnob>(Vec(38.86, 160.0), module, IdleSwitch::TIME_PARAM, 0.0, 10.0, 0.25));
    addOutput(createOutput<PJ301MPort>(Vec(80, 165.0), module, IdleSwitch::TIME_OUTPUT));

    MsDisplayWidget *time_remaining_display = new MsDisplayWidget();
    time_remaining_display->box.pos = Vec(20, 235);
    time_remaining_display->box.size = Vec(70, 24);
    time_remaining_display->value = &module->idleTimeLeftMS;
    addChild(time_remaining_display);

    addOutput(createOutput<PJ301MPort>(Vec(37, 280.0), module, IdleSwitch::IDLE_GATE_OUTPUT));

    addChild(createLight<LargeLight<RedLight>>(Vec(41, 310.0), module, IdleSwitch::IDLE_GATE_LIGHT));
}

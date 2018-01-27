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
    SchmittTrigger heartbeatTrigger;

    int frameCount = 0;
    int maxFrameCount = 0;

    float idleGateOutput;
    float idleGateLightBrightness;

    float deltaTime;

    bool is_idle = false;

    IdleSwitch() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;
};


void IdleSwitch::step() {

    // Compute the length of our idle time based on the knob + time cv
    deltaTime = params[TIME_PARAM].value;
    if (inputs[TIME_INPUT].active) {
        deltaTime += clampf(inputs[TIME_INPUT].value, 0.0, 10.0);
    }

    // the idle time depends on time cv and knob, so always needs to be updated
    // event without an active
    float sampleRate = engineGetSampleRate();
    maxFrameCount = (int)ceilf(deltaTime * sampleRate);

    // info("is_idle: %d frameCount: %d maxFrameCount: %d ",is_idle, frameCount, maxFrameCount);
    float delay = deltaTime;
    idleTimeoutMS = std::round(delay*1000);

    if (inputs[HEARTBEAT_INPUT].active &&
          heartbeatTrigger.process(inputs[HEARTBEAT_INPUT].value)) {
            frameCount = 0;
    }

    float frames_left = fmax(maxFrameCount - frameCount, 0);
    float time_left_s = frames_left / sampleRate;

    if (frameCount > maxFrameCount || is_idle) {
        idleGateOutput = 10.0;
        idleGateLightBrightness = 1.0;
        is_idle = true;
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
            frameCount = 0;
    }

    outputs[TIME_OUTPUT].value = deltaTime;
    outputs[IDLE_GATE_OUTPUT].value = idleGateOutput;
    lights[IDLE_GATE_LIGHT].setBrightness(idleGateLightBrightness);
}


//  From AS DelayPlus.cpp https://github.com/AScustomWorks/AS
struct MsDisplayWidget : TransparentWidget {

  int *value;
  std::shared_ptr<Font> font;

  MsDisplayWidget() {
    font = Font::load(assetPlugin(plugin, "res/Segment7Standard.ttf"));
  };

  // this seems like a lot to do every ms or more, but presumably
  // draw is only called at framerate or lower. Not sure where/how
  // this gets buffered (FrameBufferWidget.step()?)
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
    // addChild(createScrew<ScrewSilver>(Vec(box.size.x - 30, 0)));
    // addChild(createScrew<ScrewSilver>(Vec(15, 365)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 365)));

    addInput(createInput<PJ301MPort>(Vec(43, 32.0), module, IdleSwitch::INPUT_SOURCE_INPUT));
    addInput(createInput<PJ301MPort>(Vec(43, 70.0), module, IdleSwitch::HEARTBEAT_INPUT));

    //  DISPLAY
    MsDisplayWidget *idle_time_display = new MsDisplayWidget();
    idle_time_display->box.pos = Vec(20, 135);
    idle_time_display->box.size = Vec(70, 24);
    idle_time_display->value = &module->idleTimeoutMS;
    addChild(idle_time_display);

    addInput(createInput<PJ301MPort>(Vec(10, 165.0), module, IdleSwitch::TIME_INPUT));
    addParam(createParam<Davies1900hBlackKnob>(Vec(38.86, 160.0), module, IdleSwitch::TIME_PARAM, 0.0, 10.0, 0.25));
    addOutput(createOutput<PJ301MPort>(Vec(80, 165.0), module, IdleSwitch::TIME_OUTPUT));

    MsDisplayWidget *time_left_display = new MsDisplayWidget();
    time_left_display->box.pos = Vec(20, 240);
    time_left_display->box.size = Vec(70, 24);
    time_left_display->value = &module->idleTimeLeftMS;
    addChild(time_left_display);

    addOutput(createOutput<PJ301MPort>(Vec(42.25, 280.0), module, IdleSwitch::IDLE_GATE_OUTPUT));

    addChild(createLight<LargeLight<RedLight>>(Vec(48, 310.0), module, IdleSwitch::IDLE_GATE_LIGHT));
}

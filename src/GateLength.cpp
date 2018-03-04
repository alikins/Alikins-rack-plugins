#include "dsp/digital.hpp"

#include "alikins.hpp"
#include "MsDisplayWidget.hpp"


struct GateLength : Module {
    enum ParamIds {
        GATE_LENGTH_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        TRIGGER_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        GATE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float gate_length = 2.34f;

    SchmittTrigger inputOnTrigger;
    SchmittTrigger inputOffTrigger;

    PulseGenerator gateGenerator;

    GateLength() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    
    void step() override;

    void onReset() override {
    }

};

void GateLength::step() {
    // FIXME: add way to support >10.0s gate length
    float sample_time = engineGetSampleTime();
    gateGenerator.pulseTime = params[GATE_LENGTH_PARAM].value;
    if (inputOnTrigger.process(inputs[TRIGGER_INPUT].value)) {
        debug("GL INPUT ON TRIGGER");
        gateGenerator.trigger(sample_time);
    }

    //if (muteOffTrigger.process(!params[BIG_MUTE_BUTTON_PARAM].value)) {
        // debug("MUTE OFF");
    //}

    outputs[GATE_OUTPUT].value = gateGenerator.process(sample_time) ? 10.0f : 0.0f;
    // process(engineGetSampleTime()) ? 5.0 : 0.0;
    // outputs[LEFT_OUTPUT].value = inputs[LEFT_INPUT].value * gmult2;
    // outputs[RIGHT_OUTPUT].value = inputs[RIGHT_INPUT].value * gmult2;

    // debug("state: %d, gmult2: %f", state, gmult2);

    // TODO: to eliminate worse case DC thump, also apply a RC filter of some sort?
}

struct GateLengthWidget : ModuleWidget {
    GateLengthWidget(GateLength *module);
};


GateLengthWidget::GateLengthWidget(GateLength *module) : ModuleWidget(module) {

    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/GateLength.svg")));

    addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(38.86, 150.0),
                module,
                GateLength::GATE_LENGTH_PARAM,
                0.0f, 10.0f, 0.1f));

    addInput(Port::create<PJ301MPort>(Vec(4.0f, 302.0f),
                Port::INPUT,
                module,
                GateLength::TRIGGER_INPUT));

    addOutput(Port::create<PJ301MPort>(Vec(60.0f, 302.0f),
                Port::OUTPUT,
                module,
                GateLength::GATE_OUTPUT));

    MsDisplayWidget *gate_length_display = new MsDisplayWidget();
    gate_length_display->box.pos = Vec(20, 115);
    gate_length_display->box.size = Vec(70, 24);
    gate_length_display->value = &module->params[GateLength::GATE_LENGTH_PARAM].value;
    addChild(gate_length_display);
    // addChild(Widget::create<ScrewSilver>(Vec(0.0, 0)));
    // addChild(Widget::create<ScrewSilver>(Vec(box.size.x-15, 0)));
    // addChild(Widget::create<ScrewSilver>(Vec(30, 365)));

}

Model *modelGateLength = Model::create<GateLength, GateLengthWidget>(
        "Alikins", "GateLength", "Gate Length", UTILITY_TAG);

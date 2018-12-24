#include "dsp/digital.hpp"

#include "alikins.hpp"
#include "cv_utils.hpp"
#include "MsDisplayWidget.hpp"


struct GateLength : Module {
    enum ParamIds {
        GATE_LENGTH_PARAM1,
        GATE_LENGTH_PARAM2,
        GATE_LENGTH_PARAM3,
        GATE_LENGTH_PARAM4,
        GATE_LENGTH_PARAM5,
        BPM_PARAM1,
        BPM_PARAM2,
        BPM_PARAM3,
        BPM_PARAM4,
        BPM_PARAM5,
        BEAT_LENGTH_MULTIPLIER_PARAM1,
        BEAT_LENGTH_MULTIPLIER_PARAM2,
        BEAT_LENGTH_MULTIPLIER_PARAM3,
        BEAT_LENGTH_MULTIPLIER_PARAM4,
        BEAT_LENGTH_MULTIPLIER_PARAM5,
        // TODO: length percent/multiplier params (1.0 for quarter note, 2.0 for whole, .5 for eigth, etc)
        NUM_PARAMS
    };
    enum InputIds {
        TRIGGER_INPUT1,
        TRIGGER_INPUT2,
        TRIGGER_INPUT3,
        TRIGGER_INPUT4,
        TRIGGER_INPUT5,
        GATE_LENGTH_INPUT1,
        GATE_LENGTH_INPUT2,
        GATE_LENGTH_INPUT3,
        GATE_LENGTH_INPUT4,
        GATE_LENGTH_INPUT5,
        BPM_INPUT1,
        BPM_INPUT2,
        BPM_INPUT3,
        BPM_INPUT4,
        BPM_INPUT5,
        // TODO: length percent/multiplier inputs (1.0 for quarter note, 2.0 for whole, .5 for eigth, etc)
        BEAT_LENGTH_MULTIPLIER_INPUT1,
        BEAT_LENGTH_MULTIPLIER_INPUT2,
        BEAT_LENGTH_MULTIPLIER_INPUT3,
        BEAT_LENGTH_MULTIPLIER_INPUT4,
        BEAT_LENGTH_MULTIPLIER_INPUT5,
        NUM_INPUTS
    };
    enum OutputIds {
        GATE_OUTPUT1,
        GATE_OUTPUT2,
        GATE_OUTPUT3,
        GATE_OUTPUT4,
        GATE_OUTPUT5,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float bpms[GATE_LENGTH_INPUTS];
    float bpm_labels[GATE_LENGTH_INPUTS];
    float gate_length[GATE_LENGTH_INPUTS];
    float beat_length[GATE_LENGTH_INPUTS];
    // float beat_length[GATE_LENGTH_INPUTS];

    SchmittTrigger inputOnTrigger[GATE_LENGTH_INPUTS];

    PulseGenerator gateGenerator[GATE_LENGTH_INPUTS];

    GateLength() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    void onReset() override {
    }

};

void GateLength::step() {
    // FIXME: add way to support >10.0s gate length

    float sample_time = engineGetSampleTime();

    for (int i = 0; i < GATE_LENGTH_INPUTS; i++) {
        gate_length[i] = clamp(params[GATE_LENGTH_PARAM1 + i].value + inputs[GATE_LENGTH_INPUT1 + i].value, 0.0f, 10.0f);

        // TODO/FIXME: just add param and input for now, but likely better to have an attenuator as well
        //             (ie, like fundamental LFO's FM1 input + FM1 mix param + LFO freq param)
        bpms[i] = clamp(params[BPM_PARAM1 + i].value + inputs[BPM_INPUT1 + i].value, -10.0f, 10.0f);
        bpm_labels[i] = lfo_cv_to_bpm(bpms[i]);

        if (inputs[BEAT_LENGTH_MULTIPLIER_INPUT1 + i].active) {
            beat_length[i] = clamp(inputs[BEAT_LENGTH_MULTIPLIER_INPUT1 + i].value, -10.0f, 10.0f);

            float beats_per_sec = lfo_cv_to_freq(bpms[i]);
            float quarter_note_beat_length_secs = 1.0f / beats_per_sec;
            gate_length[i] = quarter_note_beat_length_secs * beat_length[i];
            /*
            if (i == 2) {
                debug("gate_length: %f sec, beat length: %f secs, lfo_freq: %f", gate_length[i], quarter_note_beat_length_secs, lfo_cv_to_freq(bpms[i]) );
            }
            */
            // float gate_length_s =
        }

        // BEAT_LENGTH_MULTIPLIER_INPUT1

        if (inputOnTrigger[i].process(inputs[TRIGGER_INPUT1 + i].value)) {
            // debug("GL INPUT ON TRIGGER %d gate_length: %f", i, gate_length[i]);
            gateGenerator[i].trigger(gate_length[i]);
        }

        outputs[GATE_OUTPUT1 + i].value = gateGenerator[i].process(sample_time) ? 10.0f : 0.0f;
    }
}

struct SmallPurplePort : SVGPort {
	SmallPurplePort() {
		setSVG(SVG::load(assetPlugin(plugin, "res/PurplePort.svg")));
	}
};

struct GateLengthWidget : ModuleWidget {
    GateLengthWidget(GateLength *module);
};

GateLengthWidget::GateLengthWidget(GateLength *module) : ModuleWidget(module) {

    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/GateLength.svg")));

    float y_pos = 32.0f;

    for (int i = 0; i < GATE_LENGTH_INPUTS; i++) {
        float x_pos = 4.0f;
        // y_pos += 39.0f;
        //y_pos += 32.0f;

        addInput(Port::create<SmallPurplePort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLength::TRIGGER_INPUT1 + i));

        // x_pos += 30.0f;
        x_pos += 19.0f;

        MsDisplayWidget *gate_length_display = new MsDisplayWidget();
        gate_length_display->box.pos = Vec(x_pos, y_pos);
        gate_length_display->box.size = Vec(60.0f, 18.0f);
        gate_length_display->value = &module->gate_length[i];
        addChild(gate_length_display);

        addInput(Port::create<SmallPurplePort>(Vec(x_pos + 62.0f, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLength::BPM_INPUT1 + i));

        addInput(Port::create<SmallPurplePort>(Vec(x_pos + 80.0f, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLength::BEAT_LENGTH_MULTIPLIER_INPUT1 + i));

        // FIXME: use new sequential box hbox/vbox thing
        // x_pos += 84.0f;
        x_pos += 72.0f;
        x_pos += 4.0f;
        addOutput(Port::create<SmallPurplePort>(Vec(box.size.x - 21.0f, y_pos),
                                           Port::OUTPUT,
                                           module,
                                           GateLength::GATE_OUTPUT1 + i));

        x_pos = 4.0f;
        // y_pos += 26.0f;
        y_pos += 22.0f;

        addInput(Port::create<SmallPurplePort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLength::GATE_LENGTH_INPUT1 + i));

        // x_pos += 30.0f;
        x_pos += 19.0f;
        addParam(ParamWidget::create<Trimpot>(Vec(x_pos, y_pos),
                                              module,
                                              GateLength::GATE_LENGTH_PARAM1 + i,
                                              0.0f, 10.0f, 0.1f));

        x_pos += 19.0f;

        MsDisplayWidget *bpm_display = new MsDisplayWidget();
        bpm_display->box.pos = Vec(x_pos, y_pos);
        bpm_display->box.size = Vec(48.0f, 18.0f);
        bpm_display->precision = 2;
        // float *bpm_label = &module->bpm_labels[i];
        bpm_display->value = &module->bpm_labels[i];
        debug("i: %d bpm: %f", i, bpm_display->value);
        addChild(bpm_display);

        x_pos += 48.0f;

        MsDisplayWidget *beat_length_display = new MsDisplayWidget();
        beat_length_display->box.pos = Vec(x_pos, y_pos);
        beat_length_display->box.size = Vec(48.0f, 18.0f);
        beat_length_display->precision = 2;
        // float *bpm_label = &module->bpm_labels[i];
        beat_length_display->value = &module->beat_length[i];
        debug("i: %d beat_length: %f", i, beat_length_display->value);
        addChild(beat_length_display);

        y_pos += 44.0f;
    }

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

}

Model *modelGateLength = Model::create<GateLength, GateLengthWidget>(
        "Alikins", "GateLength", "Gate Length", UTILITY_TAG);

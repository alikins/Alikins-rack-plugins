#include "dsp/digital.hpp"

#include "alikins.hpp"
#include "cv_utils.hpp"
#include "MsDisplayWidget.hpp"

#define TURBO_COUNT 4

/* Notes:
   dotted note = 1.5x orig value
   double dotted = 1.75x orig value
   triple dottect = 1.875x orig value

   triplet quarter = 2/3 quarter note (.6666x orig value)
*/
struct GateLengthTurbo : Module {
    enum ParamIds {
        ENUMS(GATE_LENGTH_PARAM, TURBO_COUNT),
        ENUMS(BPM_PARAM, TURBO_COUNT),
        ENUMS(BEAT_LENGTH_MULTIPLIER_PARAM, TURBO_COUNT),

        // TODO: length percent/multiplier params (1.0 for quarter note, 2.0 for whole, .5 for eigth, etc)
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(TRIGGER_INPUT, TURBO_COUNT),
        ENUMS(GATE_LENGTH_INPUT, TURBO_COUNT),
        ENUMS(BPM_INPUT, TURBO_COUNT),
        ENUMS(BEAT_LENGTH_MULTIPLIER_INPUT, TURBO_COUNT),
        // TODO: length percent/multiplier inputs (1.0 for quarter note, 2.0 for whole, .5 for eigth, etc)
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(GATE_OUTPUT, TURBO_COUNT),
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float bpms[TURBO_COUNT];
    float bpm_labels[TURBO_COUNT];
    float gate_length[TURBO_COUNT];
    float beat_length[TURBO_COUNT];
    // float beat_length[TURBO_COUNT];

    SchmittTrigger inputOnTrigger[TURBO_COUNT];

    PulseGenerator gateGenerator[TURBO_COUNT];

    GateLengthTurbo() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    void onReset() override {
    }

};

void GateLengthTurbo::step() {
    // FIXME: add way to support >10.0s gate length

    float sample_time = engineGetSampleTime();

    for (int i = 0; i < TURBO_COUNT; i++) {
        gate_length[i] = clamp(params[GATE_LENGTH_PARAM + i].value + inputs[GATE_LENGTH_INPUT + i].value, 0.0f, 10.0f);

        // TODO/FIXME: just add param and input for now, but likely better to have an attenuator as well
        //             (ie, like fundamental LFO's FM1 input + FM1 mix param + LFO freq param)
        bpms[i] = clamp(params[BPM_PARAM + i].value + inputs[BPM_INPUT + i].value, -10.0f, 10.0f);
        bpm_labels[i] = lfo_cv_to_bpm(bpms[i]);

        if (inputs[BEAT_LENGTH_MULTIPLIER_INPUT + i].active) {
            beat_length[i] = clamp(inputs[BEAT_LENGTH_MULTIPLIER_INPUT + i].value, 0.0f, 10.0f);
            // beat_length[i] = rescale(clamp(inputs[BEAT_LENGTH_MULTIPLIER_INPUT + i].value, 0.0f, 10.0f),
            // 0.0f, 128.0f, 0.0f, 10.0f);
        } else {
            // if we increase the max of the param, need to rescale this back to the input range
            // rescale(param, 0.0f, 128.0f, 0.0f, 10.0f);
            // beat_length[i] = clamp(params[BEAT_LENGTH_MULTIPLIER_PARAM + i].value, 0.0f, 128.0f);
            float clamped_beat_length = clamp(params[BEAT_LENGTH_MULTIPLIER_PARAM + i].value, 0.0f, 160.0f);
            beat_length[i] = rescale(clamped_beat_length, 0.0f, 160.0f, 0.0f, 10.0f);
            // debug("beat_length[%d]: %f param: %f clamped: %f", i, beat_length[i], inputs[BEAT_LENGTH_MULTIPLIER_PARAM + i].value, clamped_beat_length);
        }

        float beats_per_sec = lfo_cv_to_freq(bpms[i]);
        float quarter_note_beat_length_secs = 1.0f / beats_per_sec;
        gate_length[i] = quarter_note_beat_length_secs * beat_length[i];

            /*
            if (i == 2) {
                debug("gate_length: %f sec, beat length: %f secs, lfo_freq: %f", gate_length[i], quarter_note_beat_length_secs, lfo_cv_to_freq(bpms[i]) );
            }
            */

            // float gate_length_s =

        // BEAT_LENGTH_MULTIPLIER_INPUT

        if (inputOnTrigger[i].process(inputs[TRIGGER_INPUT + i].value)) {
            // debug("GL INPUT ON TRIGGER %d gate_length: %f", i, gate_length[i]);
            gateGenerator[i].trigger(gate_length[i]);
        }

        outputs[GATE_OUTPUT + i].value = gateGenerator[i].process(sample_time) ? 10.0f : 0.0f;
    }
}

struct SmallPurplePort : SVGPort {
	SmallPurplePort() {
		setSVG(SVG::load(assetPlugin(plugin, "res/PurplePort.svg")));
	}
};

struct SmallPurpleTrimpot : Trimpot {
	SmallPurpleTrimpot() {
		// minAngle = -0.75 * M_PI;
		// maxAngle = 0.75 * M_PI;
		setSVG(SVG::load(assetPlugin(plugin, "res/SmallPurpleTrimpot.svg")));
	}
};


struct GateLengthTurboWidget : ModuleWidget {
    GateLengthTurboWidget(GateLengthTurbo *module);
};

GateLengthTurboWidget::GateLengthTurboWidget(GateLengthTurbo *module) : ModuleWidget(module) {

    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/GateLengthTurbo.svg")));

    float font_size = 12.0f;
    float y_pos = 32.0f;

    for (int i = 0; i < TURBO_COUNT; i++) {
        float x_pos = 4.0f;
        // y_pos += 39.0f;
        //y_pos += 32.0f;

        // float y_middle = 22.0f / 2.0f;
        // x_pos += 30.0f;
        // x_pos += 19.0f;    // size of input port
        addInput(Port::create<SmallPurplePort>(Vec(x_pos, y_pos + 1.0f),
                                          Port::INPUT,
                                          module,
                                          GateLengthTurbo::GATE_LENGTH_INPUT + i));

        x_pos += 19.0f;
        addParam(ParamWidget::create<SmallPurpleTrimpot>(Vec(x_pos, y_pos),
                                            module,
                                            GateLengthTurbo::GATE_LENGTH_PARAM + i,
                                            0.0f, 10.0f, 0.1f));

        x_pos += 19.0f;  // size of trimpot

        MsDisplayWidget *gate_length_display = new MsDisplayWidget();
        gate_length_display->box.pos = Vec(x_pos, y_pos + 1.0f);
        gate_length_display->box.size = Vec(60.0f, 15.0f);
        gate_length_display->value = &module->gate_length[i];
        gate_length_display->precision = 4;
        // gate_length_display->font_size = font_size;
        gate_length_display->font_size = 11.0f;
        gate_length_display->text_pos_x = 2.0f;
        gate_length_display->text_pos_y = 12.0f;
        gate_length_display->lcd_letter_spacing = 1.0f;

        addChild(gate_length_display);

        x_pos += 64.0f;
        addOutput(Port::create<SmallPurplePort>(Vec(x_pos, y_pos + 1.0f),
                                                Port::OUTPUT,
                                                module,
                                                GateLengthTurbo::GATE_OUTPUT + i));

        y_pos += 22.0f;   // next "line"
        x_pos = 4.0f;
        addInput(Port::create<SmallPurplePort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLengthTurbo::BPM_INPUT + i));

        x_pos += 19.0f;
        addParam(ParamWidget::create<SmallPurpleTrimpot>(Vec(x_pos, y_pos),
                                              module,
                                              GateLengthTurbo::BPM_PARAM + i,
                                              -5.0f, 5.0f, 0.0f));

        x_pos += 19.0f;   // size of trimpot

        MsDisplayWidget *bpm_display = new MsDisplayWidget();
        bpm_display->box.pos = Vec(x_pos, y_pos);
        bpm_display->box.size = Vec(48.0f, 18.0f);
        bpm_display->precision = 2;
        bpm_display->font_size = font_size;
        bpm_display->value = &module->bpm_labels[i];
        debug("i: %d bpm: %f", i, bpm_display->value);
        addChild(bpm_display);

        // x_pos += 84.0f;
        //  x_pos += 62.0f
        // x_pos += 48.0f;  // size of bpm display
        x_pos = 4.0f;
        y_pos += 22.0f;   // next "line"

        addInput(Port::create<SmallPurplePort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLengthTurbo::BEAT_LENGTH_MULTIPLIER_INPUT + i));

        x_pos += 19.0f;

        // TODO: if we assume the 'snapped' values are say, a 32nd up to a whole note, we could make
        //       the param value be the number of 32nd's. Not sure how to support dotted or tuplets
        //       for that case though, which would be nice to have.
        //
        // TODO: alternatively, maybe a custom Knob with it's own snap behavior?
        ParamWidget *beatLengthParam = ParamWidget::create<Trimpot>(Vec(x_pos, y_pos),
                                              module,
                                              GateLengthTurbo::BEAT_LENGTH_MULTIPLIER_PARAM + i,
                                              // 16.0f = 16x sixteenth note == one quarter note? ??
                                              0.0f, 160.0f, 16.0f);
        ((Trimpot *) beatLengthParam)->snap = true;
        addParam(beatLengthParam);
        // module->params[GateLengthTurbo::BEAT_LENGTH_MULTIPLIER_PARAM + i];
        // beatLengthParam)->snap = true;
        //addParam(beatLengthParam);

        // module->params[GateLengthTurbo::BEAT_LENGTH_MULTIPLIER_PARAM + i]->
        x_pos += 19.0f;  // size of beat length trimpot

        MsDisplayWidget *beat_length_display = new MsDisplayWidget();
        beat_length_display->box.pos = Vec(x_pos, y_pos);
        beat_length_display->box.size = Vec(48.0f, 18.0f);
        beat_length_display->precision = 2;
        beat_length_display->font_size = font_size;
        beat_length_display->value = &module->beat_length[i];
        debug("i: %d beat_length: %f", i, beat_length_display->value);
        addChild(beat_length_display);

        // y_pos += 26.0f;
        // x_pos += 30.0f;
        x_pos += 52.0f;

        addInput(Port::create<SmallPurplePort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLengthTurbo::TRIGGER_INPUT + i));

        //y_pos += 35.0f;
        y_pos += 30.0f;
    }

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

}

Model *modelGateLengthTurbo = Model::create<GateLengthTurbo, GateLengthTurboWidget>(
        "Alikins", "GateLengthTurbo", "Gate Length Turbo", UTILITY_TAG);

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

   # note length name in 'quarter notes' -> common name -> decimal mult of quarter note
   quarter note / 1  = quarter note ->    1.0
                                          0.9375    15 x (1.0 / 16)
              double dotted 8th note ->   0.875     14 x (1.0 / 16)
                                          0.8125    13 x (1.0 / 16)
                     dotted 8th note ->   0.75      12 x (1.0 / 16)
                                          0.6875    11 x (1.0 / 16)
                single quarter triplet -> 0.66
                                          0.625     10 x (1.0 / 16)
                                          0.5625    9 x (1.0 / 16)
   quarter note / 2 = 8th note ->         0.5       8 x (1.0 / 16)
               double dotted 16th note -> 0.4375    7 x (1.0 / 16)
               0.0416666625
                      dotted 16th note -> 0.375     6 x (1.0 / 16)
                   single 8th triplet  -> 0.33333
                                          0.3125    5 x ( 1.0 / 16)
   quarter note / 4 = 16th note ->        0.25      4 x (1.0 / 16)
               double dotted 32nd note -> 0.21875
                      dotted 32nd note -> 0.1875    3 x (1.0 / 16)
                   single 16th triplet -> 0.16666
   quarter note / 8 = 32nd note ->        0.125     2 x (1.0 / 16)
              double dotted 64th note ->  0.109375
                      dotted 64th note -> 0.09375                  3 x (1.0 / 32)
                   single 32nd triplet -> 0.0833333
   quarter note / 16  = 64th note ->      0.0625    1 x (1.0 / 16)
*/

// Map the nearest int of the note length param value to it's string name
// For ex, the param value range may be 0 -> 160.0f, so nearest int of param
// value 80.0f -> 80 should map to say
// std::map<int, std::string> note_length_param_map = gen_note_length_param_map();


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
            /*
            float bli = clamp(inputs[BEAT_LENGTH_MULTIPLIER_INPUT + i].value, 0.0f, 10.0f);
            float baseline_bl = 0.0f;
            float bl_ratio = 1.23456;
            //float baseline_bl = inputs[BPM_INPUT + i].value;
            // float bl_ratio = bli / baseline_bl;

            // frew_to_cv log2f(lfo_freq / LFO_BASELINE_FREQ * powf(2.0f, LFO_BASELINE_VOLTAGE));
            //  log2f(lfo_freq / 120.0f * powf(2.0f, 0.0f))
            // cv_to_freq LFO_BASELINE_FREQ / powf(2.0f, LFO_BASELINE_VOLTAGE) * powf(2.0f, volts);

            // float bl_ratio_freq = lfo_cv_to_freq(bli) / lfo_cv_to_freq(baseline_bl);
            float bl_ratio_freq = lfo_cv_to_freq(bli) / powf(2.0f, 0.0f) * powf(2.0f, bli);
            // float bl_ratio_freq  = lfo_cv_to_freq(bl_ratio);
            if ( i == 2 ) {
                debug("i: %d, bli: %f, baseline_bl: %f, bl_ratio: %f, bl_ratio_freq: %f", i, bli, baseline_bl, bl_ratio, bl_ratio_freq);
            }
            beat_length[i] = bl_ratio_freq;
            */

            // beat_length[i] = clamp(inputs[BEAT_LENGTH_MULTIPLIER_INPUT + i].value, 0.0f, 10.0f);
            beat_length[i] = rescale(clamp(inputs[BEAT_LENGTH_MULTIPLIER_INPUT + i].value, 0.0f, 10.0f),  0.0f, 10.0f, 0.0f, 10.0f);
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

struct NoteLengthChoiceMenuButton : LedDisplay {
    // std::string text = stringf("\xE2 \x99 \xA9 %f", 1.0f);
    // float noteLengths[3];
    float noteLength = 1.0f;
    float noteLengths[3] = { 1.0f, 0.5f, 0.3333f};
    std::string noteNames[3] = {"♩", "♪", "�"};

    NoteLengthChoiceMenuButton();
    void step() override;

};

struct SymbolicNoteLengthItem : MenuItem {
	// PatternWidget *widget = NULL;
	// int pattern;
    float noteLength = 1.0f;
    int index;
    NoteLengthChoiceMenuButton *noteLengthWidget;

	void onAction(EventAction &e) override {
        debug("note length item onAction %f index: %d noteLengthWidget->noteLength: %f ", noteLength, index, noteLengthWidget->noteLength);

        //noteLengthWidget->text = stringf("♩ %f", noteLength);
        // noteLengthWidget->text = stringf("\xE299A9 %f", 1.0f);
        /* ♩ */
        // noteLengthWidget->text = stringf(u8"\u2669 %f", noteLength);
        // noteLength =
        // noteLengthWidget->text = stringf("%0.4f", noteLength);
        // dynamic_cast<LedDisplayChoice *>(noteLengthWidget)->text = stringf("%s", noteLengthWidget->noteNames[index].c_str());
        noteLengthWidget->noteLength = noteLength;
                debug("AFTER note length item onAction %f index: %d noteLengthWidget->noteLength: %f", noteLength, index, noteLengthWidget->noteLength );
	}
};



struct SymbolicNoteLengthChoice : LedDisplayChoice {
	// GatelengthTurboWidget *widget = NULL;
    NoteLengthChoiceMenuButton *noteLengthWidget;

    // float noteLengths[3] = { 1.0f, 0.5f, 0.3333f};
    // std::string noteNames[3] = {"♩", "♩♩", "3♩" };

    SymbolicNoteLengthChoice() {
        LedDisplayChoice();
        box.size = mm2px(Vec(0, 28.0 / 3));
        font = Font::load(assetGlobal("res/fonts/DejaVuSans.ttf"));
	    // color = nvgRGB(1.0f, 0.0f, 0.0f);
        // box.size = mm2px(Vec(0, 28.0 / 3));
    	textOffset = Vec(1, 8);
    }

	void onAction(EventAction &e) override {
		Menu *menu = gScene->createMenu();
		menu->addChild(construct<MenuLabel>(&MenuLabel::text, "Note Length"));

        for (int i = 0; i < 3; i++) {
            SymbolicNoteLengthItem *item = new SymbolicNoteLengthItem();
            // item->text = stringf("some_text_%d", i);
            item->text = stringf("%s", noteLengthWidget->noteNames[i].c_str());
            item->noteLength = noteLengthWidget->noteLengths[i];
            item->rightText = stringf("%0.4f", noteLengthWidget->noteLengths[i]);
            item->visible = true;
            item->index = i;
            item->noteLengthWidget = noteLengthWidget;
            menu->addChild(item);
        }
    }


	void step() override {
        // debug("SymNoteLengthChoice text: %s", text.c_str());

        text = stringf("%f", noteLengthWidget->noteLength);

        LedDisplayChoice::step();
        // text = noteLengthWidget->text;
        // text = dynamic_cast<LedDisplayChoice *>(noteLengthWidget)->text;
        //text = stringf("♩ %: %f", &module->beat_length[0]);
        // text = stringf("the_text_from_step");
        // debug("noteLengthChoice step");
	}
};

NoteLengthChoiceMenuButton::NoteLengthChoiceMenuButton() {

	// textOffset = Vec(10, 18);

    float x_pos = 0.0f;
    float y_pos = 0.0f;

    Vec pos = Vec(x_pos + 5.0f, y_pos);

    debug("NoteLengthChoiceMenuButton");
    SVGWidget *sw = new SVGWidget();

    addChild(sw);
    sw->setSVG(SVG::load(assetPlugin(plugin, "res/NoteLengthChoiceBackground.svg")));
    sw->wrap();

    SymbolicNoteLengthChoice *noteLengthChoice = new SymbolicNoteLengthChoice();
    noteLengthChoice->box.pos = pos;
    // noteLengthChoice->
    // noteLengthChoice->textOffset = Vec(0, 8);
    noteLengthChoice->box.size = sw->box.size;
    //noteLengthChoice->box.size = Vec(20.0f, 30.0f);
    noteLengthChoice->noteLengthWidget = this;
    // text = noteLengthChoice->text;

    addChild(noteLengthChoice);
    pos = noteLengthChoice->box.getBottomLeft();

    LedDisplaySeparator *separator = Widget::create<LedDisplaySeparator>(pos);
    separator->box.size.x = box.size.x;
    addChild(separator);

    LedDisplaySeparator *noteLengthSep = Widget::create<LedDisplaySeparator>(pos);
    addChild(noteLengthSep);
}

void NoteLengthChoiceMenuButton::step() {
    LedDisplay::step();
}

struct GateLengthFrame : OpaqueWidget {
    Port *lengthInputPort;
    Port *gateOutput;
    Port *bpmInput;
    Port *beatLengthMultiplierInput;
    Port *triggerInput;

    ParamWidget *gateLengthParam;
    ParamWidget *bpmParam;
    ParamWidget *beatLengthParam;

    float font_size = 10.0f;

    GateLengthFrame(GateLengthTurbo *module, int index) {
        debug("GateLengthFrame");
        SVGWidget *sw = new SVGWidget();

        addChild(sw);
        sw->setSVG(SVG::load(assetPlugin(plugin, "res/GateLengthFrame.svg")));

        sw->wrap();

        float x_pos = 4.0f;
        float y_pos = 0.0f;
        // y_pos += 39.0f;
        //y_pos += 32.0f;

        // float y_middle = 22.0f / 2.0f;
        // x_pos += 30.0f;
        // x_pos += 19.0f;    // size of input port
        lengthInputPort = Port::create<SmallPurplePort>(Vec(x_pos, 0.0f + 1.0f),
                                          Port::INPUT,
                                          module,
                                          GateLengthTurbo::GATE_LENGTH_INPUT + index);

        addChild(lengthInputPort);

        x_pos += 19.0f;
        gateLengthParam = ParamWidget::create<SmallPurpleTrimpot>(Vec(x_pos, 0.0f),
                                            module,
                                            GateLengthTurbo::GATE_LENGTH_PARAM + index,
                                            0.0f, 10.0f, 0.1f);
        addChild(gateLengthParam);

        x_pos += 19.0f;

        MsDisplayWidget *gate_length_display = new MsDisplayWidget();
        gate_length_display->box.pos = Vec(x_pos, y_pos);
        gate_length_display->box.size = Vec(60.0f, 15.0f);
        gate_length_display->value = &module->gate_length[index];
        gate_length_display->precision = 4;
        // gate_length_display->font_size = font_size;
        gate_length_display->font_size = font_size;
        gate_length_display->text_pos_x = 2.0f;
        gate_length_display->text_pos_y = 12.0f;
        gate_length_display->lcd_letter_spacing = 1.0f;

        addChild(gate_length_display);

        x_pos += 64.0f;
        gateOutput = Port::create<SmallPurplePort>(Vec(x_pos, 0.0f),
                                                Port::OUTPUT,
                                                module,
                                                GateLengthTurbo::GATE_OUTPUT + index);
        addChild(gateOutput);

        y_pos += 22.0f;   // next "line"
        x_pos = 4.0f;
        bpmInput = Port::create<SmallPurplePort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLengthTurbo::BPM_INPUT + index);

        addChild(bpmInput);

        x_pos += bpmInput->box.size.x;


        // x_pos += noteLengthChoiceMenuButton->box.size.x;

        bpmParam  = ParamWidget::create<SmallPurpleTrimpot>(Vec(x_pos, y_pos),
                                              module,
                                              GateLengthTurbo::BPM_PARAM + index,
                                              -5.0f, 5.0f, 0.0f);
        addChild(bpmParam);

        x_pos += 19.0f;   // size of trimpot
        MsDisplayWidget *bpm_display = new MsDisplayWidget();
        bpm_display->box.pos = Vec(x_pos, y_pos);
        bpm_display->box.size = Vec(48.0f, 18.0f);
        bpm_display->precision = 2;
        bpm_display->font_size = font_size;
        bpm_display->value = &module->bpm_labels[index];
        debug("i: %d bpm: %f", index, bpm_display->value);
        addChild(bpm_display);

        x_pos = 4.0f;
        y_pos += 22.0f;   // next "line"

        beatLengthMultiplierInput = Port::create<SmallPurplePort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLengthTurbo::BEAT_LENGTH_MULTIPLIER_INPUT + index);
        addChild(beatLengthMultiplierInput);

        x_pos += 19.0f;

        // TODO: if we assume the 'snapped' values are say, a 32nd up to a whole note, we could make
        //       the param value be the number of 32nd's. Not sure how to support dotted or tuplets
        //       for that case though, which would be nice to have.
        //
        // TODO: alternatively, maybe a custom Knob with it's own snap behavior?
        beatLengthParam = ParamWidget::create<Trimpot>(Vec(x_pos, y_pos),
                                              module,
                                              GateLengthTurbo::BEAT_LENGTH_MULTIPLIER_PARAM + index,
                                              // 16.0f = 16x sixteenth note == one quarter note? ??
                                              0.0f, 160.0f, 16.0f);
        ((Trimpot *) beatLengthParam)->snap = true;
        addChild(beatLengthParam);

        // module->params[GateLengthTurbo::BEAT_LENGTH_MULTIPLIER_PARAM + i]->
        x_pos += 19.0f;  // size of beat length trimpot

        // select the base note (ie, quarter note etc)
        Vec pos = Vec(x_pos, y_pos);

        NoteLengthChoiceMenuButton *noteLengthChoiceMenuButton = new NoteLengthChoiceMenuButton();
        noteLengthChoiceMenuButton->box.pos = pos;
        addChild(noteLengthChoiceMenuButton);

        x_pos += 30.0f;

        MsDisplayWidget *beat_length_display = new MsDisplayWidget();
        beat_length_display->box.pos = Vec(x_pos, y_pos);
        beat_length_display->box.size = Vec(48.0f, 18.0f);
        beat_length_display->precision = 2;
        beat_length_display->font_size = font_size;
        beat_length_display->value = &module->beat_length[index];
        debug("i: %d beat_length: %f", index, beat_length_display->value);
        addChild(beat_length_display);

        x_pos += 52.0f;

        triggerInput = Port::create<SmallPurplePort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLengthTurbo::TRIGGER_INPUT + index);
        addChild(triggerInput);
        // addParam(beatLengthParam);
        y_pos += 26.0f;

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
        GateLengthFrame *frame = new GateLengthFrame(module, i);

        frame->box.pos = Vec(x_pos, y_pos);
        frame->font_size = font_size;
        addChild(frame);
        debug("y_pos: %f box.size.y: %f", y_pos, frame->box.size.y);
        // y_pos += frame->box.size.y;
        y_pos += 65.0f;

        inputs.push_back(frame->lengthInputPort);
        // addChild(frame->lengthInputPort);
        //addInput(frame->lengthInputPort);
        /*addInput(Port::create<SmallPurplePort>(Vec(x_pos, y_pos + 1.0f),
                                          Port::INPUT,
                                          module,
                                          GateLengthTurbo::GATE_LENGTH_INPUT + i));
        */
        // x_pos += 19.0f;
        params.push_back(frame->gateLengthParam);

        // TODO: get noteLengthWidget selection to the module

/*
        x_pos += 64.0f;
        addOutput(Port::create<SmallPurplePort>(Vec(x_pos, y_pos + 1.0f),
                                                Port::OUTPUT,
                                                module,
                                                GateLengthTurbo::GATE_OUTPUT + i));
*/
        outputs.push_back(frame->gateOutput);

/*
        y_pos += 22.0f;   // next "line"
        x_pos = 4.0f;
        addInput(Port::create<SmallPurplePort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLengthTurbo::BPM_INPUT + i));
*/
        inputs.push_back(frame->bpmInput);

/*
        x_pos += 19.0f;
        addParam(ParamWidget::create<SmallPurpleTrimpot>(Vec(x_pos, y_pos),
                                              module,
                                              GateLengthTurbo::BPM_PARAM + i,
                                              -5.0f, 5.0f, 0.0f));
*/
        params.push_back(frame->bpmParam);

        /*
        x_pos += 19.0f;   // size of trimpot

        MsDisplayWidget *bpm_display = new MsDisplayWidget();
        bpm_display->box.pos = Vec(x_pos, y_pos);
        bpm_display->box.size = Vec(48.0f, 18.0f);
        bpm_display->precision = 2;
        bpm_display->font_size = font_size;
        bpm_display->value = &module->bpm_labels[i];
        debug("i: %d bpm: %f", i, bpm_display->value);
        addChild(bpm_display);
        */

        // x_pos += 84.0f;
        //  x_pos += 62.0f
        // x_pos += 48.0f;  // size of bpm display
        /*
        x_pos = 4.0f;
        y_pos += 22.0f;   // next "line"

        addInput(Port::create<SmallPurplePort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLengthTurbo::BEAT_LENGTH_MULTIPLIER_INPUT + i));

        */
        inputs.push_back(frame->beatLengthMultiplierInput);

        /*
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
        */
        params.push_back(frame->beatLengthParam);

        /*
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
        */

        // y_pos += 26.0f;
        // x_pos += 30.0f;
        /*
        x_pos += 52.0f;

        addInput(Port::create<SmallPurplePort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          GateLengthTurbo::TRIGGER_INPUT + i));

        //y_pos += 35.0f;
        // Vec pos = Vec();
        */
        inputs.push_back(frame->triggerInput);

        /*
        Vec pos = Vec(x_pos + 5.0f, y_pos);
        SymbolicNoteLengthChoice *noteLengthChoice = Widget::create<SymbolicNoteLengthChoice>(pos);
        noteLengthChoice->textOffset = Vec(2, 7);
        noteLengthChoice->box.size = Vec(30.0f, 30.0f);
        // noteLengthChoice->box.size.x = box.size.x;

	    // PatternChoice *patternChoice = Widget::create<PatternChoice>(pos);
	    // patternChoice->widget = this;
	    addChild(noteLengthChoice);
        pos = noteLengthChoice->box.getBottomLeft();

        // LedDisplaySeparator *separator = Widget::create<LedDisplaySeparator>(pos);
        // separator->box.size.x = box.size.x;
        // addChild(separator);

        // LedDisplaySeparator *noteLengthSep = Widget::create<LedDisplaySeparator>(pos);
	    // addChild(noteLengthSep);
        */


    }

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

}

Model *modelGateLengthTurbo = Model::create<GateLengthTurbo, GateLengthTurboWidget>(
        "Alikins", "GateLengthTurbo", "Gate Length Turbo", UTILITY_TAG);

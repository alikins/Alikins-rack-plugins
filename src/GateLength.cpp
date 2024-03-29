#include "alikins.hpp"
#include "MsDisplayWidget.hpp"

/*
    This module was inspired by the question and discussion at:
    https://www.facebook.com/groups/vcvrack/permalink/161960391130780/

    "Okay , Rackheads... ;) Looking for a module that can "stretch" the length , extend the duration , of a gate/ trigger pulse."
*/

struct GateLength : Module {
    enum ParamIds {
        GATE_LENGTH_PARAM1,
        GATE_LENGTH_PARAM2,
        GATE_LENGTH_PARAM3,
        GATE_LENGTH_PARAM4,
        GATE_LENGTH_PARAM5,
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

    float gate_length[GATE_LENGTH_INPUTS];

    dsp::SchmittTrigger inputOnTrigger[GATE_LENGTH_INPUTS];

    dsp::PulseGenerator gateGenerator[GATE_LENGTH_INPUTS];

    GateLength() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        for (int i = 0; i < GATE_LENGTH_INPUTS; i++) {
            configParam(GATE_LENGTH_PARAM1 + i,
                    0.0f, 10.0f, 0.1f, "Length of gate", " sec");
        }
    }
	void process(const ProcessArgs &args) override;

    void onReset() override {
    }

};

void GateLength::process(const ProcessArgs &args) {
    // FIXME: add way to support >10.0s gate length

    float sample_time = args.sampleTime;

    for (int i = 0; i < GATE_LENGTH_INPUTS; i++) {
        gate_length[i] = clamp(params[GATE_LENGTH_PARAM1 + i].getValue() + inputs[GATE_LENGTH_INPUT1 + i].getVoltage(), 0.0f, 10.0f);

        if (inputOnTrigger[i].process(inputs[TRIGGER_INPUT1 + i].getVoltage())) {
            // debug("GL INPUT ON TRIGGER %d gate_length: %f", i, gate_length[i]);
            gateGenerator[i].trigger(gate_length[i]);
        }

        outputs[GATE_OUTPUT1 + i].setVoltage(gateGenerator[i].process(sample_time) ? 10.0f : 0.0f);
    }
}

struct GateLengthWidget : ModuleWidget {
    GateLengthWidget(GateLength *module) {
        setModule(module);
        box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/GateLength.svg")));

        float y_pos = 2.0f;

        for (int i = 0; i < GATE_LENGTH_INPUTS; i++) {
            float x_pos = 4.0f;
            y_pos += 39.0f;

            addInput(createInput<PJ301MPort>(Vec(x_pos, y_pos),
                                              module,
                                              GateLength::TRIGGER_INPUT1 + i));

            x_pos += 30.0f;

            MsDisplayWidget *gate_length_display = new MsDisplayWidget();
            gate_length_display->box.pos = Vec(x_pos, y_pos + 1.0f);
            gate_length_display->box.size = Vec(84, 24);
            if (module) {
                gate_length_display->value = &module->gate_length[i];
            }
            addChild(gate_length_display);

            // FIXME: use new sequential box hbox/vbox thing
            x_pos += 84.0f;
            x_pos += 4.0f;
            addOutput(createOutput<PJ301MPort>(Vec(x_pos, y_pos),
                                               module,
                                               GateLength::GATE_OUTPUT1 + i));

            x_pos = 4.0f;
            y_pos += 26.0f;

            addInput(createInput<PJ301MPort>(Vec(x_pos, y_pos),
                                              module,
                                              GateLength::GATE_LENGTH_INPUT1 + i));

            x_pos += 30.0f;
            addParam(createParam<Trimpot>(Vec(x_pos, y_pos + 3.0f),
                                                  module,
                                                  GateLength::GATE_LENGTH_PARAM1 + i));
        }

        addChild(createWidget<ScrewSilver>(Vec(0.0f, 0.0f)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
        addChild(createWidget<ScrewSilver>(Vec(0.0f, 365.0f)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

    }
};

Model *modelGateLength = createModel<GateLength, GateLengthWidget>("GateLength");

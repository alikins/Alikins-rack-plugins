#include "alikins.hpp"

#define VALUE_COUNT 4
#define CLOSE_ENOUGH 0.01f

struct ValueSaverWidget;

struct ValueSaver : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        ENUMS(VALUE_INPUT, VALUE_COUNT),
        NUM_INPUTS
    };
    enum OutputIds {
        ENUMS(VALUE_OUTPUT, VALUE_COUNT),
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    ValueSaver() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

	void process(const ProcessArgs &args) override;

    void onReset() override {
    }

    json_t *dataToJson() override;
    void dataFromJson(json_t *rootJ) override;

    float values[VALUE_COUNT] = {0.0f};
    float prevInputs[VALUE_COUNT] = {0.0f};

    bool changingInputs[VALUE_COUNT] = {};

    dsp::SchmittTrigger valueUpTrigger[VALUE_COUNT];
    dsp::SchmittTrigger valueDownTrigger[VALUE_COUNT];

    ValueSaverWidget* widget;
};

void ValueSaver::process(const ProcessArgs &args)
{
    // states:
    //  - active inputs, meaningful current input -> output
    //  - active inputs,
    //  - in active inputs, meaningful 'saved' input -> output
    //  - in active inputs, default/unset value -> output
    //
    for (int i = 0; i < VALUE_COUNT; i++) {
        // Just output the "saved" value if NO ACTIVE input
        if (!inputs[VALUE_INPUT + i].isConnected()) {
            outputs[VALUE_OUTPUT + i].setVoltage(prevInputs[i]);
            continue;
        }

        // ACTIVE INPUTS
        // process(rescale(in, low, high, 0.f, 1.f))
        // if we haven't already figured out this is a useful changing input, check
        if (!changingInputs[i]) {
            float down = rescale(inputs[VALUE_INPUT + i].getVoltage(), 0.0f, -CLOSE_ENOUGH, 0.f, 1.f);
            float up = rescale(inputs[VALUE_INPUT + i].getVoltage(), 0.0f, CLOSE_ENOUGH, 0.f, 1.f);

            // TODO: if input is changing from 0.0f
            //       if input is 0.0f but that is changing from prevInput (explicitly sent 0.0f)
            if (valueUpTrigger[i].process(up) || valueDownTrigger[i].process(down)) {
                // debug("value*Trigger[%d] triggered value: %f %f", i, values[i], up);
                changingInputs[i] = true;
            }
        }

        if (!changingInputs[i]) {
            // active input but it is 0.0f, like a midi-1 on startup that hasn't sent any signal yet
            // debug("[%d] ACTIVE but input is ~0.0f, prevInputs[%d]=%f", i, i, prevInputs[i]);
            values[i] = prevInputs[i];
            outputs[VALUE_OUTPUT + i].setVoltage(values[i]);
        }
        else {
            // input value copied to output value and stored in values[]
            values[i] = inputs[VALUE_INPUT + i].getVoltage();;
            outputs[VALUE_OUTPUT + i].setVoltage(values[i]);
            prevInputs[i] = values[i];

            // We are getting meaningful input values (ie, not just 0.0f), we can
            // pay attention to the inputs now
            changingInputs[i] = true;
            continue;
        }
    }
}

struct LabelDisplay : LedDisplay {
	LedDisplayTextField* textField = createWidget<LedDisplayTextField>(Vec(0, 0));

    LabelDisplay() {
		textField->multiline = true;
        textField->textOffset = Vec(-2.0f, -3.0f);
        textField->color = SCHEME_CYAN;
		addChild(textField);
    }

    void setFieldSize(rack::math::Vec boxsize) {
        textField->box.size = boxsize;
    }

    void setTextField(std::string text) {
        textField->text = text;
    }

    std::string getTextField() {
        return textField->text;
    }
};

struct ValueSaverWidget : ModuleWidget {
    LabelDisplay *labelDisplays[VALUE_COUNT];

    ValueSaverWidget(ValueSaver *module) {
        setModule(module);

        if (module)
          module->widget = this;

        box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ValueSaverPanel.svg")));

        float y_baseline = 48.0f;
        float y_pos = y_baseline;

        for (int i = 0; i < VALUE_COUNT; i++) {
            float x_pos = 4.0f;

            addInput(createInput<PJ301MPort>(Vec(x_pos, y_pos),
                                              module,
                                              ValueSaver::VALUE_INPUT + i));

            x_pos += 30.0f;

            addOutput(createOutput<PJ301MPort>(Vec(box.size.x - 30.0f, y_pos),
                                               module,
                                               ValueSaver::VALUE_OUTPUT + i));

            y_pos += 28.0f;
            labelDisplays[i] = new LabelDisplay();

            labelDisplays[i]->box.pos = (Vec(4.0f, y_pos));
            labelDisplays[i]->box.size = Vec(box.size.x - 8.0f, 40.0f);
            labelDisplays[i]->setTextField("");
            labelDisplays[i]->setFieldSize(Vec(85, 40));
            addChild(labelDisplays[i]);

            y_pos += labelDisplays[i]->box.size.y;
            y_pos += 14.0f;
        }

        addChild(createWidget<ScrewSilver>(Vec(0.0f, 0.0f)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
        addChild(createWidget<ScrewSilver>(Vec(0.0f, 365.0f)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

    }
};

json_t* ValueSaver::dataToJson() {
    json_t *rootJ = json_object();
    json_t *valuesJ = json_array();
    json_t *labelsJ = json_array();

    for (int i = 0; i < VALUE_COUNT; i++) {
        json_t *valueJ = json_real(values[i]);
        json_array_append_new(valuesJ, valueJ);
        if (widget) {
            json_t *labelJ  = json_string(widget->labelDisplays[i]->getTextField().c_str());
            json_array_append_new(labelsJ, labelJ);
        }
    }
    json_object_set_new(rootJ, "values", valuesJ);
    json_object_set_new(rootJ, "labels", labelsJ);

    return rootJ;
}

void ValueSaver::dataFromJson(json_t *rootJ) {
    json_t *valuesJ = json_object_get(rootJ, "values");
    float savedInput;

    if (valuesJ) {
        for (int i = 0; i < VALUE_COUNT; i++) {
            json_t *valueJ = json_array_get(valuesJ, i);
            if (valueJ) {
                savedInput = json_number_value(valueJ);
                prevInputs[i] = savedInput;
                values[i] = savedInput;
                changingInputs[i] = false;
            }
        }
    }

    json_t *labelsJ = json_object_get(rootJ, "labels");
    if (labelsJ) {
        for (int i = 0; i < VALUE_COUNT; i++) {
            json_t *labelJ = json_array_get(labelsJ, i);
            if (labelJ && widget) {
                widget->labelDisplays[i]->setTextField(json_string_value(labelJ));
            }
        }
    }
}

Model *modelValueSaver = createModel<ValueSaver, ValueSaverWidget>("ValueSaver");

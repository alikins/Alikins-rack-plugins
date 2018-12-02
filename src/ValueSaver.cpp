#include "dsp/digital.hpp"
#include "alikins.hpp"

#define VALUE_COUNT 8
#define CLOSE_ENOUGH 0.01f

struct ValueSaver : Module {
    enum ParamIds {
        ENUMS(VALUE_PARAM, VALUE_COUNT),
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

    ValueSaver() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    void step() override;

    void onReset() override {
    }

    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

    float values[VALUE_COUNT] = {0.0f};
    bool activeAndChangingInputs[VALUE_COUNT] = {};
    SchmittTrigger valueChangeTrigger[VALUE_COUNT];

};

void ValueSaver::step()
{
    bool trig = false;
    for (int i = 0; i < VALUE_COUNT; i++) {
        if (inputs[VALUE_INPUT + i].active) {
            trig = valueChangeTrigger[i].process(rescale(inputs[i].value,
             values[i] - CLOSE_ENOUGH, values[i], 0.0f, 1.0f));

            if (trig) {
                debug("input[%d] %f was around values[%d] %f, chg? eps %f",
                i, inputs[i].value, i, values[i], values[i] - CLOSE_ENOUGH );
                activeAndChangingInputs[i] = true;
            }
            // float gate = inputs[triggerInput].value;
		    // int triggered = trigger.process(rescale(gate, params[PARAM_TRIGGER].value - 0.1f, params[PARAM_TRIGGER].value, 0.0f, 1.0f));

            // saved value is about the input value, close enough to start input?
            // if (fabs(inputs[VALUE_INPUT + i].value - values[i]) < CLOSE_ENOUGH) {
            //    activeAndChangingInputs[i] = true;
            // }
        } else {
            activeAndChangingInputs[i] = false;
        }

        if (activeAndChangingInputs[i]) {
            // start changing the saved value, ie, reading from the inputs
            values[i] = inputs[VALUE_INPUT + i].value;
        }
        outputs[VALUE_OUTPUT + i].value = values[i];
    }
}

json_t* ValueSaver::toJson() {
    json_t *rootJ = json_object();

    // json_object_set_new(rootJ, "valueCount", json_integer(inputRange));
    json_t *valuesJ = json_array();
    for (int i = 0; i < VALUE_COUNT; i++)
    {
        debug("toJson current values[%d]: %f", i, values[i]);
        json_t *valueJ = json_real(values[i]);
        json_array_append_new(valuesJ, valueJ);
    }
    json_object_set_new(rootJ, "values", valuesJ);

    return rootJ;
}

void ValueSaver::fromJson(json_t *rootJ) {
    json_t *valuesJ = json_object_get(rootJ, "values");
    if (valuesJ)
    {
        for (int i = 0; i < VALUE_COUNT; i++)
        {
            // debug("fromJson i: %d", i);
            debug("current values[%d]: %f", i, values[i]);
            json_t *valueJ = json_array_get(valuesJ, i);
            if (valueJ) {
                debug("fromJson values[%d] value: %f", i, json_number_value(valueJ));
                values[i] = json_number_value(valueJ);
                activeAndChangingInputs[i] = false;
            }
        }
    }

}

struct ValueSaverWidget : ModuleWidget {
    ValueSaverWidget(ValueSaver *module);
};

ValueSaverWidget::ValueSaverWidget(ValueSaver *module) : ModuleWidget(module) {

    box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/ValueSaverPanel.svg")));

    float y_pos = 2.0f;

    for (int i = 0; i < VALUE_COUNT; i++) {
        float x_pos = 4.0f;

        addInput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                                          Port::INPUT,
                                          module,
                                          ValueSaver::VALUE_INPUT + i));

        x_pos += 30.0f;

        addOutput(Port::create<PJ301MPort>(Vec(x_pos, y_pos),
                                           Port::OUTPUT,
                                           module,
                                           ValueSaver::VALUE_OUTPUT + i));

        y_pos += 35.0f;
    }

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

}

Model *modelValueSaver = Model::create<ValueSaver, ValueSaverWidget>(
        "Alikins", "ValueSaver", "Value Saver", UTILITY_TAG);

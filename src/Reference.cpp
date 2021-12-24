#include "alikins.hpp"


struct Reference : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        MINUS_TEN_OUTPUT,
        MINUS_FIVE_OUTPUT,
        MINUS_ONE_OUTPUT,
        ZERO_OUTPUT,
        PLUS_ONE_OUTPUT,
        PLUS_FIVE_OUTPUT,
        PLUS_TEN_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    Reference() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
    }

    void process(const ProcessArgs &args) override;

    void onReset() override {
    }

};

void Reference::process(const ProcessArgs &args) {
    outputs[MINUS_TEN_OUTPUT].setVoltage(-10.0f);
    outputs[MINUS_FIVE_OUTPUT].setVoltage(-5.0f);
    outputs[MINUS_ONE_OUTPUT].setVoltage(-1.0f);

    outputs[ZERO_OUTPUT].setVoltage(0.0f);

    outputs[PLUS_ONE_OUTPUT].setVoltage(1.0f);
    outputs[PLUS_FIVE_OUTPUT].setVoltage(5.0f);
    outputs[PLUS_TEN_OUTPUT].setVoltage(10.0f);

}

struct ReferenceWidget : ModuleWidget {
    ReferenceWidget(Reference *module) {
        setModule(module);
        // box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Reference.svg")));

        float y_pos = 18.0f;

        float x_pos = 2.0f;
        float y_offset = 50.0f;

        addOutput(createOutput<PJ301MPort>(Vec(x_pos, y_pos),
                                           module,
                                           Reference::PLUS_TEN_OUTPUT));

        y_pos += y_offset;
        addOutput(createOutput<PJ301MPort>(Vec(x_pos, y_pos),
                                           module,
                                           Reference::PLUS_FIVE_OUTPUT));

        y_pos += y_offset;
        addOutput(createOutput<PJ301MPort>(Vec(x_pos, y_pos),
                                           module,
                                           Reference::PLUS_ONE_OUTPUT));

        y_pos += y_offset;
        addOutput(createOutput<PJ301MPort>(Vec(x_pos, y_pos),
                                           module,
                                           Reference::ZERO_OUTPUT));

        y_pos += y_offset;
        addOutput(createOutput<PJ301MPort>(Vec(x_pos, y_pos),
                                           module,
                                           Reference::MINUS_ONE_OUTPUT));

        y_pos += y_offset;
        addOutput(createOutput<PJ301MPort>(Vec(x_pos, y_pos),
                                           module,
                                           Reference::MINUS_FIVE_OUTPUT));

        y_pos += y_offset;
        addOutput(createOutput<PJ301MPort>(Vec(x_pos, y_pos),
                                           module,
                                           Reference::MINUS_TEN_OUTPUT));


        addChild(createWidget<ScrewSilver>(Vec(0.0f, 0.0f)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
        addChild(createWidget<ScrewSilver>(Vec(0.0f, 365.0f)));
        addChild(createWidget<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));
    }
};


Model *modelReference = createModel<Reference, ReferenceWidget>("Reference");

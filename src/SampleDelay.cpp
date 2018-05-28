#include "dsp/digital.hpp"
#include "dsp/samplerate.hpp"
#include "dsp/ringbuffer.hpp"

#include "alikins.hpp"
#include "MsDisplayWidget.hpp"

// ?
#define HISTORY_SIZE (1<<21)

struct SampleDelay : Module {
    enum ParamIds {
        PARAM1,
        NUM_PARAMS
    };
    enum InputIds {
        INPUT1,
        NUM_INPUTS
    };
    enum OutputIds {
        OUTPUT1,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    //SchmittTrigger inputOnTrigger[GATE_LENGTH_INPUTS];
    DoubleRingBuffer<float, HISTORY_SIZE> historyBuffer;
    DoubleRingBuffer<float, 16> outBuffer;

    PulseGenerator gateGenerator[GATE_LENGTH_INPUTS];

    SampleDelay() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    
    void step() override;

    void onReset() override {
    }

};

void SampleDelay::step() {
    //float sample_time = engineGetSampleTime();
    outputs[OUTPUT1].value = inputs[INPUT1].value;
}

struct SampleDelayWidget : ModuleWidget {
    SampleDelayWidget(SampleDelay *module);
};

SampleDelayWidget::SampleDelayWidget(SampleDelay *module) : ModuleWidget(module) {
    setPanel(SVG::load(assetPlugin(plugin, "res/SampleDelay.svg")));

    float y_pos = 2.0f;
    
    addInput(Port::create<PJ301MPort>(Vec(5.0f, 5.0f),
                                          Port::INPUT,
                                          module,
                                          SampleDelay::INPUT1));

    MsDisplayWidget *sample_count_display = new MsDisplayWidget();
    sample_count_display->box.pos = Vec(5.0f, 50.0f);
    sample_count_display->box.size = Vec(84, 24);
    // sample_count_display->value = &module->sample_count;
    //gate_length_display->value = &module->gate_length[i];
    addChild(sample_count_display);



    addParam(ParamWidget::create<Trimpot>(Vec(5.0f, 75.0f),
                                          module,
                                          SampleDelay::PARAM1,
                                          0.0f, 48000.0f, 1.0f));

    addOutput(Port::create<PJ301MPort>(Vec(5.0f, 1000.0f),
                                       Port::OUTPUT,
                                       module,
                                       SampleDelay::OUTPUT1));

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 15.0f, 365.0f)));

}

Model *modelSampleDelay = Model::create<SampleDelay, SampleDelayWidget>(
        "Alikins", "SampleDelay", "Delay by number of samples", UTILITY_TAG);

#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <string>

// #include "jansson.h"

#include "alikins.hpp"
#include "dsp/digital.hpp"
#include "util.hpp"

/*
 * Store and display author info and metadata
 */


struct Credits : Module {
    enum ParamIds {
        NUM_PARAMS
    };
    enum InputIds {
        NUM_INPUTS
    };
    enum OutputIds {
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    void load_author();
    // json_t *author_data;

    // TODO: whatever the best c++ way to init this is
    Credits() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        load_author();
    }

    // TODO: do we need a step?
    // void step() override;
};


void Credits::load_author() {
    json_error_t error;
    char *blob;

    json_t *a_data = json_load_file("/Users/adrian/src/Rack-v0.5/blippy.json", 0, &error);

    json_t *modules_data = json_object_get(a_data, "authors");
    blob = json_dumps(modules_data, 0);

    info("blob: %s", blob);

    if (!a_data) {
        warn("JSON parsing error loading file=%s line=%d column=%d error=%s", error.source, error.line, error.column, error.text);
        // todo: error handling
    }

}

// TODO: dont really need
// void Credits::step() {}

// TODO: custom text/display widgets?

CreditsWidget::CreditsWidget() {
    Credits *module = new Credits();
    setModule(module);
    setPanel(SVG::load(assetPlugin(plugin, "res/Credits.svg")));

    addChild(createScrew<ScrewSilver>(Vec(5, 0)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 365)));

}

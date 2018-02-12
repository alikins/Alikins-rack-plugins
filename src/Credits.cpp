#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <string>

#include "alikins.hpp"
#include "dsp/digital.hpp"
#include "util.hpp"

/*
 * Store and display author info and metadata
 */

/* TODO:
    show the credits info
     (optional? could make it a tiny 1hp data only)
    methods to detect changes
    methods to detect new authors
 */

/*
 * credits model
 *
 * 'credits' key in json data
 *
 * CreditList - list of Credit()
 *
 * Credit
 *  .authorList
 *      Author()
 *          string/unicode name
 *          string/unicode url
 *          # author_id is only uniq and meaningful within a particular doc
 *          # ie, not globally uniq though could be uuid
 *          string/unicode author_id
 *   string/unicode credit_id
 *
 * EditionList   - list of Edition()
 *   Edition  - data for a 'save' (ie, version, revision, update, release, etc...)
 *      # credit_id can point to a Credit() which can repr multiple authors
 *      string/unicode credit_id
 *      string  date   (iso8601 date format)
 *
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

    // credits_path = "credits.json"
    //
    // toJson
    //    serialize list of authors to 'authors' key
    //
    // fromJson
    //    deserialize list of authors
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


// TODO: custom text/display widgets?

CreditsWidget::CreditsWidget() {
    Credits *module = new Credits();
    setModule(module);
    setPanel(SVG::load(assetPlugin(plugin, "res/Credits.svg")));

    addChild(createScrew<ScrewSilver>(Vec(5, 0)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 365)));

    float x_start = 5.0;
    float x_offset = 0.0;
    float y_start = 10.0;
    float y_offset = 32.0;
    float x_pos = x_start;
    float y_pos = y_start;

    int widget_index = 0;

    TextField *author_name;
    TextField *author_url;

    author_name = new TextField();
    author_name->text = "Default Author Name";
    author_name->box.pos = Vec(x_pos, y_pos);
    author_name->box.size = Vec(200, 28);
    addChild(author_name);

    widget_index++;
    x_pos = x_start + x_offset;
    y_pos = y_start + (widget_index * y_offset);

    author_url = new TextField();
    author_url->text = "http://default.author.example.com";
    author_url->box.pos = Vec(x_pos, y_pos);
    author_url->box.size = Vec(200, 28);
    addChild(author_url);

}

#include <stdio.h>
#include <sstream>
#include <iomanip>
#include <vector>
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
        CREDITS_SET_PARAM,
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
    json_t* toJson() override;

    void fromJson(json_t *rootJ) override;

    void step() override;
    // TODO: whatever the best c++ way to init this is
    Credits() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {
        load_author();

    }

    std::string author_name;
    std::string author_date;
    std::string author_url;


    // CreditData credits[1];
    bool credits_set = false;

    std::vector<CreditData*> vcredits;
    CreditData *default_credit;

    // TODO: from/to json
    //       reuse author model and encode
    // credits_path = "credits.json"
    //
    // toJson
    //    serialize list of authors to 'authors' key
    //
    // fromJson
    //    deserialize list of authors
};


void Credits::step() {
    bool credits_set = false;
}

json_t* Credits::toJson() {
    debug("to_json");
    json_t *rootJ = json_object();

    json_t *vcreditsJ = json_array();

    for (CreditData *vcredit : vcredits) {
        debug("toJson vcredits vcredit.author_name: %s", vcredit->author_name.c_str());
        json_t *vcredit_dataJ = json_object();

        json_object_set_new(vcredit_dataJ, "name", json_string(vcredit->author_name.c_str()));
        json_object_set_new(vcredit_dataJ, "date", json_string(vcredit->author_date.c_str()));
        json_object_set_new(vcredit_dataJ, "url", json_string(vcredit->author_url.c_str()));

        json_array_append_new(vcreditsJ, vcredit_dataJ);
    }

    // TODO: add default_credit if not already in vcredits

    // json_t *credit1 = json_string(author_name.c_str());
    // json_array_append_new(creditsJ, credit1);

    // json_object_set_new(rootJ, "credits", creditsJ);
    json_object_set_new(rootJ, "vcredits", vcreditsJ);

    json_object_set_new(rootJ, "namee", json_string(author_name.c_str()));
        return rootJ;
    }


void Credits::fromJson(json_t *rootJ) {
    debug("fromJson");
    json_t *nameJ = json_object_get(rootJ, "namee");
    if (nameJ) {
        author_name = json_string_value(nameJ);
        // setModel(json_string_value(nameJ));
    }

    json_t *vcreditsJ = json_object_get(rootJ, "vcredits");
    if (!json_is_array(vcreditsJ)) {
        warn("fromJson JSON parsing error 'vcredits' is not an array");
        //fprintf(stderr, "error: commit %d: message is not a string\n", i + 1);
        json_decref(rootJ);
    }
    size_t index;
    json_t *valueJ;

    json_array_foreach(vcreditsJ, index, valueJ) {
        // json_t *creditJ = json_object();
        json_t *cname = json_object_get(valueJ, "name");
        CreditData *credit_data = new CreditData();

        if (json_is_string(cname)) {
            debug("fromJson cname: %s", json_string_value(cname));
            credit_data->author_name = json_string_value(cname);
        }

        json_t *cdate = json_object_get(valueJ, "date");
        if (json_is_string(cdate)) {
            debug("fromJson cdate: %s", json_string_value(cdate));
            credit_data->author_date = json_string_value(cdate);
        }

        json_t *curl = json_object_get(valueJ, "url");
        if (json_is_string(curl)) {
            debug("fromJson curl: %s", json_string_value(curl));
            credit_data->author_url = json_string_value(curl);
        }
        vcredits.push_back(credit_data);
    }

}



void Credits::load_author() {
    json_error_t json_error;
    char *blob;

    json_t *a_data = json_load_file("/Users/adrian/src/Rack-v0.5/blippy.json", 0, &json_error);

    if (!a_data) {
        warn("load_author JSON parsing error loading file=%s line=%d column=%d error=%s", json_error.source, json_error.line, json_error.column, json_error.text);
        return;
        // todo: error handling
    }

    json_t *authors_list = json_object_get(a_data, "authors");

    // FIXME: just grabbing first element
    json_t *authors_data = json_array_get(authors_list, 0);
    json_t *author_name_data = json_object_get(authors_data, "name");
    json_t *author_date_data = json_object_get(authors_data, "date");
    json_t *author_url_data = json_object_get(authors_data, "url");

    if (!json_is_string(author_name_data))
    {
        warn("JSON parsing error author_name is not a string");
        //fprintf(stderr, "error: commit %d: message is not a string\n", i + 1);
        json_decref(a_data);
    }

    if (!json_is_string(author_date_data))
    {
        warn("JSON parsing error author_date is not a string");
        //fprintf(stderr, "error: commit %d: message is not a string\n", i + 1);
        json_decref(a_data);
    }

    if (!json_is_string(author_url_data))
    {
        warn("load_author JSON parsing error author_url is not a string");
        //fprintf(stderr, "error: commit %d: message is not a string\n", i + 1);
        json_decref(a_data);
    }

    // FIXME: handle error
    author_name = json_string_value(author_name_data);
    author_date = json_string_value(author_date_data);
    author_url = json_string_value(author_url_data);

    info("load_author author_name: %s", author_name.c_str());
    info("load_author author_date: %s", author_date.c_str());
    info("load_author author_url: %s", author_url.c_str());
    blob = json_dumps(authors_data, 0);

    default_credit = new CreditData();
    default_credit->author_name = author_name;
    default_credit->author_date = author_date;
    default_credit->author_url = author_url;

    info("blob: %s", blob);
}

// from Rack/src/app/AddModuleWindow.cpp
struct ListMenu : OpaqueWidget {

    void draw(NVGcontext *vg) override {
        Widget::draw(vg);
    }

    void step() override {
        Widget::step();

        box.size.y = 0;
        for (Widget *child : children) {
            if (!child->visible)
                continue;
            // Increase height, set position of child
            child->box.pos = Vec(0, box.size.y);
            box.size.y += child->box.size.y;
            child->box.size.x = box.size.x;
        }
    }
};


struct CreditItem : MenuItem {
    void onAction(EventAction &e) override {
        info("CreditItem onAction");
        // e.consumed = false;
    }
};

struct CreditMenu : ListMenu {
    Credits *module;
    std::vector<CreditData*> vcredits;

    void step() override {
        // info("CreditMenu.step");
        // info("vcredits.empty(): %i", vcredits.size());
        if (!module->credits_set) {
            for (CreditData *vcredit_data : module->vcredits) {
                debug("CreditMenu.override addChild for %s", vcredit_data->author_name.c_str());
                addChild(construct<CreditItem>(&MenuEntry::text, vcredit_data->author_name));
                //addChild(construct<MenuLabel>(&MenuLabel::text, vcredit_data->author_name));
            }
            module->credits_set = true;

        }
        ListMenu::step();
    }

    CreditMenu() {
        debug("CreditMenu constructor");
        addChild(construct<MenuLabel>(&MenuLabel::text, "Credits"));

    }
};



// TODO: custom text/display widgets?

CreditsWidget::CreditsWidget() {
    Credits *module = new Credits();
    setModule(module);
    setPanel(SVG::load(assetPlugin(plugin, "res/Credits.svg")));

    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 20, 0)));
    addChild(createScrew<ScrewSilver>(Vec(box.size.x - 50, 365)));

    float x_start = 5.0;
    float y_start = 10.0;
    float x_pos = x_start;
    float y_pos = y_start;

    debug("CreditsWidget");
    //module->vcredits.push_back(module->default_credit);
    /*
    for (CreditData *vcredit : module->vcredits) {
        debug("setting up widgets for credits: %s", vcredit->author_name.c_str());
        addCreditTextEntry(vcredit, x_pos, y_pos);
        y_pos = y_pos + 30;
    }
    */
    // addCreditTextEntry(module->default_credit, x_pos, y_pos + 20);

    CreditMenu *creditMenu = new CreditMenu();
    creditMenu->box.size.x = 150;
    creditMenu->module = module;

    ScrollWidget *creditScroll = new ScrollWidget();
    creditScroll->container->addChild(creditMenu);
    creditScroll->box.pos = Vec(5, y_pos);
    // creditScroll->box.size = Vec(100, box.size.y - y_pos);
    creditScroll->box.size = Vec(200, 300);
    addChild(creditScroll);


}

void CreditsWidget::step() {
    Widget::step();
    // debug("CreditsWidget.step");
}

void CreditsWidget::addCreditTextEntry(CreditData *credit_data, float x_pos, float y_pos) {
    debug("setting up widgets for credits: %s", credit_data->author_name.c_str());

    float y_offset = 30.0;

    TextField *author_name;
    TextField *author_date;
    TextField *author_url;

    author_name = new TextField();
    author_name->text = credit_data->author_name;;
    author_name->box.pos = Vec(x_pos, y_pos);
    author_name->box.size = Vec(200, 28);
    addChild(author_name);

    y_pos = y_pos + y_offset;

    author_date = new TextField();
    author_date->text = credit_data->author_date;;
    author_date->box.pos = Vec(x_pos, y_pos);
    author_date->box.size = Vec(200, 28);
    addChild(author_date);

    y_pos = y_pos + y_offset;

    author_url = new TextField();
    author_url->text = credit_data->author_url;
    author_url->box.pos = Vec(x_pos, y_pos);
    author_url->box.size = Vec(200, 28);
    addChild(author_url);
}

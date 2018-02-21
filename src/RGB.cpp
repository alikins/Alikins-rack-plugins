#include "alikins.hpp"

#include "util/math.hpp"

struct RGB : Module {
    enum ParamIds {
        RED_PARAM,
        GREEN_PARAM,
        BLUE_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        RED_INPUT,
        GREEN_INPUT,
        BLUE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;

    enum InputRange {
        ZERO_TEN,
        MINUS_PLUS_FIVE
    };

    InputRange inputRange = MINUS_PLUS_FIVE;
    const float in_min[2] = {0.0, -5.0};
    const float in_max[2] = {10.0, 5.0};

    enum ColorMode {
        RGB_MODE,
        HSL_MODE,
    };

    ColorMode colorMode = HSL_MODE;

    RGB() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
    void step() override;

    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

};

json_t* RGB::toJson() {
    json_t *rootJ = json_object();

    json_object_set_new(rootJ, "inputRange", json_integer(inputRange));
    json_object_set_new(rootJ, "colorMode", json_integer(colorMode));

    return rootJ;
};

void RGB::fromJson(json_t *rootJ) {
    json_t *inputRangeJ = json_object_get(rootJ, "inputRange");
    if (inputRangeJ) {
        inputRange = (InputRange) json_integer_value(inputRangeJ);
    }

    json_t *colorModeJ = json_object_get(rootJ, "colorMode");
    if (colorModeJ) {
        colorMode = (ColorMode) json_integer_value(colorModeJ);
    }
};


void RGB::step() {
    if (inputs[RED_INPUT].active) {
        float in_value = clamp(inputs[RED_INPUT].value, in_min[inputRange], in_max[inputRange]);
        red = rescale(in_value, in_min[inputRange], in_max[inputRange], 0.0f, 1.0f);
    }
    if (inputs[GREEN_INPUT].active) {
        float in_value = clamp(inputs[GREEN_INPUT].value, in_min[inputRange], in_max[inputRange]);
        green = rescale(in_value, in_min[inputRange], in_max[inputRange], 0.0f, 1.0f);
    }
    if (inputs[BLUE_INPUT].active) {
        float in_value = clamp(inputs[BLUE_INPUT].value, in_min[inputRange], in_max[inputRange]);
        blue = rescale(in_value, in_min[inputRange], in_max[inputRange], 0.0f, 1.0f);
    }
}

struct RGBPanel : TransparentWidget {
    RGB *module;
    RGB::ColorMode colorMode;

    // std::vector<CreditData*> vcredits;
    float red = 0.5f;
    float green = 0.5f;
    float blue = 0.5f;

    RGBPanel() {
    }

    void step() override {
        Widget::step();
        red = module->red;
        green = module->green;
        blue = module->blue;
        colorMode = module->colorMode;
    }

    void draw(NVGcontext *vg) override {
        // FIXME: not really red, green, blue anymore
        // debug("RgbPanel.draw red=%f, green=%f, blue=%f", red, green, blue);
        NVGcolor panelColor = nvgRGBf(red, green, blue);
        if (module->colorMode == RGB::HSL_MODE) {
            panelColor = nvgHSL(red, green, blue);
        }

        nvgBeginPath(vg);

        nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);
        nvgFillColor(vg, panelColor);
        nvgFill(vg);
  }
};


struct RGBWidget : ModuleWidget {
    RGBWidget(RGB *module);
    Menu *createContextMenu() override;
};


RGBWidget::RGBWidget(RGB *module) : ModuleWidget(module) {

    box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    setPanel(SVG::load(assetPlugin(plugin, "res/RGB.svg")));

    RGBPanel *rgbPanel = new RGBPanel();

    // FIXME: move to init/constr?
    rgbPanel->box.pos = Vec(0, 0);
    rgbPanel->box.size = box.size;
    rgbPanel->module = module;
    addChild(rgbPanel);
    debug("box.size (%f, %f)", box.size.x, box.size.y);
    addInput(Port::create<PJ301MPort>(Vec(5.0f, 340.0f),
                Port::INPUT,
                module,
                RGB::RED_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(30.0f, 340.0f),
                Port::INPUT,
                module,
                RGB::GREEN_INPUT));
    addInput(Port::create<PJ301MPort>(Vec(55.0f, 340.0f),
                Port::INPUT,
                module,
                RGB::BLUE_INPUT));

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 0.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-15, 0.0f)));

    addChild(Widget::create<ScrewSilver>(Vec(0.0f, 365.0f)));
    addChild(Widget::create<ScrewSilver>(Vec(box.size.x-15, 365.0f)));

}

struct ColorModeItem : MenuItem {

        RGB *rgb;
        RGB::ColorMode colorMode;

        void onAction(EventAction &e) override {
            rgb->colorMode = colorMode;
        };

        void step() override {
            rightText = (rgb->colorMode == colorMode)? "✔" : "";
        };

};



struct RGBRangeItem : MenuItem {

        RGB *rgb;
        RGB::InputRange inputRange;

        void onAction(EventAction &e) override {
            rgb->inputRange = inputRange;
        };

        void step() override {
            rightText = (rgb->inputRange == inputRange)? "✔" : "";
        };

};

Menu *RGBWidget::createContextMenu() {

        Menu *menu = ModuleWidget::createContextMenu();

        MenuLabel *spacerLabel = new MenuLabel();
        menu->addChild(spacerLabel);

        RGB *rgb = dynamic_cast<RGB*>(module);
        assert(rgb);

        MenuLabel *colorModeLabel = new MenuLabel();
        colorModeLabel->text = "ColorMode";
        menu->addChild(colorModeLabel);

        // FIXME: colorModeItem looks too much like colorModelItem
        ColorModeItem *rgbModeItem = new ColorModeItem();
        rgbModeItem->text = "RGB";
        rgbModeItem->rgb = rgb;
        rgbModeItem->colorMode = RGB::RGB_MODE;
        menu->addChild(rgbModeItem);

        ColorModeItem *hslModeItem = new ColorModeItem();
        hslModeItem->text = "HSL";
        hslModeItem->rgb = rgb;
        hslModeItem->colorMode = RGB::HSL_MODE;
        menu->addChild(hslModeItem);


        MenuLabel *modeLabel2 = new MenuLabel();
        modeLabel2->text = "Input Range";
        menu->addChild(modeLabel2);

        RGBRangeItem *zeroTenItem = new RGBRangeItem();
        zeroTenItem->text = "0 - +10V (uni)";
        zeroTenItem->rgb = rgb;
        zeroTenItem->inputRange = RGB::ZERO_TEN;
        menu->addChild(zeroTenItem);

        RGBRangeItem *fiveFiveItem = new RGBRangeItem();
        fiveFiveItem->text = "-5 - +5V (bi)";
        fiveFiveItem->rgb = rgb;
        fiveFiveItem->inputRange = RGB::MINUS_PLUS_FIVE;;
        menu->addChild(fiveFiveItem);

        return menu;
}


Model *modelRGB = Model::create<RGB, RGBWidget>(
        "Alikins", "RGB", "RGB Panel", UTILITY_TAG);

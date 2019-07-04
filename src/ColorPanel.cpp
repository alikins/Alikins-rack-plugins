#include "alikins.hpp"

#include "math.hpp"

struct ColorPanel : Module {
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

    float color_tup[3] = {0.0f, 0.0f, 0.0f};

    enum RGB {
        RED,
        GREEN,
        BLUE
    };

    enum HSL {
        HUE,
        SAT,
        LIGHTNESS
    };

    enum ColorMode {
        RGB_MODE,
        HSL_MODE,
    };

    ColorMode colorMode = HSL_MODE;

    ColorPanel() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(RED_PARAM,
                    0.0f, 1.0f, 0.5f, "Red");
        configParam(GREEN_PARAM,
                    0.0f, 1.0f, 0.5f, "Green");
        configParam(BLUE_PARAM,
                    0.0f, 1.0f, 0.5f, "Blue");
    }

    void process(const ProcessArgs &args) override;

    json_t *dataToJson() override;
    void dataFromJson(json_t *rootJ) override;

};

json_t* ColorPanel::dataToJson() {
    json_t *rootJ = json_object();

    json_object_set_new(rootJ, "inputRange", json_integer(inputRange));
    json_object_set_new(rootJ, "colorMode", json_integer(colorMode));

    return rootJ;
}

void ColorPanel::dataFromJson(json_t *rootJ) {
    json_t *inputRangeJ = json_object_get(rootJ, "inputRange");
    if (inputRangeJ) {
        inputRange = (InputRange) json_integer_value(inputRangeJ);
    }

    json_t *colorModeJ = json_object_get(rootJ, "colorMode");
    if (colorModeJ) {
        colorMode = (ColorMode) json_integer_value(colorModeJ);
    }

}

void ColorPanel::process(const ProcessArgs &args)
{
    if (inputs[RED_INPUT].isConnected()) {
        float in_value = clamp(inputs[RED_INPUT].getVoltage(), in_min[inputRange], in_max[inputRange]);
        red = rescale(in_value, in_min[inputRange], in_max[inputRange], 0.0f, 1.0f);
        // color_tup[RED] = red;
        // params[RED_PARAM].setValue(red);
    } else {
        red = params[RED_PARAM].getValue();
    }

    if (inputs[GREEN_INPUT].isConnected()) {
        float in_value = clamp(inputs[GREEN_INPUT].getVoltage(), in_min[inputRange], in_max[inputRange]);
        green = rescale(in_value, in_min[inputRange], in_max[inputRange], 0.0f, 1.0f);
        // color_tup[GREEN] = green;
        // params[GREEN].setValue(green);
    } else {
        green = params[GREEN_PARAM].getValue();
    }

    if (inputs[BLUE_INPUT].isConnected()) {
        float in_value = clamp(inputs[BLUE_INPUT].getVoltage(), in_min[inputRange], in_max[inputRange]);
        blue = rescale(in_value, in_min[inputRange], in_max[inputRange], 0.0f, 1.0f);
        // color_tup[BLUE] = blue;
        // params[BLUE].setValue(blue);
    } else {
        blue = params[BLUE_PARAM].getValue();
    }
}

// From Rack/src/core/Blank.cpp
struct ColorPanelModuleResizeHandle : OpaqueWidget {
    bool right = false;
    float dragX;
    Vec dragPos;

    Rect originalBox;

    ColorPanelModuleResizeHandle() {
        box.size = Vec(RACK_GRID_WIDTH * 1, RACK_GRID_HEIGHT);
        dragX = 0.0f;
    }

    // void onMouseDown(EventMouseDown &e) override {
    void onButton(const event::Button &e) override {
        if ((e.button == 0) && (e.action == GLFW_PRESS)) {
            // e.consumed = true;
            //ie.target = this;
            e.consume(this);
        }
    }

    void onDragStart(const event::DragStart &e) override {
        // dragX = gRackWidget->lastMousePos.x;
        dragX = APP->scene->rack->mousePos.x;
        ModuleWidget *m = getAncestorOfType<ModuleWidget>();
        originalBox = m->box;
    }

    void onDragMove(const event::DragMove &e) override {
        ModuleWidget *m = getAncestorOfType<ModuleWidget>();

        float newDragX = APP->scene->rack->mousePos.x;
        float deltaX = newDragX - dragX;

        Rect newBox = originalBox;
		Rect oldBox = m->box;

        const float minWidth = 6 * RACK_GRID_WIDTH;

        if (right) {
            newBox.size.x += deltaX;
            newBox.size.x = std::fmax(newBox.size.x, minWidth);
            newBox.size.x = std::round(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
        } else {
            newBox.size.x -= deltaX;
            newBox.size.x = std::fmax(newBox.size.x, minWidth);
            newBox.size.x = std::round(newBox.size.x / RACK_GRID_WIDTH) * RACK_GRID_WIDTH;
            newBox.pos.x = originalBox.pos.x + originalBox.size.x - newBox.size.x;
        }
        m->box = newBox;
        if (!APP->scene->rack->requestModulePos(m, newBox.pos)) {
            m->box = oldBox;
        }
        // gRackWidget->requestModuleBox(m, newBox);
    }
};



struct ColorFrame : TransparentWidget {

    ColorPanel *module;
    ColorPanel::ColorMode colorMode;

    // std::vector<CreditData*> vcredits;
    float red = 0.5f;
    float green = 0.5f;
    float blue = 0.5f;

    // purple-ish
    NVGcolor defaultColor = nvgRGB(93, 0, 235);
    NVGcolor panelColor = defaultColor;

    ColorFrame() {
    }

    void step() override {
        Widget::step();

        if (!module) {
            return;
        }

        if (module->colorMode == ColorPanel::HSL_MODE) {
            panelColor = nvgHSL(module->red, module->green, module->blue);
        }

        if (module->colorMode == ColorPanel::RGB_MODE) {
            panelColor = nvgRGB(module->red, module->green, module->blue);
        }

    }

	void draw(const DrawArgs &args) override {
        // FIXME: not really red, green, blue anymore
        // could include alpha
        // debug("RgbPanel.draw red=%f, green=%f, blue=%f", red, green, blue);

        nvgBeginPath(args.vg);

        nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
        nvgFillColor(args.vg, panelColor);
        nvgFill(args.vg);
  }
};

struct ColorModeItem : MenuItem {

    ColorPanel *colorPanel;
    ColorPanel::ColorMode colorMode;

    void onAction(const event::Action &e) override {
        colorPanel->colorMode = colorMode;
    };

    void step() override {
        rightText = (colorPanel->colorMode == colorMode)? "✔" : "";
    };

};



struct InputRangeItem : MenuItem {
    ColorPanel *colorPanel;
    ColorPanel::InputRange inputRange;

    void onAction(const event::Action &e) override {
        colorPanel->inputRange = inputRange;
    };

    void step() override {
        rightText = (colorPanel->inputRange == inputRange)? "✔" : "";
    };

};


struct ColorPanelWidget : ModuleWidget {
	Widget *rightHandle;
    ColorFrame *panel;

    // Menu *createContextMenu();

    void step() override;

    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

    ColorPanelWidget(ColorPanel *module) {
        setModule(module);
        box.size = Vec(6 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);

        {
            panel = new ColorFrame();
            panel->box.size = box.size;
            panel->module = module;
            addChild(panel);
        }

        ColorPanelModuleResizeHandle *leftHandle = new ColorPanelModuleResizeHandle();
        ColorPanelModuleResizeHandle *rightHandle = new ColorPanelModuleResizeHandle();
        rightHandle->right = true;
        this->rightHandle = rightHandle;
        addChild(leftHandle);
        addChild(rightHandle);

        // FIXME: how do I figure that out before creating the instance?
        float port_width = 24.672108f;
        float empty_space = box.size.x - (3.0f * port_width);
        float interstitial = empty_space / 5.0f;

        float x_pos = interstitial;
        addInput(createInput<PJ301MPort>(Vec(x_pos, 340.0f),
                    module,
                    ColorPanel::RED_INPUT));

        x_pos = x_pos + interstitial + port_width;;
        addInput(createInput<PJ301MPort>(Vec(x_pos, 340.0f),
                    module,
                    ColorPanel::GREEN_INPUT));

        x_pos = x_pos + interstitial + port_width;;
        addInput(createInput<PJ301MPort>(Vec(x_pos, 340.0f),
                    module,
                    ColorPanel::BLUE_INPUT));

    }

    void appendContextMenu(Menu *menu) override {
        MenuLabel *spacerLabel = new MenuLabel();
        menu->addChild(spacerLabel);

        ColorPanel *colorPanel = dynamic_cast<ColorPanel*>(module);
        assert(colorPanel);

        MenuLabel *colorModeLabel = new MenuLabel();
        colorModeLabel->text = "ColorMode";
        menu->addChild(colorModeLabel);

        // FIXME: colorModeItem looks too much like colorModelItem
        ColorModeItem *colorModeItem = new ColorModeItem();
        colorModeItem->text = "RGB";
        colorModeItem->colorPanel = colorPanel;
        colorModeItem->colorMode = ColorPanel::RGB_MODE;
        menu->addChild(colorModeItem);

        ColorModeItem *hslModeItem = new ColorModeItem();
        hslModeItem->text = "HSL";
        hslModeItem->colorPanel = colorPanel;
        hslModeItem->colorMode = ColorPanel::HSL_MODE;
        menu->addChild(hslModeItem);


        MenuLabel *modeLabel2 = new MenuLabel();
        modeLabel2->text = "Input Range";
        menu->addChild(modeLabel2);

        InputRangeItem *zeroTenItem = new InputRangeItem();
        zeroTenItem->text = "0 - +10V (uni)";
        zeroTenItem->colorPanel = colorPanel;
        zeroTenItem->inputRange = ColorPanel::ZERO_TEN;
        menu->addChild(zeroTenItem);

        InputRangeItem *fiveFiveItem = new InputRangeItem();
        fiveFiveItem->text = "-5 - +5V (bi)";
        fiveFiveItem->colorPanel = colorPanel;
        fiveFiveItem->inputRange = ColorPanel::MINUS_PLUS_FIVE;
        menu->addChild(fiveFiveItem);
    }
};


void ColorPanelWidget::step() {
    panel->box.size = box.size;
    rightHandle->box.pos.x = box.size.x - rightHandle->box.size.x;

    // debug("box.size (%f, %f)", box.size.x, box.size.y);
    ModuleWidget::step();
}

json_t *ColorPanelWidget::toJson() {
	json_t *rootJ = ModuleWidget::toJson();
	json_object_set_new(rootJ, "width", json_real(box.size.x));
	return rootJ;
}

void ColorPanelWidget::fromJson(json_t *rootJ) {
	ModuleWidget::fromJson(rootJ);
	json_t *widthJ = json_object_get(rootJ, "width");
    if (widthJ)
        box.size.x = json_number_value(widthJ);
}

Model *modelColorPanel = createModel<ColorPanel, ColorPanelWidget>("ColorPanel");

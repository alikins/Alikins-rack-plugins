#include <string.h>

#include "alikins.hpp"

struct BarGraphElement : Module {
    enum ParamIds {
        PITCH_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        VALUE_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        SINE_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        BLINK_LIGHT,
        NUM_LIGHTS
    };

    enum InputRange {
        MINUS_PLUS_TEN,
        ZERO_TEN,
        MINUS_PLUS_FIVE,
        ZERO_ONE,
    };

    enum LineUnit {
        NO_LINES,
        ONE_VOLT,
        HALF_VOLT,
        // one semitone CV 1.0/12.0
        ONE_SEMITONE,
        TEN_CENTS,
    };

    InputRange inputRange = MINUS_PLUS_TEN;
    LineUnit lineUnit = ONE_VOLT;

    bool showLines = false;

    float input_value = 0.0f;

    BarGraphElement() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

};

json_t* BarGraphElement::toJson() {
    json_t *rootJ = json_object();

    json_object_set_new(rootJ, "inputRange", json_integer(inputRange));
    json_object_set_new(rootJ, "showLines", json_boolean(showLines));
    json_object_set_new(rootJ, "lineUnit", json_integer(lineUnit));

    return rootJ;
}

void BarGraphElement::fromJson(json_t *rootJ) {
    json_t *inputRangeJ = json_object_get(rootJ, "inputRange");
    if (inputRangeJ) {
        inputRange = (InputRange) json_integer_value(inputRangeJ);
    }

    json_t *showLinesJ = json_object_get(rootJ, "showLines");
    if (showLinesJ) {
        showLines = json_is_true(showLinesJ);
    }

    json_t *lineUnitJ = json_object_get(rootJ, "lineUnit");
    if (lineUnitJ) {
        lineUnit = (LineUnit) json_integer_value(lineUnitJ);
    }
}

struct BarGraphWidget : VirtualWidget {
    Module *module;

    // guestimate of how tall the text will be
    // TODO: can we query that?
    float approx_text_height = 10.0f;
    float input_value = 11.1f;

    const float lineUnitValues[5] = {0.0f, 1.0f, 0.5f, 1.0f/12.0f, 1.0f/120.0f};
    const int lineUnitLargeCount[5] = {1, 5, 10, 12, 120};

    BarGraphWidget() {
    };


    void step() override {
        BarGraphElement *bar = dynamic_cast<BarGraphElement*>(module);

        input_value = clamp(module->inputs[BarGraphElement::VALUE_INPUT].value,
            voltage_min[bar->inputRange],
            voltage_max[bar->inputRange]);

        VirtualWidget::step();
    };

    void draw(NVGcontext *vg) override {
        // Leave some room for the text display
        float bar_area_height = box.size.y - approx_text_height;

        // debug("bar_area_height: %f, size.x: %f", bar_area_height, box.size.x);

        BarGraphElement *bar = dynamic_cast<BarGraphElement*>(module);

        float y_origin = bar_area_height / 2.0f;
        float bar_height_max = bar_area_height - y_origin;
        float bar_height_min = -bar_height_max;
        float div_unit = lineUnitValues[bar->lineUnit];

        // how many small lines before a periodical large line
        int large_div_count = lineUnitLargeCount[bar->lineUnit];

        float total_input_range = voltage_max[bar->inputRange] - voltage_min[bar->inputRange];
        int divs = static_cast<int>(total_input_range / div_unit);

        //debug("div_unit: %f, total_input_range: %f, lu: %d ldc: %d divs: %d",
        //    div_unit, total_input_range, bar->lineUnit, large_div_count, divs);

        if (bar->inputRange == BarGraphElement::ZERO_TEN ||
            bar->inputRange == BarGraphElement::ZERO_ONE) {

            y_origin = bar_area_height;
            bar_height_max = bar_area_height;
            bar_height_min = 0.0f;
        }

        // debug("max_d: %d size_d_f: %f n:%d d:%d x:%d y:%d r:%f b:%f", max_d, size_d_f, n, d, x, y, red, blue);

        nvgBeginPath(vg);
        float bar_height = rescale(input_value, voltage_min[bar->inputRange],
            voltage_max[bar->inputRange], bar_height_min, bar_height_max);

        float x_middle = box.size.x / 2.0f;

        // positive values are red, negative green
        float hue = input_value > 0 ? 0.0f : 0.35f;

        // make the bars slightly light for large values, just to add a touch of animation
        float sat = rescale(abs(input_value), 0.0f, 10.0f, 0.75f, .6f);
        float sat_adj = rescale(abs(input_value), 0.0f, 10.0f, 0.0, 0.05f);
        float lightness = 0.5f;

        NVGcolor barColor = nvgHSL(hue, sat+sat_adj, lightness);

        nvgRect(vg, 0.0f, y_origin, box.size.x, -bar_height);

        nvgFillColor(vg, barColor);
        nvgFill(vg);
        nvgStrokeColor(vg, nvgRGBAf(0.f, 0.f, 0.f, .5f));
        nvgStroke(vg);

        nvgFontSize(vg, 10.0f);
        nvgBeginPath(vg);

        // outline the boxes a bit to make separate
        nvgStrokeColor(vg, nvgRGBAf(0.f, 0.f, 0.f, 1.0f));

        nvgFillColor(vg, nvgRGBAf(0.f, 0.0f, 0.0f, 1.0f));

        // TODO: make text display optional, possible extract to method or widget
        std::string valueStr = stringf("%#.3g", input_value);
        nvgTextAlign(vg, NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
        nvgText(vg, x_middle, bar_area_height + (approx_text_height / 2.0f), valueStr.c_str(), NULL);
        nvgStroke(vg);
        nvgFill(vg);

        if (bar->lineUnit == BarGraphElement::NO_LINES) {
            return;
        }

        // reticular lines
        float ret_line_centers = bar_area_height / divs;

        for (int i=0; i<=divs; i++) {
            nvgBeginPath(vg);

            float y_line = 0.0f + (i * ret_line_centers);
            float wideness = 10.0f;

            // draw a larger bolder line for larger units
            if (i % large_div_count == 0) {
                // nvgStrokeColor(vg, nvgRGBAf(0.f, 0.f, 0.f, 0.2f));
                wideness = 5.0f;
            }

            // debug("wideness: %f, y_line: %f, x-w: %f",
            //    wideness, y_line, box.size.x - wideness);
            nvgMoveTo(vg, wideness, y_line);
            nvgLineTo(vg, box.size.x - wideness, y_line);

            if (i % large_div_count == 0) {
                nvgStrokeColor(vg, nvgRGBAf(0.f, 0.f, 0.f, 0.2f));
            } else {
                nvgStrokeColor(vg, nvgRGBAf(0.f, 0.f, 0.f, 0.1f));
            }

            nvgStroke(vg);

        }
    }
};

struct BarGraphElementWidget : ModuleWidget {
    // another reinvented quantitywidget
    float input_value = -13.0f;

    BarGraphWidget *barGraphWidget = NULL;
    Menu *createContextMenu() override;

    BarGraphElementWidget(BarGraphElement *module) : ModuleWidget(module) {
        setPanel(SVG::load(assetPlugin(plugin, "res/BarGraphElement.svg")));

        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
        addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

        Port *input = Port::create<PJ301MPort>(Vec(33, box.size.y), Port::INPUT, module, BarGraphElement::VALUE_INPUT);

        input->box.pos.x = 2.0f;
        input->box.pos.y = box.size.y - input->box.size.y - 20.0f;
        addInput(input);

        barGraphWidget = Widget::create<BarGraphWidget>(Vec(0.0f, 0.0f));
        barGraphWidget->box.pos.y = box.pos.y;
        barGraphWidget->box.pos.y = 15.0f;
        barGraphWidget->box.size.x = box.size.x;
        barGraphWidget->box.size.y = box.size.y - 60.0f;
        barGraphWidget->module = module;
        addChild(barGraphWidget);
    }
};

struct InputRangeItem : MenuItem {
    BarGraphElement *bar;
    BarGraphElement::InputRange inputRange;

    void onAction(EventAction &e) override {
        bar->inputRange = inputRange;
    };

    void step() override {
        rightText = (bar->inputRange == inputRange)? "✔" : "";
    };
};

struct LinesItem : MenuItem {
    BarGraphElement *bar;

    void onAction(EventAction &e) override {
        bar->showLines = !bar->showLines;
    };

    void step() override {
        rightText = bar->showLines ? "✔" : "";
    };
};

struct LineUnitItem : MenuItem {
    BarGraphElement *bar;
    BarGraphElement::LineUnit lineUnit;

    void onAction(EventAction &e) override {
        bar->lineUnit = lineUnit;
    };

    void step() override {
        rightText = (bar->lineUnit == lineUnit)? "✔" : "";
    };
};

Menu *BarGraphElementWidget::createContextMenu() {

    Menu *menu = ModuleWidget::createContextMenu();

    MenuLabel *spacerLabel = new MenuLabel();
    menu->addChild(spacerLabel);

    BarGraphElement *bar = dynamic_cast<BarGraphElement*>(module);
    assert(bar);
    assert(module);

    MenuLabel *modeLabel2 = new MenuLabel();
    modeLabel2->text = "Input Range";
    menu->addChild(modeLabel2);

    InputRangeItem *tenTenItem = new InputRangeItem();
    tenTenItem->text = "-10V - +10V";
    tenTenItem->bar = bar;
    tenTenItem->inputRange = BarGraphElement::MINUS_PLUS_TEN;
    menu->addChild(tenTenItem);

    InputRangeItem *zeroTenItem = new InputRangeItem();
    zeroTenItem->text = "0 - +10V (uni)";
    zeroTenItem->bar = bar;
    zeroTenItem->inputRange = BarGraphElement::ZERO_TEN;
    menu->addChild(zeroTenItem);

    InputRangeItem *fiveFiveItem = new InputRangeItem();
    fiveFiveItem->text = "-5 - +5V (bi)";
    fiveFiveItem->bar = bar;
    fiveFiveItem->inputRange = BarGraphElement::MINUS_PLUS_FIVE;
    menu->addChild(fiveFiveItem);

    InputRangeItem *zeroOneItem = new InputRangeItem();
    zeroOneItem->text = "0 - +1V";
    zeroOneItem->bar = bar;
    zeroOneItem->inputRange = BarGraphElement::ZERO_ONE;
    menu->addChild(zeroOneItem);

    MenuLabel *lineSpacerLabel = new MenuLabel();
    menu->addChild(lineSpacerLabel);

    MenuLabel *markingLinesLabel = new MenuLabel();
    markingLinesLabel->text = "Marking lines";
    menu->addChild(markingLinesLabel);

    LineUnitItem *noLinesItem = new LineUnitItem();
    noLinesItem->text = "No lines";
    noLinesItem->bar = bar;
    noLinesItem->lineUnit = BarGraphElement::NO_LINES;
    menu->addChild(noLinesItem);

    LineUnitItem *oneVoltLinesItem = new LineUnitItem();
    oneVoltLinesItem->text = "1V";
    oneVoltLinesItem->bar = bar;
    oneVoltLinesItem->lineUnit = BarGraphElement::ONE_VOLT;
    menu->addChild(oneVoltLinesItem);

    LineUnitItem *oneSemitoneLinesItem = new LineUnitItem();
    oneSemitoneLinesItem->text = "1 CV semitone";
    oneSemitoneLinesItem->bar = bar;
    oneSemitoneLinesItem->lineUnit = BarGraphElement::ONE_SEMITONE;
    menu->addChild(oneSemitoneLinesItem);

    return menu;
}

Model *modelBarGraphElement = Model::create<BarGraphElement, BarGraphElementWidget>("Alikins", "BarGraphElement", "Bar Graph Element - visualize a value", VISUAL_TAG, UTILITY_TAG);

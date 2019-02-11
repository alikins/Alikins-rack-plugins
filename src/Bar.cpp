#include <string.h>

#include "alikins.hpp"

struct Bar : Module {
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
        MINUS_PLUS_FIVE
    };

    InputRange inputRange = MINUS_PLUS_TEN;

	float input_value = 0.0f;

	Bar() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}

    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;

};

json_t* Bar::toJson() {
    json_t *rootJ = json_object();

    json_object_set_new(rootJ, "inputRange", json_integer(inputRange));

    return rootJ;
}

void Bar::fromJson(json_t *rootJ) {
    json_t *inputRangeJ = json_object_get(rootJ, "inputRange");
    if (inputRangeJ) {
        inputRange = (InputRange) json_integer_value(inputRangeJ);
    }

}

struct BarGraphWidget : VirtualWidget {
	Module *module;

	// guestimate of how tall the text will be
	// TODO: can we query that?
	float approx_text_height = 10.0f;
	float input_value = 11.1f;

	BarGraphWidget() {
	};


	void step() override {
		Bar *bar = dynamic_cast<Bar*>(module);

		input_value = clamp(module->inputs[Bar::VALUE_INPUT].value,
			voltage_min[bar->inputRange],
			voltage_max[bar->inputRange]);

		VirtualWidget::step();
	};

	void draw(NVGcontext *vg) override {
		nvgBeginPath(vg);

		// Leave some room for the text display
		float bar_area_height = box.size.y - approx_text_height;

		Bar *bar = dynamic_cast<Bar*>(module);

		float y_origin = bar_area_height / 2.0f;
		float bar_height_max = bar_area_height - y_origin;
		float bar_height_min = -bar_height_max;

		if (bar->inputRange == Bar::ZERO_TEN) {
			y_origin = bar_area_height;
			bar_height_max = bar_area_height;
			bar_height_min = 0.0f;
		}

		// debug("max_d: %d size_d_f: %f n:%d d:%d x:%d y:%d r:%f b:%f", max_d, size_d_f, n, d, x, y, red, blue);

		float bar_height = rescale(input_value, voltage_min[bar->inputRange], voltage_max[bar->inputRange], bar_height_min, bar_height_max);

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
		// snprintf(value, 100, "Velocity: %06.3fV (Midi %03d)", displayVelocity * 10.f, (int)(127 * displayVelocity));
		std::string valueStr = stringf("%#.3g", input_value);
    	nvgTextAlign(vg, NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
    	nvgText(vg, x_middle, bar_area_height + (approx_text_height / 2.0f), valueStr.c_str(), NULL);
		nvgStroke(vg);
    	nvgFill(vg);

	}
};

struct BarWidget : ModuleWidget {
	// another reinvented quantitywidget
	float input_value = -13.0f;

	BarGraphWidget *barGraphWidget = NULL;
	Menu *createContextMenu() override;

	BarWidget(Bar *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Bar.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		Port *input = Port::create<PJ301MPort>(Vec(33, box.size.y), Port::INPUT, module, Bar::VALUE_INPUT);

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
    Bar *bar;
    Bar::InputRange inputRange;

    void onAction(EventAction &e) override {
        bar->inputRange = inputRange;
    };

    void step() override {
        rightText = (bar->inputRange == inputRange)? "✔" : "";
    };

};

Menu *BarWidget::createContextMenu() {

    Menu *menu = ModuleWidget::createContextMenu();

    MenuLabel *spacerLabel = new MenuLabel();
    menu->addChild(spacerLabel);

    Bar *bar = dynamic_cast<Bar*>(module);
    assert(bar);
	assert(module);

    MenuLabel *modeLabel2 = new MenuLabel();
    modeLabel2->text = "Input Range";
    menu->addChild(modeLabel2);

	InputRangeItem *tenTenItem = new InputRangeItem();
    tenTenItem->text = "-10V - +10V";
    tenTenItem->bar = bar;
    tenTenItem->inputRange = Bar::MINUS_PLUS_TEN;
    menu->addChild(tenTenItem);

    InputRangeItem *zeroTenItem = new InputRangeItem();
    zeroTenItem->text = "0 - +10V (uni)";
    zeroTenItem->bar = bar;
    zeroTenItem->inputRange = Bar::ZERO_TEN;
    menu->addChild(zeroTenItem);

    InputRangeItem *fiveFiveItem = new InputRangeItem();
    fiveFiveItem->text = "-5 - +5V (bi)";
    fiveFiveItem->bar = bar;
    fiveFiveItem->inputRange = Bar::MINUS_PLUS_FIVE;
    menu->addChild(fiveFiveItem);

    return menu;
}

// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelBar = Model::create<Bar, BarWidget>("Alikins", "BarGraphElement", "Bar Graph Element - visualize a value", VISUAL_TAG, UTILITY_TAG);

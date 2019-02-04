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
	void step() override;

};

void Bar::step() {
	/*
	if (inputs[VALUE_INPUT].active) {
        input_value = clamp(inputs[VALUE_INPUT].value, voltage_min[inputRange], voltage_max[inputRange]);
	}
	*/
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
		debug("BarGraphWidget.step() input_value:%f", input_value);
		VirtualWidget::step();
	};


	// need an onChange? can set dirty from onChange?  if
	// we make this a paramwidget/quantitywidget

	void draw(NVGcontext *vg) override {
		//debug("dirty: %d", dirty);

		debug("draw value: %f", input_value);
		// drawCount++;

		nvgBeginPath(vg);
		// nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);
		//float size = module->inputs[Bar::VALUE_INPUT].value;

		float drawing_area_height = box.size.y;
		// Leave some room for the text display
		float bar_area_height = box.size.y - approx_text_height;

		Bar *bar = dynamic_cast<Bar*>(module);

		float y_origin = bar_area_height / 2.0f;
		float bar_height = bar_area_height - y_origin;

		if (bar->inputRange == Bar::ZERO_TEN) {
			// y_origin = box.size.y;
			y_origin = bar_area_height;
			bar_height = bar_area_height;
		}

		// debug("max_d: %d size_d_f: %f n:%d d:%d x:%d y:%d r:%f b:%f", max_d, size_d_f, n, d, x, y, red, blue);

		// float half_box_height = bar_area_height - y_origin;
		// debug("box.size.y: %f b_a_h: %f y_origin: %f", box.size.y, bar_area_height, y_origin);
		float clamped_input_value = clamp(input_value, voltage_min[bar->inputRange], voltage_max[bar->inputRange]);
		float box_height = rescale(clamped_input_value, voltage_min[bar->inputRange], voltage_max[bar->inputRange], -bar_height, bar_height);


		// debug("input: %f box_height: %f", size, box_height);
		float x_middle = box.size.x / 2.0f;

		// positive values are red, negative green
		float hue = clamped_input_value > 0 ? 0.0f : 0.35f;

		float sat = rescale(abs(clamped_input_value), 0.0f, 10.0f, 0.75f, .6f);
		float sat_adj = rescale(abs(clamped_input_value), 0.0f, 10.0f, 0.0, 0.05f);
		float lightness = 0.5f;
		NVGcolor barColor = nvgHSL(hue, sat+sat_adj, lightness);

		nvgRect(vg, 0.0f, y_origin, box.size.x, -box_height);

		nvgFillColor(vg, barColor);
		nvgFill(vg);
		nvgStrokeColor(vg, nvgRGBAf(0.f, 0.f, 0.f, .5f));
		nvgStroke(vg);

		// TODO: make text display optional, possible extract to method or widget
		// snprintf(value, 100, "Velocity: %06.3fV (Midi %03d)", displayVelocity * 10.f, (int)(127 * displayVelocity));
		std::string valueStr = stringf("%#.3g", clamped_input_value);
		nvgFontSize(vg, 10.0f);
		nvgBeginPath(vg);
    	nvgStrokeColor(vg, nvgRGBAf(0.f, 0.f, 0.f, 1.0f));
    	// nvgFillColor(vg, nvgRGBAf(1.f, 1.0f, 1.30f, 1.0f));
		nvgFillColor(vg, nvgRGBAf(0.f, 0.0f, 0.0f, 1.0f));
    	nvgTextAlign(vg, NVG_ALIGN_CENTER|NVG_ALIGN_MIDDLE);
		// nvgText()
    	nvgText(vg, x_middle, bar_area_height + (approx_text_height / 2.0f), valueStr.c_str(), NULL);
		nvgStroke(vg);
    	nvgFill(vg);

	}
};

struct BarGraphFrameBuffer : FramebufferWidget {
	Module *module;
	int stepCount = 0;
	int drawCount = 0;

	// another reinvented quantitywidget
	float input_value = -13.0f;
	float oldValue = -11.0f;

	BarGraphWidget *barGraphWidget = NULL;

	BarGraphFrameBuffer() {
		barGraphWidget = Widget::create<BarGraphWidget>(Vec(0.0f, 0.0f));
		barGraphWidget->box.pos.y = box.pos.y;
        barGraphWidget->box.size.x = box.size.x;
		barGraphWidget->box.size.y = box.size.y;
		barGraphWidget->module = module;
		addChild(barGraphWidget);

		dirty = true;
	};

	void inputChanged() {
		debug("inputChanged old: %f input_value: %f dirty: %d", oldValue, input_value, dirty);
		dirty = true;
		oldValue = input_value;
		barGraphWidget->input_value = input_value;
		// FramebufferWidget::step();
	};

	void step() override {
		Bar *bar = dynamic_cast<Bar*>(module);

		input_value = clamp(module->inputs[Bar::VALUE_INPUT].value,
			voltage_min[bar->inputRange],
			voltage_max[bar->inputRange]);

		stepCount++;

		if (!isNear(oldValue, input_value, 1.0e-2f)){
			inputChanged();
		} else {
			// dirty = false;
		}

		FramebufferWidget::step();
	};

	void draw(NVGcontext *vg) override {
		debug("BarGraphFrameBuffer::draw dirty:%d", dirty);
		FramebufferWidget::draw(vg);
		// Widget::draw(vg);
		debug("BarGraphFrameBuffer::draw2 dirty:%d", dirty);
	};

};


struct BarWidget : ModuleWidget {
	Menu *createContextMenu() override;

	BarWidget(Bar *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Bar.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		// addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(28, 87), module, Bar::PITCH_PARAM, -3.0, 3.0, 0.0));

		Port *input = Port::create<PJ301MPort>(Vec(33, box.size.y), Port::INPUT, module, Bar::VALUE_INPUT);

		input->box.pos.x = 2.0f;
		input->box.pos.y = box.size.y - input->box.size.y - 20.0f;
		addInput(input);

		// addOutput(Port::create<PJ301MPort>(Vec(33, 275), Port::OUTPUT, module, Bar::SINE_OUTPUT));

		// BarGraphWidget *barGraphWidget = new BarGraphWidget();
		// BarGraphFrameBuffer *barGraphFrameBuffer = Widget::create<BarGraphFrameBuffer>(Vec(0.0f, 0.0f));
		BarGraphFrameBuffer *barGraphFrameBuffer = new BarGraphFrameBuffer();

		barGraphFrameBuffer->box.pos.y = 15.0f;
        barGraphFrameBuffer->box.size.x = box.size.x;
		barGraphFrameBuffer->box.size.y = box.size.y - 60.0f;
		barGraphFrameBuffer->module = module;
		addChild(barGraphFrameBuffer);
		barGraphFrameBuffer->dirty = true;
		// addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(41, 59), module, Bar::BLINK_LIGHT));
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
Model *modelBar = Model::create<Bar, BarWidget>("Alikins", "Bar", "Bar", OSCILLATOR_TAG);

#include <string.h>

#include "alikins.hpp"



struct Bar : Module {
	enum ParamIds {
		PITCH_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		PITCH_INPUT,
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

	float phase = 0.0;
	float blinkPhase = 0.0;

	Bar() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS) {}
	void step() override;

};


void Bar::step() {
}


//rotate/flip a quadrant appropriately
void rot(int n, int *x, int *y, int rx, int ry) {
    if (ry == 0) {
        if (rx == 1) {
            *x = n-1 - *x;
            *y = n-1 - *y;
        }

        //Swap x and y
        int t  = *x;
        *x = *y;
        *y = t;
    }
}

int xy2d (int n, int x, int y) {
    int rx, ry, s, d=0;
    for (s=n/2; s>0; s/=2) {
        rx = (x & s) > 0;
        ry = (y & s) > 0;
        d += s * s * ((3 * rx) ^ ry);
        rot(s, &x, &y, rx, ry);
    }
    return d;
}

//convert d to (x,y)
void d2xy(int n, int d, int *x, int *y) {
    int rx, ry, s, t=d;
    *x = *y = 0;
    for (s=1; s<n; s*=2) {
        rx = 1 & (t/2);
        ry = 1 & (t ^ rx);
        rot(s, x, y, rx, ry);
        *x += s * rx;
        *y += s * ry;
        t /= 4;
    }
}


struct BarGraphWidget : FramebufferWidget {
	Module *module;
	int stepCount = 0;
	int drawCount = 0;

	// another reinvented quantitywidget
	float value = -13.0f;
	float oldValue = -11.0f;

	// guestimate of how tall the text will be
	// TODO: can we query that?
	float approx_text_height = 10.0f;

	BarGraphWidget() {
	};

	void step() override {
		value = module->inputs[Bar::PITCH_INPUT].value;

		// TODO: figure out if we are dirty... weird
		// TODO: though, probably should do this is Module.step() instead of widget
		//       store value that we last rendered?
		// if so, update any values if changed, tweak dirty, then FramebufferWidget::step()
		if (!isNear(oldValue, value, 1.0e-2f) | dirty){
			dirty = true;
			// debug("changing value old: %f new: %f diff: %f", oldValue, value, oldValue-value);
		}

		// TODO: do we need this?
		FramebufferWidget::step();
		// debug("step %d", stepCount);
		stepCount++;
		oldValue = value;
		// dirty = false;
	};

	// need an onChange? can set dirty from onChange?  if
	// we make this a paramwidget/quantitywidget

	void draw(NVGcontext *vg) override {
		// debug("draw %d value: %f", drawCount, value);
		drawCount++;

		nvgBeginPath(vg);
		// nvgRect(vg, 0.0, 0.0, box.size.x, box.size.y);
		float size = module->inputs[Bar::PITCH_INPUT].value;

		float drawing_area_height = box.size.y;
		// Leave some room for the text display
		float bar_area_height = box.size.y - approx_text_height;

		// For unipolar / all positive values
		// float y_origin = box.size.y;

		// For +/- value draw y origin (0) in center of box
		//y_origin = box.size.y / 2.0f;
		float y_origin = bar_area_height / 2.0f;
		int x = 0;
		int y = 0;
		// int d = floor(abs(size));
		int n = 64;

		int max_d = xy2d(n, n-1, n-1);
		float size_d_f = rescale(size, -10.0f, 10.0f, 0.0f, max_d);
		// int d = floor(abs(size_d_f));
		int d = round(size_d_f);
		d2xy(n, d, &x, &y);
		// debug("max_d: %d size_d_f: %f n:%d d:%d x:%d y:%d", max_d, size_d_f, n, d, x, y);

		float red = rescale(x, 0, n, 0.0f, 1.0f);
		float blue = rescale(y, 0, n, 0.0f, 1.0f);

		// debug("max_d: %d size_d_f: %f n:%d d:%d x:%d y:%d r:%f b:%f", max_d, size_d_f, n, d, x, y, red, blue);

		float half_box_height = bar_area_height - y_origin;
		// debug("box.size.y: %f b_a_h: %f y_origin: %f", box.size.y, bar_area_height, y_origin);
		float box_height = rescale(size, -10.0f, 10.0f, -half_box_height, half_box_height);
		// debug("input: %f box_height: %f", size, box_height);
		float x_middle = box.size.x / 2.0f;

		/*
		NVGcolor backgroundColor = nvgRGBf(.75f, 0.75f, 0.75f);
		nvgRect(vg, 0.0f, -10.0f, box.size.x, 60.0f);
		nvgFillColor(vg, backgroundColor);
		nvgFill(vg);
		*/
		// nvgRect(vg, 0.0f, 0.0f, box.size.x, bar_area_height);
		//NVGcolor barColor = nvgRGBf(red, 0.5f, blue);

		float hue = size > 0 ? 0.0f : 0.35f;

		float sat = rescale(abs(size), 0.0f, 10.0f, 0.75f, .6f);
		float sat_adj = rescale(abs(size), 0.0f, 10.0f, 0.0, 0.05f);
		float lightness = 0.5f;
		//float lightness = rescale(abs(size), 0.0f, 10.0f, 0.6f, 0.5f);
		NVGcolor barColor = nvgHSL(hue, sat+sat_adj, lightness);

		/* kind of nice, although hue center seems wrong
		float hue = rescale(size, -10.0f, 10.0f, 0.0f, 1.0f);
		float sat = 0.5f;
		float lightness = 0.5f;
		NVGcolor barColor = nvgHSL(hue, sat, lightness);
		*/

		/* HSL doesnt seem to work well with a 2d hilbert, ends up with
		   the brightness flashing
		float hue = red;
		float sat = 0.5f;
		float lightness = 0.25f;
		NVGcolor barColor = nvgHSL(hue, sat, lightness);
		*/

		nvgRect(vg, 0.0f, y_origin, box.size.x, -box_height);

		nvgFillColor(vg, barColor);
		nvgFill(vg);
		nvgStrokeColor(vg, nvgRGBAf(0.f, 0.f, 0.f, .5f));
		nvgStroke(vg);

		// TODO: make text display optional, possible extract to method or widget
		// snprintf(value, 100, "Velocity: %06.3fV (Midi %03d)", displayVelocity * 10.f, (int)(127 * displayVelocity));
		std::string valueStr = stringf("%#.3g", size);
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

		// debug("box.size x:%f y:%f", box.size.x, box.size.y);
		// FramebufferWidget::draw(vg);
	}
};



struct BarWidget : ModuleWidget {
	BarWidget(Bar *module) : ModuleWidget(module) {
		setPanel(SVG::load(assetPlugin(plugin, "res/Bar.svg")));

		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(Widget::create<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(Widget::create<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));


		// addParam(ParamWidget::create<Davies1900hBlackKnob>(Vec(28, 87), module, Bar::PITCH_PARAM, -3.0, 3.0, 0.0));

		Port *input = Port::create<PJ301MPort>(Vec(33, box.size.y), Port::INPUT, module, Bar::PITCH_INPUT);

		input->box.pos.x = 2.0f;
		input->box.pos.y = box.size.y - input->box.size.y - 20.0f;
		addInput(input);

		// addOutput(Port::create<PJ301MPort>(Vec(33, 275), Port::OUTPUT, module, Bar::SINE_OUTPUT));

		// BarGraphWidget *barGraphWidget = new BarGraphWidget();
		BarGraphWidget *barGraphWidget = Widget::create<BarGraphWidget>(Vec(0.0f, 0.0f));
		barGraphWidget->box.pos.y = 15.0f;
        barGraphWidget->box.size = box.size;
		barGraphWidget->box.size.y = box.size.y - 60.0f;
		barGraphWidget->module = module;
		addChild(barGraphWidget);
		// addChild(ModuleLightWidget::create<MediumLight<RedLight>>(Vec(41, 59), module, Bar::BLINK_LIGHT));
	}
};


// Specify the Module and ModuleWidget subclass, human-readable
// author name for categorization per plugin, module slug (should never
// change), human-readable module name, and any number of tags
// (found in `include/tags.hpp`) separated by commas.
Model *modelBar = Model::create<Bar, BarWidget>("Alikins", "Bar", "Bar", OSCILLATOR_TAG);

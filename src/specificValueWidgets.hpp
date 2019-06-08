#pragma once

// #include "rack.hpp"
#include "alikins.hpp"
// using namespace rack;

struct PurpleTrimpot : Trimpot {
    Module *module;
    bool initialized = false;
    PurpleTrimpot();
    void step() override;
    // void reset() override;
    // void randomize() override;
};

PurpleTrimpot::PurpleTrimpot() : Trimpot() {
    setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PurpleTrimpot.svg")));
    shadow->blurRadius = 0.0;
    shadow->opacity = 0.10;
    shadow->box.pos = Vec(0.0, box.size.y * 0.05);
}


// FIXME: if we are getting moving inputs and we are hovering
//        over the trimpot, we kind of jitter arround.
// maybe run this via an onChange()?
void PurpleTrimpot::step() {
	// debug("paramId=%d this->initialized: %d initialized: %d this->value: %f value: %f param.value: %f",
     // paramId,  this->initialized, initialized, this->value, value, module->params[paramId].getValue());
    /*
    if (this->value != module->params[paramId].getValue()) {
		if (this != gHoveredWidget && this->initialized) {
			// this->value = module->params[paramId].getValue();
			setValue(module->params[paramId].getValue());
		} else {
			module->params[paramId].getValue() = this->value;
            this->initialized |= true;
		}
        event::Change e;
		onChange(e);
	}
   */
	Trimpot::step();
}

/*
void PurpleTrimpot::reset() {
    this->initialized = false;
    Trimpot::reset();
    }
*/

/*
void PurpleTrimpot::randomize() {
    reset();
    float value = math::rescale(random::uniform(), 0.f, 1.f, paramQuantity->getMinValue(), paramQuantity->getMaxValue());
    paramQuantity->setValue(value);
    // setValue(rescale(random::uniform(), 0.0f, 1.0f, minValue, maxValue));
}
*/

#pragma once

#include "alikins.hpp"
//using namespace rack;
//#include "rack.hpp"
// #include "ui.hpp"

// TODO/FIXME: This is more or less adhoc TextField mixed with QuantityWidget
//             just inherit from both?
struct ParamFloatField : TextField
{
    Module *module;
    float hovered_value;

    ParamFloatField(Module *module);

    void setValue(float value);
    void onChange(const event::Change &e) override;

};

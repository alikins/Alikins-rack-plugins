#include "rack.hpp"
#include "ui.hpp"
#include "ParamFloatField.hpp"

ParamFloatField::ParamFloatField(Module *_module)
{
    module = _module;
}

void ParamFloatField::setValue(float value) {
    this->hovered_value = value;
    // this->module->param_value = value;
    event::Change e;
    onChange(e);
}

void ParamFloatField::onChange(const event::Change &e) {
    std::string new_text = string::f("%#.4g", hovered_value);
    setText(new_text);
}

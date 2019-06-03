#include "alikins.hpp"

// The pluginInstance-wide instance of the Plugin class
Plugin *pluginInstance;

void init(rack::Plugin *p) {
    pluginInstance = p;

    p->addModel(modelBigMuteButton);
    p->addModel(modelGateLength);
    p->addModel(modelIdleSwitch);
    //p->addModel(modelMomentaryOnButtons);
    //p->addModel(modelReference);
    // p->addModel(modelShiftPedal);
    // p->addModel(modelValueSaver);
    // p->addModel(modelSpecificValue);
    // p->addModel(modelColorPanel);
    // p->addModel(modelHoveredValue);
    // p->addModel(modelInjectValue);
}

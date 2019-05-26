#include "alikins.hpp"

// The pluginInstance-wide instance of the Plugin class
Plugin *pluginInstance;

void init(rack::Plugin *p) {
    pluginInstance = p;

    p->website = "https://github.com/alikins/Alikins-rack-plugins";
    p->manual = "https://github.com/alikins/Alikins-rack-plugins/blob/master/README.md";

    p->addModel(modelBigMuteButton);
    p->addModel(modelColorPanel);
    p->addModel(modelGateLength);
    p->addModel(modelIdleSwitch);
    p->addModel(modelMomentaryOnButtons);
    p->addModel(modelReference);
    p->addModel(modelHoveredValue);
    p->addModel(modelInjectValue);
    p->addModel(modelShiftPedal);
    p->addModel(modelSpecificValue);
    p->addModel(modelValueSaver);
}

#include "alikins.hpp"

// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
    plugin = p;
    // This is the unique identifier for your plugin
    p->slug = TOSTRING(SLUG);
    p->version = TOSTRING(VERSION);

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
    p->addModel(modelBarGraphElement);
}

#include "alikins.hpp"


// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
    plugin = p;
    // This is the unique identifier for your plugin
    p->slug = "Alikins";
#ifdef VERSION
    p->version = TOSTRING(VERSION);
#endif
    p->website = "https://github.com/alikins/Alikins-rack-plugins";
    p->manual = "https://github.com/alikins/alikins-rack-plugins/blob/master/README.md";

    p->addModel(modelIdleSwitch);
    p->addModel(modelMomentaryOnButtons);
    // Any other plugin initialization may go here.
    // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

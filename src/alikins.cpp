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

    p->addModel(createModel<MomentaryOnButtonsWidget>("Alikins", "MomentaryOnButtons", "Momentary On Buttons", UTILITY_TAG));

    p->addModel(createModel<IdleSwitchWidget>("Alikins", "IdleSwitch", "Idle Switch", SWITCH_TAG, UTILITY_TAG));

    p->addModel(createModel<BigMuteButtonWidget>("Alikins", "BigMuteButton", "Big Mute Button", UTILITY_TAG));

    // Any other plugin initialization may go here.
    // As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

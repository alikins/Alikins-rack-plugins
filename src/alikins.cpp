#include "alikins.hpp"


// The plugin-wide instance of the Plugin class
Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	// This is the unique identifier for your plugin
	p->slug = "alikins";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/alikins/rack-plugins";
	p->manual = "https://github.com/alikins/rack-plugins/blob/master/README.md";

	// For each module, specify the ModuleWidget subclass, manufacturer slug (for saving in patches), manufacturer human-readable name, module slug, and module name
	// p->addModel(createModel<MyModuleWidget>("Tutorial", "MyModule", "My Module", OSCILLATOR_TAG));
	p->addModel(createModel<MomentaryOnButtonsWidget>("alikins", "MomentaryOnButtons", "MomentaryOnButtons", OSCILLATOR_TAG));

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}

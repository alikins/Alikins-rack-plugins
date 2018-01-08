#include "rack.hpp"


const int MOMENTARY_BUTTONS = 8;

using namespace rack;


extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

struct MomentaryOnButtonsWidget : ModuleWidget {
	MomentaryOnButtonsWidget();
};

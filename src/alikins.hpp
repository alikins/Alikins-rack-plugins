#include "rack.hpp"

const int MOMENTARY_BUTTONS = 13;
const int INPUT_SOURCES = 1;
using namespace rack;


extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

struct MomentaryOnButtonsWidget : ModuleWidget {
    MomentaryOnButtonsWidget();
};

struct IdleSwitchWidget : ModuleWidget {
    IdleSwitchWidget();
};

struct CreditsWidget : ModuleWidget {
    CreditsWidget();
};


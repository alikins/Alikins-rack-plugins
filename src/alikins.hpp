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

struct BigMuteButtonWidget : ModuleWidget {
    BigMuteButtonWidget();
};

struct ColorPanelWidget : ModuleWidget {
    Panel *panel;
    ColorPanelWidget();
	Widget *rightHandle;
    TransparentWidget *color_frame;

    Menu *createContextMenu() override;

    void step() override;

    json_t *toJson() override;
    void fromJson(json_t *rootJ) override;
};


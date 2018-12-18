#include "rack.hpp"

const int MOMENTARY_BUTTONS = 13;
const int INPUT_SOURCES = 1;
const int GATE_LENGTH_INPUTS = 5;

enum VoltageRange {
    ZERO_TEN,
    MINUS_PLUS_FIVE,
    MINUS_PLUS_TEN,
};

const float voltage_min[3] = {0.0f, -5.0f, -10.0f};
const float voltage_max[3] = {10.0f, 5.0f, 10.0f};

using namespace rack;


extern Plugin *plugin;

////////////////////
// module widgets
////////////////////

extern Model *modelIdleSwitch;
extern Model *modelMomentaryOnButtons;
extern Model *modelBigMuteButton;
extern Model *modelColorPanel;
extern Model *modelGateLength;
extern Model *modelSpecificValue;
extern Model *modelReference;
extern Model *modelHoveredValue;
extern Model *modelInjectValue;

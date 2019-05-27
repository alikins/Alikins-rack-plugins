#include <math.h>

#include "rack0.hpp"

const int MOMENTARY_BUTTONS = 13;
const int INPUT_SOURCES = 1;
const int GATE_LENGTH_INPUTS = 5;

enum VoltageRange {
    MINUS_PLUS_TEN,
    ZERO_TEN,
    MINUS_PLUS_FIVE,
};

const float voltage_min[3] = {-10.0f, 0.0f, -5.0f};
const float voltage_max[3] = {10.0f, 10.0f, 5.0f};

using namespace rack;


extern Plugin *pluginInstance;

////////////////////
// module widgets
////////////////////

extern Model *modelBigMuteButton;
extern Model *modelGateLength;
extern Model *modelIdleSwitch;
extern Model *modelMomentaryOnButtons;
extern Model *modelReference;
// extern Model *modelShiftPedal;
extern Model *modelValueSaver;

/*
extern Model *modelHoveredValue;
extern Model *modelInjectValue;
extern Model *modelColorPanel;
extern Model *modelSpecificValue;
*/

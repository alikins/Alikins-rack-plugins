#include "rack.hpp"

const int MOMENTARY_BUTTONS = 13;
const int INPUT_SOURCES = 1;
const int GATE_LENGTH_INPUTS = 5;
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


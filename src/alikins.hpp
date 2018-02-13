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

struct CreditData {
    std::string author_name;
    std::string author_date;
    std::string author_url;

};

struct CreditsWidget : ModuleWidget {
    CreditsWidget();
    void addCreditTextEntry(CreditData *credit_data, float x_pos, float y_pos);

    void step() override;
};


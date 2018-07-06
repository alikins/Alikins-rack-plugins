#include "rack.hpp"
using namespace rack;

#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

// #include "prettyprint.hpp"

// using namespace rack;

std::vector<std::string> note_name_vec = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

std::unordered_map<std::string, float> gen_note_name_map() {
    float volt = -10.0f;
    std::string note_name;
    std::unordered_map<std::string, float> note_name_map;
    std::vector<std::string>::iterator it;

    // FIXME: add a map of note name (including enharmonic) to voltage offset from C
    //        then just iterate over it for each octave
    for (int i = -6; i <= 14; i++)
    {
        for (int j = 0; j < 12; j++)
        {
            // debug("oct=%d note=%s volt=%f ", i, note_name_vec[j].c_str(), volt);
            note_name_map[stringf("%s%d",
                                  note_name_vec[j].c_str(), i)] = volt;
            volt += (1.0f / 12.0f);
        }
    }
    return note_name_map;
}

std::unordered_map<std::string, std::string> gen_enharmonic_name_map() {
    std::unordered_map<std::string, std::string> enharmonic_map;

    enharmonic_map["c"] = "C";
    enharmonic_map["C"] = "C";

    enharmonic_map["C#"] = "C#";
    enharmonic_map["c#"] = "C#";
    enharmonic_map["Db"] = "C#";
    enharmonic_map["db"] = "C#";

    enharmonic_map["d"] = "D";
    enharmonic_map["D"] = "D";

    enharmonic_map["D#"] = "D#";
    enharmonic_map["d#"] = "D#";
    enharmonic_map["Eb"] = "D#";
    enharmonic_map["eb"] = "D#";

    enharmonic_map["E"] = "E";
    enharmonic_map["e"] = "E";
    enharmonic_map["Fb"] = "E";
    enharmonic_map["fb"] = "E";

    enharmonic_map["E#"] = "F";
    enharmonic_map["e#"] = "F";
    enharmonic_map["F"] = "F";
    enharmonic_map["f"] = "F";

    enharmonic_map["F#"] = "F#";
    enharmonic_map["f#"] = "F#";
    enharmonic_map["Gb"] = "F#";
    enharmonic_map["Gb"] = "F#";

    enharmonic_map["G"] = "G";
    enharmonic_map["g"] = "G";

    enharmonic_map["G#"] = "G#";
    enharmonic_map["g#"] = "G#";
    enharmonic_map["Ab"] = "G#";
    enharmonic_map["ab"] = "G#";

    enharmonic_map["A"] = "A";
    enharmonic_map["a"] = "A";

    enharmonic_map["A#"] = "A#";
    enharmonic_map["a#"] = "A#";
    enharmonic_map["Bb"] = "A#";
    enharmonic_map["bb"] = "A#";

    enharmonic_map["B"] = "B";
    enharmonic_map["b"] = "B";
    enharmonic_map["Cb"] = "B";
    enharmonic_map["cb"] = "B";

    enharmonic_map["B#"] = "C";
    enharmonic_map["b#"] = "C";

    return enharmonic_map;
}
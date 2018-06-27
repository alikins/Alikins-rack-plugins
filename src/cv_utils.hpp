#include "rack.hpp"
using namespace rack;

// TODO: mv to header
float A440_VOLTAGE = 4.75f;
int A440_MIDI_NUMBER = 69;

// FIXME: whatever baseline 0.0v LFO
// based on bogaudio lfo, defaults to 0v == 16.35f
float LFO_BASELINE_FREQ = 16.35f;
float LFO_BASELINE_VOLTAGE = 0.0f;

// FIXME: can/should be inline
// FIXME: likely should be a NoteInfo type/struct/object
// These are assuming A440 == A4 == 4.75v
float freq_to_cv(float freq, float a440_octave) {
    float volts = log2f(freq / 440.0f * powf(2.0f, A440_VOLTAGE)) - a440_octave;
    // debug("freq_to_vc freq=%f a440_octave=%f volts=%f A440_voltage=%f", freq, a440_octave, volts, A440_VOLTAGE);
    return volts;
}

float lfo_freq_to_cv(float lfo_freq, float lfo_baseline) {
    float volts = log2f(lfo_freq / LFO_BASELINE_FREQ * powf(2.0f, LFO_BASELINE_VOLTAGE)) - lfo_baseline;
    // debug("freq_to_vc freq=%f a440_octave=%f volts=%f A440_voltage=%f", freq, a440_octave, volts, A440_VOLTAGE);
    return volts;
}

float cv_to_freq(float volts, float a440_octave) {
    float freq = 440.0f / powf(2.0f, A440_VOLTAGE) * powf(2.0f, volts + a440_octave);
    // debug("cv_to_freq freq=%f a440_octave=%f volts=%f A440_voltage=%f", freq, a440_octave, volts, A440_VOLTAGE);
    return freq;
}

float lfo_cv_to_freq(float volts, float lfo_baseline) {
    // TODO: figure out what a common LFO baseline is
    float freq = LFO_BASELINE_FREQ / powf(2.0f, LFO_BASELINE_VOLTAGE) * powf(2.0f, volts + lfo_baseline);
    return freq;
}

// can return negative
float volts_of_nearest_note(float volts) {
    float res =  roundf( (volts * 12.0f) )  / 12.0f;
    return res;
}

int volts_to_note(float volts) {
    int res = abs(static_cast<int>( roundf( (volts * 12.0f) ) ) ) % 12;
    // debug("volts_to_note volts=%f res=%d", volts, res);
    return res;
}

int volts_to_octave(float volts, float a440_octave) {
    // debug("a440_octave=%f", a440_octave);
    int octave = floor(volts + a440_octave);
    // debug("volts_to_octaves volts=%f, a440_octave=%f, octave=%d", volts, a440_octave, octave);
    return octave;
}

float volts_to_note_cents(float volts, float a440_octave) {
    float nearest_note = volts_of_nearest_note(volts);
    float cent_volt = 1.0f / 12.0f / 100.0f;

    float offset_cents = (volts-nearest_note)/cent_volt;
    // debug("volts: %f volts_of_nearest: %f volts-volts_nearest: %f offset_cents %f",
    //     volts, nearest_note, volts-nearest_note, offset_cents);

    return offset_cents;
}

int volts_to_midi(float volts, float a440_octave) {
    int midi_note = floor(volts * 12.0f + a440_octave) + 21;
    return midi_note;
}
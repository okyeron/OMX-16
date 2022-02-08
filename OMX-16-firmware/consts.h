#pragma once



const int loSkip = 0;
const int hiSkip = 0;
constexpr int range = 4096 - loSkip - hiSkip;

const float fullRangeV = 4.66;
const float fullRangeDAC = 4095.0;
const float stepsPerVolt = fullRangeDAC / fullRangeV;
const float stepsPerOctave = stepsPerVolt;
const float stepsPerSemitone = stepsPerOctave / 12;

const int midiMiddleC = 60;
const int midiLowestNote = midiMiddleC - 3 * 12; // 3 is how many octaves under middle c
const int midiHightestNote = midiLowestNote + int(fullRangeV * 12) - 1;


// FONTS
#define FONT_LABELS u8g2_font_5x8_tf
#define FONT_VALUES u8g2_font_7x14B_tf
#define FONT_SYMB u8g2_font_9x15_m_symbols
#define FONT_SYMB_BIG u8g2_font_cu12_h_symbols
#define FONT_TENFAT u8g2_font_tenfatguys_tf
#define FONT_BIG u8g2_font_helvB18_tr

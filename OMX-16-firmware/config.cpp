#include "config.h"
#include "consts.h"

const OMXMode DEFAULT_MODE = MODE_MIDI;
const uint8_t EEPROM_VERSION = 1;

// DEFINE CC NUMBERS FOR POTS // CCS mapped to Organelle Defaults
const int CC1 = 21;
const int CC2 = 22;
const int CC3 = 23;
const int CC4 = 24;
const int CC5 = 7;      // change to 25 for EYESY Knob 5

const int CC_AUX = 25; // Mother mode - AUX key
const int CC_OM1 = 26; // Mother mode - enc switch
const int CC_OM2 = 28; // Mother mode - enc turn

const int LED_BRIGHTNESS = 50;

// DONT CHANGE ANYTHING BELOW HERE

const int LED_PIN  = 8; //?????
const int LED_COUNT = 16;

const int analogPins[] = {A3,A4,A2,A5,A1}; // P1, P2, P3, X, Y

// MIDI_OUT is PIN D0

// DAC IS A0

int pots[NUM_CC_BANKS][NUM_CC_POTS] = {
	{CC1,CC2,CC3,CC4,CC5},
	{29,30,31,32,33},
	{34,35,36,37,38},
	{39,40,41,42,43},
	{91,93,103,104,7}
};          // the MIDI CC (continuous controller) for each analog input

SysSettings sysSettings;

const int gridh = 32;
const int gridw = 128;
const int PPQ = 96;

const char* modes[] = {"MI","S1","S2","OM","GR"};

float multValues[] = {.25, .5, 1, 2, 4, 8, 16};
const char* mdivs[] = {"1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "W"};

// KEY SWITCH ROWS/COLS

// Map the keys OMX16
char keys[ROWS][COLS] = {
  {0,1,2,3},
  {4,5,6,7},
  {8,9,10,11},
  {12,13,14,15}
  };
byte rowPins[ROWS] = {9, 4, 11, 7}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 2, 30, 3}; //connect to the column pinouts of the keypad

// Map the keys OMX10
// char keys[ROWS][COLS] = {
// 	{0, 1, 2, 3, 4},
// 	{5, 6, 7, 8, 9}
// };
// byte rowPins[ROWS] = {9, 4}; // row pins for key switches
// byte colPins[COLS] = {5, 2, 0, 1, 3}; // column pins for key switches

// KEYBOARD MIDI NOTE LAYOUT
const int notes[] = {0,
  61,63,   66,68,70,
60,62,64,65,67,69,71,72};

const int steps[] = {0,1,2,
 3,4,    5,6,7,   
 8,9,10,12,13,14,15};

const int midiKeyMap[] = {12,1,13,2,14,15,3,16,4,17,5,18,19,6,20,7,21,22,8,23,9,24,10,25,26};

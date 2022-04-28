// OMX-16

#include <functional>
#include <Adafruit_NeoPixel.h>
//#include <ResponsiveAnalogRead.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <elapsedMillis.h>
#include <Adafruit_TinyUSB.h>
//#include <MIDI.h>

#include "consts.h"
#include "config.h"
#include "colors.h"
#include "omx_keypad.h"
#include "MM.h"
#include "ClearUI.h"
#include "sequencer.h"
#include "noteoffs.h"
//#include "storage.h"
//#include "sysex.h"

//#include "dmadac.h"
//#include "filesystem.h"
//#include "msg.h"
//#include "samplefinder.h"
//#include "sound.h"
//#include "types.h"


// DEVICE INFO FOR ADAFRUIT M0 or M4 
char mfgstr[32] = "denki-oto";
char prodstr[32] = "OMX-16";
//char serialstr[32] = "m4676123";

U8G2_FOR_ADAFRUIT_GFX u8g2_display;

// Declare NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#include <Adafruit_DotStar.h>
Adafruit_DotStar dotstar(1, 41, 40, DOTSTAR_BRG);

// ENCODER
Encoder myEncoder(12, 10); 	// encoder pins on hardware
Button encButton(6);		// encoder button pin on hardware

//KEYPAD
//initialize an instance of custom Keypad class
unsigned long longPressInterval = 800;
unsigned long clickWindow = 200;
OMXKeypad keypad(longPressInterval, clickWindow, makeKeymap(keys), rowPins, colPins, ROWS, COLS);

//TriangleToneSource tri1;
//TriangleToneSource tri2;
//MixSource mix(tri1, tri2);
//FilterSource filt(mix);
//DelaySource delayPedal(filt);
//SoundSource& chainOut = delayPedal;

//const int potCount = 5;
//ResponsiveAnalogRead *analog[potCount];

// POTENTIOMETERS
//int volatile currentValue[potCount];
//int lastMidiValue[potCount];
//int potMin = 0;
//int potMax = 8190;
//int temp;

// ANALOGS
//int potbank = 0;
//int analogValues[] = {0,0,0,0,0};		// default values
//int potValues[] = {0,0,0,0,0};
//int potCC = pots[potbank][0];
//int potVal = analogValues[0];
//int potNum = 0;
//bool plockDirty[] = {false,false,false,false,false};
//int prevPlock[] = {0,0,0,0,0};

// global sequencer shared state
SequencerState sequencer = defaultSequencer();

// VARIABLES / FLAGS
float step_delay;
bool dirtyPixels = false;
bool dirtyDisplay = false;
bool blinkState = false;
bool slowBlinkState = false;
bool noteSelect = false;
bool noteSelection = false;
bool patternParams = false;
bool seqPages = false;
int noteSelectPage = 0;
int selectedNote = 0;
int selectedStep = 0;
bool stepSelect = false;
bool stepRecord = false;
bool stepDirty = false;
bool dialogFlags[] = {false, false, false, false, false, false};
unsigned dialogDuration = 1000;

bool copiedFlag = false;
bool pastedFlag = false;
bool clearedFlag = false;

bool enc_edit = false;
bool midiAUX = false;

int defaultVelocity = 100;
int octave = 0;			// default C4 is 0 - range is -4 to +5
int newoctave = octave;
int transpose = 0;
int rotationAmt = 0;
int hline = 8;
int pitchCV;
uint8_t RES;
uint16_t AMAX;
int V_scale;


float midi_note_to_frequency(uint32_t midi_note) {
  float semitones_away_from_a4 = (float)(midi_note) - (float)(69);
  return powf(2.0f, semitones_away_from_a4 / 12.0f) * 440.0f;
}

// Synth handlers
void synthOn(byte channel, byte note, byte velocity) {
//	Serial.print(note);
//	Serial.print(":");
//	Serial.println(midi_note_to_frequency(note));
//	tri1.playNote(midi_note_to_frequency(note), 1.0f);
	
	
//	Serial.print(note);
//	Serial.println("note-on");
}
void synthOff(byte channel, byte note, byte velocity) {
//	Serial.println("note-off");
}
void synthControlChange(byte control, byte value, byte channel){
}
void synthProgChange(byte program, byte channel){
}



// ####### SETUP ######
void setup() {

	USBDevice.setManufacturerDescriptor(mfgstr);
	USBDevice.setProductDescriptor(prodstr);

	Serial.begin(115200);
	keypad.begin();
//	MM::begin();
//	MM::setNoteHandlers(synthOn, synthOff, synthControlChange, synthProgChange);
	
	dotstar.begin();           // INITIALIZE DotStar
	dotstar.show();           
	dotstar.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

	// Init Display
	initializeDisplay();
	u8g2_display.begin(display);

	// Startup screen
	display.clearDisplay();
	testdrawrect();
	delay(200);
	display.clearDisplay();
	u8g2_display.setForegroundColor(WHITE);
	u8g2_display.setBackgroundColor(BLACK);
	drawLoading();


	strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
	strip.show();            // Turn OFF all pixels ASAP
	strip.setBrightness(LED_BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)
	for(int i=0; i<LED_COUNT; i++) { // For each pixel...
		strip.setPixelColor(i, HALFWHITE);
		strip.show();   // Send the updated pixel colors to the hardware.
		delay(5); // Pause before next pass through loop
	}
	rainbow(5); // rainbow startup pattern
	delay(500);
	// clear LEDs
	strip.fill(0, 0, LED_COUNT);
	strip.show();
	delay(100);

}

void loop() {	
	keypad.tick();

	//MM::usbMidiRead();

	// ############### ENCODER ###############
	//
	auto u = myEncoder.update();
	if (u.active()) {
		auto amt = u.accel(0); // where 5 is the acceleration factor if you want it, 0 if you don't)
    	Serial.println(u.dir() < 0 ? "ccw " : "cw ");
    	Serial.println(amt);
	}
	// ############### ENCODER BUTTON ###############
	//
	auto s = encButton.update();
	switch (s) {
		// SHORT PRESS
		case Button::Down: Serial.println("Button down");
		// LONG PRESS
		case Button::DownLong: Serial.println("Button downlong");
			break;
		case Button::Up: Serial.println("Button up");
			break;
		case Button::UpLong: Serial.println("Button uplong");
			break;
		default:
			break;
	}
	// END ENCODER BUTTON


	

	while(keypad.available()){
		auto e = keypad.next();
		int thisKey = e.key();
		
		Serial.println(thisKey);
	
		byte thisnote = notes[thisKey] + (octave * 12); // adjust key for octave range

    	if (!e.held()){
			if (e.down()){
				strip.setPixelColor(thisKey, strip.Color(128,   128,   128));
//				MM::sendNoteOn(thisnote, 127, 1);
				synthOn(1, thisnote, 127);
			} else {
				strip.setPixelColor(thisKey, strip.Color(0,   0,   0));
//				MM::sendNoteOff(thisnote, 0, 1);
				synthOff(1, thisnote, 0);
			}
    	}
	}
	strip.show();                          //  Update strip to match
//	while (MM::usbMidiRead()) {
//		// incoming messages - see handlers
//	}

}





void u8g2centerText(const char* s, int16_t x, int16_t y, uint16_t w, uint16_t h) {
//  int16_t bx, by;
	uint16_t bw, bh;
	bw = u8g2_display.getUTF8Width(s);
	bh = u8g2_display.getFontAscent();
	u8g2_display.setCursor(
		x + (w - bw) / 2,
		y + (h - bh) / 2
	);
	u8g2_display.print(s);
}

void u8g2centerNumber(int n, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	char buf[8];
	itoa(n, buf, 10);
	u8g2centerText(buf, x, y, w, h);
}


// #### LED STUFF
// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
	// Hue of first pixel runs 5 complete loops through the color wheel.
	// Color wheel has a range of 65536 but it's OK if we roll over, so
	// just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
	// means we'll make 5*65536/256 = 1280 passes through this outer loop:
	for(long firstPixelHue = 0; firstPixelHue < 1*65536; firstPixelHue += 256) {
		for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
			// Offset pixel hue by an amount to make one full revolution of the
			// color wheel (range of 65536) along the length of the strip
			// (strip.numPixels() steps):
			int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());

			// strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
			// optionally add saturation and value (brightness) (each 0 to 255).
			// Here we're using just the single-argument hue variant. The result
			// is passed through strip.gamma32() to provide 'truer' colors
			// before assigning to each pixel:
			strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
		}
		strip.show(); // Update strip with new contents
		delay(wait);  // Pause for a moment
	}
}
void setAllLEDS(int R, int G, int B) {
	for(int i=0; i<LED_COUNT; i++) { // For each pixel...
		strip.setPixelColor(i, strip.Color(R, G, B));
	}
	dirtyPixels = true;
}

// #### OLED STUFF
void testdrawrect(void) {
	display.clearDisplay();

	for(int16_t i=0; i<display.height()/2; i+=2) {
		display.drawRect(i, i, display.width()-2*i, display.height()-2*i, SSD1306_WHITE);
		display.display(); // Update screen with each newly-drawn rectangle
		delay(1);
	}

	delay(500);
}

void drawLoading(void) {
	const char* loader[] = {"\u25f0", "\u25f1", "\u25f2", "\u25f3"};
	display.clearDisplay();
	u8g2_display.setFontMode(0);
	for(int16_t i=0; i<16; i+=1) {
		display.clearDisplay();
		u8g2_display.setCursor(18,18);
		u8g2_display.setFont(FONT_TENFAT);
		u8g2_display.print("OMX-16");
		u8g2_display.setFont(FONT_SYMB_BIG);
		u8g2centerText(loader[i%4], 80, 10, 32, 32); // "\u00BB\u00AB" // // dice: "\u2685"
		display.display();
		delay(100);
	}

	delay(100);
}

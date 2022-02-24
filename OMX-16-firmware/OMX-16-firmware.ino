// OMX-16


#include <functional>
#include <Adafruit_NeoPixel.h>
//#include <ResponsiveAnalogRead.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include <elapsedMillis.h>

#include "consts.h"
#include "config.h"
#include "colors.h"
#include "MM.h"
#include "ClearUI.h"
#include "omx_keypad.h"

// DEVICE INFO FOR ADAFRUIT M0 or M4 
char mfgstr[32] = "denki-oto";
char prodstr[32] = "OMX-16";
//char serialstr[32] = "m4676123";

#include <Adafruit_DotStar.h>
Adafruit_DotStar dotstar(1, 41, 40, DOTSTAR_BRG);

# MOZZI BIZ
#include <MozziGuts.h>
#include <Oscil.h>
#include <tables/saw_analogue512_int8.h> // oscillator waveform
#include <tables/cos2048_int8.h> // filter modulation waveform
#include <LowPassFilter.h>
#include <mozzi_rand.h>  // for rand()
#include <mozzi_midi.h>  // for mtof()

Oscil<SAW_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aOsc1(SAW_ANALOGUE512_DATA);
Oscil<SAW_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aOsc2(SAW_ANALOGUE512_DATA);
Oscil<SAW_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aOsc3(SAW_ANALOGUE512_DATA);
Oscil<SAW_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aOsc4(SAW_ANALOGUE512_DATA);
Oscil<SAW_ANALOGUE512_NUM_CELLS, AUDIO_RATE> aOsc5(SAW_ANALOGUE512_DATA);
Oscil<COS2048_NUM_CELLS, CONTROL_RATE> kFilterMod(COS2048_DATA);

LowPassFilter lpf;
uint8_t resonance = 170; // range 0-255, 255 is most resonant
uint8_t notes[] = {33, 34, 31}; // possible notes to play MIDI A1, A1#, G1
uint16_t note_duration = 17500;
uint8_t note_id = 0;
uint32_t lastMillis = 0;


U8G2_FOR_ADAFRUIT_GFX u8g2_display;

// Declare NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

//initialize an instance of class NewKeypad
//Adafruit_Keypad customKeypad = Adafruit_Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS); 

//initialize an instance of custom Keypad class
unsigned long longPressInterval = 800;
unsigned long clickWindow = 200;
OMXKeypad keypad(longPressInterval, clickWindow, makeKeymap(keys), rowPins, colPins, ROWS, COLS);





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


void setup() {
	USBDevice.setManufacturerDescriptor(mfgstr);
	USBDevice.setProductDescriptor(prodstr);
//	USBDevice.setSerialDescriptor(serialstr);

	Serial.begin(115200);
	MM::begin();

	// while ( !USBDevice.mounted() ) delay(1);
	
	randomSeed(analogRead(A3));
	srand(analogRead(A3));

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

	keypad.begin();

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

	dotstar.begin();           // INITIALIZE DotStar
	dotstar.show();            // Turn OFF all pixels ASAP
	dotstar.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

	startMozzi();
	kFilterMod.setFreq(0.08f);
	lpf.setCutoffFreqAndResonance(20, resonance);
	setNotes();

}

void loop() {
	keypad.tick();

	audioHook();

	while(keypad.available()){
		auto e = keypad.next();
		int thisKey = e.key();

		Serial.println(thisKey);
    	if (e.down()){
    		strip.setPixelColor(thisKey, strip.Color(128,   128,   128));
    		MM::sendNoteOn(60+thisKey, 127, 1);
    	} else {
    		strip.setPixelColor(thisKey, strip.Color(0,   0,   0));
    		MM::sendNoteOff(60+thisKey, 0, 1);
    	}
    	
  }
  strip.show();                          //  Update strip to match

  delay(10);
}


void setNotes() {
  float f = mtof(notes[note_id]);
  aOsc2.setFreq( f + (float)rand(100)/100); // orig 1.001, 1.002, 1.004
  aOsc3.setFreq( f + (float)rand(100)/100);
  aOsc4.setFreq( f + (float)rand(100)/100);
  aOsc5.setFreq( (f/2) + (float)rand(100)/1000);  
}

// mozzi function, called every CONTROL_RATE
void updateControl() {
  // filter range (0-255) corresponds with 0-8191Hz
  // oscillator & mods run from -128 to 127
  byte cutoff_freq = 67 + kFilterMod.next()/2;
  lpf.setCutoffFreqAndResonance(cutoff_freq, resonance);
  
  if(rand(CONTROL_RATE) == 0) { // about once every second
    Serial.println("!");
    kFilterMod.setFreq((float)rand(255)/4096);  // choose a new modulation frequency
    setNotes(); // wiggle the tuning a little
  }
  
  if( millis() - lastMillis > note_duration )  {
    lastMillis = millis();
    note_id = rand(3); // (note_id+1) % 3;
    Serial.println((byte)notes[note_id]);
  }
}
// mozzi function, called every AUDIO_RATE to output sample
AudioOutput_t updateAudio() {
  long asig = lpf.next(aOsc1.next() + 
                       aOsc2.next() + 
                       aOsc3.next() +
                       aOsc4.next() +
                       aOsc5.next()
                       );
  return MonoOutput::fromAlmostNBit(11,asig);
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

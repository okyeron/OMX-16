// OMX-16


#include <functional>
#include <Adafruit_NeoPixel.h>
//#include <ResponsiveAnalogRead.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>
#include <elapsedMillis.h>

#include "config.h"
#include "omx_keypad.h"
#include "MM.h"


//initialize an instance of class NewKeypad
//Adafruit_Keypad customKeypad = Adafruit_Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS); 

//initialize an instance of custom Keypad class
unsigned long longPressInterval = 800;
unsigned long clickWindow = 200;
OMXKeypad keypad(longPressInterval, clickWindow, makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Declare NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#include <Adafruit_DotStar.h>
Adafruit_DotStar dotstar(1, 41, 40, DOTSTAR_BRG);


// DEVICE INFO FOR ADAFRUIT M0 or M4 
char mfgstr[32] = "denki-oto";
char prodstr[32] = "OMX-16";
//char serialstr[32] = "m4676123";

void setup() {
	USBDevice.setManufacturerDescriptor(mfgstr);
	USBDevice.setProductDescriptor(prodstr);
//	USBDevice.setSerialDescriptor(serialstr);

	Serial.begin(115200);
	MM::begin();

	while ( !USBDevice.mounted() ) delay(1);
	

	keypad.begin();

	strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
	strip.show();            // Turn OFF all pixels ASAP
	strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

	dotstar.begin();           // INITIALIZE DotStar
	dotstar.show();            // Turn OFF all pixels ASAP
	dotstar.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

}

void loop() {
	keypad.tick();

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

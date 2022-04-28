#include "MM.h"

#include <MIDI.h>
#include <Adafruit_TinyUSB.h>

/*
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDIusb); // USB MIDI
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDIclassic);  // classic midi over RX/TX


	MIDIusb.begin(MIDI_CHANNEL_OMNI);
	MIDIhw.begin(MIDI_CHANNEL_OMNI);
	MIDIusb.turnThruOff();
	MIDIhw.turnThruOff();
*/

namespace {
	using SerialMIDI = midi::SerialMIDI<HardwareSerial>;
	using MidiInterface = midi::MidiInterface<SerialMIDI>;
	SerialMIDI theSerialInstance(Serial1);
	MidiInterface HWMIDI(theSerialInstance);

	Adafruit_USBD_MIDI usb_midi;
	using USBMIDI = midi::SerialMIDI<Adafruit_USBD_MIDI>;
	using usbMidiInterface = midi::MidiInterface<USBMIDI>;
	USBMIDI theUSBInstance(usb_midi);
	usbMidiInterface usbMIDI(theUSBInstance);
}

namespace MM {
	void begin() {
		HWMIDI.begin();
		usbMIDI.begin();
		HWMIDI.turnThruOff();
		usbMIDI.turnThruOff();
	}

	void setNoteHandlers(NoteCallBack handleNoteOn, NoteCallBack handleNoteOff, NoteCallBack handleControlChange, ProgramChangeCallBack handleProgramChange) {
		usbMIDI.setHandleNoteOn(handleNoteOn);
		usbMIDI.setHandleNoteOff(handleNoteOff);
		usbMIDI.setHandleControlChange(handleControlChange);
		usbMIDI.setHandleProgramChange(handleProgramChange);
// 		usbMIDI.setHandleSystemExclusive(OnSysEx);
	}
  
	void sendNoteOn(int note, int velocity, int channel) {
		usbMIDI.sendNoteOn(note, velocity, channel);
		HWMIDI.sendNoteOn(note, velocity, channel);
	}

	void sendNoteOnHW(int note, int velocity, int channel) {
		HWMIDI.sendNoteOn(note, velocity, channel);
	}

	void sendNoteOff(int note, int velocity, int channel) {
		usbMIDI.sendNoteOff(note, velocity, channel);
		HWMIDI.sendNoteOff(note, velocity, channel);
	}

	void sendNoteOffHW(int note, int velocity, int channel) {
		HWMIDI.sendNoteOff(note, velocity, channel);
	}

	void sendControlChange(int control, int value, int channel) {
		usbMIDI.sendControlChange(control, value, channel);
		HWMIDI.sendControlChange(control, value, channel);
	}

	void sendControlChangeHW(int control, int value, int channel) {
		HWMIDI.sendControlChange(control, value, channel);
	}

	void sendProgramChange(int program, int channel) {
		usbMIDI.sendProgramChange(program, channel);
		HWMIDI.sendProgramChange(program, channel);
	}

	void sendSysEx(uint32_t length, const uint8_t *sysexData, bool hasBeginEnd) {
 		usbMIDI.sendSysEx(length, sysexData, hasBeginEnd);
	}

	void sendClock() {
		usbMIDI.sendClock();
		HWMIDI.sendClock();
	}

	void startClock(){
		usbMIDI.sendStart();
		HWMIDI.sendStart();
	}

	void continueClock(){
		usbMIDI.sendContinue();
		HWMIDI.sendContinue();
	}

	void stopClock(){
		usbMIDI.sendStop();
		HWMIDI.sendStop();
	}

	// NEED SOMETHING FOR usbMIDI.read() / MIDI.read()

	bool usbMidiRead(){
		return usbMIDI.read();
	}

// 	bool midiRead(){
//  		return HWMIDI.read();
// 	}
}

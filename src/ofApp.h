#pragma once

#include "ofMain.h"
#include <ofxMidiIn.h>
#include "Oscillator.h"

class ofApp : public ofBaseApp, ofxMidiListener {

public:
	void setup() override;
	void update() override;
	void draw() override;

	void keyPressed(int key) override;
	void newMidiMessage(ofxMidiMessage& msg) override;

	void audioOut(ofSoundBuffer& buffer) override;
private:
	int samplerate = 48000;
	ofxMidiIn midiIn;
	ofSoundBuffer lastBuffer;
	mutex audioMutex;
	ofPolyline waveform;
	TableOscillator* _osc;
	int _curTick = 0;
};

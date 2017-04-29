#pragma once

#include "ofMain.h"
#include <ofxMidiIn.h>

class ofApp : public ofBaseApp, ofxMidiListener {

public:
	void setup() override;
	void update() override;
	void draw() override;

	void newMidiMessage(ofxMidiMessage& msg) override;

	void audioOut(ofSoundBuffer& buffer) override;
private:
	ofxMidiIn midiIn;
	ofSoundBuffer lastBuffer;
	mutex audioMutex;
	float speed = 1024.f;
	ofPolyline waveform;
};

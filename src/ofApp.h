#pragma once

#include "ofMain.h"
#include <ofxMidiIn.h>
#include "Oscillator.h"
#include "MidiDevice.h"

class ofApp : public ofBaseApp, ofxMidiListener {

public:

	void exit() override;

	ofApp()
		: _input(new ConstantGenerator(_settings, 0.f))
	{
	}

	void setup() override;
	void update() override;
	void draw() override;

	void keyPressed(int key) override;
	void newMidiMessage(ofxMidiMessage& msg) override;

	void audioOut(ofSoundBuffer& buffer) override;
private:
	DSPSettings _settings;
	MidiManager _midi;
	ofSoundBuffer lastBuffer;
	mutex audioMutex;
	ofPolyline waveform;
	shared_ptr<AudioInput> _input;
	int _curTick = 0;
};

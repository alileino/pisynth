#include "ofApp.h"

void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);

	midiIn.listPorts();
	midiIn.openPort(0);
	midiIn.ignoreTypes(false, false, false);

	midiIn.setVerbose(true);
	midiIn.addListener(this);

	ofSoundStreamSetup(2, 0, 48000, 512, 3);
}

void ofApp::update() {
	unique_lock<mutex> lock(audioMutex);
	waveform.clear();
	for (size_t i = 0; i < lastBuffer.getNumFrames(); i++) {
		float sample = lastBuffer.getSample(i, 0);
		float x = ofMap(i, 0, lastBuffer.getNumFrames(), 0, ofGetWidth());
		float y = ofMap(sample, -1, 1, 0, ofGetHeight());
		waveform.addVertex(x, y);
	}
}

void ofApp::draw() {
	ofBackground(ofColor::black);
	ofSetColor(ofColor::white);
	ofSetLineWidth(1 + (1 * 30.));
	waveform.draw();
}

void ofApp::newMidiMessage(ofxMidiMessage& msg) {
	speed = (msg.pitch - 35) * 50;
}

void ofApp::audioOut(ofSoundBuffer& buffer) {
	int x = 0;
	for (size_t i = 0; i < buffer.getNumFrames(); i++) {
		x += 1;
		if (x > speed)
			x = 0;
		float val = x / speed*2.0 - 1.0;
		buffer.getSample(i, 0) = val;
		buffer.getSample(i, 1) = val;
	}
	unique_lock<mutex> lock(audioMutex);
	lastBuffer = buffer;
}

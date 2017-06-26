#include "ofApp.h"

void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);

	midiIn.listPorts();
	midiIn.openPort(0);
	midiIn.ignoreTypes(false, false, false);

	midiIn.setVerbose(true);
	midiIn.addListener(this);
	samplerate = 48000;
	ofSoundStreamSetup(2, 0, samplerate, 512, 3);
	_osc = new TableOscillator(1024, samplerate);

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
//	ofSetColor(ofColor::white);
	ofSetColor(255, 0, 255);
	ofSetLineWidth(1 + (1 * 30.));
	waveform.draw();
}

void ofApp::newMidiMessage(ofxMidiMessage& msg) {
	float freq = 440.0*pow(2.0, (msg.pitch - 69.0) / 12.0);
	_osc->getFreqBinding().update(freq);
}

void ofApp::audioOut(ofSoundBuffer& buffer) {

//	vector<float> oscbuf = vector<float>(buffer.getNumFrames());
//	_osc->play(oscbuf, 0, oscbuf.size());

	for (size_t i = 0; i < buffer.getNumFrames(); i++) {
		float val = _osc->play(true);
		buffer.getSample(i, 0) = val;
		buffer.getSample(i, 1) = val;
	}
	unique_lock<mutex> lock(audioMutex);
	lastBuffer = buffer;
}

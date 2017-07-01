#include "ofApp.h"
#include "tests.h"


void ofApp::setup() {
	ofSetLogLevel(OF_LOG_VERBOSE);
//
//	midiIn.listPorts();
//	midiIn.openPort(0);
//	midiIn.ignoreTypes(false, false, false);
//
//	midiIn.setVerbose(true);
//	midiIn.addListener(this);
	samplerate = 48000;
	_midi.Init();
//	_osc.reset(new TableOscillatorTest(1024, samplerate, 512, 1));
	ofSoundStreamSetup(2, 0, samplerate, 512, 3);
	_osc.reset(new TableOscillator(1024, samplerate, 512));
	static_cast<TableOscillator*>(_osc.get())->addSource(_midi.getDevice().get()->gen, FREQ);
//	shared_ptr<SignalGeneratorAbstract> osc2(nullptr);
//	osc2.reset(new TableOscillator(512, samplerate, 512));
//	static_cast<TableOscillator*>(_osc.get())->addSource(osc2, FREQ);
}

void ofApp::update() {
	unique_lock<mutex> lock(audioMutex);
	waveform.clear();
//	for (size_t i = 0; i < lastBuffer.getNumFrames(); i++) {
//		float sample = lastBuffer.getSample(i, 0);
//		float x = ofMap(i, 0, lastBuffer.getNumFrames(), 0, ofGetWidth());
//		float y = ofMap(sample, -1, 1, 0, ofGetHeight());
//		waveform.addVertex(x, y);
//	}
}

void ofApp::draw() {
	ofBackground(ofColor::black);
//	ofSetColor(ofColor::white);
	ofSetColor(255, 0, 255);
	ofSetLineWidth(1 + (1 * 30.));
	waveform.draw();
}

void ofApp::keyPressed(int key)
{
	ofBaseApp::keyPressed(key);
	if (key == 27)
		ofExit();
}

void ofApp::newMidiMessage(ofxMidiMessage& msg) {
	float freq = 440.0*pow(2.0, (msg.pitch - 69.0) / 12.0);

	
//	cout << msg.toString() << endl;
//	_osc->getFreqBinding().update(freq);
}

void ofApp::audioOut(ofSoundBuffer& buffer) {
	
//	vector<float> oscbuf = vector<float>(buffer.getNumFrames());
//	_osc->play(oscbuf, 0, oscbuf.size());

	unique_lock<mutex> lock(audioMutex);

	vector<float>& oscb = _osc->play(_curTick);
	for (size_t i = 0; i < buffer.getNumFrames(); i++) {
		float val = oscb[i];
		buffer.getSample(i, 0) = val;
		buffer.getSample(i, 1) = val;
	}
	lastBuffer = buffer;
	_curTick++;
}

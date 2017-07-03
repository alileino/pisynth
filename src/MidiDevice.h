#pragma once

#include <unordered_set>
#include "Utils.h"
#include <ofxMidi.h>
#include "Oscillator.h"

class MidiDevice : public ofxMidiListener
{
	shared_ptr<ConstantGenerator> _attack;
	shared_ptr<ConstantGenerator> _decay;
	shared_ptr<ConstantGenerator> _sustain;
	shared_ptr<ConstantGenerator> _release;
public:

	shared_ptr<ConstantGenerator> gen;
	shared_ptr<ADSRProcessor> adsr;
	shared_ptr<TableOscillator> osc;
	

	void newMidiMessage(ofxMidiMessage& msg) override
	{
		cout << msg.channel << " channel\n";
		cout << msg.control << " countrol\n";
		cout << msg.deltatime << " time \n";
		cout << msg.pitch << " pitch \n";
		cout << msg.portName << " portName \n";
		cout << msg.portNum << " portNum \n";
		cout << msg.status << " status \n";
		cout << msg.velocity << " velocity \n";
		cout << "-----------\n";
		switch(msg.status)
		{
		case MIDI_NOTE_ON:
		case MIDI_NOTE_OFF:
			note(msg);
		default: break;
		}

	};

	vector<int> notesOn;

	void note(ofxMidiMessage& msg)
	{
		if(msg.status == MIDI_NOTE_ON)
		{
			notesOn.push_back(msg.pitch);
		}
		if(msg.status == MIDI_NOTE_OFF || (msg.status==MIDI_NOTE_ON && msg.velocity==0))
		{
			notesOn.erase(find(notesOn.begin(), notesOn.end(), msg.pitch));
		}
		cout << notesOn.size() << " notes ON" << endl;
		if (notesOn.size() > 0) {
			int last = notesOn.back();
			float freq = 440.0*pow(2.0, (last - 69.0) / 12.0);
			gen->setValue(freq);
			if(adsr->getAmplitude() <= 0)
				osc->resetPhase();
			adsr->attack();
		}
		else
		{
			adsr->release();
		}

	}


	explicit MidiDevice(const DSPSettings& settings)
		: _attack(new ConstantGenerator(settings, 0.1)),
		_decay(new ConstantGenerator(settings, 0.3)),
		_sustain(new ConstantGenerator(settings, 0.6)),
		_release(new ConstantGenerator(settings, 0.5)),
		gen(new ConstantGenerator(settings, 0)),
		adsr(new ADSRProcessor(settings)),
		osc(new TableOscillator(settings, 1024))
	{
		std::cout << "Construct\n";
		adsr->addSource(_attack, ATTACK);
		adsr->addSource(_decay, DECAY);
		adsr->addSource(_sustain, SUSTAIN);
		adsr->addSource(_release, RELEASE);

		TableOscillator* myosc = static_cast<TableOscillator*>(osc.get());
		myosc->addSource(gen, FREQ);

		adsr->addSource(osc, SIGNAL);
	}
	MidiDevice() = delete;
	MidiDevice(MidiDevice const&) = delete;
	void operator=(MidiDevice const&) = delete;
};


class MidiManager 
{
		ofxMidiIn midiIn;
		vector<shared_ptr<MidiDevice>> _devices;
	public:
		MidiManager(MidiManager const&) = delete;
		void operator=(MidiManager const&) = delete;

		MidiManager() {}

		const shared_ptr<MidiDevice>& getDevice()
		{
			return _devices.front();
		}

		void Init(const DSPSettings& settings)
		{
			midiIn.listPorts();
			midiIn.openPort(0);
			midiIn.ignoreTypes(false, false, false);
			
			midiIn.setVerbose(true);
			_devices.push_back(make_shared<MidiDevice>(settings));
			midiIn.addListener(_devices[0].get());
		}
};

#pragma once

#include <unordered_set>
#include "src/Utils.h"

class MidiDevice : public ofxMidiListener
{
public:

	shared_ptr<ConstantGenerator> gen = make_shared<ConstantGenerator>();

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
		if(msg.status == MIDI_NOTE_OFF)
		{
			notesOn.erase(find(notesOn.begin(), notesOn.end(), msg.pitch));
		}
		cout << notesOn.size() << " notes ON" << endl;
		int last = notesOn.back();
		float freq = 440.0*pow(2.0, (last - 69.0) / 12.0);
		gen->setValue(freq);
	}



	MidiDevice() {}

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

		void Init()
		{
			midiIn.listPorts();
			midiIn.openPort(1);
			midiIn.ignoreTypes(false, false, false);
			
			midiIn.setVerbose(true);
			_devices.push_back(make_shared<MidiDevice>());
			midiIn.addListener(_devices[0].get());
		}
};

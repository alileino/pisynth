#pragma once
#include <vector>
#include "Utils.h"
#include <utils/ofConstants.h>


class TableOscillator : public SignalGeneratorAbstract, SignalConsumerAbstract
{
protected:
	void produce(vector<float>& dest, int tick) override 
	{
		vector<float> freq = _freq->play(tick);
		float incrBase = _tableSize / _sr;
		vector<float>& t = *_current;
		for (size_t i = 0; i < dest.size(); i++)
		{
			int p = int(_phase);
			float tableCur = t[p % _tableSize];
			float tableNext = t[(p + 1) % _tableSize];
			// Linear interpolation
			float k = tableNext - tableCur;
			float val = tableCur + k*(_phase - p);
			dest[i] = val;
			_phase += incrBase*freq[p%freq.size()];
		}
	}
private:
	double _phase = 0.0;
	int _tableSize;
	float _sr;
	vector<float>* _current;

	shared_ptr<SignalGeneratorAbstract> _freq;


	static ConstantGenerator defaultFrequency;

public:
	
	TableOscillator(const DSPSettings& settings, int tableSize)
		: SignalGeneratorAbstract(settings),
		_freq(new ConstantGenerator(settings, 440.f))
	{
		_tableSize = tableSize;
		_sr = settings.sampleRate;
		std::vector<float>* sine = new std::vector<float>(tableSize);
		float increment = 2 * PI / _tableSize;
		for (int i = 0; i < _tableSize; i++)
		{
			(*sine)[i] = sin(i*increment);
		}
		_current = sine;

		cout << this << "Freq bound to " << _freq.get() << endl;
	}

	void resetPhase()
	{
		_phase = 0;
	}

	void addSource(const shared_ptr<SignalGeneratorAbstract>& source, ParamName param) override
	{
		_freq = source;
		cout << this << "Freq REbound to " << (_freq.get()) << endl;
	}


	~TableOscillator()
	{
	}
};


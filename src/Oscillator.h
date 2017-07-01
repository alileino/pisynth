#pragma once
#include <vector>
#include "Utils.h"
#include <utils/ofConstants.h>


class TableOscillator : public SignalGeneratorAbstract, SignalConsumerAbstract
{
protected:
	void produce(vector<float>& dest) override 
	{
		vector<float> freq = _freq.play(0);
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
			_phase += incrBase*freq[0];
		}
	}
private:
	double _phase = 0.0;
	int _tableSize;
	float _sr;
	vector<float>* _current;

	SignalGeneratorAbstract& _freq;


	static ConstantGenerator defaultFrequency;

public:


	TableOscillator(int tableSize, float sampleRate, int bufferSize)
		: SignalGeneratorAbstract(bufferSize),
		_freq(defaultFrequency)
	{
		_tableSize = tableSize;
		_sr = sampleRate;
		std::vector<float>* sine = new std::vector<float>(tableSize);
		float increment = 2 * PI / _tableSize;
		for (int i = 0; i < _tableSize; i++)
		{
			(*sine)[i] = sin(i*increment);
		}
		_current = sine;
	}


	void addSource(SignalGeneratorAbstract& source, ParamName param) override
	{
		_freq = source;
	}
//
//	// TODO: Frequency interpolation
//	void play(std::vector<float> &buffer, int pos, int count)
//	{
//		float freqOld, freqNew;
//		std::tie(freqOld, freqNew) = _freq.consume();
//		float incrBase = _tableSize / _sr;
//		vector<float> t = *_current;
//		for (int i = 0; i < count; i++)
//		{
//			float k = -(t[(int)_phase % _tableSize] - t[(int)(_phase + 1) % _tableSize]);
//			float val = t[(int)_phase % _tableSize] + k*(_phase - (int)_phase);
//			buffer[pos + i] = val;
//			_phase += incrBase*freqNew;
//		}
//	}
//
//	float play(bool consume)
//	{
//		float freqOld, freqNew;
//		tie(freqOld, freqNew) = _freq.consume();
//
//		const vector<float> &t = *_current;
//		int phasei = (int)_phase;
//		float k = t[(phasei + 1) % _tableSize]- t[phasei % _tableSize];
//		float val = t[phasei % _tableSize] + k*(_phase - phasei);
//		
//		if (consume) {
//
//			float incr = freqNew * _tableSize / _sr;
//			_phase += incr;
//		}
//		return val;
//	}

	~TableOscillator()
	{
	}
};


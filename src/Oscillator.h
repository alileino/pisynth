#pragma once
#include <vector>

template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
	return std::max(lower, std::min(n, upper));
}
enum ParamNames
{
	FREQ
};

template<typename T>
class ParamBinding
{
public:

	ParamBinding(T initialValue, T minValue, T maxValue)
	{
		_newValue = _oldValue = clip(initialValue, minValue, maxValue);
		
		_minValue = minValue;
		_maxValue = maxValue;
	}

	void update(T newValue)
	{
		T result = clip(newValue, _minValue, _maxValue);
		unique_lock<mutex> lock(_mutex);
		_newValue = result;
	}

	tuple<T, T> consume()
	{
		unique_lock<mutex> lock(_mutex);
		T old = _oldValue;
		_oldValue = _newValue;
		return std::make_tuple(old, _newValue);
	}

	T peek()
	{
		return _newValue;
	}

	T getMin() { return _minValue; }
	T getMax() { return _maxValue; }

private:
	T _newValue;
	T _oldValue;
	T _minValue;
	T _maxValue;
	mutex _mutex;
};


class TableOscillator
{
private:
	ParamBinding<float> _freq;
	double _phase = 0.0;
	int _tableSize;
	float _sr;
	vector<float>* _current;
public:

	TableOscillator(int tableSize, float sampleRate)
		: _freq(440.0f, 1.0f, 96000.0f)
	{
		_tableSize = tableSize;
		_sr = sampleRate;
		vector<float>* sine = new vector<float>(tableSize);
		float increment = 2 * PI / _tableSize;
		for(int i = 0; i < _tableSize; i++)
		{
			(*sine)[i] = sin(i*increment);
		}
		_current = sine;
	}


	ParamBinding<float>& getFreqBinding()
	{
		return _freq;
	}

	// TODO: Frequency interpolation
	void play(std::vector<float> &buffer, int pos, int count)
	{
		float freqOld, freqNew;
		std::tie(freqOld, freqNew) = _freq.consume();
		float incrBase = _tableSize /  _sr;
		vector<float> t = *_current;
		for(int i = 0; i < count; i++)
		{
			float k = -(t[(int)_phase % _tableSize] - t[(int)(_phase + 1) % _tableSize]);
			float val = t[(int)_phase % _tableSize] + k*(_phase - (int)_phase);
			buffer[pos + i] = val;
			_phase += incrBase*freqNew;
		}

	}

	~TableOscillator()
	{
	}
};


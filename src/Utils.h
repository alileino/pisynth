#pragma once

#include <cassert>
#include <memory>
#include <iostream>
#include <limits>
#ifdef DEBUG
#define DEBUG_IF(x) if(x)
#else
#define DEBUG_IF(x) if(false)
#endif

struct DSPSettings
{
	float sampleRate;
	int bufferSize;
};

class Utils
{
public:

	Utils()
	{
	}

	~Utils()
	{
	}
};

template <typename T>
T clip(const T& n, const T& lower, const T& upper) {
	return std::max(lower, std::min(n, upper));
}

class SignalGeneratorAbstract
{
private:

	int _curTick = -1;
	std::vector<float> _buffer;
protected:
	const DSPSettings& _settings;

public:

	SignalGeneratorAbstract(const SignalGeneratorAbstract& that) = delete;
	SignalGeneratorAbstract() = delete;

	SignalGeneratorAbstract(const DSPSettings& settings)
		:_buffer(settings.bufferSize),
		_settings(settings)
	{
	}

	SignalGeneratorAbstract(const DSPSettings& settings, int bufferSize)
		:_buffer(bufferSize),
		_settings(settings)
	{
		
	}

	std::vector<float>& play(int tick)
	{
		if (tick > _curTick)
		{
			_curTick = tick;

//			assert(_buffer.size() == _settings.bufferSize);
			produce(_buffer, tick);
		}

		return _buffer;
	}
	virtual ~SignalGeneratorAbstract()
	{
		std::cout << this << "destroyed" << std::endl;
	}

	const DSPSettings& getSettings() const
	{
		return _settings;
	}

protected:
	void freeze()
	{
		_curTick = std::numeric_limits<int>::max();
	}

	void unfreeze()
	{
		_curTick = std::numeric_limits<int>::min();
	}

	virtual void produce(std::vector<float>& dest, int curTick) = 0;

	//	virtual void reset() = 0;
};


enum ParamName
{
	FREQ,
	AMPLITUDE,
	SIGNAL,
	DELAY,
	ATTACK,
	HOLD,
	DECAY,
	SUSTAIN,
	RELEASE
};

class SignalConsumerAbstract
{
public:
	virtual ~SignalConsumerAbstract()
	{
	}

	virtual void addSource(const std::shared_ptr<SignalGeneratorAbstract>& source, ParamName param) = 0;
	virtual std::vector<ParamName> getParams() {
		return std::vector<ParamName>();
	}
};


class ConstantGenerator : public SignalGeneratorAbstract
{
private:
	float _value;
public:
	ConstantGenerator() = delete;
	explicit ConstantGenerator(const DSPSettings& settings, float value = 440.f)
		: SignalGeneratorAbstract(settings, 1),
		_value(value)
	{
	}

	void setValue(float value)
	{
		_value = value;
		unfreeze();
	}

	float getValue() { return _value; }

protected:
	void produce(std::vector<float>& dest, int tick) override {
		assert(dest.size() == 1);
		dest[0] = _value;
		freeze();
	}
};

class SignalProcessor : public SignalConsumerAbstract
{ 
protected:
	std::shared_ptr<SignalGeneratorAbstract> _source;

public:
	SignalProcessor(const DSPSettings& settings)
		: _source(new ConstantGenerator(settings, 0.f))
	{}

	virtual ~SignalProcessor()
	{}

	virtual void addSource(const std::shared_ptr<SignalGeneratorAbstract>& source, ParamName param) override
	{
		_source = source;
	}


	virtual const std::vector<float>& play(int curTick) = 0;
};

class ASDRProcessor : public SignalProcessor
{
	std::shared_ptr<SignalGeneratorAbstract> _attack;
	std::shared_ptr<SignalGeneratorAbstract> _decay;
	std::shared_ptr<SignalGeneratorAbstract> _sustain;
	std::shared_ptr<SignalGeneratorAbstract> _release;

	const float _deltaT;
	int _curEnvelope = 0;
	float _phase = 0.f;
	std::vector<float> _silence = {0};

	enum EnvelopePhase
	{
		NONE   =0,
		ATTACK =1,
		DECAY  =2,
		SUSTAIN=3,
		RELEASE=4
	};

	int linearApply(std::vector<float>& src, 
		const std::shared_ptr<SignalGeneratorAbstract>& signal,
		int curTick, int pos, float k, float intercept)
	{
		std::cout << _curEnvelope << " " << pos << " " << _phase << " k" << k << std::endl;
		float value = signal->play(curTick)[0];
		for (int i = pos; i < src.size(); ++i)
		{
			if (_phase > value)
			{
				return i;
			}
			float mult = intercept + k*_phase / value;
			src[i] *= mult;
			_phase += _deltaT;
		}
		return src.size();
	}

	void applySustain(std::vector<float>& src, int curTick)
	{
		const float mult = _sustain->play(curTick)[0];
		for(int i = 0; i < src.size(); i++)
		{
			src[i] *= mult;
		}
	}

	void applyEnvelope(std::vector<float>& src, int curTick)
	{
		if (_curEnvelope == NONE)
			return;
		int pos = 0;
		while(pos < src.size())
		{
			switch(_curEnvelope)
			{
			case ATTACK:
				pos = linearApply(src, _attack, curTick, pos, 1, 0);
				break;
			case DECAY:
				pos = linearApply(src, _decay, curTick, pos, -1+_sustain->play(curTick)[0], 1);
				break;
			case SUSTAIN:
				applySustain(src, curTick);
				return;
			case RELEASE:
				pos = linearApply(src, _release, curTick, pos, -0.5, _sustain->play(curTick)[0]);
				break;
			}
			if(pos < src.size())
			{
				_phase = 0;
				if(_curEnvelope == RELEASE)
				{
					_curEnvelope = 0;
					return;
				}
				_curEnvelope++;
			}
		}
	}
public:
	ASDRProcessor(const DSPSettings& settings)
		: SignalProcessor(settings),
		_deltaT(1.f/settings.sampleRate),
		_attack(new ConstantGenerator(settings, 0.f)),
		_curEnvelope(NONE)
	{
	}

	void attack()
	{
		_curEnvelope = ATTACK;
	}
	
	void release()
	{
		_phase = 0;
		_curEnvelope = RELEASE;
	}

	const std::vector<float>& play(int curTick) override
	{
		if (_curEnvelope != NONE) {
			std::vector<float>& src = _source->play(curTick);
			applyEnvelope(src, curTick);
			return src;
		}
		return _silence;
	}


	void addSource(const std::shared_ptr<SignalGeneratorAbstract>& source, ParamName param) override {

		switch(param)
		{
		case ParamName::ATTACK:
				_attack = source;
				break;
		case ParamName::DECAY:
			_decay = source;
			break;
		case ParamName::SUSTAIN:
			_sustain = source;
			break;
		case ParamName::RELEASE:
			_release = source;
			break;
		default:
			SignalProcessor::addSource(source, param);
		}
	}
protected:
};

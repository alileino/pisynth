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

class AudioInput
{
public:
	virtual ~AudioInput() {}

	virtual const std::vector<float>& play(int tick) = 0;
};

class SignalGeneratorAbstract : public AudioInput
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

	std::vector<float>& play(int tick) override
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

class SignalProcessor : public SignalConsumerAbstract, public AudioInput
{ 
protected:
	std::shared_ptr<SignalGeneratorAbstract> _source;
	const DSPSettings& _settings;
public:
	SignalProcessor(const DSPSettings& settings)
		: _source(new ConstantGenerator(settings, 0.f)),
		_settings(settings)
	{}

	virtual ~SignalProcessor()
	{}

	virtual void addSource(const std::shared_ptr<SignalGeneratorAbstract>& source, ParamName param) override
	{
		_source = source;
	}

	const DSPSettings& getSettings()
	{
		return _settings;
	}

	virtual const std::vector<float>& play(int curTick) = 0;
};

class ADSRProcessor : public SignalProcessor
{
	std::shared_ptr<SignalGeneratorAbstract> _attack;
	std::shared_ptr<SignalGeneratorAbstract> _decay;
	std::shared_ptr<SignalGeneratorAbstract> _sustain;
	std::shared_ptr<SignalGeneratorAbstract> _release;

	const float _deltaT;
	int _curEnvelope = 0;

	float _curAmplitude = 0;
	std::vector<float> _silence = {0};

	enum EnvelopePhase
	{
		NONE   =0,
		ATTACK =1,
		DECAY  =2,
		SUSTAIN=3,
		RELEASE=4
	};

	int newApply(std::vector<float>& src,
		const std::shared_ptr<SignalGeneratorAbstract>& signal,
		int curTick, int pos,
		float a0, float at)
	{

		std::cout << _curEnvelope << " " << _curAmplitude << std::endl;
		float value = signal->play(curTick)[0];
		float deltaM = (at-a0)*(1.f/(value* _settings.sampleRate));
		for(int i = pos; i < src.size(); ++i)
		{
			if ((at - a0)*(at - _curAmplitude) <= 0)
			{
				_curAmplitude = at;
				return i;
			}

			_curAmplitude = _curAmplitude + deltaM;
			src[i] *= _curAmplitude;

		}
		return src.size();

	}

	void applySustain(std::vector<float>& src, int curTick, int pos)
	{
		_curAmplitude = _sustain->play(curTick)[0];

		for(int i = pos; i < src.size(); i++)
		{
			src[i] *= _curAmplitude;
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
				pos = newApply(src, _attack, curTick, pos, 0, 1);
				break;
			case DECAY: 
			{
				float at = _sustain->play(curTick)[0];
				if (at > _curAmplitude) {
					_curEnvelope = ATTACK;
					continue;
				}
				else {
					pos = newApply(src, _decay, curTick, pos, 1, at);
				}
				break;
			}
			case SUSTAIN:
				applySustain(src, curTick, pos);
				return;
			case RELEASE:
				pos = newApply(src, _release, curTick, pos, _sustain->play(curTick)[0], 0);
				break;
			}
			if(pos < src.size())
			{
				if(_curEnvelope == RELEASE)
				{
					std::fill(src.begin() + pos, src.end(), 0);
					_curEnvelope = 0;
					return;
				}
				_curEnvelope++;
			}
		}
	}
public:
	ADSRProcessor(const DSPSettings& settings)
		: SignalProcessor(settings),
		_attack(new ConstantGenerator(settings, 0.f)),
		_deltaT(1.f/settings.sampleRate),
		_curEnvelope(NONE)
	{
	}

	void attack()
	{
		if (_curAmplitude == 0)
			_curEnvelope = ATTACK;
		else 
			_curEnvelope = DECAY;
	}
	
	void release()
	{
		_curEnvelope = RELEASE;
	}

	float getAmplitude() const
	{
		return _curAmplitude;
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

	virtual ~ADSRProcessor(){}
protected:
};

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

			assert(_buffer.size() == _settings.bufferSize);
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
	FREQ
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

protected:
	void produce(std::vector<float>& dest, int tick) override {
		dest[0] = _value;
		freeze();
	}
};

class InterpolatingGenerator : public SignalGeneratorAbstract
{
	explicit InterpolatingGenerator(int bufferSize)
		: SignalGeneratorAbstract(bufferSize)
	{
	}

protected:
	void produce(std::vector<float>& dest, int tick) override;
};

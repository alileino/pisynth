#pragma once

#include <cassert>

#ifdef DEBUG
#define DEBUG_IF(x) if(x)
#else
#define DEBUG_IF(x) if(false)
#endif

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
	int _bufferSize;
public:


	SignalGeneratorAbstract(int bufferSize)
		:_buffer(bufferSize)
	{
		_bufferSize = bufferSize;
	}

	std::vector<float>& play(int tick)
	{
		if (tick > _curTick)
		{
			_curTick = tick;
			produce(_buffer);
			assert(_buffer.size() != _bufferSize);
		}

		return _buffer;
	}
	virtual ~SignalGeneratorAbstract()
	{
	}

protected:

	virtual void produce(std::vector<float>& dest) = 0;

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

	virtual void addSource(SignalGeneratorAbstract& source, ParamName param) = 0;
	virtual std::vector<ParamName> getParams() {
		return std::vector<ParamName>();
	}
};


class ConstantGenerator : public SignalGeneratorAbstract
{
private:
	float _value;
public:
	explicit ConstantGenerator(float value = 440.f)
		: SignalGeneratorAbstract(1),
		_value(value)
	{
	}

protected:
	void produce(std::vector<float>& dest) override {
		dest[0] = _value;
	}
};

class InterpolatingGenerator : public SignalGeneratorAbstract
{
	explicit InterpolatingGenerator(int bufferSize)
		: SignalGeneratorAbstract(bufferSize)
	{
	}

protected:
	void produce(std::vector<float>& dest) override;
};
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
	size_t _bufferSize;

	SignalGeneratorAbstract(const SignalGeneratorAbstract& that);
public:


	SignalGeneratorAbstract(size_t bufferSize)
		:_buffer(bufferSize)
	{
		_bufferSize = bufferSize;
	}

	std::vector<float>& play(int tick)
	{
		if (tick > _curTick)
		{
			_curTick = tick;

			assert(_buffer.size() == _bufferSize);
			produce(_buffer, tick);
		}

		return _buffer;
	}
	virtual ~SignalGeneratorAbstract()
	{
		std::cout << this << "destroyed" << std::endl;

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
	explicit ConstantGenerator(float value = 440.f)
		: SignalGeneratorAbstract(1),
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

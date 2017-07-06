#pragma once
#include <limits>
#include <queue>
#include "src/Utils.h"

class MidiBinding :public AudioInput {
private:
	float _scaleFactor = 1.f;
	float _minValue = 0.f;
	float _maxValue = 0.f;
protected:
	std::queue<float> _input;
public:
	MidiBinding(float scaleFactor, float minValue, float maxValue)
		: 
		_scaleFactor(scaleFactor),
		_minValue(minValue),
		_maxValue(maxValue)
	{}

	MidiBinding() {
		_minValue = std::numeric_limits<float>::min();
		_maxValue = std::numeric_limits<float>::max();
	}

	void push(float value)
	{
		_input.push(value);
	}

	bool hasInput() const {
		return !_input.empty();
	}

	float consume()
	{
		float value = _input.front();
		_input.pop();
		return value;
	}
};

class MidiConstantBinding : MidiBinding {
private:
	ConstantGenerator _generator;
public:
	MidiConstantBinding(const DSPSettings& settings, float defaultValue) 
	: _generator(settings, defaultValue) {

	}

	const std::vector<float>& play(int tick) override {
		while (hasInput())
			_generator.setValue(consume());
		return _generator.play(tick);
	}
};

#pragma once
#include <vector>
#include "Utils.h"

class ADSRProcessor : public SignalProcessor
{
	std::shared_ptr<AudioInput> _attack;
	std::shared_ptr<AudioInput> _decay;
	std::shared_ptr<AudioInput> _sustain;
	std::shared_ptr<AudioInput> _release;

	const float _deltaT;
	int _curEnvelope = 0;

	float _curAmplitude = 0;
	std::vector<float> _silence = { 0 };

	enum EnvelopePhase
	{
		NONE = 0,
		ATTACK = 1,
		DECAY = 2,
		SUSTAIN = 3,
		RELEASE = 4
	};

	int newApply(std::vector<float>& src,
		const std::shared_ptr<AudioInput>& signal,
		int curTick, int pos,
		float a0, float at)
	{

		std::cout << _curEnvelope << " " << _curAmplitude << std::endl;
		float value = signal->play(curTick)[0];
		float deltaM = (at - a0)*(1.f / (value* _settings.sampleRate));
		for (int i = pos; i < src.size(); ++i)
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

		for (int i = pos; i < src.size(); i++)
		{
			src[i] *= _curAmplitude;
		}
	}

	void applyEnvelope(std::vector<float>& src, int curTick)
	{
		if (_curEnvelope == NONE)
			return;
		int pos = 0;
		while (pos < src.size())
		{
			switch (_curEnvelope)
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
			if (pos < src.size())
			{
				if (_curEnvelope == RELEASE)
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
		_deltaT(1.f / settings.sampleRate),
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
	std::vector<float>& play(int curTick) override
	{
		if (_curEnvelope != NONE) {
			std::vector<float>& src = _source->play(curTick);
			applyEnvelope(src, curTick);
			return src;
		}
		return _silence;
	}


	void addSource(const std::shared_ptr<AudioInput>& source, ParamName param) override {

		switch (param)
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

	virtual ~ADSRProcessor() {}
protected:
};

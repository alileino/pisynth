
#include "Oscillator.h"

class TableOscillatorTest : public SignalGeneratorAbstract
{
protected:
	void produce(std::vector<float>& dest, int curTick) override
	{
		for (int i = 0; i < dest.size(); i++) dest[i] = 0;
		for(auto i =oscs.begin();
			i != oscs.end(); ++i)
		{
		    vector<float> temp = (*i)->play(curTick);
			for(int j = 0; j < temp.size() && j < dest.size(); j++)
			{
				dest[j] += 1.0/oscs.size()*temp[j];
			}
		}

	}
private:
	vector<TableOscillator*> oscs;
public:
	explicit TableOscillatorTest(int tableSize, int sampleRate, int bufferSize, int numosc)
		: SignalGeneratorAbstract(bufferSize)
	{
		for (int i = 0; i < numosc; i++)
		{
			oscs.push_back(new TableOscillator(tableSize, sampleRate, bufferSize));
		}
	}
};

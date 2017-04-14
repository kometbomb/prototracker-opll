#pragma once

struct OPLLSynth;

#include "IOscillator.h"

extern "C" {
    struct __OPLL;
}


class OPLLTrack: public IOscillator
{
	int mChannelIndex;
	OPLLSynth& mSynth;
	struct __OPLL* mOPLL;
	
	int mAttenuation, mFNumber, mBlockNumber, mInstrument;
	bool mKeyOn;
	
	void sendFrequency(bool keyOn);
	
public:
	
	OPLLTrack(int channel, OPLLSynth& synth);
	virtual ~OPLLTrack();
	
	void triggerRhythm(int mask);
	void releaseNote();
	
	virtual void triggerNote();
	virtual void handleTrackState(ITrackState& trackState);
	virtual void setFrequency(float frequency);
	virtual void setVolume(int volume);
	virtual void setInstrument(int instrument);
	virtual void update(int numSamples);
	virtual void render(Sample16 *buffer, int numSamples, int offset = 0);
};

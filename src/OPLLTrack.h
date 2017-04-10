#pragma once

#include "IOscillator.h"

extern "C" {
    struct __OPLL;
}


class OPLLTrack: public IOscillator
{
	struct __OPLL* mOPLL;
public:
	
	OPLLTrack(struct __OPLL* aOPLL);
	virtual ~OPLLTrack();
	
	virtual void triggerNote();
	virtual void handleTrackState(ITrackState& trackState);
	virtual void setFrequency(float frequency);
	virtual void setVolume(int volume);
	virtual void update(int numSamples);
	virtual void render(Sample16 *buffer, int numSamples, int offset = 0);
};

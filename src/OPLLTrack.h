#pragma once

#include "IOscillator.h"

struct OPLL;

class OPLLTrack: public IOscillator
{
	OPLL* mOPLL;
public:
	
	OPLLTrack(OPLL* aOPLL);
	virtual ~OPLLTrack();
	
	virtual void triggerNote();
	virtual void handleTrackState(ITrackState& trackState);
	virtual void setFrequency(float frequency);
	virtual void setVolume(int volume);
	virtual void update(int numSamples);
	virtual void render(Sample16 *buffer, int numSamples, int offset = 0);
};

#pragma once

#include "ITrackState.h"

struct OPLLTrackState: public ITrackState
{
	// Current instrument
	int mInstrument;
	
	// Bitmask for drums
	int mRhythmBits;
	
	// Signal key off
	bool mKeyOff;
	
	OPLLTrackState();
	
	virtual bool handleEffectZeroTick(const EffectParam& effect, PlayerState& playerState);
	virtual void handleEffectAnyTick(const EffectParam& effect, PlayerState& playerState);
};


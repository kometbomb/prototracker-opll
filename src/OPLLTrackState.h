#pragma once

#include "ITrackState.h"

struct OPLLTrackState: public ITrackState
{
	int mInstrument;
	
	OPLLTrackState();
	
	virtual bool handleEffectZeroTick(const EffectParam& effect, PlayerState& playerState);
	virtual void handleEffectAnyTick(const EffectParam& effect, PlayerState& playerState);
};


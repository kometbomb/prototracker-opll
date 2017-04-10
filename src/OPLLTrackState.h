#pragma once

#include "ITrackState.h"

struct OPLLTrackState: public ITrackState
{
	OPLLTrackState();
	
	virtual bool handleEffectZeroTick(const EffectParam& effect, PlayerState& playerState);
	virtual void handleEffectAnyTick(const EffectParam& effect, PlayerState& playerState);
};


#include "OPLLTrackState.h"
#include "EffectParam.h"

OPLLTrackState::OPLLTrackState()
	: ITrackState()
{

}


bool OPLLTrackState::handleEffectZeroTick(const EffectParam& effect, PlayerState& playerState)
{
	//int asByte = effect.getParamsAsByte();
	
	switch (effect.effect)
	{
		default:
			return ITrackState::handleEffectZeroTick(effect, playerState);
	}
	
	return false;
}


void OPLLTrackState::handleEffectAnyTick(const EffectParam& effect, PlayerState& playerState)
{
	ITrackState::handleEffectAnyTick(effect, playerState);
}

#include "OPLLTrackState.h"
#include "EffectParam.h"

OPLLTrackState::OPLLTrackState()
	: ITrackState(), mInstrument(1)
{

}


bool OPLLTrackState::handleEffectZeroTick(const EffectParam& effect, PlayerState& playerState)
{
	int asByte = effect.getParamsAsByte();
	
	switch (effect.effect)
	{
		case 'i':
			mInstrument = asByte & 15;
			break;
		
		default:
			return ITrackState::handleEffectZeroTick(effect, playerState);
	}
	
	return false;
}


void OPLLTrackState::handleEffectAnyTick(const EffectParam& effect, PlayerState& playerState)
{
	ITrackState::handleEffectAnyTick(effect, playerState);
}

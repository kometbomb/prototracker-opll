#include "OPLLTrackState.h"
#include "EffectParam.h"
#include "PlayerState.h"

OPLLTrackState::OPLLTrackState()
	: ITrackState(), mInstrument(1), mRhythmBits(0), mKeyOff(false)
{

}


bool OPLLTrackState::handleEffectZeroTick(const EffectParam& effect, PlayerState& playerState)
{
	int asByte = effect.getParamsAsByte();
	
	switch (effect.effect)
	{
		// Select the OPLL instrument 0-15 (0 = user instrument)
		
		case 'i':
			mInstrument = asByte & 15;
			break;
		
		// Trigger OPLL rhythm (bitmask)
		
		case 'r':
			mRhythmBits = asByte & 15;
			break;
		
		default:
			return ITrackState::handleEffectZeroTick(effect, playerState);
	}
	
	return false;
}


void OPLLTrackState::handleEffectAnyTick(const EffectParam& effect, PlayerState& playerState)
{
	int asByte = effect.getParamsAsByte();
	
	switch (effect.effect)
	{
		// We overwrite the Kxx effect here for our own purposes
		
		case 'k':
			if (asByte == playerState.tick)
				mKeyOff = true;
			break;
			
		default:
			ITrackState::handleEffectAnyTick(effect, playerState);
	}
}

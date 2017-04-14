#include "OPLLTrackState.h"
#include "EffectParam.h"
#include "PlayerState.h"
#include <algorithm>

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
			mRhythmBits = asByte & 31;
			break;
		
		case '4':
			mCarrierAD = asByte;
			break;
			
		case '5':
			mModulatorAD = asByte;
			break;
			
		case '6':
			mCarrierSR = asByte;
			break;
			
		case '7':
			mModulatorSR = asByte;
			break;
			
		case '8':
			mCarrierKeyscale = effect.param1;
			mModulatorKeyscale = effect.param2;
			break;
			
		case '9':
			mModulation = std::min(asByte, 63);
			break;
			
		case 'x':
			mCarrierMul = effect.param1;
			mModulatorMul = effect.param2;
			break;
			
		case 'y':
			mCarrierVibAMEG = effect.param1;
			mModulatorVibAMEG = effect.param2;
			break;
			
		case 'z':
			mFeedback = asByte & 7;
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
		
		case 'o':
			if (asByte == playerState.tick)
				mKeyOff = true;
			break;
			
		default:
			ITrackState::handleEffectAnyTick(effect, playerState);
	}
}

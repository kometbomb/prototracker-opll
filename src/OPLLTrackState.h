#pragma once

#include "ITrackState.h"

struct OPLLTrackState: public ITrackState
{
	// Current instrument
	int mInstrument;
	
	// User instrument data
	int mCarrierMul, mModulatorMul;
	int mCarrierAD, mModulatorAD;
	int mCarrierSR, mModulatorSR;
	int mFeedback, mCarrierKeyscale, mModulatorKeyscale;
	int mCarrierVibAMEG, mModulatorVibAMEG;
	int mModulation;
	
	// Bitmask for drums
	int mRhythmBits;
	
	// Signal key off
	bool mKeyOff;
	
	OPLLTrackState();
	
	virtual bool handleEffectZeroTick(const EffectParam& effect, PlayerState& playerState);
	virtual void handleEffectAnyTick(const EffectParam& effect, PlayerState& playerState);
};


#include "OPLLTrack.h"
#include <cmath>
#include <cstdio>
#include <algorithm>
#include "SDL.h"
#include "Sample.h"
#include "SequenceRow.h"
#include "OPLLTrackState.h"
#include "OPLLSynth.h"
#include "emu2413/emu2413.h"

OPLLTrack::OPLLTrack(int channel, OPLLSynth& synth)
	: IOscillator(), mChannelIndex(channel), mSynth(synth), mAttenuation(8), mFNumber(0), mBlockNumber(0), mInstrument(0), mKeyOn(false)
{
	mOPLL = mSynth.getOPLL();
}


OPLLTrack::~OPLLTrack()
{
}


void OPLLTrack::handleTrackState(ITrackState& trackState)
{
	IOscillator::handleTrackState(trackState);
	
	OPLLTrackState& opllTrackState = static_cast<OPLLTrackState&>(trackState);
	
	setInstrument(opllTrackState.mInstrument);
	
	// Check if key off is signaled by OPLLTrackState
	
	if (opllTrackState.mKeyOff)
	{
		releaseNote();
		
		// mKeyOff signal is consumed - set to false
		opllTrackState.mKeyOff = false;
	}
	
	if (opllTrackState.mRhythmBits)
	{
		triggerRhythm(opllTrackState.mRhythmBits);
		
		// Consumed the bits - set zero
		opllTrackState.mRhythmBits = 0;
	}
	
	/*
	int mCarrierAD, mModulatorAD;
	int mCarrierSR, mModulatorSR;
	int mCarrierKeyscale, mModulatorKeyscale;
	int mCarrierVibAmpEG, mModulatorVibAmpEG;
	*/
	
	if (opllTrackState.mModulation >= 0)
	{
		mSynth.sendModulation(opllTrackState.mModulation);
		opllTrackState.mModulation = -1;
	}
	
	if (opllTrackState.mFeedback >= 0)
	{
		mSynth.sendFeedback(opllTrackState.mFeedback);
		opllTrackState.mFeedback = -1;
	}
	
	if (opllTrackState.mCarrierMul >= 0)
	{
		mSynth.sendMul(0, opllTrackState.mCarrierMul);
		opllTrackState.mCarrierMul = -1;
	}
	
	if (opllTrackState.mModulatorMul >= 0)
	{
		mSynth.sendMul(1, opllTrackState.mModulatorMul);
		opllTrackState.mModulatorMul = -1;
	}
	
	if (opllTrackState.mCarrierAD >= 0)
	{
		mSynth.sendEnvelopeAD(0, opllTrackState.mCarrierAD);
		opllTrackState.mCarrierAD = -1;
	}
	
	if (opllTrackState.mModulatorAD >= 0)
	{
		mSynth.sendEnvelopeAD(1, opllTrackState.mModulatorAD);
		opllTrackState.mModulatorAD = -1;
	}
	
	if (opllTrackState.mCarrierSR >= 0)
	{
		mSynth.sendEnvelopeSR(0, opllTrackState.mCarrierSR);
		opllTrackState.mCarrierSR = -1;
	}
	
	if (opllTrackState.mModulatorSR >= 0)
	{
		mSynth.sendEnvelopeSR(1, opllTrackState.mModulatorSR);
		opllTrackState.mModulatorSR = -1;
	}
	
	if (opllTrackState.mCarrierVibAMEG >= 0)
	{
		mSynth.sendAMVibEGKSR(0, opllTrackState.mCarrierVibAMEG);
		opllTrackState.mCarrierVibAMEG = -1;
	}
	
	if (opllTrackState.mModulatorVibAMEG >= 0)
	{
		mSynth.sendAMVibEGKSR(1, opllTrackState.mModulatorVibAMEG);
		opllTrackState.mModulatorVibAMEG = -1;
	}
	
	if (opllTrackState.mCarrierKeyscale >= 0)
	{
		mSynth.sendKeyScale(0, opllTrackState.mCarrierKeyscale);
		opllTrackState.mCarrierKeyscale = -1;
	}
	
	if (opllTrackState.mModulatorKeyscale >= 0)
	{
		mSynth.sendKeyScale(1, opllTrackState.mModulatorKeyscale);
		opllTrackState.mModulatorKeyscale = -1;
	}
}


void OPLLTrack::triggerRhythm(int mask)
{
	// Send key off (all rhythm bits off) OR'd with rythm mode bit
	OPLL_writeReg(mOPLL, 0x0e, 0x20);
	
	// Send key ons from the mask OR'd with rhythm mode bit
	OPLL_writeReg(mOPLL, 0x0e, mask | 0x20);
}


void OPLLTrack::triggerNote()
{
	// Send key off
	sendFrequency(false);
	
	// Send key on
	sendFrequency(true);
	
	OPLL_writeReg(mOPLL, 0x30 + mChannelIndex, (mInstrument << 4) | mAttenuation);
	
	mKeyOn = true;
}




void OPLLTrack::sendFrequency(bool keyOn)
{
	// Send the lower 8 F-number bits
	OPLL_writeReg(mOPLL, 0x10 + mChannelIndex, mFNumber & 0xff);
	
	// Send key on/key off OR'd with sustain bit and the F-number + block
	OPLL_writeReg(mOPLL, 0x20 + mChannelIndex, (keyOn ? 0x30 : 0x20) | (mFNumber >> 8));
	
	// Instrument number and attenuation
	OPLL_writeReg(mOPLL, 0x30 + mChannelIndex, (mInstrument << 4) | mAttenuation);
}


void OPLLTrack::setInstrument(int instrument)
{
	mInstrument = instrument & 15;
}


void OPLLTrack::setFrequency(float frequency)
{
	mFNumber = frequency * 440 * (1 << (24)) / 49716;
	mBlockNumber = 0;
	
	// While F-number exceeds the F-number limits raise the block (octave)
	// and halve the F-number
	
	while (mFNumber >= 0x200 && mBlockNumber < 7)
	{
		mFNumber >>= 1;
		mBlockNumber++;
	}
	
	// Combine block number with F-number
	
	mFNumber |= mBlockNumber << 9;
	
	sendFrequency(mKeyOn);
}


void OPLLTrack::setVolume(int volume)
{
	// Should be converted to a linear ramp
	
	mAttenuation = std::max(0, 15 - 15 * volume / 64);
	
	sendFrequency(mKeyOn);
}


void OPLLTrack::releaseNote()
{
	mKeyOn = false;
	
	sendFrequency(false);
}


void OPLLTrack::update(int numSamples)
{
	
}


void OPLLTrack::render(Sample16 *buffer, int numSamples, int offset)
{
	if (mChannelIndex > 0)
		return;
	
	for (int i = offset ; i < numSamples ; ++i)
	{
		int sample[2];
		
		OPLL_calc_stereo(mOPLL, sample);
		
		buffer[i].left += sample[0];
		buffer[i].right += sample[1];
	}
}

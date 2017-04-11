#include "OPLLTrack.h"
#include <cmath>
#include <cstdio>
#include <algorithm>
#include "SDL.h"
#include "Sample.h"
#include "SequenceRow.h"
#include "OPLLTrackState.h"
#include "emu2413/emu2413.h"

OPLLTrack::OPLLTrack(int channel, struct __OPLL* aOPLL)
	: IOscillator(), mChannelIndex(channel), mOPLL(aOPLL), mAttenuation(8), mFNumber(0), mBlockNumber(0), mInstrument(0), mKeyOn(false)
{
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
	int ar = 8;
    int dr = 8;
    OPLL_writeReg(mOPLL, 4, (ar << 4) + dr);
    
    int sl = 8;
    int rr = 8;
	int vl = mAttenuation;
	
	OPLL_writeReg(mOPLL, 6, (sl << 4) + rr);
	
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

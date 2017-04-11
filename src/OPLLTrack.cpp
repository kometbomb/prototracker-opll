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
	
	sendFrequency(false);
	sendFrequency(true);
	
	OPLL_writeReg(mOPLL, 0x30 + mChannelIndex, (mInstrument << 4) | vl);
	
	mKeyOn = true;
}


void OPLLTrack::sendFrequency(bool keyOn)
{
	OPLL_writeReg(mOPLL, 0x10 + mChannelIndex, mFNumber & 0xff);
	OPLL_writeReg(mOPLL, 0x20 + mChannelIndex, (keyOn ? 0x10 : 0) | (mFNumber >> 8));
	OPLL_writeReg(mOPLL, 0x30 + mChannelIndex, (mInstrument << 4) | mAttenuation);
}


void OPLLTrack::setInstrument(int instrument)
{
	mInstrument = instrument & 15;
}


void OPLLTrack::setFrequency(float frequency)
{
	mFNumber = frequency * 440 * (1 << (20)) / 49716;
	mBlockNumber = 0;
	
	while (mFNumber >= 0x200 && mBlockNumber < 7)
	{
		mFNumber >>= 1;
		mBlockNumber++;
	}
	
	mFNumber |= mBlockNumber << 9;
	
	//printf("%x %x\n", mFNumber, mBlockNumber);
	
	sendFrequency(mKeyOn);
}


void OPLLTrack::setVolume(int volume)
{
	// This sets to the volume to 1/SequenceRow::maxTracks (25 %)
	// to avoid clipping if all tracks output at max signal.
	
	//mVolume = volume * volumeResolution / TrackState::maxVolume / SequenceRow::maxTracks;
	
	mAttenuation = std::max(0, 15 - 15 * volume / 64);
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
		
		//sample [0]= 10000;
		//sample [1]= 10000;
		
		buffer[i].left += sample[0];
		buffer[i].right += sample[1];
	}
}

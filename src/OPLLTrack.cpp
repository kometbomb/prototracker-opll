#include "OPLLTrack.h"
#include <math.h>
#include <cstdio>
#include "SDL.h"
#include "Sample.h"
#include "SequenceRow.h"
#include "OPLLTrackState.h"

OPLLTrack::OPLLTrack(OPLL* aOPLL)
	: IOscillator(), mOPLL(aOPLL)
{
}


OPLLTrack::~OPLLTrack()
{
}


void OPLLTrack::handleTrackState(OPLLTrackState& trackState)
{
	IOscillator::handleTrackState(trackState);
	
}


void OPLLTrack::triggerNote()
{
	// We do nothing here. Just let the wave run free.
}


void OPLLTrack::setFrequency(float frequency)
{
	//mSpeed = frequency * oscillatorResolution * oscillatorLength*2;
}


void OPLLTrack::setVolume(int volume)
{
	// This sets to the volume to 1/SequenceRow::maxTracks (25 %)
	// to avoid clipping if all tracks output at max signal.
	
	//mVolume = volume * volumeResolution / TrackState::maxVolume / SequenceRow::maxTracks;
}


void OPLLTrack::update(int numSamples)
{
	
}


void OPLLTrack::render(Sample16 *buffer, int numSamples, int offset)
{
	
	
	for (int i = 0 ; i < numSamples ; ++i)
	{
		int sample = 0;
		
		buffer[i].left += sample;
		buffer[i].right += sample;
	}
}

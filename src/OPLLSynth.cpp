#include "OPLLSynth.h"
#include "OPLLTrack.h"
#include "SequenceRow.h"
#include "Sample.h"
#include "SDL.h"
#include "emu2413/emu2413.h"

OPLLSynth::OPLLSynth()
	: ISynth()
{
	mOPLL = OPLL_new(4000000, 44100);
	
	/* 
	
	Initialize the audio tracks.
	
	*/
	
	for (int i = 0 ; i < SequenceRow::maxTracks ; ++i)
	{
		OPLLTrack *oscillator = new OPLLTrack(mOPLL);
		mOscillator[i] = oscillator;
	}
}


OPLLSynth::~OPLLSynth()
{
	OPLL_delete(mOPLL);
	
	/*
	
	NOTE: ~ISynth() will delete the Oscillator objects we initialized
	above! No need to cleanup yourself.
	
	*/
}

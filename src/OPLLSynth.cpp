#include "OPLLSynth.h"
#include "OPLLTrack.h"
#include "SequenceRow.h"
#include "Sample.h"
#include "SDL.h"
#include "emu2413/emu2413.h"

OPLLSynth::OPLLSynth()
	: ISynth()
{
	mOPLL = OPLL_new(3579545, 44100);
	
	/* 
	
	Initialize the audio tracks.
	
	*/
	
	for (int i = 0 ; i < SequenceRow::maxTracks ; ++i)
	{
		OPLLTrack *oscillator = new OPLLTrack(i, mOPLL);
		mOscillator[i] = oscillator;
	}
	
	/*
	
	Set up OPLL rhythm mode
	
	*/
	
	OPLL_writeReg(mOPLL, 0x0e, 0x20);
	
	/*
	
	Yamaha's suggestions from the manual:
	
		$16 - Set to $20   F-Num LSB for channel 7 (slots 13,16)
		$17 - Set to $50   F-Num LSB for channel 8 (slots 14,17)
		$18 - Set to $C0   F-Num LSB for channel 9 (slots 15,19)
		$26 - Set to $05   Block/F-Num MSB for channel 7
		$27 - Set to $05   Block/F-Num MSB for channel 8
		$28 - Set to $01   Block/F-Num MSB for channel 9
	*/
	
	OPLL_writeReg(mOPLL, 0x16, 0x20);
	OPLL_writeReg(mOPLL, 0x17, 0x50);
	OPLL_writeReg(mOPLL, 0x18, 0xc0);
	OPLL_writeReg(mOPLL, 0x26, 0x05);
	OPLL_writeReg(mOPLL, 0x27, 0x05);
	OPLL_writeReg(mOPLL, 0x28, 0x01);
}


OPLLSynth::~OPLLSynth()
{
	OPLL_delete(mOPLL);
	
	/*
	
	NOTE: ~ISynth() will delete the Oscillator objects we initialized
	above! No need to cleanup yourself.
	
	*/
}

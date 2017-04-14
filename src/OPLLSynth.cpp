#include "OPLLSynth.h"
#include "OPLLTrack.h"
#include "SequenceRow.h"
#include "Sample.h"
#include "SDL.h"
#include "emu2413/emu2413.h"

#ifndef SAMPLERATE
#define SAMPLERATE 44100
#endif

OPLLSynth::OPLLSynth()
	: ISynth()
{
	mOPLL = OPLL_new(3579545, SAMPLERATE);
	
	/* 
	
	Initialize the audio tracks.
	
	*/
	
	for (int i = 0 ; i < SequenceRow::maxTracks ; ++i)
	{
		OPLLTrack *oscillator = new OPLLTrack(i, *this);
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


struct __OPLL* OPLLSynth::getOPLL()
{
	return mOPLL;
}


void OPLLSynth::sendModulation(int modulation)
{
	mModulation = modulation;
	
	updateRegister02();
}


void OPLLSynth::sendMul(int op, int mul)
{
	mOp[op].MUL = mul & 15;
	updateRegister00(op);
}


void OPLLSynth::sendFeedback(int feedback)
{
	mFeedback = feedback;
	updateRegister03();
}


void OPLLSynth::sendAMVibEGKSR(int op, int bits)
{
	mOp[op].AM = (bits & 0x8) ? 1 : 0;
	mOp[op].VIB = (bits & 0x4) ? 1 : 0;
	mOp[op].EG = (bits & 0x2) ? 1 : 0;
	mOp[op].KSR = (bits & 0x1);
	
	updateRegister00(op);
}


void OPLLSynth::sendKeyScale(int op, int keyScale)
{
	if (op == 0)
	{
		mCarrierKeyScale = keyScale & 3;
		updateRegister02();
	}
	else
	{
		mModulatorKeyScale = keyScale & 3;
		updateRegister03();
	}
}


void OPLLSynth::sendEnvelopeAD(int op, int ad)
{
	OPLL_writeReg(mOPLL, 0x04 + op, ad);
}


void OPLLSynth::sendEnvelopeSR(int op, int sr)
{
	OPLL_writeReg(mOPLL, 0x06 + op, sr);
}


void OPLLSynth::sendShape(int op, int shape)
{
	if (op == 0)
	{
		mCarrierShape = shape ? 1 : 0;
	}
	else
	{
		mModulatorShape = shape ? 1 : 0;
	}
	
	updateRegister03();
}


void OPLLSynth::updateRegister00(int op)
{
	OPLL_writeReg(mOPLL, 0x00 + op, (mOp[op].AM << 7) | (mOp[op].VIB << 6) | (mOp[op].EG << 5) | (mOp[op].KSR << 4) | mOp[op].MUL);
}


void OPLLSynth::updateRegister02()
{
	OPLL_writeReg(mOPLL, 0x02, (mCarrierKeyScale << 6) | mModulation);
}


void OPLLSynth::updateRegister03()
{
	OPLL_writeReg(mOPLL, 0x03, (mModulatorKeyScale << 6) | (mCarrierShape << 4) | (mModulatorShape << 3) | mFeedback);
}

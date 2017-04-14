#pragma once

#include "ISynth.h"

extern "C" {
    struct __OPLL;
}

class OPLLSynth: public ISynth
{
	struct __OPLL* mOPLL;
	
	// Address $00-$01
	struct {
		int AM, VIB, EG, KSR, MUL;
	} mOp[2];
	
	// Address $02
	int mCarrierKeyScale, mModulation; 	
	
	// Address $03
	int mModulatorKeyScale, mCarrierShape, mModulatorShape, mFeedback; 
	
	// Combine mOp values and put to register $00+op
	void updateRegister00(int op);
	
	// Combine values and put to register $02
	void updateRegister02();
	
	// Combine values and put to register $03
	void updateRegister03();
	
public:
	OPLLSynth();
	virtual ~OPLLSynth();
	
	// op: 0 = carrier, 1 = modulator
	void sendMul(int op, int mul);
	void sendFeedback(int feedback);
	void sendKeyScale(int op, int feedback);
	void sendAMVibEGKSR(int op, int bits);
	void sendEnvelopeAD(int op, int ad);
	void sendEnvelopeSR(int op, int sr);
	void sendShape(int op, int shape);
	void sendModulation(int modulation);
	
	struct __OPLL* getOPLL();
};

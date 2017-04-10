#pragma once

#include "ISynth.h"

struct OPLL;

class OPLLSynth: public ISynth
{
	OPLL *mOPLL;
public:
	OPLLSynth();
	virtual ~OPLLSynth();
};

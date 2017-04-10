#pragma once

#include "ISynth.h"

extern "C" {
    struct __OPLL;
}

class OPLLSynth: public ISynth
{
	struct __OPLL* mOPLL;
public:
	OPLLSynth();
	virtual ~OPLLSynth();
};

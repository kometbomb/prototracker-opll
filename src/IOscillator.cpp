#include "IOscillator.h"
#include "ITrackState.h"

IOscillator::IOscillator()
	: mSampleRate(0)
{
}


IOscillator::~IOscillator()
{
}


void IOscillator::handleTrackState(ITrackState& trackState)
{
}


void IOscillator::setSampleRate(int rate)
{
	mSampleRate = rate;
}

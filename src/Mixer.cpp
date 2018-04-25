#include "Debug.h"
#include "Mixer.h"
#include "SDL.h"
#include "IPlayer.h"
#include "Sample.h"
#include "ISynth.h"
#include "SequenceRow.h"
#include "ITrackState.h"
#include "IOscillator.h"
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#ifndef TUNING
#define TUNING 440.0
#endif

#ifndef SAMPLERATE
#define SAMPLERATE 44100
#endif

Mixer::Mixer(IPlayer& player, ISynth& synth)
	: mPlayer(player), mSynth(synth), mSampleRate(0), mThread(NULL), mAudioOpened(false), mBuffer(NULL),
	mDeviceId(0)
{
	mConvert = static_cast<SDL_AudioCVT*>(SDL_malloc(sizeof(SDL_AudioCVT)));
	buildDeviceList();
}


Mixer::~Mixer()
{
	deinitAudio();

	free(mConvert);
}


bool Mixer::runThread(const char *deviceName)
{
	debug("Opening audio device %s", deviceName);

	if (!initAudio(deviceName))
		return false;

	SDL_PauseAudioDevice(mDeviceId, 0);

	return true;
}


void Mixer::stopThread()
{
	deinitAudio();
}


bool Mixer::initAudio(const char *deviceName)
{
	if (deviceName == NULL)
	{
		// Just use the first device that we can find
		buildDeviceList();
		if (getNumDevices() == 0)
			return false;

		return initAudio(getDevice(0));
	}
	SDL_AudioSpec want, have;

	SDL_memset(&want, 0, sizeof(want));

	want.freq = SAMPLERATE;
	want.format = AUDIO_S16SYS;
	want.channels = 2;
	want.samples = 1024;
	want.callback = static_cast<SDL_AudioCallback>(audioCallback);
	want.userdata = this;

	if (mAudioOpened)
		SDL_CloseAudio();

	mAudioOpened = false;
	mDeviceId = SDL_OpenAudioDevice(deviceName, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);

	if (mDeviceId <= 0)
	{
		debug("Failed to open audio: %s", SDL_GetError());
		return false;
	}

	strncpy(mCurrentDevice, deviceName, MAX_DEVICE_NAME_SIZE);

	mAudioOpened = true;

	mSampleRate = have.freq;
	mSamples = 0;
	mBufferSize = have.samples;

	if (mBuffer != NULL)
		delete[] mBuffer;

	mBuffer = new Sample16[mBufferSize];

	SDL_BuildAudioCVT(mConvert, want.format, want.channels, have.freq, have.format, have.channels, have.freq);

	//printf("Got %d Hz format=%d (wanted %d Hz/%d) buffer = %d\n", have.freq, have.format, want.freq, want.format, want.samples);

	mSynth.setSampleRate(mSampleRate);

	return true;
}


void Mixer::deinitAudio()
{
	mThreadRunning = false;

	if (mThread != NULL)
	{
		SDL_WaitThread(mThread, NULL);
	}

	if (mAudioOpened)
		SDL_CloseAudioDevice(mDeviceId);

	mAudioOpened = false;

	if (mBuffer != NULL)
		delete[] mBuffer;

	mBuffer = NULL;
	mDeviceId = 0;
}


void Mixer::audioCallback(void* userdata, unsigned char* stream, int len)
{
	Uint64 startTicks = SDL_GetPerformanceCounter();
	Mixer& mixer = *static_cast<Mixer*>(userdata);
	IPlayer& player = mixer.getPlayer();

	Sample16 *data = reinterpret_cast<Sample16*>(stream);
	int length = mixer.mBufferSize; //len / sizeof(Sample16);
	int chunk = mixer.getSampleRate() / player.getPlayerState().songRate;
	float hzConversion = static_cast<float>(TUNING / 2) / (float)mixer.getSampleRate(); // 1.0 = 440 Hz
	int samples = std::min(length, (chunk - mixer.getSamples() % chunk) % chunk);

	/*
	 * Render "leftovers" before the next sequence tick
	 */

	if (samples > 0)
	{
		mixer.getSynth().render(data, samples);
		mixer.getSynth().update(samples);
		mixer.getSamples() += samples;

		player.lock();
		player.getPlayerState().setUpdated(PlayerState::OscillatorProbe);
		player.unlock();
	}

	/*
	 * Fill the rest of the buffer and handle ticks
	 */

	for (int i = samples ; i < length ; i += chunk)
	{
		player.lock();

		player.runTick();
		player.advanceTick();

		for (int track = 0 ; track < SequenceRow::maxTracks ; ++track)
		{
			ITrackState& trackState = player.getTrackState(track);
			IOscillator& oscillator = mixer.getSynth().getOscillator(track);

			if (trackState.triggered)
			{
				trackState.triggered = false;
				oscillator.triggerNote();
			}

			oscillator.handleTrackState(trackState);
			oscillator.setFrequency(trackState.trackState.frequency * trackState.macroState.frequency * hzConversion);

			if (trackState.enabled)
				oscillator.setVolume(trackState.trackState.volume * trackState.macroState.volume / ITrackState::maxVolume);
			else
				oscillator.setVolume(0);
		}

		chunk = mixer.getSampleRate() / player.getPlayerState().songRate;

		player.unlock();

		int toBeWritten = std::min(length - i, chunk);

		ISynth& synth = mixer.getSynth();
		synth.render(data + i, toBeWritten);
		synth.update(toBeWritten);
		mixer.getSamples() += toBeWritten;

		player.lock();
		player.getPlayerState().setUpdated(PlayerState::OscillatorProbe);
		player.unlock();
	}

	mixer.mConvert->len = length * sizeof(Sample16);
	mixer.mConvert->buf = stream;
	SDL_ConvertAudio(mixer.mConvert);

	player.lock();
	player.getPlayerState().cpuUse = 100 * static_cast<float>(SDL_GetPerformanceCounter() - startTicks) / SDL_GetPerformanceFrequency() / (static_cast<float>(length) / mixer.getSampleRate());
	player.unlock();
}


int Mixer::getSampleRate() const
{
	return mSampleRate;
}


IPlayer& Mixer::getPlayer()
{
	return mPlayer;
}

ISynth& Mixer::getSynth()
{
	return mSynth;
}


int& Mixer::getSamples()
{
	return mSamples;
}


bool Mixer::isThreadRunning() const
{
	return mThreadRunning;
}


void Mixer::buildDeviceList()
{
	mNumDevices = SDL_GetNumAudioDevices(0);
	for (int index = 0 ; index < mNumDevices && index < MAX_DEVICES ; ++index)
	{
		strncpy(mDeviceList[index], SDL_GetAudioDeviceName(index, 0), MAX_DEVICE_NAME_SIZE);
	}
}


const int Mixer::getNumDevices() const
{
	return mNumDevices;
}


const char* Mixer::getDevice(int index) const
{
	return mDeviceList[index];
}


SDL_AudioDeviceID Mixer::getCurrentDeviceID() const
{
	return mDeviceId;
}


const char* Mixer::getCurrentDeviceName() const
{
	if (mDeviceId <= 0)
		return NULL;

	return mCurrentDevice;
}

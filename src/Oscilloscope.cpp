#include "Oscilloscope.h"
#include "Renderer.h"
#include "Color.h"
#include "Sample.h"
#include "IPlayer.h"
#include "TrackState.h"
#include "Oscillator.h"

Oscilloscope::Oscilloscope(EditorState& editorState, IPlayer& player, int channel)
	: Editor(editorState, false), mPlayer(player), mChannel(channel)
{
}

	
Oscilloscope::~Oscilloscope()
{
}


void Oscilloscope::setBuffer(const Sample16 *buffer, int length)
{
	mBuffer = buffer;
	mBufferLength = length;
	setDirty(true);
}


void Oscilloscope::onDraw(Renderer& renderer, const SDL_Rect& area)
{
	setDirty(false);
	renderer.renderBackground(area);
	
	SDL_Point points[area.w];

	for (int x = 0 ; x < area.w ; ++x)
	{
		int bufferPos = mBufferLength * x / area.w;
		int y = mBuffer[bufferPos].left * area.h / Oscillator::oscillatorResolution / 2 + area.h / 2;
		
		if (y >= area.h)
			y = area.h - 1;
		else if (y < 0)
			y = 0;
		
		points[x].x = area.x + x;
		points[x].y = area.y + y;
	}
	
	renderer.renderPoints(points, area.w, Color());
}


bool Oscilloscope::onEvent(SDL_Event& event)
{
	if (event.type == SDL_MOUSEBUTTONDOWN)
	{
		mPlayer.getTrackState(mChannel).enabled ^= true;
		return true;
	}
	
	return false;
}


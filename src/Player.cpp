#include "Player.h"
#include "OPLLTrackState.h"
#include "SequenceRow.h"

Player::Player(Song& song)
	: IPlayer(song)
{
	/*
	
	Initialize the ITrackState derived TrackState for each track
	
	*/
	
	for (int i = 0 ; i < SequenceRow::maxTracks ; ++i)
		trackState[i] = new OPLLTrackState();
}

Player::~Player()
{
	/* 
	
	NOTE: ~IPlayer() will handle cleanup, including deleting the
	TrackState objects initialized above!
	
	*/
}

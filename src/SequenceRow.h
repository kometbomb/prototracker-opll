#pragma once

struct SequenceRow
{
	static const int maxTracks = 6;
	
	int pattern[maxTracks];
	
	void clear();
	bool isEmpty() const;
	
	SequenceRow();
};




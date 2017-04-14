#include "Song.h"
#include "Sequence.h"
#include "SequenceRow.h"
#include "Pattern.h"
#include "Sequence.h"
#include "Macro.h"
#include "PatternRow.h"
#include "FileSection.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <strings.h>
#include <algorithm>

Song::Song()
	: patternLength(64), sequenceLength(1)
{
	sequence = new Sequence();
	patterns = new Pattern[maxPatterns];
	macros = new Macro[maxMacros];
	strcpy(name, "new song");
}


Song::~Song()
{
	delete[] patterns;
	delete[] macros;
	delete sequence;
}


Sequence& Song::getSequence()
{
	return *sequence;
}


int Song::getLastPatternUsed() const
{
	int last = -1;
	
	for (int i = 0 ; i < maxPatterns ; ++i)
	{
		if (!patterns[i].isEmpty())
			last = std::max(last, i);
	}
	
	return last;
}


int Song::getLastMacroUsed() const
{
	int last = -1;

	for (int i = 0 ; i < maxMacros ; ++i)
	{
		if (strlen(macros[i].getName()) > 0 || !macros[i].isEmpty())
			last = std::max(last, i);
	}
	
	return last;
}


int Song::getPatternLength() const
{
	return patternLength;
}


int Song::getSequenceLength() const
{
	return sequenceLength;
}


Pattern& Song::getPattern(int pattern)
{
	return patterns[pattern];
}


Macro& Song::getMacro(int macro)
{
	return macros[macro];
}


void Song::setPatternLength(int length)
{
	patternLength = length;
}


void Song::setSequenceLength(int length)
{
	sequenceLength = length;
}


FileSection* Song::pack() 
{
	FileSection * song = FileSection::createSection("SONG");
	
	/*
	 * Basic song info
	 */
	
	// File format version
	song->writeByte(songVersion);
	song->writeByte(SequenceRow::maxTracks);
	song->writeString(name);
	song->writeByte(patternLength - 1);
	song->writeByte(sequenceLength - 1);
	
	/*
	 * Sequence data
	 */
	
	FileSection * sequenceData = FileSection::createSection("SEQU");
	int sequenceCount = sequence->getLastSequenceRowUsed() + 1;
	sequenceData->writeByte(sequenceCount);
	for (int track = 0 ; track < SequenceRow::maxTracks ; ++track)
	{
		for (int i = 0 ; i < sequenceCount ; ++i)
		{			
			SequenceRow& sequenceRow = sequence->getRow(i);
			sequenceData->writeByte(sequenceRow.pattern[track]);
		}
	}
	song->writeSection(*sequenceData);
	delete sequenceData;
	
	/*
	 * Patterns
	 */
	
	FileSection * patternData = FileSection::createSection("PATT");
	int patternCount = getLastPatternUsed() + 1;
	patternData->writeByte(patternCount);
	for (int i = 0 ; i < patternCount ; ++i)
	{
		Pattern& pattern = getPattern(i);
		patternData->writePattern(pattern);
	}
	song->writeSection(*patternData);
	delete patternData;
	
	/*
	 * Macros
	 */
	
	FileSection * macroData = FileSection::createSection("MACR");
	int macroCount = getLastMacroUsed() + 1;
	printf("Saving %d macros\n", macroCount);
	macroData->writeByte(macroCount);
	for (int i = 0 ; i < macroCount ; ++i)
	{
		Macro& macro = getMacro(i);
		macroData->writeString(macro.getName());
		macroData->writePattern(macro);
	}
	song->writeSection(*macroData);
	delete macroData;
	
	return song;
}


void Song::clear()
{
	sequence->clear();
	
	strcpy(name, "");
	
	for (int i = 0 ; i < maxPatterns ; ++i)
	{
		patterns[i].clear();
	}
	
	for (int i = 0 ; i < maxMacros ; ++i)
	{
		macros[i].clear();
	}
	
	sequenceLength = 1;
	patternLength = 64;
}


bool Song::unpack(const FileSection& section)
{
	if (strcmp(section.getName(), "SONG") != 0)
		return false;
	
	clear();
	
	int offset = 0;
	int version = section.readByte(offset);
	
	if (version > songVersion)
	{
		// TODO: display a proper error if the version is newer than we can handle
		return false;
	}
	
	// Default trackCount for version == 0
	int trackCount = 4;
	
	if (version == FileSection::invalidRead)
		return false;
	
	if (version >= 1)
	{
		trackCount = section.readByte(offset);
		
		if (trackCount == FileSection::invalidRead)
			return false;
	}
	
	const char *songName = section.readString(offset);
	
	if (songName == NULL)
		return false;
	
	strncpy(name, songName, songNameLength + 1);
	
	int temp = section.readByte(offset);
	
	if (temp == FileSection::invalidRead)
		return false;
	
	patternLength = temp + 1;
	
	temp = section.readByte(offset);
	
	if (temp == FileSection::invalidRead)
		return false;
	
	sequenceLength = temp + 1;
	
	bool returnValue = true;
	
	do
	{
		printf("offset = %d\n", offset);
		FileSection *subSection = section.readSection(offset);
		int subOffset = 0;
		
		if (subSection == NULL)
			break;
		
		const char *sectionName = subSection->getName();
		
		if (sectionName != NULL)
		{
			printf("Read section %s (%d bytes)\n", sectionName, subSection->getSize());
			
			if (strcmp(sectionName, "SEQU") == 0)
			{
				int temp = subSection->readByte(subOffset);
				
				if (temp == FileSection::invalidRead)
				{
					returnValue = false;
				}
				else
				{
					int count = temp;
					
					printf("Reading %d sequences\n", count);
					
					if (count > maxSequenceRows)
					{
						returnValue = false;
					}
					else
					{
						for (int track = 0 ; track < trackCount && returnValue ; ++track)
						{
							for (int i = 0 ; i < count ; ++i)
							{
								int temp = subSection->readByte(subOffset);
								
								if (temp == FileSection::invalidRead)
								{
									returnValue = false;
									break;
								}
								
								// Skip tracks that can't fit in our sequence
								if (track < SequenceRow::maxTracks)
									sequence->getRow(i).pattern[track] = temp;
							}
						}
					}
				}
			}
			else if (strcmp(sectionName, "PATT") == 0)
			{
				int temp = subSection->readByte(subOffset);
				
				if (temp == FileSection::invalidRead)
				{
					returnValue = false;
				}
				else
				{
					int count = temp;
					
					printf("Reading %d patterns\n", count);
					
					for (int i = 0 ; i < count ; ++i)
					{
						if (!subSection->readPattern(patterns[i], subOffset))
						{
							returnValue = false;
							break;
						}
					}
				}
			}
			else if (strcmp(sectionName, "MACR") == 0)
			{
				int temp = subSection->readByte(subOffset);
				
				if (temp == FileSection::invalidRead)
				{
					returnValue = false;
				}
				else
				{
					int count = temp;
					
					printf("Reading %d macros\n", count);
					
					for (int i = 0 ; i < count ; ++i)
					{
						const char *macroName = subSection->readString(subOffset);
						
						if (macroName == NULL)
						{
							returnValue = false;
							break;
						}
						
						strncpy(macros[i].getName(), macroName, Macro::macroNameLength + 1);
						
						if (!subSection->readPattern(macros[i], subOffset))
						{
							returnValue = false;
							break;
						}
					}
				}
			}
			else
			{
				printf("Unknown section %s\n", sectionName);
				returnValue = false;
			}
		}
		else
		{
			returnValue = false;
		}
		
		//offset += subOffset;
		
		delete subSection;
	}
	while (returnValue);
	
	return returnValue;
}


char *Song::getSongName()
{
	return name;
}


int Song::getSize() const
{
	return 0;
}

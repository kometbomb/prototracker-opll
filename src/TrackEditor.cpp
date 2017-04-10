#include "TrackEditor.h"
#include "PatternRow.h"
#include "Renderer.h"
#include "Color.h"
#include "Player.h"
#include "Pattern.h"
#include "EditorState.h"
#include <cstdio>


TrackEditor::TrackEditor(EditorState& editorState, TrackEditorState& trackEditorState, IPlayer& player, Song& song, int tracks)
	: ColumnEditor(editorState, trackEditorState, tracks, PatternRow::NumColumns), mPlayer(player), mSong(song), mTriggerNotes(true), mAddMacroEffect(true)
{
	
}


TrackEditor::~TrackEditor()
{
}


void TrackEditor::setTriggerNotes(bool state)
{
	mTriggerNotes = state;
}


void TrackEditor::setAddMacroEffect(bool state)
{
	mAddMacroEffect = state;
}


int TrackEditor::getColumnFlagsFromModifier(int mod) const
{
	int flags = PatternRow::FlagAllColumns;
	
	if (mod & KMOD_LALT)
		flags = PatternRow::FlagEffect;
	else if (mod & KMOD_LCTRL)
		flags = PatternRow::FlagNote;
	
	return flags;
}


void TrackEditor::changeColumn(int d)
{
	if (d < 0)
	{
		--mTrackEditorState.currentColumn;
		if (mTrackEditorState.currentColumn == 1 && getCurrentPatternRow().shouldSkipParam1())
			mTrackEditorState.currentColumn = 0;
		else if (mTrackEditorState.currentColumn < 0)
		{
			mTrackEditorState.currentColumn = mColumns - 1;
			changeTrack(-1);
		}
	}
	else 
	{
		++mTrackEditorState.currentColumn;
		if (mTrackEditorState.currentColumn == 1 && getCurrentPatternRow().shouldSkipParam1())
			mTrackEditorState.currentColumn = 2;
		if (mTrackEditorState.currentColumn >= mColumns)
		{
			mTrackEditorState.currentColumn = 0;
			changeTrack(1);
		}
	}
}


void TrackEditor::killTrack(int track)
{
	getCurrentPattern(track).clear();
	mTrackEditorState.currentRow.notify();
}


bool TrackEditor::onEvent(SDL_Event& event)
{
	switch (event.type)
	{
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym)
			{
				case SDLK_F3:
					copyTrack(mTrackEditorState.currentTrack);
					return true;
					
				case SDLK_F4:
					pasteTrack(mTrackEditorState.currentTrack);
					return true;
				
				case SDLK_PAGEDOWN:
					scrollView(16, false);
					
					return true;
					break;
					
				case SDLK_PAGEUP:
					scrollView(-16, false);
					
					return true;
					break;
				
				case SDLK_DOWN:
					scrollView(1);
					
					return true;
					break;
					
				case SDLK_UP:
					scrollView(-1);
					
					return true;
					break;
					
				case SDLK_HOME:
					mTrackEditorState.currentRow = 0;
					return true;
					break;
				
				case SDLK_END:
					mTrackEditorState.currentRow = maxRows - 1;
					return true;
					break;
					
				case SDLK_TAB:
					if (event.key.keysym.mod & KMOD_SHIFT)
						changeTrack(-1);
					else
						changeTrack(1);
					return true;
					break;
					
				case SDLK_LEFT:
					changeColumn(-1);
					return true;
					
				case SDLK_RIGHT:
					changeColumn(1);
					return true;
					
				case SDLK_BACKSPACE:
					if (mTrackEditorState.currentRow == 0)
						break;
					scrollView(-1);
					if (event.key.keysym.mod & (KMOD_LSHIFT|KMOD_LALT|KMOD_LCTRL))
					{
						deleteRow(false, getColumnFlagsFromModifier(event.key.keysym.mod));
					}
					else
					{
						playRow();
					}
					return true;
					break;
					
				case SDLK_DELETE:
					emptyRow(false, getColumnFlagsFromModifier(event.key.keysym.mod));
					scrollView(mTrackEditorState.editSkip);
					return true;
					break;
					
				case SDLK_INSERT:
					insertRow(false, getColumnFlagsFromModifier(event.key.keysym.mod));
					return true;
					break;
					
				case SDLK_RETURN:
					if (event.key.keysym.mod & (KMOD_LSHIFT|KMOD_LALT|KMOD_LCTRL))
					{
						if (mTrackEditorState.currentRow == maxRows - 1)
							break;
						insertRow(false, getColumnFlagsFromModifier(event.key.keysym.mod));
					}
					else
					{
						playRow();
					}
					
					scrollView(1);
					return true;
					break;
				
				default:
				{
					if (event.key.keysym.mod & KMOD_LCTRL)
					{
						int hex = getHexFromKey(event.key.keysym.sym);
						if (hex >= 0 && hex <= 9)
						{
							mTrackEditorState.editSkip = hex;
						}
						else if (event.key.keysym.sym == SDLK_c)
						{
							copyBlock(mTrackEditorState.currentTrack);
						}
						else if (event.key.keysym.sym == SDLK_v)
						{
							pasteBlock(mTrackEditorState.currentTrack);
						}
						else if (event.key.keysym.sym == SDLK_b)
						{
							setBlockStart(mTrackEditorState.currentRow);
						}
						else if (event.key.keysym.sym == SDLK_e)
						{
							setBlockEnd(mTrackEditorState.currentRow);
						}
						else if (event.key.keysym.sym == SDLK_k)
						{
							killTrack(mTrackEditorState.currentTrack);
						}
						else if (event.key.keysym.sym == SDLK_u)
						{
							findUnusedTrack(mTrackEditorState.currentTrack);
						}
						else
							return false;
						
						return true;
					}
					
					if (event.key.keysym.mod & (KMOD_CTRL|KMOD_ALT))
						return false;
					
					if (!mEditorState.editMode)
					{
						if (event.key.repeat == 0)
						{
							int note = getNoteFromKey(event.key.keysym.scancode);
									
							if (note != -1/* && mTrackEditorState.currentColumn == PatternRow::Column::Note && !(event.key.keysym.mod & KMOD_SHIFT)*/)
							{
								mPlayer.triggerNoteWithReset(mTrackEditorState.currentTrack, note + mEditorState.octave * 12, mEditorState.macro);
								return true;
							}
						}
						
						break;
					}
					
					PatternRow& patternRow = getCurrentPatternRow();
					bool handled = false;
					
					{
						switch (static_cast<int>(mTrackEditorState.currentColumn))
						{
							case PatternRow::Column::EffectType:
							{
								int c = getCharFromKey(event.key.keysym.sym);
								if (c != -1)
								{
									patternRow.effect.effect = c;
									handled = true;
								}
								
							}
								break;
								
							case PatternRow::Column::EffectParam1:
							{
								int hex = getHexFromKey(event.key.keysym.sym);
								if (hex != -1)
								{
									patternRow.effect.param1 = hex;
									handled = true;
								}
							}
								break;
							
							case PatternRow::Column::EffectParam2:
							{
								int hex = getHexFromKey(event.key.keysym.sym);
								if (hex != -1)
								{
									patternRow.effect.param2 = hex;
									handled = true;
								}								
							}
								break;						
							
							case PatternRow::Column::NoteParam1:
							{
								int hex = getHexFromKey(event.key.keysym.sym);
								if (hex != -1)
								{
									if (patternRow.note.effect != 'n')
									{
										patternRow.note.param1 = hex;
									}
									
									handled = true;
								}								
							}
								break;
								
								
							case PatternRow::Column::NoteParam2:
							{
								int hex = getHexFromKey(event.key.keysym.sym);
								if (hex != -1)
								{
									patternRow.note.param2 = hex;
									handled = true;
								}								
							}
								break;
							
							case PatternRow::Column::Note:
								if (event.key.keysym.mod & KMOD_SHIFT)
								{
									int c = getCharFromKey(event.key.keysym.sym);
									if (c != -1)
									{
										patternRow.note.effect = c;
										
										if (c == 'n')
										{
											patternRow.note.param1 = std::min(11, patternRow.note.param1);
											patternRow.note.param2 = std::min(15, patternRow.note.param2);
										}
										
										handled = true;
									}
								}
								else
								{
									int note = getNoteFromKey(event.key.keysym.scancode);
									
									if (note != -1)
									{
										patternRow.setNoteAndOctave(mEditorState.octave * 12 + note);
										
										if (mAddMacroEffect && patternRow.effect.isEmpty())
										{
											patternRow.effect.effect = 'm';
											patternRow.effect.setParamsFromByte(mEditorState.macro);
										}
										
										handled = true;
										
										//mPlayer.triggerNote(mTrackEditorState.currentTrack, patternRow);
										if (mEditorState.followPlayPosition && mTriggerNotes)
											mPlayer.triggerNoteWithReset(mTrackEditorState.currentTrack, note + mEditorState.octave * 12, mEditorState.macro);
									}
								
								}
								
								break;
						}
						
						if (handled)
						{
							mTrackEditorState.currentRow += mTrackEditorState.editSkip;
							mTrackEditorState.currentRow %= maxRows;
							
							return true;
						}
					}
				}
				break;
			}
	}
	
	return false;
}


void TrackEditor::renderPatternRow(Renderer& renderer, const SDL_Rect& textArea, const PatternRow& row, const Color& color, int columnFlags)
{
	SDL_Rect effectPosition = textArea;
	effectPosition.x += renderer.getFontWidth() * 3;
	
	if (row.getNote() != PatternRow::NoNote)
		renderer.renderTextV(textArea, color * Color::getEffectColor(row.note), "%s%x", PatternRow::getNoteName(row.getNote()), row.getOctave());
	else if (!row.note.isEmpty())
		renderer.renderTextV(textArea, color * Color::getEffectColor(row.note), "%c%x%x", row.note.effect, row.note.param1, row.note.param2);
	else
		renderer.renderText(textArea, color * Color::getEffectColor(row.note), "---");
	
	if (row.effect.isEmpty())
		renderer.renderText(effectPosition, color * Color::getEffectColor(row.effect), "---");
	else
		renderer.renderTextV(effectPosition, color * Color::getEffectColor(row.effect), "%c%x%x", row.effect.effect, row.effect.param1, row.effect.param2);
}


void TrackEditor::onDraw(Renderer& renderer, const SDL_Rect& area)
{
	setDirty(false);
	
	renderer.renderBackground(area);
	
	int rowNumberWidth = 3 * renderer.getFontWidth() + mRowNumberMargin;
	int trackWidth = mColumns * renderer.getFontWidth() + mTrackMargin;
	int rowHeight = renderer.getFontHeight();
	
	int countVisible = area.h / rowHeight;
	int firstVisible = mTrackEditorState.currentRow - countVisible / 2;
	int lastVisible = mTrackEditorState.currentRow + countVisible / 2;
	
	int centerY = (area.h / 2) & ~7;
	
	SDL_Rect editBar = {area.x, area.y + centerY, area.w, rowHeight};
	renderer.renderRect(editBar, Color());
	
	if (firstVisible < 0)
		firstVisible = 0;
	
	if (lastVisible > maxRows)
		lastVisible = maxRows;
	
	for (int row = firstVisible ; row < lastVisible ; ++row)
	{
		SDL_Rect textArea = {area.x, (row - mTrackEditorState.currentRow) * rowHeight + area.y + centerY, trackWidth, rowHeight};
		Color color = Color::getColor(Color::ColorType::RowCounter);
			
		if (row == mTrackEditorState.currentRow)
			color = Color(0,0,0);
		else if (row >= mTrackEditorState.blockStart && row <= mTrackEditorState.blockEnd)
			color = Color(255,0,0);
		
		renderer.renderTextV(textArea, color, "%03d", row);
	}
	
	if (hasFocus())
	{
		int columnWidth = renderer.getFontWidth();
		
		int columnX = renderer.getFontWidth() * mTrackEditorState.currentColumn;
		
		SDL_Rect textArea = {mTrackEditorState.currentTrack * trackWidth + area.x + rowNumberWidth + columnX, area.y + centerY, columnWidth, rowHeight};
		renderer.renderRect(textArea, Color::getColor(mEditorState.editMode ? Color::ColorType::EditCursor : Color::ColorType::NonEditCursor));
	}	
	
	for (int track = 0 ; track < maxTracks ; ++track)
	{
		for (int row = firstVisible ; row < lastVisible ; ++row)
		{
			PatternRow& patternRow = getPatternRow(track, row);
			SDL_Rect textArea = {track * trackWidth + area.x + rowNumberWidth, (row - mTrackEditorState.currentRow) * rowHeight + area.y + centerY, trackWidth, rowHeight};
			
			Color color;
			
			if (row == mTrackEditorState.currentRow)
				color = Color(0,0,0);
			
			renderPatternRow(renderer, textArea, patternRow, color);
		}
	}
}


void TrackEditor::playRow()
{
	for (int track = 0 ; track < maxTracks ; ++track)
		mPlayer.triggerNote(track, getPatternRow(track, mTrackEditorState.currentRow));
}


void TrackEditor::insertRow(bool allTracks, int flags)
{
	getCurrentPattern(mTrackEditorState.currentTrack).insertRow(mTrackEditorState.currentRow, flags);
	mTrackEditorState.currentRow.notify();
}


void TrackEditor::deleteRow(bool allTracks, int flags)
{
	getCurrentPattern(mTrackEditorState.currentTrack).deleteRow(mTrackEditorState.currentRow, flags);
}


void TrackEditor::emptyRow(bool allTracks, int flags)
{
	getCurrentPattern(mTrackEditorState.currentTrack).getRow(mTrackEditorState.currentRow).clear(flags);
}


void TrackEditor::copyTrack(int track)
{
	printf("copy----\n");
	printf("opic %c\n", mEditorState.copyBuffer.getRow(0).effect.effect);
	printf("opic %c\n", getCurrentPattern(track).getRow(0).effect.effect);
	mEditorState.copyBuffer.copy(getCurrentPattern(track), 0, 255);
	printf("opic %c\n", mEditorState.copyBuffer.getRow(0).effect.effect);
	printf("opic %c\n", getCurrentPattern(track).getRow(0).effect.effect);
}


void TrackEditor::pasteTrack(int track)
{
	printf("opic %c\n", mEditorState.copyBuffer.getRow(0).effect.effect);
	printf("opic %c\n", getCurrentPattern(track).getRow(0).effect.effect);
	mEditorState.copyBuffer.paste(getCurrentPattern(track), 0);
	mTrackEditorState.currentRow.notify();
}


void TrackEditor::setBlockStart(int row)
{
	mTrackEditorState.blockStart = row;
	
	if (mTrackEditorState.blockEnd < 0)
		mTrackEditorState.blockEnd = row;
	
	/*if (mTrackEditorState.blockStart > mTrackEditorState.blockEnd)
	{
		int temp = mTrackEditorState.blockStart;
		mTrackEditorState.blockStart = mTrackEditorState.blockEnd;
		mTrackEditorState.blockEnd = temp;
	}*/
	
	mTrackEditorState.currentRow.notify();
}


void TrackEditor::setBlockEnd(int row)
{
	mTrackEditorState.blockEnd = row;
	
	if (mTrackEditorState.blockStart < 0)
		mTrackEditorState.blockStart = row;
	
	/*if (mTrackEditorState.blockStart > mTrackEditorState.blockEnd)
	{
		int temp = mTrackEditorState.blockStart;
		mTrackEditorState.blockStart = mTrackEditorState.blockEnd;
		mTrackEditorState.blockEnd = temp;
	}*/
	
	mTrackEditorState.currentRow.notify();
}


void TrackEditor::copyBlock(int track)
{
	if (mTrackEditorState.blockStart < 0 || mTrackEditorState.blockEnd < 0) 
		return;
		
	mEditorState.copyBuffer.copy(getCurrentPattern(track), mTrackEditorState.blockStart, mTrackEditorState.blockEnd);
}


void TrackEditor::pasteBlock(int track)
{
	mEditorState.copyBuffer.paste(getCurrentPattern(track), mTrackEditorState.currentRow);
	mTrackEditorState.currentRow.notify();
}

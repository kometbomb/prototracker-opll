#include "SequenceRowEditor.h"
#include "SequenceRow.h"
#include "Sequence.h"
#include "Song.h"
#include "EditorState.h"
#include "Renderer.h"
#include "Color.h"

SequenceRowEditor::SequenceRowEditor(EditorState& editorState, Song& song)
	: ColumnEditor(editorState, editorState.sequenceEditor, SequenceRow::maxTracks, 2), mSong(song)
{
	mTrackEditorState.currentRow.addListener(this);
	mTrackEditorState.currentColumn.addListener(this);
	mTrackEditorState.currentTrack.addListener(this);
}


SequenceRowEditor::~SequenceRowEditor()
{
}


void SequenceRowEditor::insertRow(bool allTracks, int flags)
{
	mSong.getSequence().insertRow(mTrackEditorState.currentRow, allTracks);
	
	if (mSong.getSequenceLength() < Song::maxSequenceRows)
		mSong.setSequenceLength(mSong.getSequenceLength() + 1);
	
	mTrackEditorState.currentRow.notify();
}


void SequenceRowEditor::deleteRow(bool allTracks, int flags)
{
	mSong.getSequence().deleteRow(mTrackEditorState.currentRow, allTracks);
	
	if (mSong.getSequenceLength() > 1)
		mSong.setSequenceLength(mSong.getSequenceLength() - 1);
	
	if (mTrackEditorState.currentRow >= mSong.getSequenceLength())
	{
		mTrackEditorState.currentRow = mSong.getSequenceLength() - 1;
	}
	
	mTrackEditorState.currentRow.notify();
}


void SequenceRowEditor::emptyRow(bool allTracks, int flags)
{
	//mSong.getSequence().emptyRow(mEditorState.currentRow);
}


bool SequenceRowEditor::onEvent(SDL_Event& event)
{
	switch (event.type)
	{
		case SDL_KEYDOWN:
			
			switch (event.key.keysym.sym)
			{
				case SDLK_DOWN:
					scrollView(1, false);
					
					return true;
					break;
					
				case SDLK_UP:
					scrollView(-1, false);
					
					return true;
					break;
					
				case SDLK_LEFT:
					changeColumn(-1);
					return true;
					
				case SDLK_RIGHT:
					changeColumn(1);
					return true;
					
				case SDLK_HOME:
					mTrackEditorState.currentRow = 0;
					
					return true;
					break;
				
				case SDLK_END:
					mTrackEditorState.currentRow = mSong.getSequenceLength() - 1;
					
					return true;
					break;
					
				case SDLK_BACKSPACE:
					if (mTrackEditorState.currentRow == 0)
						break;
					scrollView(-1, false);
					deleteRow(false, 0);
					return true;
					break;
					
				case SDLK_DELETE:
					deleteRow(false, 0);
					return true;
					break;
					
				case SDLK_INSERT:
					insertRow(false, 0);
					return true;
					break;
					
				case SDLK_RETURN:
					/*if (mTrackEditorState.currentRow == mSong.getSequenceLength() - 1)
						break;*/
					insertRow(false, 0);
					
					scrollView(1, false);
					return true;
					break;
					
				default:
				{
					if (event.key.keysym.mod & KMOD_CTRL)
					{
						if (event.key.keysym.sym == SDLK_d)
						{
							duplicateRow();
							return true;
						}
						
						break;
					}
					
					if (!mEditorState.editMode)
						break;
					
					SequenceRow& sequenceRow = mSong.getSequence().getRow(mTrackEditorState.currentRow);
					
					int hex = getHexFromKey(event.key.keysym);
					if (hex != -1)
					{
						int& pattern = sequenceRow.pattern[mTrackEditorState.currentTrack];
						
						if (mTrackEditorState.currentColumn == 0)
						{
							pattern = (pattern & 0x0f) | (hex << 4);
						}
						else
						{
							pattern = (pattern & 0xf0) | hex;
						}
						
						scrollView(1, false);
						
						return true;
					}
				}
					break;
					
			}
			break;
			
			case SDL_CONTROLLERBUTTONDOWN: 
			{
				bool aPressed = SDL_GameControllerGetButton(SDL_GameControllerFromInstanceID(event.cbutton.which), SDL_CONTROLLER_BUTTON_A);
				SequenceRow& sequenceRow = mSong.getSequence().getRow(mTrackEditorState.currentRow);
				
				switch (event.cbutton.button)
				{
					case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
					case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
						if (!aPressed)
						{
							changeColumn(event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT ? -1 : 1);
						}
						else
						{
							int& pattern = sequenceRow.pattern[mTrackEditorState.currentTrack];
						
							pattern = std::min(Song::maxPatterns - 1, std::max(0, pattern + (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT ? -1 : 1)));
							
							// We need to signal the listeners that the sequence has changed (i.e. tell PatternEditor to redraw)
							// Otherwise this is automatically caused by scrollView() when typing in numbers
							mTrackEditorState.currentRow.notify();
						}
						
						break;
					
					case SDL_CONTROLLER_BUTTON_DPAD_UP:
					case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
						if (!aPressed)
						{
							scrollView(event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP ? -1 : 1);
						}
						else
						{
							int& pattern = sequenceRow.pattern[mTrackEditorState.currentTrack];
						
							pattern = std::min(Song::maxPatterns - 1, std::max(0, pattern + (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN ? -16 : 16)));
							
							// We need to signal the listeners that the sequence has changed (i.e. tell PatternEditor to redraw)
							// Otherwise this is automatically caused by scrollView() when typing in numbers
							mTrackEditorState.currentRow.notify();
						}
						
						break;
				}
				break;
			}
	}
	return false;
}


void SequenceRowEditor::onDraw(Renderer& renderer, const SDL_Rect& area)
{
	setDirty(false);
	
	renderer.renderBackground(area);
	
	int rowNumberWidth = 2 * 8 + 2;
	int trackWidth = 8 * 2;
	int rowHeight = 8;
	
	int countVisible = area.h / 8;
	int firstVisible = mTrackEditorState.currentRow - countVisible / 2;
	int lastVisible = mTrackEditorState.currentRow + countVisible / 2;
	
	int centerY = (area.h / 2) & ~7;
	
	if (firstVisible < 0)
		firstVisible = 0;
	
	if (lastVisible >= mSong.getSequenceLength())
		lastVisible = mSong.getSequenceLength() - 1;
	
	if (hasFocus())
	{
		int columnWidth = 8;
		
		int columnX = 8 * mTrackEditorState.currentColumn;
		
		SDL_Rect textArea = {mTrackEditorState.currentTrack * trackWidth + area.x + rowNumberWidth + columnX, area.y + centerY, columnWidth, rowHeight};
		renderer.renderRect(textArea, Color::getColor(mEditorState.editMode ? Color::ColorType::EditCursor : Color::ColorType::NonEditCursor));
	}
	
	for (int row = firstVisible ; row <= lastVisible ; ++row)
	{
		SDL_Rect textArea = {area.x, (row - mTrackEditorState.currentRow) * rowHeight + area.y + centerY, trackWidth, rowHeight};
		Color color = Color::getColor(Color::ColorType::RowCounter);
			
		if (row == mTrackEditorState.currentRow)
			color = Color(0,0,0);
		
		renderer.renderTextV(textArea, color, "%02d", row);
	}
	
	for (int row = firstVisible ; row <= lastVisible ; ++row)
	{
		SequenceRow& sequenceRow = mSong.getSequence().getRow(row);
		for (int track = 0 ; track < SequenceRow::maxTracks ; ++track)
		{
			SDL_Rect textArea = {track * trackWidth + area.x + rowNumberWidth, (row - mTrackEditorState.currentRow) * rowHeight + area.y + centerY, trackWidth, rowHeight};
			
			Color color;
			
			if (row == mTrackEditorState.currentRow)
				color = Color(0,0,0);
			
			renderer.renderTextV(textArea, color, "%02x", sequenceRow.pattern[track]);
		}
	}
}


void SequenceRowEditor::duplicateRow()
{
	if (mTrackEditorState.currentRow >= Sequence::maxRows && mSong.getSequenceLength() < Song::maxSequenceRows)
		return;
	
	mSong.getSequence().insertRow(mTrackEditorState.currentRow, true);
	
	SequenceRow& destination = mSong.getSequence().getRow(mTrackEditorState.currentRow);
	SequenceRow& source = mSong.getSequence().getRow(mTrackEditorState.currentRow + 1);
	
	for (int track = 0 ; track < SequenceRow::maxTracks ; ++track)
	{
		destination.pattern[track] = source.pattern[track];
	}
	
	mSong.setSequenceLength(mSong.getSequenceLength() + 1);
	
	mTrackEditorState.currentRow.notify();
	
	showMessage(MessageInfo, "Duplicated sequence row");
}

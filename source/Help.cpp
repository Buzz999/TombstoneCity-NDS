/*
 *  Copyright (C) 2008-2011 Florent Bedoiseau (electronique.fb@free.fr)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Help.h"

const char *help[]={
  "Object",
  "------",
  "The object of the game is quite ",
  "simple. The player controls a",
  "schooner,shooting aliens (Morgs)",
  "and tumbleweeds in order to woo ",
  "people back to livein an",
  "abandoned city somewhere in the ",
  "Southwestern United States. The ",
  "score is listed as Population.",
  "",
  "The game is basically never- ",
  "ending, meaning the only way the",
  "game will end is when all",
  "schooners are gone.",
  "",
  "Gameplay",
  "--------",
  "The player's schooner starts out",
  "in the middle of sixteen blocks.",
  "These are used as shelter as the",
  "Morgs cannot enter this area. ",
  "The player must venture out to",
  "shoot either tumbleweeds",
  "(100 points each) or, more ",
  "importantly,the Morgs (150 ",
  "points each).",
  "",
  "The Morgs are produced via one",
  "of the cacti out in the field.",
  "They can only appear from a",
  "cactus that is touching another ",
  "cactus. The goal is to leave",
  "only cacti standing individually",
  "so that no more Morgs can appear",
  "",
  "Strategy",
  "--------",
  "The Morgs can only appear from a",
  "cactus that is immediately next ",
  "to another cactus, and whenever ",
  "a Morg is shot and killed, it ",
  "turns into another cactus. One ",
  "useful strategy is to try to ",
  "shoot a Morg against two cacti,",
  "because all three cacti will dis",
  "appear and become another Morg ",
  "that, if shot immediately, will ",
  "turn into a stand-alone cactus.",
  "",  
  "The day ends when all cacti are ",
  "standing alone and not up ",
  "against one another. (The day",
  "number, indicating the game",
  "level, is located at the bottom ",
  "left hand corner of the screen.)",
  "A new day starts with the cacti ",
  "in new positions, and the player",
  "receives a bonus schooner.",
  "",
  "Tumbleweeds are simply",
  "replenished after the last one",
  "is eliminated.",
  "",  
  "The sixteen blocks in the center",
  "of the screen are used as base ",
  "and shelter for the schooner.",
  "Any time a schooner is killed by",
  "a Morg, the new schooner starts ",
  "in the middle. If a Morg is",
  "killed right next to one of the ",
  "exits, a cactus is placed there ",
  "and the exit is blocked off for ",
  "the day. When all exits are ",
  "blocked off, the schooner is",
  "automatically moved to somewhere",
  "outside. If the schooner is",
  "killed at this point, the new",
  "schooner appears somewhere else ",
  "on the board. Usually by the",
  "time it is located by the player",
  "the Morg(s) have already",
  "decended upon it.",
  "",
  "Documentation comes from : ",
  "http://en.wikipedia.org/wiki/",
  "Tombstone_City: 21st Century",
  "",
  "Keys",
  "----",
  "Pad arrows : move",
  "A or X     : fire",
  "R or L     : panic",
  0
};

Help::Help ():_index(0), _lines(0) {
  _lines=0;
  while (help[_lines]!=0) _lines++;
}

Help::~Help() {
}

void Help::Process() {
  bool redraw = true;
  PA_ClearTextBg (0);
  PA_OutputSimpleText(0, 0, 0, "Touch screen to quit help");
  PA_OutputSimpleText(0, 0, 1, "Use arrows to navigate");
  while (Stylus.Held) { PA_WaitForVBL(); }

  while (1) {
	if (Pad.Held.Down) { /* DOWN */
	  if (_index < (_lines - 22)) {
		_index++;
		redraw=true;
	  }
	}
	if (Pad.Held.Up) {
	  if (_index > 0) {
		_index--;
		redraw=true;
	  }
	}
	if (Stylus.Newpress) {
	  PA_ClearTextBg (0);
	  
	  return; // End
	}
	
	if (redraw) {
	  int i;
	  redraw=false;
	  PA_SetTextTileCol(0, TEXT_BLUE);
	  for (i=0; i < 22; i++) {
		PA_OutputText(0, 0, i+2, "%s", "                                ");
		PA_OutputText(0, 0, i+2, "%s", help[i + _index]);
	  }
	  PA_SetTextTileCol(0, TEXT_BLACK);
	}
	PA_WaitForVBL();
  }
}

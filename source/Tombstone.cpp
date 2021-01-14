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

// TO DO
// ShakeListPosition: pas necessaire

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h> // gettimeofday 

#include "Tombstone.h"
#include "Help.h"

int _message_num = 0;
void ShowMessage ();

// --------------------------------
// FPOINT
// --------------------------------
FPoint::FPoint (const FPoint& aFPoint) {
    x=aFPoint.x;
    y=aFPoint.y;
}

FPoint& FPoint::operator=(const FPoint& aFPoint) {
    if (this==&aFPoint) return *this;
    x=aFPoint.x;
    y=aFPoint.y;
    return *this;
}


// -------------------------------
// SPRITE
// -------------------------------
Sprite::Sprite () {
    game=0;
    x=0;
    y=0;
    old_x=0;
    old_y=0;
    tileNumber=0;
    oldtileNumber=0;
    shown=0;
}

Sprite::Sprite (int atile_num, int ax, int ay, Game *g) {
    game=g;
    x=ax;
    y=ay;
    old_x=ax;
    old_y=ay;
    tileNumber=atile_num;
    oldtileNumber=FLOOR; // game->GetBoard (x, y);
    shown=0;
}

Sprite::Sprite (const Sprite& aSprite) {
    game=aSprite.game;
    x=aSprite.x;
    y=aSprite.y;
    old_x=aSprite.old_x;
    old_y=aSprite.old_y;
    tileNumber=aSprite.tileNumber;
    oldtileNumber=aSprite.oldtileNumber;
    shown=aSprite.shown;
}

Sprite& Sprite::operator=(const Sprite& aSprite) {
    if (this==&aSprite) return *this;
    game=aSprite.game;
    x=aSprite.x;
    y=aSprite.y;
    old_x=aSprite.old_x;
    old_y=aSprite.old_y;
    tileNumber=aSprite.tileNumber;
    oldtileNumber=aSprite.oldtileNumber;
    shown=aSprite.shown;

    return *this;
}

Sprite::~Sprite (void) {
}

void Sprite::Move (void) {
    Hide ();
    old_x=x;
    old_y=y;
    Show ();
}

void Sprite::Show () {
    if (shown==1) return;
    oldtileNumber=game->GetBoard (x, y);
    game->DrawPixmap (x, y, tileNumber);
    shown=1;
}

void Sprite::Hide () {
    if (shown==0) return;
    game->DrawPixmap (old_x, old_y, oldtileNumber);
    shown=0;
}

// -------------------------------
// GAME
// -------------------------------
int Game::TimerEvent () {
    if (_end_flag==1) return _end_flag; // Do nothing

    TimeMoveBuissons ();
    TimeMoveMonsters ();
    TimeMovePlayer ();
    TimeFirePlayer (); 
    TimeFire ();
    TimeShoot ();
    TimeCreateMonsters ();
    TimeCreateBuissons ();

    if (_end_flag==1) {
	PA_ClearTextBg (0);
	_message_num = 0; // "Press Start to begin"
	ShowMessage();
    }

    return _end_flag;
}

void Game::CreateTiles (void) {
    tiles [BLOCK_1]= block_1;
    tiles [BLOCK_2]= block_2;
    tiles [MONSTER_1]=monster_1;
    tiles [MONSTER_2]=monster_2;
    tiles [CACTUS_1]= cactus_1;
    tiles [CACTUS_2]= cactus_2;
    tiles [PLAYER_1]=player_1;
    tiles [PLAYER_2]=player_2;
    tiles [PLAYER_3]=player_3;
    tiles [PLAYER_4]=player_4;
    tiles [FLOOR]=floor;
    tiles [TIR_1]=tir_1;
    tiles [TIR_2]=tir_2;
    tiles [SPLASH]=splash;
    tiles [BUISSON_1]=buisson_1;
    tiles [BUISSON_2]=buisson_2;
    tiles [BOARD]=board;
    tiles [SCORE]=score;
    tiles [GAMEOVER]=gameover;
    tiles [OUT]=0; // Not a tile
}

void Game::DeleteTiles (void) {
}

void Game::DrawPixmap (int x, int y, int aTile) {
    _board [y][x]=aTile;
    if (tiles [aTile]) {
	PA_LoadGifXY (1, (x*8) + 16, y*8,(void *) tiles [aTile]);
    }
}

Game::Game () {
    _score=0; /* POPULATION */
    _day=1;
    _life=10; /* SHOONERS */
    _end_flag=0;
    for (int i=0; i < SOUND_MAX; i++) {
	_stoppedChannel[i]= true;
    }

    CreateTiles (); // load bitmaps for all tiles
    InitGame ();
}

Game::~Game () {
    EndGame ();
    DeleteTiles ();
    for (int i=0; i < SOUND_MAX; i++) {
	StopSoundChannel(i);
    }
}

void Game::StartSoundChannel (int ch) {
    if (ch >= SOUND_MAX || ch < 0) return;
    if (_stoppedChannel == false) return; // Already running

    _stoppedChannel[ch]=false;
    switch (ch) {
	case SOUND_THEME:
	    PA_PlaySoundRepeat(ch, theme);
	    break;
	case SOUND_FIRE:
	     PA_PlaySimpleSound(ch, fire); // Only once
	    break;
	case SOUND_KILLED:
	    PA_PlaySimpleSound(ch, dead); // Only once
	    break;
	case SOUND_MONSTER:
	    PA_PlaySimpleSound(ch, monster); // Only once
	    break;
	default: break;
    }
}

void Game::StopSoundChannel (int ch) {
    if (ch >= SOUND_MAX || ch < 0) return;
    if (_stoppedChannel[ch]==true) return; // Already stopped

    PA_StopSound(ch); // Stop
    _stoppedChannel[ch]=true;
}

bool Game::IsStoppedSoundChannel (int ch) {
    if (ch >= SOUND_MAX || ch < 0) return false;
    return _stoppedChannel[ch]; 
}


int Game::CheckIfBlocked () const {
    int x,y,c;
    int nb=0;
    for (x=12;x<=16;x+=2) {  
	c=GetBoard (x,7);
	if (c==CACTUS_1 || c==CACTUS_2) nb++;
	c=GetBoard (x,15);
	if (c==CACTUS_1 || c==CACTUS_2) nb++;
    }
 
    for (y=9;y<=13;y+=2) {
	c=GetBoard (10,y);
	if (c==CACTUS_1 || c==CACTUS_2) nb++;
	c=GetBoard (18,y);
	if (c==CACTUS_1 || c==CACTUS_2) nb++;
    }

    if (nb!=12) return 0;
    return 1; // Blocked
}

void Game::CreatePlayer (void) {
    if (!player_sprite) {
	time_player=0;
	player_direction=1; /* Up */
	player_sprite=new Sprite (PLAYER_1, 14, 11, this);
	player_sprite->Show ();
	player_blocked=0;
    }
}

void Game::ChangeDay (void) {
    _day++;
    EndGame ();
    InitGame ();
}

void Game::InitGame (void) { 
    int i,j;

    /* Board vide */
    for (j=0; j < 24; j++) {
	for (i=0; i < 28; i++) {
	    _board [j][i]=FLOOR; 
	}
    }

    /* Les choses n'etant pas systematiquement creees */
    /* FIRE */
    fire_sprite=0;
    fire_direction=0; 
    fire_in_progress=NO_FIRE;

    shoot_monster=0;
    splash_sprite=0;
 
    mListOfMonsters.clear();
    mListOfCactus.clear ();
    mListOfBuissons.clear ();
    mListOfCages.clear ();

    /* Autre */
    mx=0;
    my=0;
    cx=0;
    cy=0;
    MonsterToBeCreated=0;

    /* Les flags de temps */
    time_create_monster=0; /* Pas de monstre a creer */
    time_move_monster=0; /* Pas de monstre a deplacer */
    time_move_buissons=0; /* Pas de buisson a deplacer */
    time_create_buissons=0; /* Pas de buisson a creer */
    time_splash=0; /* Pas de splash */

    ShowBoard ();
    ShowScore ();
    CreateAllCactus ();
    CreateAllBuissons ();
    CreateAllCages (BLOCK_1);
    CreatePlayer ();

    for (int i=0; i < SOUND_MAX; i++) {
	StopSoundChannel(i);
    }
}

void Game::FreePlayer (void) {
    if (player_sprite) {
	player_sprite->Hide ();
	delete player_sprite;
	player_sprite=0;
	player_direction=1;
	player_blocked=0;
    }
}

void Game::CreateFire (int aTile, int x, int y) {
    if (!fire_sprite) {
	fire_sprite=new Sprite (aTile, x, y, this);
	fire_sprite->Show ();
    }
}

void Game::FreeFire (void) {
    if (fire_sprite) {
	fire_sprite->Hide ();
	delete fire_sprite;
	fire_sprite=0;
    }
    fire_direction=0; /* None */
}

void Game::EndGame (void) { 
    FreeAllMonsters ();
    FreePlayer ();
    FreeSplash ();
    FreeAllCactus ();
    FreeAllBuissons ();
    FreeAllCages ();
    FreeFire ();
}

void Game::ChangeLife (void) {
    StartSoundChannel(SOUND_KILLED);

    if (_life <= 0) { /* Game Over */
	player_sprite->setXY(14, 11); 
	player_direction=1; /* Up */
	player_sprite->setTileNumber (PLAYER_1);
	player_sprite->Move ();

	_end_flag=1;
    
	if (tiles [GAMEOVER]) {
	    PA_LoadGifXY (1, 16, 0,(void *) tiles [GAMEOVER]);
	}
	return;
    }
  
    /* Restart board */
    fire_in_progress=NO_FIRE;
    _life--;
    FreeFire ();
    FreePlayer ();
    FreeAllCages ();
    FreeSplash ();

    ShowScore ();
    CreateAllCages (BLOCK_1);
    CreatePlayer ();

    for (int i=0; i < SOUND_MAX; i++) {
	StopSoundChannel(i);
    }
}

void Game::CreateSplash (int x, int y) {
    time_splash=SHOOT_TICK;
    if (!splash_sprite) {
	splash_sprite=new Sprite (SPLASH, x, y, this);
	splash_sprite->Show ();
    }
}

void Game::FreeSplash (void) {
    time_splash=0;
    if (splash_sprite) {
	splash_sprite->Hide ();
	delete splash_sprite;
	splash_sprite=0;
    }
}


void Game::CreateAllCages (int tileCage) {
    mListOfCages.clear ();
    for (int y=8; y <= 14; y+=2) {
	for (int x=11; x <= 17; x+=2) {  
	    Sprite aSprite (tileCage, x, y, this);
	    aSprite.Show ();
	    mListOfCages.push_back (aSprite);
	}
    }
}

void Game::CreateAllCactus (void) {
    int tab_dir [8]; /* 8 directions a scruter */
    int n, nb;
    int trouve_first, trouve_second;
    int i;
    int x1=0;
    int y1=0;
    int x2=0; 
    int y2=0;

    mListOfCactus.clear ();

    n=5 + (4 * (_day - 1)); /* Le nombre de cactus total / 2 */

    for (nb=0; nb < n; ++nb) {
	trouve_second=0;
	while (!trouve_second) {
	    /* On cherche une premiere case vide */
	    trouve_first=0;
	    while (!trouve_first) {
		x1=rand () % 28;
		y1=(rand () % 18) + 3;
		if (CheckObstacles (x1, y1)==FLOOR) trouve_first=1;
	    }
      
	    ShakeDirection (tab_dir);
	    for (i=0; (i < 8) && trouve_second==0; i++) {
		switch (tab_dir [i]) {
		    case 0: x2=x1; y2=y1 - 1; break;
		    case 1: x2=x1 + 1; y2=y1 - 1; break;
		    case 2: x2=x1 + 1; y2=y1; break;
		    case 3: x2=x1 + 1; y2=y1 + 1; break;
		    case 4: x2=x1; y2=y1 + 1; break;
		    case 5: x2=x1 - 1; y2=y1 + 1; break;
		    case 6: x2=x1 - 1; y2=y1; break;
		    case 7: x2=x1 - 1; y2=y1 - 1; break;
		    default: break;
		}
	
		if (CheckObstacles (x2, y2)==FLOOR) trouve_second=1;
	    }
	}

	/* On cree le sprite et on l'ajoute a la liste */
	Sprite aSprite1 (CACTUS_1, x1, y1, this);
	aSprite1.Show ();
	mListOfCactus.push_back (aSprite1);

	Sprite aSprite2 (CACTUS_1, x2, y2, this);
	aSprite2.Show ();
	mListOfCactus.push_back (aSprite2);
    }
}

void Game::CreateAllBuissons (void) {
    int x, y;
    int n, nb;

    n=20; /* Le nombre de buissons */
    mListOfBuissons.clear ();
  
    for (nb=0; nb < n; ++nb) {
	FindEmptySpace (x,y);
  
	/* On cree le sprite et on l'ajoute a la liste */
	Sprite aSprite (BUISSON_1, x, y, this);
	aSprite.Show ();
	mListOfBuissons.push_back (aSprite);
    }
}

void Game::FreeAllBuissons (void) {
    ListOfSpritesIterator it=mListOfBuissons.begin();
    while (it != mListOfBuissons.end ()) {
	Sprite & aSprite=*it;
	aSprite.Hide ();
	it++;
    }

    mListOfBuissons.clear ();
}

void Game::FreeAllMonsters (void) {
    ListOfSpritesIterator it=mListOfMonsters.begin();
    while (it != mListOfMonsters.end ()) {
	Sprite & aSprite=*it;
	aSprite.Hide ();
	it++;
    }
    mListOfMonsters.clear ();
}

void Game::FreeAllCactus (void) {
    ListOfSpritesIterator it=mListOfCactus.begin ();
    while (it != mListOfCactus.end ()) {
	Sprite & aSprite=*it;
	aSprite.Hide ();
	it++;
    }
    mListOfCactus.clear ();
}

void Game::FreeAllCages (void) {
    ListOfSpritesIterator it=mListOfCages.begin ();
    while (it != mListOfCages.end ()) {
	Sprite & aSprite=*it;
	aSprite.Hide ();
	it++;
    }
    mListOfCages.clear ();
}

/***************/
/* PERIODIQUES */
/***************/
void Game::TimeMovePlayer (void) {
    int x, y;
  
    if (time_player==0) {
	time_player=PLAYER_TICK;
	player_sprite->getXY(x,y);
    
	if (Pad.Held.Down) { /* DOWN */
	    if (player_sprite->getTileNumber() != PLAYER_3) {
		player_sprite->setTileNumber (PLAYER_3);
		player_direction=3;
		player_sprite->Move ();
		time_player=PLAYER_TICK_CHANGE;
	    }
	    else
		if (y < 20 && _board [y+1][x]==0) {
		    player_sprite->incXY(0, 1);
		    player_sprite->Move ();
		}
	}
	else if (Pad.Held.Up) { /* UP */
	    if (player_sprite->getTileNumber () != PLAYER_1) {
		player_sprite->setTileNumber (PLAYER_1);
		player_direction=1;
		player_sprite->Move();
		time_player=PLAYER_TICK_CHANGE;
	    }
	    else
		if (y > 3 && _board [y-1][x]==0) {
		    player_sprite->incXY (0, -1);
		    player_sprite->Move ();
		}
	}
	else if (Pad.Held.Left) { /* LEFT */
	    if (player_sprite->getTileNumber () != PLAYER_4) {
		player_sprite->setTileNumber (PLAYER_4);
		player_direction=4;
		player_sprite->Move ();
		time_player=PLAYER_TICK_CHANGE;
	    }
	    else
		if (x > 0 && _board [y][x-1]==0) {
		    player_sprite->incXY(-1, 0);
		    player_sprite->Move ();
		}
	}
	else if (Pad.Held.Right) { /* RIGHT */
	    if (player_sprite->getTileNumber () != PLAYER_2) {
		player_sprite->setTileNumber (PLAYER_2);
		player_direction=2;
		player_sprite->Move ();
		time_player=PLAYER_TICK_CHANGE;
	    }
	    else
		if (x < 27 && _board [y][x+1]==0) {
		    player_sprite->incXY(1, 0);
		    player_sprite->Move ();
		}
	}
	else if (Pad.Held.L || Pad.Held.R) { /* PANIC */
	    player_sprite->setXY(14, 11);
	    player_direction=1; /* Up */
	    player_sprite->setTileNumber (PLAYER_1);
	    player_sprite->Move ();
	    _score=0;
	    ShowScore ();
	}
    }
    else {
	time_player--;
    }
}

void Game::TimeFirePlayer (void) {

    if (fire_in_progress != NO_FIRE) return; 
    if (!Pad.Held.A && !Pad.Held.X) return; // No fire wanted

    StartSoundChannel(SOUND_FIRE);

    fire_in_progress=FIRE_IN_PROGRESS;
    fire_direction=player_direction;
    shoot_monster=0; 
  
    int tile_direction=TIR_1; 
    /* Coordonnees initiales du tir */
    switch (fire_direction) {
	case 1: 
	case 3: 
	    tile_direction=TIR_1; // UP - DOWN
	    break;

	case 2:
	case 4: 
	    tile_direction=TIR_2; // RIGHT - LEFT
	    break;
	default:
	    break;
    } 

    int ax,ay;
    player_sprite->getXY(ax,ay);
    fire_sprite=new Sprite (tile_direction, ax, ay, this);
}
  
void Game::TimeFire (void) {
    if (fire_in_progress != FIRE_IN_PROGRESS) return;

    int ax, ay;
    fire_sprite->getXY(ax,ay);

    /* On calcule la prochaine position */
    switch (fire_direction) {
	case 1: /* UP */
	    ay -= 1;
	    break;
	case 2: /* RIGHT */
	    ax += 1;
	    break;
	case 3: /* DOWN */
	    ay += 1;
	    break;
	case 4: /* LEFT */
	    ax -= 1;
	    break;
	default:
	    break;
    }
  
    /* On regarde ou va le tir */
    int bounces=CheckBounces (ax, ay);
  
    switch (bounces) {
	case FLOOR:
	    fire_sprite->setXY (ax, ay);
	    fire_sprite->Move ();
	    break;
    
	case OUT:
	case BLOCK_1:
	case BLOCK_2: 
	case CACTUS_1:
	case CACTUS_2:
	    FreeFire (); 
	    fire_in_progress=NO_FIRE;
	    break;
    
	case BUISSON_1:
	case BUISSON_2:
	    FreeFire ();
	    StartSoundChannel(SOUND_MONSTER);
	    fire_in_progress=FIRE_SPLASH;
	    RemoveXYObject (mListOfBuissons, ax, ay);
	    CreateSplash (ax, ay);
	    _score+=100;
	    break;
    
	case MONSTER_1:
	case MONSTER_2: {
	    shoot_monster=1;
	    FreeFire ();
	    StartSoundChannel(SOUND_MONSTER);
	    fire_in_progress=FIRE_SPLASH;
	    RemoveXYObject (mListOfMonsters, ax, ay);
	    Sprite aSprite  (CACTUS_1, ax, ay, this);
	    aSprite.Show ();
	    mListOfCactus.push_back(aSprite);
	    CreateSplash (ax, ay);
	    _score+=150;
	}
	    break;
    
	default: 
	    break;
    } 
}


void Game::TimeShoot (void) {
    if (fire_in_progress != FIRE_SPLASH) return;

    if (time_splash > 0) {
	time_splash--;
	return;
    }

    /* Position eventuelle d'un nouveau monster */
    int x,y;
    splash_sprite->getXY (x,y);

    FreeSplash (); /* On retire le splash */

    /* Si l'objet shoote etait un monstre on le remplace par un catus */
    if (shoot_monster==1) {
	shoot_monster=0;
	ReplaceCactusByMonster (x, y);
    }

    fire_in_progress=NO_FIRE;

    ShowScore ();
}

void Game::TimeCreateBuissons (void) { 
    if ( mListOfBuissons.empty ()==true) {
	if (time_create_buissons >= CREATE_BUISSONS_TICK) {
	    time_create_buissons=0;
	    CreateAllBuissons ();
	}
	else time_create_buissons++;
    }
}

void Game::TimeMoveBuissons (void) {
    if ( mListOfBuissons.empty ()==true) return;

    if (time_move_buissons==MOVE_BUISSONS_TICK) {
	int px, py;
	player_sprite->getXY (px,py);

	ListOfSpritesIterator it=mListOfBuissons.begin();
	while (it != mListOfBuissons.end ()) {
	    Sprite & aSprite=*it;
	    ++it;
	    int x,y;
	    aSprite.getXY (x,y);

	    /* Changement de look */
	    if (aSprite.getTileNumber()==BUISSON_1) aSprite.setTileNumber (BUISSON_2);
	    else aSprite.setTileNumber (BUISSON_1);

	    if ((x < px) && (y==py)) {
		/* Deplacement a gauche */
		if ( CheckObstacles (x - 1, y)==FLOOR) {
		    aSprite.incXY (-1, 0);
		    aSprite.Move ();
		}
	    }
	    else if ((x > px) && (y==py)) {
		/* Deplacement a droite */
		if ( CheckObstacles (x + 1, y)==FLOOR) {
		    aSprite.incXY (1, 0);
		    aSprite.Move ();
		}
	    }   
	    else if ((y < py) && (x==px)) {
		/* Deplacement en haut */
		if ( CheckObstacles (x, y - 1)==FLOOR) {
		    aSprite.incXY (0, -1);
		    aSprite.Move ();
		}
	    }
	    else if ((y > py) && (x==px)) {
		/* Deplacement en bas */
		if ( CheckObstacles (x, y + 1)==FLOOR) {
		    aSprite.incXY (0, 1);
		    aSprite.Move ();
		}
	    }
	}
    
	time_move_buissons=0;
    }
    else {
	time_move_buissons++;
    }
}

void Game::FindEmptySpace (int &x, int &y) const {
    // Try to find an empty space
    while (1) {
	x=rand () % 28;
	y=(rand () % 18) + 3;
	if (CheckObstacles (x, y)==FLOOR) return;
    }
}

void Game::TimeCreateMonsters (void) {
    /* Changement du type de cage au bout de 5 secondes pour annoncer l'apparition d'un monstre */
    if (time_create_monster==CHANGE_CAGE_TICK) {

	if (CheckIfBlocked ()==1) { // Is there an exit into the cages ?
	    if (player_blocked==0) { // Stick it up !
		int xx,yy;
		FindEmptySpace (xx,yy);
		player_sprite->setXY(xx,yy);
		player_sprite->Move ();
		player_blocked=1;
	    }
	}
	else {
	    player_blocked=0;
	}

	FreeAllCages ();
	CreateAllCages (BLOCK_2);
	time_create_monster++;

	/* On regarde si l'on peut creer un monstre */
	if ((MonsterToBeCreated=ComputeNewMonsterPosition ())==1) {
	    /* mx, my, cx et cy sont initialises */
      
	    /* On remplace le cactus pres duquel va apparaitre le monstre */
	    if (RemoveXYObject (mListOfCactus, cx, cy)) {
		Sprite aSprite (CACTUS_2, cx, cy, this);
		aSprite.Show ();
		mListOfCactus.push_back (aSprite);
	    }
	}
    
	return;
    }
  
    if (time_create_monster >= CREATE_MONSTERS_TICK) {
	time_create_monster=0;
    
	/* Changement de jour si plus possible de creer de monstres et plus de monstre */
	if (MonsterToBeCreated==0 && (mListOfMonsters.empty()==true)) {
	    ChangeDay ();
	}
	else {    
	    /* Soit il reste des monstres, soit il faut en creer 1 */
	    /* Remplacement des cages et apparation d'un monstre au bout de 7 secondes */
	    FreeAllCages (); 
	    CreateAllCages (BLOCK_1);    
      
	    /* On remplace le cactus pres duquel va apparaitre le monstre */
	    /* et on cree un monstre si le cactus existe encore */
	    if (RemoveXYObject (mListOfCactus, cx, cy)) { /* Si cactus existe encore on doit creer un monstre */
		Sprite aSprite (CACTUS_1, cx, cy, this);
		aSprite.Show ();
		mListOfCactus.push_back (aSprite);
	
		if (MonsterToBeCreated==1) {
		    MonsterToBeCreated=0;
		    Sprite aSprite (MONSTER_1, mx, my, this);
		    aSprite.Show ();
		    mListOfMonsters.push_back (aSprite);
		}
	    }
	    else { /* Le cactus n'existe plus, on n'a plus besoin de creer le monstre */
		MonsterToBeCreated=0;
	    }
	}
    }
    else time_create_monster++;
}

void Game::TimeMoveMonsters (void) {
    if (time_move_monster >= MOVE_MONSTERS_TICK) {
	time_move_monster=0;
	MoveMonsters ();
    }
    else time_move_monster++;
}

/***************/
/* UTILITAIRES */
/***************/
int Game::MoveMonsterX (Sprite & aSprite, int dx) {
    int incx, incy;
    int check;
  
    /* On cherche en premier un depacement en horizontal */
    incx=sign (dx);   
    incy=0;
    if (incx==1 || incx==-1) {
	check=CheckObstacles (aSprite.getX() + incx, aSprite.getY() + incy); 
    
	/* Hit ? */
	if (check==PLAYER_1 || check==PLAYER_2 || check==PLAYER_3 || check==PLAYER_4) {
	    ChangeLife (); 
	    return 1;
	}
	else if (check==FLOOR) { /* Deplacement possible */
	    aSprite.incXY (incx, incy);
	    aSprite.Move ();
	    return 1;
	}
	else { /* Il faut essayer un deplacement en y */
	    incy=(rand () % 3) -1;
	    if (incy==0) incx=sign (dx);
	    else incx=0;
      
	    check=CheckObstacles (aSprite.getX () + incx, aSprite.getY () + incy); 
      
	    /* Hit ? */
	    if (check==PLAYER_1 || check==PLAYER_2 || check==PLAYER_3 || check==PLAYER_4) {
		ChangeLife ();
		return 1;
	    }
	    else if (check==FLOOR) { /* Deplacement possible */
		aSprite.incXY (incx, incy);
		aSprite.Move ();
		return 1;
	    }
	}
    }

    return 0;
}

int Game::MoveMonsterY (Sprite & aSprite, int dy) {
    int incx, incy;
    int check;

    /* On cherche en premier un depacement en vertical */
    incy=sign (dy);
    incx=0;
    if (incy==1 || incy==-1) {
	check=CheckObstacles (aSprite.getX() + incx, aSprite.getY () + incy); 
    
	/* Hit ? */
	if (check==PLAYER_1 || check==PLAYER_2 || check==PLAYER_3 || check==PLAYER_4) {
	    ChangeLife ();
	    return 1;
	}
	else if (check==FLOOR) { /* Deplacement possible */
	    aSprite.incXY (incx, incy);
	    aSprite.Move ();
	    return 1;
	}
	else { /* Il faut essayer un deplacement en x */
	    /* incx=sign (dx); */
	    incx=(rand () % 3) -1;
	    if (incx==0) incy=sign (dy);
	    else incy=0;
      
	    check=CheckObstacles (aSprite.getX () + incx, aSprite.getY () + incy); 
      
	    /* Hit ? */
	    if (check==PLAYER_1 || check==PLAYER_2 || check==PLAYER_3 || check==PLAYER_4) {
		ChangeLife ();
		return 1;
	    }
	    else if (check==FLOOR) { /* Deplacement possible */
		aSprite.incXY (incx, incy);
		aSprite.Move ();
		return 1;
	    }
	}
    }

    return 0;
}

void Game::MoveMonsters (void) {
    int dx, dy;
    int xp, yp;
    player_sprite->getXY (xp,yp);

    ListOfSpritesIterator it=mListOfMonsters.begin ();

    while (it != mListOfMonsters.end ()) {
	Sprite & aSprite=*it;
	++it;
    
	/* Changement de look */
	if (aSprite.getTileNumber ()==MONSTER_1) aSprite.setTileNumber (MONSTER_2);
	else aSprite.setTileNumber (MONSTER_1);
    
	dx=xp - aSprite.getX ();
	dy=yp - aSprite.getY ();
    
	/* Tirage au sort pour ne pas privilegier une direction */
	if (rand () % 2==0) {
	    /* Si on peut se deplacer en X on ne se deplace pas en Y */
	    if (!MoveMonsterX (aSprite, dx)) MoveMonsterY (aSprite, dy);
	}
	else {     
	    /* Si on peut se deplacer en Y on ne se deplace pas en X */
	    if (!MoveMonsterY (aSprite, dy)) MoveMonsterX (aSprite, dx);  
	}
    }
}

// TO DO ???
#if 0
GSList *Game::ShakeListPosition (GSList *aList) {
    GSList *reorderList;
    SPRITE *aSprite;
    int len, index;
    reorderList=0;

    len=g_slist_length (aList);
    while (len > 0) {
	index=rand () % len;
	aSprite=g_slist_nth_data (aList, index); /* Pick un a random sprite from original list */
	reorderList=g_slist_append (reorderList, aSprite); /* Add the sprite to the final list */
	aList=g_slist_remove (aList, aSprite); /* Remove the sprite from original list */
	len--;
    }

    return reorderList;
}
#endif

/* Calcule la position d'un nouveau monstre sans le creer */
/* 1 : Position obtenue */
/* 0 : pas de monstre en vu */
int Game::ComputeNewMonsterPosition (void) {
    /* Reorder list */
    //theGame->cactusList=ShakeListPosition (theGame->cactusList); // TO BE PORTED

    ListOfSpritesIterator it=mListOfCactus.begin ();
    while (it != mListOfCactus.end ()) {
	Sprite & aSprite=*it;
	++it;
	int x, y;
	aSprite.getXY (x,y);
    
	cx=x;
	cy=y;
    
	/* X+1, Y */
	if (CheckObstacles (x+1, y)==CACTUS_1) {
	    if (CheckObstacles (x, y-1)==FLOOR) { mx=x; my=y-1; return 1; }
	    if (CheckObstacles (x, y+1)==FLOOR) { mx=x; my=y+1; return 1; }
	    if (CheckObstacles (x+1, y-1)==FLOOR) { mx=x+1; my=y-1; return 1; }
	    if (CheckObstacles (x+1, y+1)==FLOOR) { mx=x+1; my=y+1; return 1; }	
	}
    
	/* X-1, Y */
	if (CheckObstacles (x-1, y)==CACTUS_1) {
	    if (CheckObstacles (x, y-1)==FLOOR) { mx=x; my=y-1; return 1; }
	    if (CheckObstacles (x, y+1)==FLOOR) { mx=x; my=y+1; return 1; }
	    if (CheckObstacles (x-1, y-1)==FLOOR) { mx=x-1; my=y-1; return 1; }
	    if (CheckObstacles (x-1, y+1)==FLOOR) { mx=x-1; my=y+1; return 1; }		
	}
    
	/* X, Y+1 */
	if (CheckObstacles (x, y+1)==CACTUS_1) {
	    if (CheckObstacles (x-1, y)==FLOOR) { mx=x-1; my=y; return 1; }
	    if (CheckObstacles (x+1, y)==FLOOR) { mx=x+1; my=y; return 1; }
	    if (CheckObstacles (x-1, y+1)==FLOOR) { mx=x-1; my=y+1; return 1; }
	    if (CheckObstacles (x+1, y+1)==FLOOR) { mx=x+1; my=y+1; return 1; }		
	}
    
	/* X, Y-1 */
	if (CheckObstacles (x, y-1)==CACTUS_1) {
	    if (CheckObstacles (x-1, y)==FLOOR) { mx=x-1; my=y; return 1; }
	    if (CheckObstacles (x+1, y)==FLOOR) { mx =x+1; my=y; return 1; }
	    if (CheckObstacles (x-1, y-1)==FLOOR) { mx=x-1; my=y-1; return 1;}
	    if (CheckObstacles (x+1, y-1)==FLOOR) { mx=x+1; my=y-1; return 1; }		
	}
    
	/* X+1, Y-1 */
	if (CheckObstacles (x+1, y-1)==CACTUS_1) {
	    if (CheckObstacles (x, y-1)==FLOOR) { mx=x; my=y-1; return 1; }
	    if (CheckObstacles (x+1, y)==FLOOR) { mx=x+1; my=y; return 1; }		
	}
    
	/* X+1, Y+1 */
	if (CheckObstacles (x+1, y+1)==CACTUS_1) {
	    if (CheckObstacles (x, y+1)==FLOOR) { mx=x; my=y+1; return 1; }
	    if (CheckObstacles (x+1, y)==FLOOR) { mx=x+1; my=y; return 1; }		
	}
    
	/* X-1, Y-1 */
	if (CheckObstacles (x-1, y-1)==CACTUS_1) {
	    if (CheckObstacles (x, y-1)==FLOOR) { mx=x; my=y-1; return 1; }
	    if (CheckObstacles (x-1, y)==FLOOR) { mx=x-1; my=y; return 1; }		
	}
    
	/* X-1, Y+1 */
	if (CheckObstacles (x-1, y+1)==CACTUS_1) {
	    if (CheckObstacles (x, y+1)==FLOOR) { mx=x; my=y+1; return 1; }
	    if (CheckObstacles (x-1, y)==FLOOR) { mx=x-1; my=y; return 1; }		
	}
    }

    return 0;
}

int Game::CheckBounces (int x, int y) const {
    if (x < 0) return OUT;
    if (x > 27) return OUT;
    if (y < 3) return OUT;
    if (y > 20) return OUT;

    return (_board [y][x]);
}

int Game::CheckObstacles (int x, int y) const {
    if (x < 0) return OUT;
    if (x > 27) return OUT;
    if (y < 3) return OUT;
    if (y > 20) return OUT;
    if (x >= 11 && x <= 17 && y >= 8 && y <= 14) return BLOCK_1;

    return (_board [y][x]);
}

int Game::RemoveXYObject (ListOfSprites &liste, int x, int y) {
    ListOfSpritesIterator it=liste.begin ();
    while (it != liste.end ()) {
	Sprite & aSprite=*it;
	if (aSprite.getX()==x && aSprite.getY()==y) {
	    aSprite.Hide ();
	    liste.erase (it);
	    return 1; /* OK removed */
	}
	++it;
    }
    return 0; /* Nothing to remove */
}

void Game::ShowBoard (void) const {
    if (tiles [BOARD]) {
	PA_LoadGifXY (1, 0, 0,(void *) tiles [BOARD]);
    }
}

void Game::ShowScore (void) const {
    char buffer [20];
    PA_OutputSimpleText(1, 0, 23,"                                ");
    /* Day */
    sprintf (buffer, "%d", _day);
    PA_OutputText(1, 4, 23,"%s", buffer);

    /* Population (score) */
    sprintf (buffer, "%d", _score);
    PA_OutputText(1, 12, 23,"%s", buffer);

    /* Shooners (life) */
    sprintf (buffer, "%d", _life);
    PA_OutputText(1, 24, 23,"%s", buffer);
}


void Game::ReplaceCactusByMonster (int x, int y) {
    ListOfPoints aList=ComputeThreeCactusList (x, y);
    if (aList.empty ()==true) return; // Pas de cactus a remplacer
    RemoveXYObject (mListOfCactus, x, y);

    ListOfPointsIterator it=aList.begin ();
    while (it != aList.end ()) {
	FPoint & aPoint=*it;
	RemoveXYObject (mListOfCactus, aPoint.getX(), aPoint.getY());
	it++;
    }
    Sprite aSprite (MONSTER_1, x, y, this);
    aSprite.Show ();
    mListOfMonsters.push_back (aSprite);
}


ListOfPoints Game::ComputeThreeCactusList (int x, int y) {
    int i, j;  
    int check;

    /* Making a first list */
    ListOfPoints firstList;
    for (j=-1; j < 2; j++) { /* -1, 0, 1 */
	for (i=-1; i < 2; i++) { /* -1, 0, 1 */
	    if ((j==0) && (i==0)) ;
	    else {
		check=CheckObstacles (x+i, y+j);
		if ( check==CACTUS_1 || check==CACTUS_2 ) { 
		    firstList.push_back (FPoint (x+i, y+j));
		}
	    }
	}				  
    }

    if (firstList.empty ()==true) {
	return firstList; /* Aucun voisin a (x,y) */
    } 

    if (firstList.size () > 1) {/* On arrete les recherches, la liste contient au moins 3 cactus */
	return firstList;
    }
    /* Taking the only one elmt */
    FPoint aPoint;
    aPoint=firstList.front ();
    int xx, yy;
    aPoint.getXY(xx,yy);

    /* Making a second list */
    ListOfPoints secondList;
    for (j=-1; j < 2; j++) { /* -1, 0, 1 */
	for (i=-1; i < 2; i++) { /* -1, 0, 1 */
	    check=CheckObstacles (xx+i, yy+j);
	    if ( check==CACTUS_1 || check==CACTUS_2) { 
		secondList.push_back (FPoint (xx+i, yy+j));
	    }
	}	 
    }

    /* The Truth */
    if (secondList.size ()==2) {
	secondList.clear (); /* Aucun voisin a (x,y). Empty list*/
    }

    return secondList;
}

/* Shake : melange 8 directions (0-7) */
void ShakeDirection (int tab_dir[]) {
    int tab_indices[8];
    int i, j, index, n_indices;
  
    /* prepare les tables de Shake-up */
    for (i=0; i<8; ++i) {
	tab_dir[i]=0;
	tab_indices[i]=i;
    }
  
    /* melange des directions */
    for (n_indices=8, i=0; i<8; ++i) {
	index=rand() % n_indices;	/* prend un indice dans la */
	/* table des cases vides */
	tab_dir[i]=tab_indices[index] + 1;
	--n_indices;	/* 1 direction de moins */
	/* suppression de la direction dans table */
	for (j=index; j<n_indices; ++j)
	    tab_indices[j]=tab_indices[j+1];
    }
}

int sign (int val) {
    if (val < 0) return -1;
    if (val > 0) return 1;
    return 0;
}

// --------------------------
// MAIN 
// --------------------------
int main (int argc, char *argv[]) {

    // Graphical init
    PA_Init();    // Initializes PA_Lib
    PA_InitVBL(); // Initializes a standard VBL
    PA_InitSound();  // Init the sound system
    PA_SetDefaultSound(127, 16384, 0); // Volume (Max), Sampling rate, format (0:raw)
    PA_Init16bitBg(1, 3);
    PA_Init16bitBg(0, 3);

    PA_InitText(0, 0);  	// Background number, from 0-3, on which to load the text system
    PA_InitText(1, 0);  	// Background number, from 0-3, on which to load the text system
    PA_SetTextCol(0, 0, 0, 0);
    PA_SetTextCol(1, 0, 0, 0);
    PA_Clear16bitBg(0);

    // Init random generator
    struct timeval td;
    gettimeofday (&td, NULL);
    srand (td.tv_sec);

    Game *theGame=new Game ();

    PA_LoadGifXY (0, 0, 0,(void *) lower_screen);
    theGame->StartSoundChannel(SOUND_THEME); // Start Theme music

    int pause_flag=1;
    PA_ClearTextBg (0);

    _message_num = 1; // "Press Start to begin"
    ShowMessage();
  
    while (1) { // On ne doit pas sortir

	if (theGame->IsItTheEnd ()==0) {
	    if (Pad.Newpress.Start) { // Pause
		if (pause_flag) {
		    pause_flag=0;
		    PA_ClearTextBg (0);
		    if ( theGame->IsStoppedSoundChannel (SOUND_THEME)) {
			theGame->StartSoundChannel(SOUND_THEME);
		    }
		    else {
			theGame->StopSoundChannel(SOUND_THEME);
		    }
		}
		else {
		    pause_flag=1;
		    _message_num = 2; // "Press Start to resume"
		    ShowMessage();
		    theGame->StartSoundChannel(SOUND_THEME);
		}
	    }
	}
	
	// Can restart at any moment
	if (Pad.Newpress.Select) { // Restart
	    theGame->StopSoundChannel(SOUND_THEME); // Stop Theme music
	    delete theGame;
	    theGame=new Game ();
	    pause_flag=1;
	    PA_ClearTextBg (0);
	    _message_num = 1; // "Press Start to begin"
	    ShowMessage ();
	    theGame->StartSoundChannel(SOUND_THEME); // Start Theme music
	}

	if (!pause_flag) {
	    theGame->TimerEvent();
	}
	else {
	    if (Stylus.Newpress) { // Show Help
		Help h;
		h.Process();
		ShowMessage();
	    }
	}

	PA_WaitForVBL();
    }

    return 0;
}

void ShowMessage () {
    switch (_message_num) {
	case 0:
	    PA_OutputSimpleText(0, 5, 16, "Press Select to restart");
	    break;
	case 1:
	    PA_OutputSimpleText(0, 5, 16, "Press Start to begin");
	    PA_OutputSimpleText(0, 5, 17, "Press Select to restart");
	    PA_OutputSimpleText(0, 5, 18, "Touch screen for help");
	    break;
	case 2:
	    PA_OutputSimpleText(0, 5, 16, "Press Start to resume");
	    PA_OutputSimpleText(0, 5, 17, "Press Select to restart");
	    PA_OutputSimpleText(0, 5, 18, "Touch screen for help");
	    break;
	default:
	    break;
    }
    PA_OutputSimpleText(0, 10, 1, "TombstoneDS");
    PA_OutputSimpleText(0, 7, 2, "Version 18-07-2011");
    PA_OutputSimpleText(0, 1, 4, "e-mail:electronique.fb@free.fr");
}

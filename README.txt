======================
Tombstone City for NDS
======================
Version : 18 July 2011

What's that
-----------
Tombstone city is a game developped in 1981 by Texas Instrusment for the TI99/A4. (see http://www.videogamehouse.net/tombstone.html or http://en.wikipedia.org/wiki/Tombstone_City:_21st_Century). 

This game corresponds to my own implementation of the original game, ready to run on a Nintendo NDS. 

For those who do not know what it is, have a look at this link :
http://www.videogamehouse.net/tombstone.html

It has been developped with the following tools :
- devkitARM release 20
- libnds
- PAlib070717
- desmume 0.7.3
- Linker R4 DS

To use it, copy the TombstoneDS/bin/TombstoneDS.nds somewhere on your linker memory and run-it !


What's new
----------
Now integrates music and sounds. 

Install
-------
To be able to run this homebrew, you need a linker such as R4, M3, ...
So, copy the TombstoneDS/bin/TombstoneDS.nds binary somewhere in its memory card and run it !

Keys 
----
X or A : Fire 
Arrows : Move 
R or L : Panic button. Put the player on a safety place but score is cleared !
Select : Restart the game
Start  : Pause / Continue

How to play
-----------
Rules are pretty simple. Stay alive and make the highest score.

* The player can freely move anywhere in the game area.
* He is in a safety place inside his cages (blue boxes).

Obstacles
---------
* monsters
* bushes
* cactus
* cages (blue boxes).

Bushes
------
* The player can shoot every movable bushes.
* A shooted bush scores 100 points (POPULATION).
* When there is no more bush, 20 others come up after 10 secondes.
* Bush moves if the player is located on its column or row.

Monsters
--------
* Monsters move as fast as the player.
* They try to catch the player.
* The player has 10 lives (SHOONERS).
* A shooted monster scores 150 points (POPULATION).
* When a monster is killed, it is transformed in a cactus.
* If 3 cactus are closy related, they disappear and are replaced by a monster.
* If 2 cactus are closy related, a monster can appear around them.
In this case, cages and one cactus change their color during 2 seconds 
in order to warn the player.

* A monster can appear every 7 seconds if there is one or more closy related cactus.
* If there is no more closy related cactus and no more monster, day counter increases (DAY).
In this case, the game is restarted with new bushes, monsters and more cactus.

For any comment, please e-mail me :
buzz.computer (at) free.fr

My web site :
buzz.computer.free.fr

Have fun !




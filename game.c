#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "SDL_mixer.h"
#include <math.h>
#include "tile.h"
#include "thing.h"
#include "physics.h"
#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef __WIN32__
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#endif

#define CAMSCROLL 15
#define SCR_WIDTH 780
#define SCR_HEIGHT 600
#define SCR_BPP 32
#define SPR_SIZE 30
#define CAMSENSE 60
#define LVLMAX 500
#define THINGMAX 250
#define BOSSMAX 10
#define PARTICLEMAX 2500
#define ROOMMAX 100
#define TELERADIUS 8
#define TELEMODIF 0.175
#define TELECAP 17.5
#define TELEDELAY 12
#define TELEACCEL 3
#define TELEFAILSPARKS 5
#define DEATHDELAY 25
#define SNAPSIZE 3
#define SNAPVEL 2
#define TEXTSHADOW -1
#define BKGSCROLL 0.5
#define CURSORRADIUS 4
#define SOUNDDIST 150
#define MFRAGTOTAL 100

#define WALK_SPEED 2.5
#define RUN_SPEED 4

#define WMEDPOS 75
#define OMEDPOS 165
#define BMEDPOS 255
#define RMEDPOS 375
#define GMEDPOS 510
#define PMEDPOS 675
#define MEDINFODIST 45

#define GREENFLAG 0x1
#define REDFLAG 0x2
#define CREATORFLAG 0x4
#define NOCOLLIDEFLAG 0x8
#define KEYPRESSED 0x10
#define REINCARNATE 0x20

#define GAMEMAX 25
#define MEDALMAX 600
#define BKG_MAX 15
#define MUSIC_MAX 14
#define NUM_CHANNELS 8

#define FPS_LIMIT 60

#define GAME_TITLE "Beret"

#define LAST_LEVEL 67

#define A_ 0x1
#define B_ 0x2
#define C_ 0x4
#define D_ 0x8
#define E_ 0x10
#define F_ 0x20
#define G_ 0x40
#define H_ 0x80

#ifdef __WIN32__
#define DIRSEP "\\"
#else
#define DIRSEP "/"
#endif

#if defined __APPLE__
#define SUPPORT_PATH "Library/Application Support/Beret/"
#define RESOURCE_PATH "Beret.app/Contents/Resources/"
#elif defined __WIN32__
#define RESOURCE_PATH ""
#else
#define SUPPORT_PATH ".beret/"
#define RESOURCE_PATH ""
#endif

#define QUITMOD_WIN KMOD_ALT
#define QUITKEY_WIN SDLK_F4
#define QUITMOD_LIN KMOD_CTRL
#define QUITKEY_LIN SDLK_c
#define QUITMOD_MAC KMOD_META
#define QUITKEY_MAC SDLK_q

#define MAX(a,b) ((a)>(b))?(a):(b)

#define UNF 1000
#define STORYLEN 21
#define CREAT1LEN 21
#define CREAT2LEN 21
#define CREAT3LEN 18
#define CONTROLLEN 19
#define MSGMAX 60



const char* creat1[CREAT1LEN] =
  {" ",
   "Escape - Menu (Playtest Room)",
   "Q - Display this guide",
   " ",
   "F1 - Save Room",
   "F4 - Load Room",
   "F9 - Set time limit for level, check blue fragments, get enemy counts",
   "F12 - All of above except set time limit",
   " ",
   "Home/End - Decrease/Increase room width",
   "Page Up/Page Down - Decrease/Increase room height",
   "- or + - Change background",
   "[ or ] - Change music",
   " ",
   "I - Open object select screen - Click to select object/tile",
   "U - Place tile at cursor or fill selection with tiles",
   "Y - Delete tile at cursor or delete tiles in selection",
   "O - Place object at cursor",
   "Tab - Change selected object subtype",
   " ",
   "Continue to next page..."};

const char* creat2[CREAT2LEN] =
  {" ",
   "Left click and drag - Move object",
   "Right click and drag - Select objects",
   "ASDW/Arrow Keys - Move selected objects",
   "Shift + ASDW/Arrow Keys - Move selected objects slowly",
   "Delete/Backspace - Delete selected objects",
   "V - Copy selected objects",
   "< or > - Change object direction",
   "L - Choose link for selected link blocks",
   " ",
   "G - Change grid",
   "H - Toggle snap to grid",
   "C - Toggle collision detection",
   "R - Clear room",
   "P - Set Beret's start position to cursor",
   " ",
   "1-0 - Choose number of entrance",
   "Shift + 1-0 - Choose entrance number to connect to",
   "E - Set room number to exit to",
   " ",
   "Continue to next page..."};

const char* creat3[CREAT3LEN] =
  {" ",
   "Notes for creating a level:",
   " ",
   "Room 0 is the starting room for the level. Set Beret's",
   "position in room 0 to the desired level entry point.",
   " ",
   "Doors and Exit Signs have an entrance number assigned",
   "to them, as well as an entrance number and a room number",
   "that they connect to. Make sure these match up between",
   "rooms or the player will get a \"file not found\" error.",
   " ",
   "When a level is finished, use F12 to check that all 100",
   "Blue Medallion fragments have been placed and to assign",
   "the fragments indices. This helps keep track of exactly",
   "which fragments have been collected. F12 also counts the",
   "enemies in each room so that the Red Medallion can be",
   "collected. Use F9 to set a time limit for the level on",
   "top of these checks."};

const char* story[STORYLEN] =
  {"For many years, Beret was a researcher and experiment in",
   "the Department of Telekinetics at a large research company",
   "called the Evil Corporation. While studying telekinetics,",
   "Beret and his research assistants succeeded in granting",
   "Beret the power of telekinesis. Shortly after this, Beret",
   "became disgusted with the injustices committed in the name",
   "of scientific advance by his employers, the Three Evils of",
   "the Evil Corporation. Beret has decided to single-handedly",
   "destroy the entire corporation and defeat the Three Evils",
   "using his remarkable power - a quite daunting task, due to",
   "the high security of the Evil Corporation. Each Department",
   "is inaccessible to anyone without the proper clearance level,",
   "which is determined by the number of Medallions owned by that",
   "person. Therefore, in order to reach the Three Evils, who",
   "reside in extremely well-protected Departments, Beret will",
   "need to collect as many Medallions as possible in each",
   "Department that he visits in order to gain access to the next.",
   " ",
   "Despite his amazing power, Beret has a perilous journey ahead",
   "of him. Only by exhibiting great cunning as well as skill",
   "will he be able to accomplish his goal."};
const char* controls[CONTROLLEN] =
  {"Running:  A and D, or left arrow key and right arrow key",
   "Walking: Hold shift",
   "Jumping:  W, up arrow key, or spacebar",
   "Entering Doors/Reading Signs:  S or down arrow key",
   " ",
   "Save State:  F1 or 1",
   "Load State:  F4 or 4 (hold Ctrl for backup)",
   "Restart Room:  R",
   "Exit Level:  Escape",
   "Pause Game:  P",
   " ",
   "Telekinesis:  Left mouse button",
   "Toggle Telekinesis Guide:  G or right mouse button",
   "Toggle Cursor Movement Mode:  M",
   " ",
   "Move Camera:  I, J, K, or L",
   " ",
   "Toggle Full Screen:  Alt+Enter",
   "Toggle Sound: Slash"};
const int levelentry[5][20] = {{UNF,UNF,2,0,6,10,UNF,UNF,16,UNF,21,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF},
                               {UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,0,37,UNF,30,43,UNF,UNF,UNF,48,55},
                               {UNF,UNF,UNF,UNF,84,80,95,UNF,UNF,UNF,88,UNF,68,UNF,74,UNF,UNF,0,65,UNF},
                               {UNF,UNF,UNF,UNF,UNF,UNF,UNF,100,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF},
                               {120,120,120,120,120,120,120,120,120,120,120,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF}};
const int levelnums[5][20] = {{UNF,UNF,2,1,3,4,UNF,UNF,5,UNF,6,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF},
                              {UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,7,9,UNF,8,10,UNF,UNF,UNF,11,12},
                              {UNF,UNF,UNF,UNF,18,17,20,UNF,UNF,UNF,19,UNF,15,UNF,16,UNF,UNF,13,14,UNF},
                              {UNF,UNF,UNF,UNF,UNF,UNF,UNF,21,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF},
                              {1,2,3,4,5,6,7,8,9,10,22,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF,UNF}};
const int numofst[22] = {1,1,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};
const int maplastlevel[5] = {10, 39, 46, 67, 99};
const char* wingnames[5] = {"West Wing", "North Wing", "East Wing", "South Wing", "Tower"};
const char* deptnames[5][20] = {{" "," ","Telekinetics","Telekinetics","Telekinetics","Telekinetics"," "," ","Connectivity"," ",
				 "Connectivity"," "," "," "," "," "," "," "," "," "},
                                {" "," "," "," "," "," "," "," "," "," ",
				 "Detonation","Detonation"," ","Detonation","Detonation"," "," "," ","Infection","Infection"},
                                {" "," "," "," ","Parapsychology","Parapsychology","Antimatter"," "," "," ",
				 "Antimatter"," ","Gravitation"," ","Gravitation"," "," ","Gravitation","Gravitation"," "},
                                {" "," "," "," "," "," "," ","Evil"," "," "," "," "," "," "," "," "," "," "," "," "},
                                {"Your Design","Your Design","Your Design","Your Design","Your Design","Your Design",
				 "Your Design","Your Design","Your Design","Your Design","Creation"," "," "," "," "," "," "," "," "," "}};
const char* divnames[5][20] = {{" "," ","Main","Inner","Experimental","Defense"," "," ","Collaboration"," ",
				"Main"," "," "," "," "," "," "," "," "," "},
                               {" "," "," "," "," "," "," "," "," "," ",
				"Metaphysics","Spiky Death"," ","Pyrotechnics","Adhesives"," "," "," ","Asphyxiation","Contagion"},
                               {" "," "," "," ","Unknown","Spectral","Main"," "," "," ",
				"Motion"," ","Localization"," ","Ubiquitous"," "," ","Object Specific","Hub"," "},
                               {" "," "," "," "," "," "," ","-"," "," "," "," "," "," "," "," "," "," "," "," "},
                               {"1st","2nd","3rd","4th","5th","6th","7th","8th","9th","10th","-"," "," "," "," "," "," "," "," "," "}};
const char* msgs[MSGMAX][8] =
  {{"This is the map screen, on which you can view a",
    "floor plan of the Evil Corporation. Each level",
    "that you can access is shown, as is the level that",
    "you are closest to opening. Move the cursor over any",
    "level to view information such as which Medallions",
    "have been collected in the level, or the number of",
    "Medallions that are required for entry to that level.",
    "Click on a level to enter it."},
   {"[ Basic Controls ]"," ",
    "Walk with A and D or the left and right arrow keys.",
    "Jump with W, the up arrow key, or the spacebar.",
    "Read signs and enter doors with S or",
    "the down arrow key.",
    "Hold the shift key to walk slowly.",
    "Pause the game by pressing P."},
   {"[ Medallions ]"," ",
    "In each level there are six Medallions to collect.",
    "Here is an introduction to the first three:"," ",
    "White Medallion - Collect somewhere in the level",
    "Orange Medallion - Collect four Medallion Corners",
    "Blue Medallion - Collect 100 Medallion Fragments"},
   {"[ Telekinesis ]"," ",
    "Click and hold down the left mouse button to use",
    "telekinesis. Most objects can be picked up and moved",
    "around. Press G or click the right mouse button to",
    "toggle the telekinesis range guide. Note that Beret must",
    "be able to see the object he is using telekinesis on,",
    "and he can not see through solid walls."},
   {"[ Enemies ]", " ",
    "Even with the power of telekinesis on your side,",
    "enemies can still be a large threat. Many enemies,",
    "such as Robots, will be destroyed if they are",
    "smashed into a wall or object. You can also destroy",
    "enemies by smashing an object such as a block",
    "into them."},
   {"[ Clearing a Room ]"," ",
    "You will obtain the Red Medallion once all the rooms",
    "in the level have been cleared. To clear a room, you",
    "must destroy all the enemies in the room and then exit",
    "that room. If you then return to that room, the",
    "enemies will be back, but the room will still be cleared."," "},
   {"[ Save States ]", " "," ",
    "Press F1 to save the the current state of a room.",
    "This state can then be loaded at any time, even after",
    "dying, by pressing F4. Once you leave a room, you can",
    "no longer load save states created in that room.",
    " "},
   {"[ Medallions ]", " ",
    "Here is a description of the rest of the Medallions:",
    "Red Medallion - Defeat all enemies in each room",
    "Green Medallion - Reach the exit before the timer on",
    "the status bar reaches zero",
    "Purple Medallion - Simply reach the exit",
    " "},
   {"[ Block Types ]", " ",
    "There are several types of blocks that you will",
    "encounter, but the three that you will see most",
    "often are Wood Blocks, Ice Blocks, and Stone Blocks.",
    "Wood Blocks have no special properties, but Ice Blocks",
    "are very slippery and slide a lot when thrown.",
    "Stone Blocks are too heavy to be lifted."},
   {"[ X Only Walls ]", " ", " ",
    "Certain types of walls only let specific types of",
    "things through. Beret Only Walls are blue, and only",
    "let Beret go through them. Object Only Walls are red,",
    "and they let everything except Beret through.", " "},
   {"[ Doors ]", " ",
    "To enter a door, press S or the down arrow key.", " ",
    "This large door is the level's exit. Go through it",
    "when you are ready to end the level!",
    " ", " "},
   {"[ Clearing a Room ]",
    "A flashing red X on the status bar indicates that the",
    "current room has not yet been cleared and that there",
    "are still enemies in the room. A red circle means that",
    "there are no enemies left in the room, but that the room",
    "has not yet been cleared - you will need to leave the room",
    "to clear it. If there is a green circle on the status bar,",
    "the current room has already been cleared."},
   {"[ Restarting and Leaving ]", " ",
    "If you need to restart a room for any reason, you can",
    "do so by pressing R. If you need to return to the map",
    "screen, press Escape. Note that your progress in the",
    "level will not be saved - in order to save your",
    "progress you must reach the exit of the level", " "},
   {"[ Spike Balls ]", " ",
    "Robots are not the only danger that you will face within",
    "the Evil Corporation. Another common type of enemy is",
    "the Spike Ball, which floats back and forth either",
    "horizontally or vertically. Like Robots, they can be",
    "smashed into walls or objects and destroyed.", " "},
   {"[ Telekinesis Blocks ]", " ",
    "Telekinesis Blocks are blocks that become solid if",
    "they detect the use of telekinesis, but are otherwise",
    "insubstantial. Anything trapped inside a Telekinesis",
    "Block when it materializes will be destroyed. This",
    "includes Beret, so watch out!", " "},
   {"[ Teleseekers and Superbots ]",
    "Unfortunately, the Robots and Spike Balls that you have",
    "encountered this far are some of the least troublesome of",
    "the enemies that you will face. Superbots are a speedy",
    "version of Robots. Red Teleseekers will chase you if they",
    "detect that you are using telekinesis, and Yellow",
    "Teleseekers will chase you if they detect that you are not.",
    "Teleseekers can not be destroyed by smashing."},
   {"[ Exiting a Level ]"," ",
    "When you leave a level through the exit door, all of the",
    "Medallions that you have collected in the level are added",
    "to your Medallion collection. All other collectables that",
    "you have gathered in the level will also be saved, and",
    "they will appear transparent the next time you play the",
    "level to indicate that they have already been collected."}, 
   {"[ Medallion Fragments ]", " ",
    "Medallion Fragments can be picked up and moved around with",
    "telekinesis. Try picking up the Fragments in the",
    "enclosure to the right of this sign.", " ",
    "A blue circle in the Blue Medallion section on the status",
    "bar indicates that there are no more Fragments in the room."},
   {"[ Aura Drops ]"," "," ",
    "Aura Drops are enemies that create an aura of deadly",
    "poison around themselves. This aura can kill Beret",
    "and other enemies - only Aura Drops are immune to it.",
    "Even if an Aura Drop is killed, its aura will remain in",
    "the location of its death."},
   {"[ Continuing Play ]", " ",
    "If you exit the game in the middle of a level, you can",
    "choose to continue your game the next time you play.",
    "If you do, you will start in the same room that you were",
    "in when you quit.",
    "You will also be able to load a save state that you",
    "had made in that room."},
   {"[ Moving the Camera ]"," "," ",
    "In some situations you may need to look around the room you",
    "are in. To do this, press I to move the camera upwards,",
    "J to move it to the left, K to move it downwards,",
    "and L to move it to the right."," "},
   {"[ Spikes and Hoppers ]", " ",
    "Be very careful around spikes - a single touch will kill",
    "you. Enemies will also be destroyed if they touch spikes.", " ",
    "Hoppers are a slightly more dangerous enemy than Robots",
    "and Spike Balls, since they are a bit smarter and can hop",
    "over low obstacles and even up stairs."},
   {"[ Link Blocks ]", " ",
    "Link Blocks are the main topic of research in the",
    "Department of Connectivity. These types of blocks link to",
    "another object and either influence that object's movements",
    "or are influenced by its movements. You can not use your",
    "telekinesis to move any object that is being influenced,",
    "whether that object is a Link Block or some other object."},
   {"[ Sign Colors ]", " ",
    "Because of the astounding size of the Evil Corporation,",
    "it was a common occurrence for a researcher to get dreadfully",
    "lost in a Department. To remedy this issue, the path to the",
    "exit of each Department has been marked by signs with green",
    "arrows on them. Golden arrows on signs mark paths to other",
    "rooms that are not directly on the path to the exit."},
   {"[ Fireworks ]", " ",
    "Researchers in the Department of Detonation study various",
    "sorts of explosives - in the Pyrotechnics Division several",
    "dangerous types of Fireworks have been created. Explosions",
    "can kill any animate object and also destroy Cracked Blocks.",
    "Orange Fireworks are the least explosive, then Red Fireworks,",
    "and Green Fireworks are the most explosive."},
   {"[ Fake Blocks ]", " ",
    "Fake Blocks are extremely sneaky enemies that disguise",
    "themselves as normal Blocks until Beret comes too close, and",
    "then they attack. The Stone Fake Block variety can not be",
    "picked up, and none of the Fake Block types can be smashed.",
    "As long as a Fake Block is disguised, it is not dangerous to",
    "touch or even stand on."},
   {"[ Choice Only Walls ]", " ", " ",
    "If a Choice Only Wall is touched by Beret, it will become a",
    "Beret Only Wall. If a Choice Only Wall is touched by any",
    "other object, it will become an Object Only Wall."," "," "},
   {"[ Cursor Movement Modes ]"," ",
    "The alternate cursor movement mode is that the cursor",
    "remains stationary relative to the room, even if the",
    "screen scrolls. Press M you would like to toggle the",
    "cursor movement mode. You can also change this option",
    "on the options menu by pressing Escape and",
    "selecting \"Options\"."},
   {"[ Save and Load State Shortcuts ]"," "," ",
    "Since saving and loading your state quickly can",
    "be crucial while trying to get the Green Medallion,",
    "you can use F1 or the 1 key to save your state, and",
    "you can use F4 or the 4 key to load state."," "},
   {"[ A Few Notes About This Door ]"," "," ",
    "Don't forget that going through this door will cause all the",
    "enemies in this room to come back when you return. If you are",
    "trying to get the Red Medallion, you may want to kill all the",
    "enemies and then come back to this door.",
    " "},
   {"[ Super Hoppers ]"," "," ",
    "Super Hoppers are an upgraded version of Hoppers",
    "that have far superior jumping ability. They can",
    "jump as high as Beret can!",
    " "," "},
   {"[ Solidity Blocks and Switches ]", " ",
    "When you hit a Solidity Switch with an object, all the Solidity",
    "Blocks in the room will toggle their solidity. Just as with",
    "Telekinesis Blocks, anything that is caught inside a Solidity",
    "Block when it becomes solid will be destroyed. Explosions can",
    "also trigger Solidity Switches."," "},
   {"[ Sticky Bombs ]",
    "In the Adhesives Division of the Department of Detonation, a",
    "type of explosive called Sticky Bombs has been created. Sticky",
    "Bombs can be attached to any surface, but then they will not",
    "come off. Once a Sticky Bomb is attached to something, it will",
    "begin a countdown, and when its timer runs out, it will explode.",
    "The explosion of a Sticky Bomb is of the same magnitude as",
    "that of an Orange Firework."},
   {"[ Annoying Blocks ]", " ",
    "Annoying Blocks come in two varieties - ones that move",
    "horizontally and ones that move vertically. Both types",
    "will try to match Beret's position. So, the horizontal",
    "Annoying Blocks will try to stay above or below Beret,",
    "and the vertical Annoying Blocks will try to stay to",
    "Beret's left or right."},
   {"[ Infectlings ]", " ", " ",
    "Infectlings are objects that can change another object",
    "into an Infectling when they touch it. Black Infectlings",
    "affect only animate objects, but White Infectlings affect",
    "all objects."," "},
   {"[ Pumping Platforms ]"," "," ",
    "A Platform can be \"pumped\" if you block its movement path",
    "and jump on it repeatedly. This technique can be used to",
    "force a Platform to travel a large distance.",
    " ", " "},
   {"[ Save States ]"," "," ",
    "If you haven't tried out save states yet, this is a good spot",
    "to do so - you wouldn't want to fall all the way down and have",
    "to climb back up, would you? Save your state using F1, and then",
    "if you fall to the bottom you can load your state using F4.",
    " "},
   {"[ Department of Infection ]"," ",
    "The Department of Infection is the last department in the North",
    "Wing of the Evil Corporation, and so the second of the Three",
    "Evils, Stoney, resides here. The Medallions in this level are",
    "very tricky to get, so if you get stuck keep in mind that you",
    "only need to get to the end of the level and defeat Stoney to",
    "progress to the East Wing of the Evil Corporation."},
   {"[ Stoney ]"," "," ",
    "Behind this menacing door awaits the second of the Three",
    "Evils. Stoney is a worthy foe - you have a difficult battle",
    "ahead of you. Once you manage to defeat him, though, you will",
    "be able to continue on to the East Wing of the Evil Corporation",
    " "},
   {"[ Save States ]"," ",
    "While save states can take a bit of time to get used to, it",
    "is certainly worth it, as they can make tricky parts of the",
    "Evil Corporation much less frustrating. If you have not yet",
    "begun using save states, you may want to try them out here,",
    "where save stating after you climb parts of this room can",
    "save you a lot of time if you fall."},
   {"[ Gravity Blocks and Floating Blocks ]"," ",
    "In the Object Specific Division of the Department of",
    "Gravitation, blocks that are affected by gravity in various",
    "ways have been designed. Gravity Blocks always fall in the",
    "direction that the arrow on them indicates. Floating Blocks",
    "are not affected by gravity at all.",
    " "},
   {"[ Force Fields ]"," ",
    "If anything touches a Force Field, that object will be forced",
    "to move in the direction that the Force Field indicates. It",
    "can sometimes be very difficult to get an object out of a",
    "Force Field, so be careful. Beret is also affected by Force",
    "Fields, which can be helpful at times but sometimes very",
    "dangerous."},
   {"[ More Teleseekers ]"," ",
    "The Red and Yellow Teleseekers that you have now come to",
    "know so well are only two of the four types of Teleseekers",
    "that inhabit the Evil Corporation. The other two types are",
    "the Blue and Purple Teleseekers. Blue Teleseekers will always",
    "chase Beret, regardless of his use of telekinesis, and Purple",
    "Teleseekers will always fly away from Beret."},
   {"[ Gravity Switches ]"," ",
    "In the Ubiquitous Division of the Department of",
    "Gravitation, researchers have created a type of switch that",
    "can change the gravity in a room. These switches can be",
    "switched in the same ways that Solidity Switches can:",
    "by hitting them with an object or an explosive. Only",
    "inanimate objects are affected by the gravity shift."},
   {"[ Turrets ]"," ",
    "The enemies that you must face will continue to grow more",
    "and more fiendish as you delve deeper into the Evil",
    "Corporation. Turrets are enemies that rapidly shoot bullets",
    "which can destroy anything that is animate. They can not be",
    "destroyed by smashing. Beret will not be harmed by touching",
    "the body of a Turret."},
   {"[ Reincarnators ]"," ",
    "Researchers in the Department of Parapsychology have",
    "created machines called Reincarnators. Once a Reincarnator",
    "is activated by the touch of a person, it will be able to",
    "sense that person's death and pull their spirit into a copy",
    "of their body, which is created at the machine. Only one",
    "Reincarnator can be active at a time."},
   {"[ Ghosts ]", " ",
    "Due to the research on spiritual energy within the",
    "Department of Parapsychology, the department has become",
    "infested with Ghosts. Ghosts will not become visible until",
    "they are fairly close to Beret, but they are still there even",
    "while invisible. Ghosts can be smashed, as if they move too",
    "quickly, they will be forced to be solid."},
   {"[ Ghost Blocks ]", " ", " ",
    "If they are moved with enough speed, Ghost Blocks become",
    "immaterial and can pass through anything, even walls. They",
    "can still be moved with telekinesis even when they are not",
    "solid.", " "},
   {"[ Blockster ]", " ",
    "The last department in each wing of the Evil Corporation is",
    "home to one of the Three Evils. The Evil in command of the",
    "West Wing is named Blockster, and his lair is on the other",
    "side of this door. Once you defeat Blockster, you will be",
    "able to continue on to the North Wing and the departments it",
    "contains."},
  {"[ Antimatter ]", " ",
   "The researchers in the Department of Antimatter have",
   "succeeded in creating chunks of Antimatter, as well as walls",
   "made out of Antimatter. Antimatter destroys anything it",
   "touches, even walls. It must be handled with caution, or the",
   "entire department could be destroyed... and you don't want",
   "that, do you?"},
  {"[ Platforms ]", " ",
   "Platforms come in four varieties, one for each orthogonal",
   "direction. When they detect the presence and then absence of",
   "pressure from above (for example, if Beret steps on a Platform",
   "and then jumps off), they will move in the direction associated",
   "with that type of Platform.",
   " "},
  {"[ Fake Fireworks ]", " ",
   "Similarly to Fake Blocks, Fake Fireworks appear to be normal",
   "Fireworks until Beret comes close, and then they lunge for the",
   "kill. They retain the explosive nature of Fireworks, making them",
   "quite dangerous, as even if they miss Beret they may still",
   "explode nearby.", " "},
  {"[ Matterly ]", " ",
   "The third and final Evil, Matterly, is waiting for you behind",
   "this door. She is trickier and more fiendish than the Evils",
   "you have defeated so far, and there are rumors that Matterly",
   "has been developing experimental weapons that she has kept",
   "secret even from the other Evils, so be on your guard!", " "},
   {"[ Enemies ]", " ", " ",
    "Don't forget to defeat all the enemies in this room",
    "if you want to get the Red Medallion!", " ", " ", " "},
   {"[ Top Hat ]", " ",
    "The Three Evils have now been defeated, but Beret's quest",
    "is not quite over! His long-lost evil twin brother, Top Hat,",
    "has been the true power behind the Evil Corporation the",
    "whole time, using the Evils as pawns in his evil plots! Now",
    "Beret must stop Top Hat before it's too late!!!!!", " "},
   {"[ Antiseekers ]", " ", " ",
    "Antiseekers chase Beret all the time, like Blue",
    "Teleseekers, and they are made of Antimatter.",
    "They have fancy visors as well.", " ", " "},
   {"[ Top Hat ]", " ",
    "The final battle! Top Hat has protected himself with a",
    "field of telekinetic energy that will repel any object",
    "inside it. In other words, nothing can touch him! However,",
    "the field is sustained by five Field Generators in this",
    "room. If they can all be destroyed, Beret will have a",
    "chance to strike at Top Hat and defeat him!"},
   {"[ The End ]", " ",
    "Congratulations!",
    "Top Hat has been defeated, and Beret has finally",
    "succeeded in destroying the Evil Corporation! You can",
    "now play as Top Hat by pressing T. When you have all",
    "of the Medallions, you will open something very special",
    "in the Tower of the Evil Corporation."},
   {"[ Alternate Run Controls ]", " ",
    "If you would like to use double-tapping to run instead",
    "of holding shift to walk slowly, you can change this",
    "setting in the options menu, which is accessed by",
    "pressing Escape and selecting \"Options\". If you use",
    "the alternate control scheme, double-tap the direction",
    "you would like to run in."},
   {"[ Backup Save State ]", " ", " ",
    "If you ever accidentally save state in a bad spot, such",
    "as right before Beret dies, you can load the save state",
    "that you had saved before the current one by pressing",
    "Ctrl when you load state.", " "}};

const int medalsprx[6] = {5,5,5,5,5,3};
const int medalspry[6] = {15,18,19,16,17,11};

const char *copyright = "Beret v1.2.1 Copyright 2011 Nigel Kilmer";

int i, j;
int quit=0;
int inactive = 0, paused = 0, guides = 0, getinput = 0, statusbar = 1;
int istophat = 0;
int fullscreenmode = 0;
int camx=0, camy=0;
int mx, my, telething = -1, canbetelething = -1, touchtele, telestat;
int telesource = -1;
int cantele = 0, alwaystele = 0, teleon = 0;
int mousecammode = 0, runningmode = 0, deathstateload = 0, opencreator = 0, secretcode = 0;
int count = 0, svstcount = 0;
int deathtime = 0, fadetime=0, fadereason, walkaway;
int curparticle = 0;
int curmusic = 0, musicon = 1, curbkg = 1;
int startx, starty, startdir;

int spminutes, spseconds, spframes, bspminutes, bspseconds, bspframes;
int enemdead[ROOMMAX], enemdeadhere, enemalldead, benemalldead, bbossvar;
int gotmwht, bgotmwht, gotgmed, bgotgmed, gotpmed, bgotpmed;
int gotmcrn[4], bgotmcrn[4];
int gotmfrag[MFRAGTOTAL], bgotmfrag[MFRAGTOTAL], mfragcount, bmfragcount;
int hassavestate, hasbkpsavestate, loadedbkpsavestate, usedsavestate;
int hasgmed, haspmed, hasrmed, hasomed, hasbmed, haswmed;

int credittime;
int creatormode = 0, crtgridsize, crtgridsnap, crtplacetile;
int crtselect, crtplacetype, crtplacesubtype, crtlinking, crtinventory;
int crtplacedir, crtentrance, crtexit, crtexitroom, crtmessage;
int selx, sely;

int ingame, ingamereturn, bwalkaway;
int cancont, contlvl;
int initmapmsg, initgamemsg;
int trgentrance, nextopenlevel, trgmap;
int hassaved, loaderror, yesno, optselect, optmax, gotallfrags;

int mouse_visible;

int tiles[LVLMAX][LVLMAX][3], tilebackup[LVLMAX][LVLMAX][3];
Thing beret;
Thing things[THINGMAX], thingbackup[THINGMAX];
int selection[THINGMAX];
Particle particles[PARTICLEMAX];

int lvlWidth, lvlHeight;
int lvlCode, areaCode, gameNum, mapCode, tempAreaCode, msgcode;
int mapselect;
int gamemedals;
char gamename[25];
int gotmedals[MEDALMAX];
char game_gotfrags[100][MFRAGTOTAL];
char game_gotcorners[100][4];
char game_enemdead[100][ROOMMAX];
int beatlevel[100];
int gravdir, switchflags;

char support_path[250];

char msgline[250];

SDL_Surface* screen = NULL;
SDL_Surface* background = NULL;
SDL_Surface* invbackground = NULL;
SDL_Surface* spritesheet = NULL;
SDL_Surface* tilesheet = NULL;
SDL_Surface* teleguide = NULL;
SDL_Surface* title = NULL;
SDL_Surface* credits = NULL;
SDL_Surface* mapbkg = NULL;
SDL_Surface* pit = NULL;
SDL_Surface* lvlnumbkg = NULL;
SDL_Surface* gameselect = NULL;
SDL_Surface* msgback = NULL;
SDL_Surface* optback= NULL;
SDL_Surface* getinputback = NULL;
SDL_Surface* message = NULL;
SDL_Surface* fades[5];

char messagestr[200], inputstr[25], getinputstr[50];
int inputpos, inputlength;

FILE* file;
FILE* msgfile;

Uint32 selcolor;

SDL_Event event;
int key1=NONE, key2=NONE, key3=NONE;
int remkey1;
int camxkey=NONE, camykey=NONE;
int freecam;

TTF_Font *font = NULL, *smfont = NULL, *medfont = NULL;
SDL_Color textcolor = {0, 0, 0}, textcolor2 = {220,220,220};
Uint32 redColor, whiteColor, greenColor, blueColor;

Mix_Music* music[MUSIC_MAX];
Mix_Chunk* sound[SOUND_MAX];


float f_sqr(float x) {return x*x;}
float f_abs(float x) {return x<0?-x:x;}


void resolve_fade(void);
void resolve_input(SDLKey);
void start_map(void);

void init_fade(int ftype) {
  if (fadetime == 0) {
    fadetime = 14;
    fadereason = ftype;
    svstcount = 0;
  }
}

SDL_Surface* load_img(char* filename) {
  SDL_Surface* loadImg = NULL;
  SDL_Surface* optImg = NULL;

  loadImg = IMG_Load(filename);

  // Create the optimized image
  if (loadImg != NULL) {
    optImg = SDL_DisplayFormatAlpha(loadImg);
    SDL_FreeSurface(loadImg);
  }

  return optImg;
}


void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* dest) {
  SDL_Rect offset;
  offset.x = x;
  offset.y = y;

  SDL_BlitSurface(source, NULL, dest, &offset);
}


void apply_particle(int x, int y, int color) {
  SDL_Rect offset;
  offset.x = x-2;
  offset.y = y-2;

  // Create the rectangle specifying area of sprite sheet to apply
  SDL_Rect clip;
  clip.x = 19*SPR_SIZE+4*(color%7);
  clip.y = 4*SPR_SIZE+4*(color/7);
  clip.w = 4;
  clip.h = 4;

  SDL_BlitSurface(spritesheet, &clip, screen, &offset);
}


void apply_sprite(int x, int y, int sprx, int spry, int sprw, int sprh,
                  SDL_Surface* source, SDL_Surface* dest) {
  SDL_Rect offset;
  offset.x = x;
  offset.y = y;

  // Create the rectangle specifying area of sprite sheet to apply
  SDL_Rect clip;
  clip.x = sprx*SPR_SIZE;
  clip.y = spry*SPR_SIZE;
  clip.w = sprw*SPR_SIZE;
  clip.h = sprh*SPR_SIZE;

  SDL_BlitSurface(source, &clip, dest, &offset);
}


int init() {

  #ifdef __WIN32__
  sprintf(support_path, "");
  #else
  char filestr[512];
  // Get the home directory of the user.
  struct passwd *pwd = getpwuid(getuid());
  if (pwd) {
    // Create the directory for application data.
    sprintf(support_path, "%s/%s", pwd->pw_dir, SUPPORT_PATH);
    mkdir(support_path, S_IRWXU);
    // Create the directory for custom room data.
    sprintf(filestr, "%s/rooms", support_path);
    mkdir(filestr, S_IRWXU);
    // Create the directory for save data.
    sprintf(filestr, "%s/saves", support_path);
    mkdir(filestr, S_IRWXU);
  }
  #endif

  if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
    printf("Error: couldn't initialize SDL\n");
    return 0;
  }

  // For some reason this causes X windows (at least under Awesome WM)
  // to refuse to accept keyboard input.
  SDL_Surface* icon = SDL_LoadBMP(RESOURCE_PATH "images" DIRSEP "block.bmp");
  SDL_WM_SetIcon(icon, NULL);

  screen = SDL_SetVideoMode(SCR_WIDTH, SCR_HEIGHT, SCR_BPP, SDL_FULLSCREEN);
  fullscreenmode = 1;
  if (screen == NULL) {
    printf("Error: couldn't initialize the screen\n");
    return 0;
  }

  if (TTF_Init() == -1) {
    printf("Error: couldn't initalize fonts\n");
    return 0;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
    printf("Error: couldn't initalize sound handling\n");
    return 0;
  }
  Mix_AllocateChannels(NUM_CHANNELS);

  SDL_ShowCursor(SDL_DISABLE);
  mouse_visible = 0;

  selcolor = SDL_MapRGB(screen->format, 0, 0xd0, 0x20);

  SDL_WM_SetCaption(GAME_TITLE, NULL);

  return 1;
}


int load_files() {
  char bkgstr[512];

  // Load images
  tilesheet = load_img(RESOURCE_PATH "images" DIRSEP "tilesheet.png");
  spritesheet = load_img(RESOURCE_PATH "images" DIRSEP "spritesheet.png");
  invbackground = load_img(RESOURCE_PATH "images" DIRSEP "inventory.png");
  background = load_img(RESOURCE_PATH "images" DIRSEP "bkg1.png");
  teleguide = load_img(RESOURCE_PATH "images" DIRSEP "teleguide.png");
  title = load_img(RESOURCE_PATH "images" DIRSEP "title.png");
  credits = load_img(RESOURCE_PATH "images" DIRSEP "credits.png");
  mapbkg = load_img(RESOURCE_PATH "images" DIRSEP "mapbkg.png");
  pit = load_img(RESOURCE_PATH "images" DIRSEP "pit.png");
  lvlnumbkg = load_img(RESOURCE_PATH "images" DIRSEP "lvlnum.png");
  gameselect = load_img(RESOURCE_PATH "images" DIRSEP "gameselect.png");
  msgback = load_img(RESOURCE_PATH "images" DIRSEP "msg.png");
  optback = load_img(RESOURCE_PATH "images" DIRSEP "opt.png");
  getinputback = load_img(RESOURCE_PATH "images" DIRSEP "getinput.png");
  for (i=1;i<=5; i++) {
    sprintf(bkgstr, "%simages%sfade%d.png", RESOURCE_PATH, DIRSEP, i);
    fades[i-1] = load_img(bkgstr);
  }

  // Load fonts
  font = TTF_OpenFont(RESOURCE_PATH "AveriaSans-Regular.ttf", 24);
  smfont = TTF_OpenFont(RESOURCE_PATH "AveriaSans-Regular.ttf", 9);
  medfont = TTF_OpenFont(RESOURCE_PATH "AveriaSans-Regular.ttf", 16);

  // Load music
  for (i=0; i<MUSIC_MAX; i++) {
    sprintf(bkgstr, "%smusic%sberet%d.ogg", RESOURCE_PATH, DIRSEP, i);
    if (!(music[i] = Mix_LoadMUS(bkgstr)))
      return 0;
  }

  //Load sound effects
  sound[SND_KNOCK] = Mix_LoadWAV(RESOURCE_PATH "sfx/knock.wav");
  sound[SND_KNOCK+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/knock2.wav");
  sound[SND_KNOCK+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/knock3.wav");
  sound[SND_CLINK] = Mix_LoadWAV(RESOURCE_PATH "sfx/clink.wav");
  sound[SND_CLINK+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/clink2.wav");
  sound[SND_CLINK+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/clink3.wav");
  sound[SND_CRUNCH] = Mix_LoadWAV(RESOURCE_PATH "sfx/crunch.wav");
  sound[SND_CRUNCH+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/crunch2.wav");
  sound[SND_CRUNCH+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/crunch3.wav");
  sound[SND_SQUELCH] = Mix_LoadWAV(RESOURCE_PATH "sfx/squelch.wav");
  sound[SND_SQUELCH+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/squelch2.wav");
  sound[SND_SQUELCH+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/squelch3.wav");
  sound[SND_STEP] = Mix_LoadWAV(RESOURCE_PATH "sfx/step1.wav");
  sound[SND_STEP+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/step2.wav");
  sound[SND_STEP+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/step3.wav");
  sound[SND_JUMP] = Mix_LoadWAV(RESOURCE_PATH "sfx/jump1.wav");
  sound[SND_JUMP+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/jump2.wav");
  sound[SND_JUMP+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/jump3.wav");
  sound[SND_COLLECT] = Mix_LoadWAV(RESOURCE_PATH "sfx/frag1.wav");
  sound[SND_COLLECT+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/frag2.wav");
  sound[SND_COLLECT+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/frag3.wav");
  sound[SND_COLLECT+3] = Mix_LoadWAV(RESOURCE_PATH "sfx/frag4.wav");
  sound[SND_COLLECT+4] = Mix_LoadWAV(RESOURCE_PATH "sfx/frag5.wav");
  sound[SND_COLLECT+5] = Mix_LoadWAV(RESOURCE_PATH "sfx/frag6.wav");
  sound[SND_BOOM] = Mix_LoadWAV(RESOURCE_PATH "sfx/boom1.wav");
  sound[SND_BOOM+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/boom2.wav");
  sound[SND_BOOM+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/boom3.wav");
  sound[SND_TICK] = Mix_LoadWAV(RESOURCE_PATH "sfx/tick1.wav");
  sound[SND_TICK+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/tick2.wav");
  sound[SND_TICK+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/tick3.wav");
  sound[SND_HOP] = Mix_LoadWAV(RESOURCE_PATH "sfx/hop1.wav");
  sound[SND_HOP+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/hop2.wav");
  sound[SND_HOP+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/hop3.wav");
  sound[SND_INFECT] = Mix_LoadWAV(RESOURCE_PATH "sfx/infect.wav");
  sound[SND_SWITCHGR] = Mix_LoadWAV(RESOURCE_PATH "sfx/switch-gr1.wav");
  sound[SND_SWITCHGR+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/switch-gr2.wav");
  sound[SND_SWITCHGR+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/switch-gr3.wav");
  sound[SND_SWITCHRD] = Mix_LoadWAV(RESOURCE_PATH "sfx/switch-rd1.wav");
  sound[SND_SWITCHRD+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/switch-rd2.wav");
  sound[SND_SWITCHRD+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/switch-rd3.wav");
  sound[SND_SWITCHGV] = Mix_LoadWAV(RESOURCE_PATH "sfx/switch-gv1.wav");
  sound[SND_SWITCHGV+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/switch-gv2.wav");
  sound[SND_SWITCHGV+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/switch-gv3.wav");
  sound[SND_CHOICEBERET] = Mix_LoadWAV(RESOURCE_PATH "sfx/choice-beret1.wav");
  sound[SND_CHOICEBERET+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/choice-beret2.wav");
  sound[SND_CHOICEBERET+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/choice-beret3.wav");
  sound[SND_CHOICEOBJECT] = Mix_LoadWAV(RESOURCE_PATH "sfx/choice-object1.wav");
  sound[SND_CHOICEOBJECT+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/choice-object2.wav");
  sound[SND_CHOICEOBJECT+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/choice-object3.wav");
  sound[SND_TURRET] = Mix_LoadWAV(RESOURCE_PATH "sfx/shot1.wav");
  sound[SND_TURRET+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/shot2.wav");
  sound[SND_TURRET+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/shot3.wav");
  sound[SND_PLATFORM] = Mix_LoadWAV(RESOURCE_PATH "sfx/platform1.wav");
  sound[SND_PLATFORM+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/platform2.wav");
  sound[SND_PLATFORM+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/platform3.wav");
  sound[SND_STICK] = Mix_LoadWAV(RESOURCE_PATH "sfx/stick.wav");
  sound[SND_ANTIMATTER] = Mix_LoadWAV(RESOURCE_PATH "sfx/antimatter1.wav");
  sound[SND_ANTIMATTER+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/antimatter2.wav");
  sound[SND_ANTIMATTER+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/antimatter3.wav");
  sound[SND_FAKE] = Mix_LoadWAV(RESOURCE_PATH "sfx/fake.wav");
  sound[SND_FAKE+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/fake2.wav");
  sound[SND_POP] = Mix_LoadWAV(RESOURCE_PATH "sfx/pop.wav");
  sound[SND_POP+1] = Mix_LoadWAV(RESOURCE_PATH "sfx/pop2.wav");
  sound[SND_POP+2] = Mix_LoadWAV(RESOURCE_PATH "sfx/pop3.wav");
  sound[SND_REGEN] = Mix_LoadWAV(RESOURCE_PATH "sfx/regen.wav");
  sound[SND_REGENINIT] = Mix_LoadWAV(RESOURCE_PATH "sfx/regeninit.wav");
  sound[SND_MEDW] = Mix_LoadWAV(RESOURCE_PATH "sfx/med-w.wav");
  sound[SND_MEDO] = Mix_LoadWAV(RESOURCE_PATH "sfx/med-o.wav");
  sound[SND_MEDB] = Mix_LoadWAV(RESOURCE_PATH "sfx/med-b.wav");
  sound[SND_MEDR] = Mix_LoadWAV(RESOURCE_PATH "sfx/med-r.wav");
  sound[SND_MEDG] = Mix_LoadWAV(RESOURCE_PATH "sfx/med-g.wav");
  sound[SND_MEDP] = Mix_LoadWAV(RESOURCE_PATH "sfx/med-p.wav");
  sound[SND_CORNER] = Mix_LoadWAV(RESOURCE_PATH "sfx/corner.wav");

  return 1;
}


void clean_up() {

  // Free surfaces
  SDL_FreeSurface(screen);
  SDL_FreeSurface(invbackground);
  SDL_FreeSurface(background);
  for (i=0; i<5; i++)
    SDL_FreeSurface(fades[i]);
  SDL_FreeSurface(tilesheet);
  SDL_FreeSurface(spritesheet);
  SDL_FreeSurface(teleguide);
  SDL_FreeSurface(title);
  SDL_FreeSurface(credits);
  SDL_FreeSurface(mapbkg);
  SDL_FreeSurface(pit);
  SDL_FreeSurface(lvlnumbkg);
  SDL_FreeSurface(gameselect);
  SDL_FreeSurface(msgback);
  SDL_FreeSurface(optback);
  SDL_FreeSurface(getinputback);

  // Free music and sounds
  for (i=0; i<SOUND_MAX; i++) Mix_FreeChunk(sound[i]);
  for (i=0; i<MUSIC_MAX; i++) Mix_FreeMusic(music[i]);
  Mix_CloseAudio();

  // Free fonts
  TTF_CloseFont(font);
  TTF_CloseFont(smfont);
  TTF_CloseFont(medfont);
  TTF_Quit();

  SDL_Quit();
}


/*
 * Taken from the SDL documentation and modified.
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 * Set alpha = -1 to invert color
 */
void putpixel(SDL_Surface *surface, int x, int y, 
              Uint32 pixel, int alpha) {
  int bpp = surface->format->BytesPerPixel;
  /* Here p is the address to the pixel we want to set */
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
  
  Uint32 oldpix = *(Uint32 *)p;
  if (alpha == -1) {
    *(Uint32 *)p = ~oldpix;
  } else {
    float falpha = (float)alpha/0xff;
    int r =((pixel&0xff0000)>>16)*falpha+((oldpix&0xff0000)>>16)*(1-falpha);
    int g =((pixel&0xff00)>>8)*falpha+((oldpix&0xff00)>>8)*(1-falpha);
    int b =(pixel&0xff)*falpha+(oldpix&0xff)*(1-falpha);
    *(Uint32 *)p = (r<<16)+(g<<8)+b;
  }
}


// draws a line from (x1, y1) to (x2, y2)
void draw_line(SDL_Surface* surface, int x1, int y1,
               int x2, int y2, Uint32 pixel) {
  int curx = x1, cury = y1;
  float slope = 2;
  if (x2 != x1) slope = 1.0*(y2-y1)/(x2-x1);
  
  while (!((f_abs(slope)>=1 && cury == y2) || 
           (f_abs(slope)<1 && curx == x2))) {
    if (curx >= 0 && curx < SCR_WIDTH &&
        cury >= 0 && cury < SCR_HEIGHT)
      putpixel(surface, curx, cury, pixel, 0xff);
    if (x1 == x2) {
      cury += cury < y2 ? 1 : -1;
    } else { 
      if (slope <= -1 || slope >= 1) {
        cury += cury < y2 ? 1 : -1;
        curx = x1 + (cury-y1)/slope;
      } else { 
        curx += curx < x2 ? 1 : -1;
        cury = y1 + slope*(curx-x1);
      }
    }
  }
  if (curx >= 0 && curx < SCR_WIDTH &&
      cury >= 0 && cury < SCR_HEIGHT)
  putpixel(surface, curx, cury, pixel, 0xff);
}


// draws a semi-transparent rectangle to the surface
void draw_rect(SDL_Surface* surface, int x1, int y1,
               int x2, int y2, Uint32 pixel, int alpha) {
  int curx, cury;
  if ( SDL_MUSTLOCK(screen) ) {
    if ( SDL_LockSurface(screen) < 0 ) {
      fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
      return;
    }
  }
  for (curx=x1; curx<=x2; curx++) {
    for (cury=y1; cury<=y2; cury++) {
      putpixel(surface, curx, cury, pixel, alpha);
    }
  }
  draw_line(surface, x1, y1, x2, y1, 0);
  draw_line(surface, x1, y1, x1, y2, 0);
  draw_line(surface, x2, y1, x2, y2, 0);
  draw_line(surface, x1, y2, x2, y2, 0);
  if (SDL_MUSTLOCK(screen)) {
    SDL_UnlockSurface(screen);
  }
}


// draws the screen fading out
void draw_fade() {
  apply_surface(0,0,fades[fadereason==0?(fadetime-2)/3:5-((fadetime+2)/3)],screen);
}


// draws the particles to the screen
void draw_particles() {
  for (i=0; i<PARTICLEMAX; i++) {
    if (particles[i].time > 0) {
      apply_particle(particles[i].x-camx,particles[i].y-camy,particles[i].color);
    }
  }
}


void create_particle(int x, int y, float vx, float vy,
                     int color, int time) {
  particles[curparticle].x = x;
  particles[curparticle].y = y;
  particles[curparticle].vx = vx/1.5;
  particles[curparticle].vy = vy/1.5;
  particles[curparticle].color = color;
  particles[curparticle].time = time*1.5;
  curparticle++;
  curparticle %= PARTICLEMAX;
}


void stop_telekinesis() {
  things[telething].telething = 0;
  things[telething].teledelay = TELEDELAY;
  telething = -1;
  canbetelething = -1;
  teleon = 0;
}


void display_message(int x, int y, TTF_Font *mfont, const char* msg, int bktext) {
  message = TTF_RenderText_Blended(mfont, msg?msg:messagestr, textcolor);
  apply_surface(x-message->w/2,y-message->h/2,message,screen);  
  SDL_FreeSurface(message);
  if (!bktext) {
    message = TTF_RenderText_Blended(mfont, msg?msg:messagestr, textcolor2);
    apply_surface(x-message->w/2+TEXTSHADOW,y-message->h/2+TEXTSHADOW,message,screen);  
    SDL_FreeSurface(message);
  }
}


void start_telekinesis() {
  if (((alwaystele && teleon) || (!alwaystele && cantele)) &&
      telething == -1 && canbetelething != -1) {
    telething = canbetelething;
    things[telething].telething = 1;
  }
}


void fix_camera() {
  if (camx < 0) camx = 0;
  if (camx > lvlWidth-SCR_WIDTH) camx = lvlWidth-SCR_WIDTH;
  if (camy < 0) camy = 0;
  if (camy > lvlHeight-SCR_HEIGHT) camy = lvlHeight-SCR_HEIGHT;
}


void clear_room() {
  // clear position
  startx = 0;
  starty = 0;

  // clear things
  for (i=0; i<THINGMAX; i++) {
    things[i].type = NOTYPE;
    thingbackup[i].type = NOTYPE;
    things[i].islinked = -1;
    thingbackup[i].islinked = -1;
  }

  // clear tiles
  for (i=0; i<LVLMAX; i++)
    for (j=0; j<LVLMAX; j++) {
      tiles[i][j][0] = EMPTY;
      tilebackup[i][j][0] = EMPTY;
    }

  lvlWidth = SCR_WIDTH;
  lvlHeight = SCR_HEIGHT;
  fix_camera();

  hassaved = 0;
}


void center_camera() {
  int curcamx = camx, curcamy = camy;
  camx = beret.x+beret.width/2-SCR_WIDTH/2;
  camy = beret.y+beret.height/2-SCR_HEIGHT/2;
  fix_camera();
  mx -= curcamx-camx;
  my -= curcamy-camy;
}


void load_backups() {
  // reset flags
  switchflags &= ~GREENFLAG;
  switchflags &= ~REDFLAG;
  switchflags &= ~REINCARNATE;
  gravdir = DOWN;

  // reset collectibles
  mfragcount = bmfragcount;
  for (i=0; i<MFRAGTOTAL; i++) gotmfrag[i] = bgotmfrag[i];
  for (i=0; i<4; i++) gotmcrn[i] = bgotmcrn[i];
  gotmwht = bgotmwht;
  gotgmed = bgotgmed;
  gotpmed = bgotpmed;
  spframes = bspframes;
  spseconds = bspseconds;
  spminutes = bspminutes;
  enemalldead = benemalldead;

  // set up vars
  deathtime = 0;
  curparticle = 0;
  gotallfrags = 0;
  int* bossvars = get_bossvars();
  for (i=0; i<BOSSMAX-1; i++) bossvars[i] = 0;
  bossvars[BOSSMAX-1] = bbossvar;
  

  // reset particles
  for (i=0; i<PARTICLEMAX; i++) particles[i].time = 0;

  // set up beret
  if (telething > -1) stop_telekinesis();
  int bspeed = beret.speed;
  make_beret(&beret, BERET, istophat, startx, starty, 1, things);
  beret.speed = bspeed?bspeed:WALK_SPEED;
  beret.dir = startdir;
  walkaway = bwalkaway;
  center_camera();

  // load things
  enemdeadhere = enemdead[areaCode] || game_enemdead[lvlCode][areaCode];
  for (i=0; i<THINGMAX; i++) {
    if (thingbackup[i].type == NOTYPE) things[i].type = NOTYPE;
    else copy_thing(&thingbackup[i], things, i);
  }

  // load tiles
  int k;
  for (i=0; i<LVLMAX; i++)
    for (j=0; j<LVLMAX; j++)
      for (k=0; k<3; k++)
        tiles[i][j][k] = tilebackup[i][j][k];
}


void check_room_dead() {
  int foundone = 0;
  int iter;
  for (iter=0; iter<THINGMAX; iter++) {
    if (things[iter].type != NOTYPE && !things[iter].dead && things[iter].animate) {
      foundone = 1;
      break;
    }
  }
  if (!foundone) {
    enemdeadhere = 1;
    for (iter=0; iter<ROOMMAX; iter++) {
      if (iter != areaCode && !enemdead[iter] && !game_enemdead[lvlCode][iter]) {foundone = 1; break;}
    }
    if (!foundone && !enemalldead && !hasrmed) {
      enemalldead = 1;
      play_sound(SND_MEDR);
      if (statusbar) {
        make_expl(RMEDPOS+15+camx,30+camy,0,0,RED,6,100);
        make_expl(RMEDPOS+15+camx,30+camy,0,0,PINK,6,40);
      }
    }
  }
}


void go_to_room(int entr, int roomnum) {
  trgentrance = entr;
  tempAreaCode = roomnum;
  init_fade(5);
}


void switch_music(int newmusic, int forceswitch) {
  if (newmusic != curmusic || forceswitch || (Mix_PlayingMusic() && (musicon == 0 || musicon == 3))) {
    curmusic = newmusic;
    if (Mix_PlayingMusic()) Mix_HaltMusic();
    if (musicon == 1 || musicon == 2) Mix_PlayMusic(music[curmusic], -1);
  }
}


// saves the state of the room and collectables 
// for returning to this point
void save_room_return(int flag) {
  char filestr[512];
  sprintf(filestr, "%ssaves%sgame%d.ret", support_path, DIRSEP, gameNum);
  file = fopen(filestr, "wb");
  if (file) {
    fwrite(&lvlCode, sizeof(int), 1, file);
    fwrite(&areaCode, sizeof(int), 1, file);
    fwrite(&bgotmwht, sizeof(int), 1, file);
    fwrite(bgotmcrn, sizeof(int), 4, file);
    fwrite(&bmfragcount, sizeof(int), 1, file);
    fwrite(bgotmfrag, sizeof(int), MFRAGTOTAL, file);
    fwrite(&bgotgmed, sizeof(int), 1, file);
    fwrite(&bgotpmed, sizeof(int), 1, file);
    fwrite(&trgentrance, sizeof(int), 1, file);
    fwrite(&bspminutes, sizeof(int), 1, file);
    fwrite(&bspseconds, sizeof(int), 1, file);
    fwrite(&bspframes, sizeof(int), 1, file);
    fwrite(&benemalldead, sizeof(int), 1, file);
    fwrite(enemdead, sizeof(int), ROOMMAX, file);
    fwrite(&haswmed, sizeof(int), 1, file);
    fwrite(&hasomed, sizeof(int), 1, file);
    fwrite(&hasbmed, sizeof(int), 1, file);
    fwrite(&hasrmed, sizeof(int), 1, file);
    fwrite(&hasgmed, sizeof(int), 1, file);
    fwrite(&haspmed, sizeof(int), 1, file);
    fwrite(&usedsavestate, sizeof(int), 1, file);
    fwrite(&hassavestate, sizeof(int), 1, file);
    fwrite(&flag, sizeof(int), 1, file);
    fwrite(&((get_bossvars())[BOSSMAX-1]), sizeof(int), 1, file);

    fclose(file);
  }
}


// sets up the room data for playing
void init_play(int flag, int savereturn) {
  char filestr[512];
  
  // reset or set collectible backup data
  if (flag) {
    // Set backups to the current state
    bgotmwht = gotmwht;
    for (i=0;i<4;i++) bgotmcrn[i] = gotmcrn[i];
    bmfragcount = mfragcount;
    for (i=0;i<MFRAGTOTAL;i++) bgotmfrag[i] = gotmfrag[i];
    bgotgmed = gotgmed;
    bgotpmed = gotpmed;
  } else {
    // Starting the level, reset backups
    bgotmwht = 0;
    for (i=0;i<4;i++) bgotmcrn[i] = 0;
    bmfragcount = 0;
    for (i=0;i<MFRAGTOTAL;i++) {
      bgotmfrag[i] = 0;
      if (game_gotfrags[lvlCode][i]) bmfragcount++;
    }
    bgotgmed = 0;
    bgotpmed = 0;
  }
  
  if (!flag) {
    startdir = 1;
    bwalkaway = 0;
    bbossvar = 0;
  }
  // set room backups
  if (flag && (get_bossvars())[BOSSMAX-1]) bbossvar = 1;
  for (i=0; i<THINGMAX; i++) {
    if (things[i].type == NOTYPE) thingbackup[i].type = NOTYPE;
    else copy_thing(&things[i], thingbackup, i);
    if (flag && things[i].type == DOOR && things[i].timer == trgentrance) {
      startx = things[i].x+things[i].width/2-10;
      starty = things[i].y+things[i].height-24;
      bwalkaway = 0;
    }
    if (flag && things[i].type == SIGN && things[i].timer == trgentrance &&
        (things[i].x <= SPR_SIZE*2 || things[i].x >= lvlWidth-SPR_SIZE*3)) {
      startx = things[i].subtype%2?lvlWidth-20:0;
      starty = things[i].y+things[i].height-24;
      startdir = things[i].subtype%2?0:1;
      bwalkaway = 2-things[i].subtype%2;
    }
    if (things[i].type == WHITEMEDAL) {
      if (bgotmwht) thingbackup[i].type = NOTYPE;
      if (haswmed) thingbackup[i].subtype = 1;
    }
    if (things[i].type == MEDALCORNER) {
      if (bgotmcrn[things[i].subtype]) thingbackup[i].type = NOTYPE;
      if (game_gotcorners[lvlCode][things[i].subtype]) thingbackup[i].subtype += 4;
    }
    if (things[i].type == MEDALFRAGMENT && things[i].dir > -1) {
      if (bgotmfrag[things[i].dir]) thingbackup[i].type = NOTYPE;
      if (game_gotfrags[lvlCode][things[i].dir]) thingbackup[i].subtype += 2;
    }
  }
  int k;
  for (i=0; i<LVLMAX; i++)
    for (j=0; j<LVLMAX; j++)
      for (k=0; k<3; k++)
        tilebackup[i][j][k] = tiles[i][j][k];
  hassavestate = 0;
  hasbkpsavestate = 0;
  loadedbkpsavestate = 0;
  if (!flag) usedsavestate = 0;
  freecam = 0;
  
  // load rooms with enemies in them from level metadata
  // load timer from level metadata
  if (!flag) {
    char lcode;
    if (lvlCode < 80) {
      sprintf(filestr, RESOURCE_PATH "rooms/metas");
    } else {
      sprintf(filestr, "%srooms%slvl%d.meta", support_path, DIRSEP, lvlCode);
    }
    file = fopen(filestr, "rb");
    if (file) {
      do {
	if (lvlCode < 80) fread(&lcode, sizeof(char), 1, file);
	fread(enemdead, sizeof(int), ROOMMAX, file);
	fread(&bspminutes, sizeof(int), 1, file);
	fread(&bspseconds, sizeof(int), 1, file);
      } while (lvlCode < 80 && lcode != lvlCode && !feof(file));
      fclose(file);
    } else {
      for (i=0; i<ROOMMAX; i++) enemdead[i] = 0;
      bspminutes = 0;
      bspseconds = 0;
    }
    bspframes = 0;
    benemalldead = 0;
  } else {
    bspminutes = spminutes;
    bspseconds = spseconds;
    bspframes = spframes;
    if (enemdeadhere) enemdead[tempAreaCode] = 1;
    benemalldead = enemalldead;
  }
  
  if (!flag) {
    haswmed = gotmedals[lvlCode*6];
    hasomed = gotmedals[lvlCode*6+1];
    hasbmed = gotmedals[lvlCode*6+2];
    hasrmed = gotmedals[lvlCode*6+3];
    hasgmed = gotmedals[lvlCode*6+4];
    haspmed = gotmedals[lvlCode*6+5];
  }
  
  if (ingame == 3 && savereturn) save_room_return(flag);
  
  load_backups();
}


void game_init() {
  gravdir = DOWN;
  switchflags = 0;
  lvlCode = 0;
  areaCode = 0;
  gameNum = 1;
  mfragcount = 0;
  enemalldead = 0;
  freecam = 0;

  initialize_thingnodes();

  initmapmsg = 0;
  initgamemsg = 0;

  mousecammode = 1;

  ingame = 0;
  walkaway = 0;

  switch_music(0, 1);

  crtplacetile = PURPLETILE;
  crtplacetype = WOODBLOCK;
  crtplacesubtype = 0;
  crtplacedir = 0;
  crtentrance = 0;
  crtexit = 0;
  crtexitroom = 0;
  crtmessage = 0;

  read_level();
  init_play(1,1);
}
 

void clear_selection() {
  for (i=0; i<THINGMAX; i++) {
    selection[i] = 0;
  }
}


void init_creator() {
  crtgridsize = 1;
  crtgridsnap = 1;
  crtlinking = 0;
  crtinventory = 0;
  switchflags |= NOCOLLIDEFLAG;
  crtselect = 0;
  clear_selection();
  load_backups();
}


// removes the room return data
void remove_room_return() {
  char filestr[512];
  sprintf(filestr, "%ssaves%sgame%d.ret", support_path, DIRSEP, gameNum);
  remove(filestr);
}


// reloads room return data
void load_room_return() {
  char filestr[512];
  int flag, hsvsttemp;
  sprintf(filestr, "%ssaves%sgame%d.ret", support_path, DIRSEP, gameNum);
  file = fopen(filestr, "rb");
#define daskjdf

  if (file) {
    fread(&lvlCode, sizeof(int), 1, file);
    fread(&areaCode, sizeof(int), 1, file);
    fread(&gotmwht, sizeof(int), 1, file);
    fread(gotmcrn, sizeof(int), 4, file);
    fread(&mfragcount, sizeof(int), 1, file);
    fread(gotmfrag, sizeof(int), MFRAGTOTAL, file);
    fread(&gotgmed, sizeof(int), 1, file);
    fread(&gotpmed, sizeof(int), 1, file);
    fread(&trgentrance, sizeof(int), 1, file);
    fread(&spminutes, sizeof(int), 1, file);
    fread(&spseconds, sizeof(int), 1, file);
    fread(&spframes, sizeof(int), 1, file);
    fread(&enemalldead, sizeof(int), 1, file);
    fread(enemdead, sizeof(int), ROOMMAX, file);
    fread(&haswmed, sizeof(int), 1, file);
    fread(&hasomed, sizeof(int), 1, file);
    fread(&hasbmed, sizeof(int), 1, file);
    fread(&hasrmed, sizeof(int), 1, file);
    fread(&hasgmed, sizeof(int), 1, file);
    fread(&haspmed, sizeof(int), 1, file);
    fread(&usedsavestate, sizeof(int), 1, file);
    fread(&hsvsttemp, sizeof(int), 1, file);
    fread(&flag, sizeof(int), 1, file);
    if (!fread(&((get_bossvars())[BOSSMAX-1]), sizeof(int), 1, file))
      (get_bossvars())[BOSSMAX-1] = 0;
    mapCode = lvlCode / 20;

    fclose(file);

    read_level();

    if (!loaderror) {
      ingame = 3;
      init_play(flag, 0);
    } else {
      ingame = 2;
      start_map();
    }

    hassavestate = hsvsttemp;
    hasbkpsavestate = 0;
    loadedbkpsavestate = 0;
  }
}


void switch_background(int newbkg) {
  char filestr[512];

  if (newbkg != curbkg) {
    curbkg = newbkg;
    if (curbkg > 0) {
      SDL_FreeSurface(background);
      sprintf(filestr, RESOURCE_PATH "images%sbkg%d.png", DIRSEP, curbkg);
      background = load_img(filestr);
    }
  }
}


// Plays the sound with the given index once on the first free channel.
void play_sound(int index) {
  if (musicon == 1 || musicon == 3) Mix_PlayChannel(-1, sound[index], 0);
}


// Determines if the object is on the visible screen.
int on_screen(int xpos, int ypos, int xsize, int ysize) {
  return xpos + xsize > camx - SOUNDDIST &&
    xpos < camx + SCR_WIDTH + SOUNDDIST &&
    ypos + ysize > camy - SOUNDDIST &&
    ypos < camy + SCR_HEIGHT + SOUNDDIST;
}


void set_up_input(char* inptstr, int inputtype, 
                  int ilength, int deflt) {
  sprintf(getinputstr, "%s", inptstr);
  getinput = inputtype;
  inputlength = ilength;
  inputpos = 0;
  if (deflt > -1) {
    sprintf(inputstr, "%d", deflt);
    inputpos = strlen(inputstr);
  } else strcpy(inputstr, "-");
  yesno = 0;
}


/* // Read level data stored in the executable. */
/* void read_intern_level() { */
/*   int k; */
/*   clear_room(); */
/*   char *stp, *enp; */
/*   int musictemp, bkgtemp; */
/*   if (get_intern_ptrs(&stp, &enp)) { */
/*     hassaved = 1; */
/*     bkgtemp = *(int*)stp; */
/*     stp += sizeof(int); */
/*     musictemp = *(int*)stp; */
/*     stp += sizeof(int); */
/*     lvlWidth = *(int*)stp; */
/*     stp += sizeof(int); */
/*     lvlHeight = *(int*)stp; */
/*     stp += sizeof(int); */
/*     startx = *(int*)stp; */
/*     stp += sizeof(int); */
/*     starty = *(int*)stp; */
/*     stp += sizeof(int); */
/*     for (i = 0; i < THINGMAX; i++) { */
/*       memcpy(&things[i], stp, sizeof(Thing)); */
/*       stp += sizeof(Thing); */
/*     } */
/*     for (i = 0; i < lvlWidth/SPR_SIZE; i++) */
/*       for (j = 0; j < lvlHeight/SPR_SIZE; j++) */
/* 	for (k = 0; k < 3; k++) { */
/* 	  tiles[i][j][k] = *(int*)stp; */
/* 	  stp += sizeof(int); */
/* 	} */
/*     if (bkgtemp < 1 || bkgtemp > BKG_MAX - 1) bkgtemp = 1; */
/*     if (musictemp < 0 || musictemp > MUSIC_MAX - 1) musictemp = 0; */
/*     clear_selection(); */
/*     switch_music(musictemp, 0); */
/*     switch_background(bkgtemp); */
/*   } else { */
/*     loaderror = 1; */
/*     set_up_input("The file was not found",1,-1,-1); */
/*   } */
/* } */


void read_level() {
  clear_room();
  char filestr[512];
  int musictemp, bkgtemp, k;
  char lcode, acode;
  if (lvlCode < 80) {
    sprintf(filestr, RESOURCE_PATH "rooms/rooms");
  } else {
    sprintf(filestr, "%srooms%sl%dr%d.room", support_path, DIRSEP, lvlCode, areaCode);
  }
  file = fopen(filestr, "rb");
  if (file) {
    hassaved = 1;
    do {
      if (lvlCode < 80) {
	fread(&lcode, sizeof(char), 1, file);
	fread(&acode, sizeof(char), 1, file);
      }
      fread(&bkgtemp, sizeof(int), 1, file);
      fread(&musictemp, sizeof(int), 1, file);
      fread(&lvlWidth, sizeof(int), 1, file);
      fread(&lvlHeight, sizeof(int), 1, file);
      fread(&startx, sizeof(int), 1, file);
      fread(&starty, sizeof(int), 1, file);
      fread(things, sizeof(Thing), THINGMAX, file);
      for (i=0;i<lvlWidth/SPR_SIZE;i++)
	for (j=0;j<lvlHeight/SPR_SIZE;j++)
	  for (k=0;k<3;k++)
	    fread(&tilebackup[i][j][k], sizeof(int), 1, file);
    } while ((lcode != lvlCode || acode != areaCode) &&
	     !feof(file) && lvlCode < 80);
    for (i=0;i<lvlWidth/SPR_SIZE;i++)
      for (j=0;j<lvlHeight/SPR_SIZE;j++)
	for (k=0;k<3;k++)
	  tiles[i][j][k] = tilebackup[i][j][k];
    if (bkgtemp < 1 || bkgtemp > BKG_MAX - 1) bkgtemp = 1;
    if (musictemp < 0 || musictemp > MUSIC_MAX - 1) musictemp = 0;
    clear_selection();
    switch_music(musictemp, 0);
    switch_background(bkgtemp);
    fclose(file);
  } else {
    loaderror = 1;
    if (lvlCode < 80 || creatormode) 
      set_up_input("The file was not found",1,-1,-1);
    else set_up_input("Make the level first!",1,-1,-1);
  }
}


int check_level_exists() {
  char filestr[512];
  if (lvlCode < 80) {
    sprintf(filestr, RESOURCE_PATH "rooms/rooms");
  } else {
    sprintf(filestr, "%srooms%sl%dr%d.room", support_path, DIRSEP, lvlCode, areaCode);
  }
  file = fopen(filestr, "rb");
  if (file) {
    fclose(file);
    return 1;
  }
  return 0;
}


void write_level() {
  char filestr[512];
  int k;
  sprintf(filestr, "%srooms%sl%dr%d.room", support_path, DIRSEP, lvlCode, areaCode);
  file = fopen(filestr, "wb");
  if (file) {
    hassaved = 1;
    fwrite(&curbkg, sizeof(int), 1, file);
    fwrite(&curmusic, sizeof(int), 1, file);
    fwrite(&lvlWidth, sizeof(int), 1, file);
    fwrite(&lvlHeight, sizeof(int), 1, file);
    fwrite(&startx, sizeof(int), 1, file);
    fwrite(&starty, sizeof(int), 1, file);
    fwrite(things, sizeof(Thing), THINGMAX, file);
    for (i=0;i<lvlWidth/SPR_SIZE;i++)
      for (j=0;j<lvlHeight/SPR_SIZE;j++)
	for (k=0;k<3;k++)
	  fwrite(&tiles[i][j][k], sizeof(int), 1, file);
    fclose(file);
  } else {
    set_up_input("The file was not opened",1,-1,-1);
  }
}


void write_metalevel() {
  char filestr[512];
  int oldroom = areaCode, oldbkg = curbkg, oldmusic = curmusic;
  int fragcount=0;
  int iter=0;

  for (iter=0; iter<ROOMMAX; iter++) {
    enemdead[iter] = 1;
    areaCode = iter;
    sprintf(filestr, "%srooms%sl%dr%d.room", support_path, DIRSEP, lvlCode, areaCode);
    file = fopen(filestr, "rb");
    if (file) {
      fread(&curbkg, sizeof(int), 1, file);
      fread(&curmusic, sizeof(int), 1, file);
      fread(&lvlWidth, sizeof(int), 1, file);
      fread(&lvlHeight, sizeof(int), 1, file);
      fread(&startx, sizeof(int), 1, file);
      fread(&starty, sizeof(int), 1, file);
      fread(things, sizeof(Thing), THINGMAX, file);
      for (i=0;i<lvlWidth/SPR_SIZE;i++)
        for (j=0;j<lvlHeight/SPR_SIZE;j++)
          fread(tiles[i][j], sizeof(int), 3, file);
      for (i=0; i<THINGMAX; i++) {
        if (things[i].type != NOTYPE && things[i].animate) {
          enemdead[iter] = 0;
        }
        if (things[i].type == MEDALFRAGMENT) {
          things[i].dir = fragcount++;
        }
      }
      fclose(file);
      write_level();
    }
  }

  sprintf(filestr, "%srooms%slvl%d.meta", support_path, DIRSEP, lvlCode);
  file = fopen(filestr, "wb");
  if (file) {
    fwrite(enemdead, sizeof(int), ROOMMAX, file);
    fwrite(&spminutes, sizeof(int), 1, file);
    fwrite(&spseconds, sizeof(int), 1, file);
    fclose(file);
  } else {
    set_up_input("The file was not opened",1,-1,-1);
  }

  if (fragcount != MFRAGTOTAL) {
    sprintf(filestr, "%d fragments found", fragcount);
    set_up_input(filestr,1,-1,-1);
  }

  areaCode = oldroom;
  curbkg = oldbkg;
  curmusic = oldmusic;
  read_level();
}


void set_start_pos() {
  int temp1 = mx, temp2 = my;
  if (crtgridsnap && crtgridsize > 0) {
    if (crtgridsize == 1) {
      temp1 = temp1-temp1%SPR_SIZE+SPR_SIZE/2;
      temp2 = temp2-temp2%SPR_SIZE+SPR_SIZE-beret.height/2;
    } else {
      temp1 = temp1-temp1%(SPR_SIZE/2)+SPR_SIZE/4;
      temp2 = temp2-temp2%(SPR_SIZE/2)+SPR_SIZE/2-beret.height/2;
    }
  }
  startx = temp1-beret.width/2;
  starty = temp2-beret.height/2;
}


void paste_helper(int leftmost, int topmost,
                  int copyindices[250], int index, int flag) {
  int copyindex;
  if (key3 != SHIFT) selection[index] = 0;
  if ((copyindex = copy_thing(&things[index], things, -1)) == -1) return;
  copyindices[index] = copyindex;
  things[copyindex].x = things[index].x-leftmost+mx;
  things[copyindex].y = things[index].y-topmost+my;
  if (things[copyindex].x > lvlWidth-things[copyindex].width)
    things[copyindex].x = lvlWidth-things[copyindex].width;
  if (things[copyindex].y > lvlHeight-things[copyindex].height)
    things[copyindex].y = lvlHeight-things[copyindex].height;
  things[copyindex].vx = 0.01;
  things[copyindex].vy = 0.01;
  if (flag && things[index].link > -1) {
    if (copyindices[things[index].link] > -1) {
      things[copyindex].link = copyindices[things[index].link];
      if (things[index].subtype % 2 == 0)
        things[copyindices[things[index].link]].islinked = copyindex;
      else things[copyindex].islinked = copyindices[things[index].link];
    }
  }
  selection[copyindex] = (copyindex<i?1:-1);
}


// copies every selected object and pastes them to the mouse position
void paste_objects() {
  int leftmost = lvlWidth, topmost = lvlHeight;
  int copyindices[250];
  for (i=0; i<THINGMAX; i++) {
    if (things[i].type != NOTYPE && selection[i]) {
      if (things[i].x < leftmost) leftmost = things[i].x;
      if (things[i].y < topmost) topmost = things[i].y;
    }
    copyindices[i] = -1;
  }
  for (j=0; j < 2; j++) {
    for (i=0; i<THINGMAX; i++) {
      if (things[i].type != NOTYPE && (things[i].type == LINKBLOCK) == j) {
        if (selection[i] == 1) {
          paste_helper(leftmost, topmost, copyindices, i, j);
        } else if (selection[i] == -1) selection[i] = 1;
      }
    }
  }
}


// creates an instance of the current object
void create_object() {
  if (key3 != SHIFT) clear_selection();
  int empty;
  if ((empty = find_empty(things)) == -1) return;
  make_thing(empty, crtplacetype, crtplacesubtype, mx, my, 
             get_param(crtplacetype, crtplacedir, -1), things);
  if (things[empty].x > lvlWidth-things[empty].width)
    things[empty].x = lvlWidth-things[empty].width;
  if (things[empty].y > lvlHeight-things[empty].height)
    things[empty].y = lvlHeight-things[empty].height;
  things[empty].vx = 0.01;
  things[empty].vy = 0.01;
  things[empty].islinked = -1;
  selection[empty] = 1;
  if (crtplacetype == DOOR || crtplacetype == SIGN) {
    things[empty].timer = crtentrance;
    things[empty].status = crtexit;
    things[empty].dir = crtexitroom;
  }
  if (crtplacetype == READSIGN) {
    things[empty].dir = crtmessage;
  }
  hassaved = 0;
}


// looks at adjacent tiles and makes the tile borders pretty
void fix_tile_borders(int xpos, int ypos, int recurse) {

  /* ABC
     H D
     GFE */

  int dirflags = 0;
  int b = BLANK;
  int type = tiles[xpos][ypos][0];
  int rside = lvlWidth/SPR_SIZE-1, bside = lvlHeight/SPR_SIZE-1;

  if (xpos == 0) dirflags |= (A_ | H_ | G_);
  if (ypos == 0) dirflags |= (A_ | B_ | C_);
  if (xpos == rside) dirflags |= (C_|D_|E_);
  if (ypos == bside) dirflags |= (E_|F_|G_);
  if (xpos > 0) {
    if (tiles[xpos-1][ypos][0] == type) dirflags |= H_;
    if (recurse) fix_tile_borders(xpos-1,ypos,0);
  }
  if (ypos > 0) {
    if (tiles[xpos][ypos-1][0] == type) dirflags |= B_;
    if (recurse) fix_tile_borders(xpos,ypos-1,0);
  }
  if (xpos < rside) {
    if (tiles[xpos+1][ypos][0] == type) dirflags |= D_;
    if (recurse) fix_tile_borders(xpos+1,ypos,0);
  }
  if (ypos < bside) {
    if (tiles[xpos][ypos+1][0] == type) dirflags |= F_;
    if (recurse) fix_tile_borders(xpos, ypos+1, 0);
  }
  if (xpos > 0 && ypos > 0) {
    if (tiles[xpos-1][ypos-1][0] == type) dirflags |= A_;
    if (recurse) fix_tile_borders(xpos-1,ypos-1,0);
  }
  if (xpos < rside) {
    if (ypos > 0 && tiles[xpos+1][ypos-1][0] == type) dirflags |= C_;
    if (recurse) fix_tile_borders(xpos+1,ypos-1,0);
  }
  if (xpos < rside && ypos < bside) {
    if (tiles[xpos+1][ypos+1][0] == type) dirflags |= E_;
    if (recurse) fix_tile_borders(xpos+1,ypos+1,0);
  }
  if (xpos > 0 && ypos < bside) {
    if (tiles[xpos-1][ypos+1][0] == type) dirflags |= G_;
    if (recurse) fix_tile_borders(xpos-1,ypos+1,0);
  }

  if (type == OBJONLY || type == BERETONLY || type == CHOICEONLY || 
      (type >= SPIKEU && type <= SPIKEL) || (type >= MOVERU && type <= MOVERL) || type == DARKNESSTILE) {
    tiles[xpos][ypos][2] = BLANK;
    return;
  }

  if (!(dirflags & (B_|D_|F_|H_))) b = ALL;
  else if (dirflags == 0xff) b = BLANK;
  else if (dirflags == (B_|D_|F_|H_)) b = ALLDOTS;
  else if (dirflags == (B_|C_|D_|E_|F_|G_|H_)) b = ULDOT;
  else if (dirflags == (A_|B_|D_|E_|F_|G_|H_)) b = URDOT;
  else if (dirflags == (A_|B_|C_|D_|E_|F_|H_)) b = DLDOT;
  else if (dirflags == (A_|B_|C_|D_|F_|G_|H_)) b = DRDOT;
  else if (dirflags == (B_|D_|E_|F_|G_|H_)) b = UDOTS;
  else if (dirflags == (A_|B_|D_|F_|G_|H_)) b = RDOTS;
  else if (dirflags == (A_|B_|C_|D_|F_|H_)) b = DDOTS;
  else if (dirflags == (B_|C_|D_|E_|F_|H_)) b = LDOTS;
  else if (dirflags == (A_|B_|D_|F_|H_)) b = NULDOT;
  else if (dirflags == (C_|B_|D_|F_|H_)) b = NURDOT;
  else if (dirflags == (E_|B_|D_|F_|H_)) b = NDRDOT;
  else if (dirflags == (G_|B_|D_|F_|H_)) b = NDLDOT;
  else if (dirflags == (B_|C_|D_|F_|G_|H_)) b = ULDRDOTS;
  else if (dirflags == (A_|B_|D_|E_|F_|H_)) b = URDLDOTS;
  else if ((dirflags | (A_|B_|C_)) == 0xff) b = U;
  else if ((dirflags | (C_|D_|E_)) == 0xff) b = R;
  else if ((dirflags | (E_|F_|G_)) == 0xff) b = D;
  else if ((dirflags | (G_|H_|A_)) == 0xff) b = L;
  else if ((dirflags | (A_|B_|C_|E_|G_)) == 0xff) {
    if (dirflags & E_) b = UWLDOT;
    else if (dirflags & G_) b = UWRDOT;
    else b = UW2DOTS;
  } else if ((dirflags | (C_|D_|E_|G_|A_)) == 0xff) {
    if (dirflags & A_) b = RWDDOT;
    else if (dirflags & G_) b = RWUDOT;
    else b = RW2DOTS;
  } else if ((dirflags | (E_|F_|G_|A_|C_)) == 0xff) {
    if (dirflags & A_) b = DWRDOT;
    else if (dirflags & C_) b = DWLDOT;
    else b = DW2DOTS;
  } else if ((dirflags | (G_|H_|A_|C_|E_)) == 0xff) {
    if (dirflags & C_) b = LWDDOT;
    else if (dirflags & E_) b = LWUDOT;
    else b = LW2DOTS;
  } 
  else if ((dirflags | (A_|H_|G_|C_|D_|E_)) == 0xff) b = LR;
  else if ((dirflags | (A_|B_|C_|G_|E_|F_)) == 0xff) b = UD;
  else if ((dirflags | (A_|B_|C_|D_|E_)) == 0xff) b = UR;
  else if ((dirflags | (C_|D_|E_|F_|G_)) == 0xff) b = DR;
  else if ((dirflags | (A_|H_|G_|F_|E_)) == 0xff) b = DL;
  else if ((dirflags | (A_|B_|C_|G_|H_)) == 0xff) b = UL;
  else if ((dirflags | (A_|B_|C_|D_|E_|G_)) == 0xff) b = URWDOT;
  else if ((dirflags | (C_|D_|E_|F_|G_|A_)) == 0xff) b = DRWDOT;
  else if ((dirflags | (A_|H_|G_|F_|E_|C_)) == 0xff) b = DLWDOT;
  else if ((dirflags | (A_|B_|C_|G_|H_|E_)) == 0xff) b = ULWDOT;
  else if ((dirflags | (A_|C_|D_|E_|F_|G_|H_)) == 0xff) b = NU;
  else if ((dirflags | (A_|B_|C_|E_|F_|G_|H_)) == 0xff) b = NR;
  else if ((dirflags | (A_|B_|C_|D_|E_|G_|H_)) == 0xff) b = ND;
  else if ((dirflags | (A_|B_|C_|D_|E_|F_|G_)) == 0xff) b = NL;

  tiles[xpos][ypos][2] = b;

  /* ABC
     H D
     GFE */
}


// returns 1 if given type is transparent but not an "only" wall
int see_through(int type) {
  return (type >= WHITEFUZZSEE && type <= GREENFUZZSEE) ||
    type == GLASS || (type >= PURPLETILESEE && type <= WHITETILESEE) ||
    type == CLEAR || type == REDFUZZSEE || type == BLACKTILESEE ||
    (type >= WHITESPRSEE && type <= PURPLESPRSEE) ||
    (type >= REDSEECRPT && type <= BLUESEECRPT) || type == ANTITILE ||
    (type >= SPIKEU && type <= SPIKEL);
}


// creates a grid of the current tile type
void create_tiles(int type) {
  int sell, selr, selt, selb;
  if (crtselect) {
    if (selx < mx) {
      sell = selx; selr = mx;
    } else {
      sell = mx; selr = selx;
    }
    if (sely < my) {
      selt = sely; selb = my;
    } else {
      selt = my; selb = sely;
    }
    sell /= SPR_SIZE;
    selr /= SPR_SIZE;
    selt /= SPR_SIZE;
    selb /= SPR_SIZE;
  } else {
    sell = mx/SPR_SIZE;
    selr = mx/SPR_SIZE;
    selt = my/SPR_SIZE;
    selb = my/SPR_SIZE;
  }
  for (i=sell;i<=selr;i++) {
    for (j=selt;j<=selb;j++) {
      tiles[i][j][0] = type;
      switch (type) {
      case OBJONLY : tiles[i][j][1] = OBJONLYF; break;
      case BERETONLY : tiles[i][j][1] = BERETONLYF; break;
      case MOVERU : case MOVERR : case MOVERD : case MOVERL :
      case CHOICEONLY : case DARKNESSTILE: tiles[i][j][1] = NOTSOLIDF; break;
      default : tiles[i][j][1] = SOLID;
      }
      if (see_through(type)) tiles[i][j][1] = SOLIDF;
      fix_tile_borders(i,j,1);
    }
  }
  hassaved = 0;
}


void load_state(int backup) {
  if (hassavestate) {
    int trash; // dump unneeded values
    int oldcamx = camx, oldcamy = camy;
    char filestr[512];
    char bkpstr[5];
    if (backup && hasbkpsavestate) sprintf(bkpstr, "bkp");
    else bkpstr[0] = 0;
    sprintf(filestr, "%ssaves%sgame%d.sav%s", support_path, DIRSEP, gameNum, bkpstr);
    file = fopen(filestr, "rb");
    if (file) {
      if (backup && hasbkpsavestate) loadedbkpsavestate = 1;
      else loadedbkpsavestate = 0;
      fread(&beret, sizeof(Thing), 1, file);
      beret.subtype = istophat;
      fread(&camx, sizeof(int),1,file);
      fread(&camy, sizeof(int),1,file);
      fread(things, sizeof(Thing), THINGMAX, file);
      for (i=0;i<lvlWidth/SPR_SIZE;i++)
        for (j=0;j<lvlHeight/SPR_SIZE;j++)
          fread(tiles[i][j], sizeof(int), 3, file);
      fread(particles, sizeof(Particle), PARTICLEMAX, file);
      fread(&mfragcount, sizeof(int), 1, file);
      fread(gotmfrag, sizeof(int), MFRAGTOTAL, file);
      fread(gotmcrn, sizeof(int), 4, file);
      fread(&gotmwht, sizeof(int), 1, file);
      fread(&gotgmed, sizeof(int), 1, file);
      fread(&gotpmed, sizeof(int), 1, file);
      fread(&enemdeadhere, sizeof(int), 1, file);
      fread(&enemalldead, sizeof(int), 1, file);
      fread(&spframes, sizeof(int), 1, file);
      fread(&spseconds, sizeof(int), 1, file);
      fread(&spminutes, sizeof(int), 1, file);
      fread(&telething, sizeof(int), 1, file);
      fread(&switchflags, sizeof(int), 1, file);
      if (!mousecammode) {
        fread(&mx, sizeof(int), 1, file);
        fread(&my, sizeof(int), 1, file);
      } else {
        fread(&trash, sizeof(int), 1, file);
        fread(&trash, sizeof(int), 1, file);
      }
      if (!fread(&gotallfrags, sizeof(int), 1, file))
        gotallfrags = 0;
      fread(get_bossvars(), sizeof(int), BOSSMAX, file);
      fread(&gravdir, sizeof(int), 1, file);
      int jumpgrace, runmargin;
      if (!fread(&jumpgrace, sizeof(int), 1, file)) jumpgrace = 0;
      if (!fread(&runmargin, sizeof(int), 1, file)) runmargin = 0;
      set_jumpgrace(jumpgrace);
      set_runmargin(runmargin);

      if (!mousecammode) SDL_WarpMouse(mx-camx, my-camy);
      else {
	mx -= oldcamx - camx;
	my -= oldcamy - camy;
      }

      fix_camera();
      deathtime = 0;
      fadetime = 0;
      walkaway = 0;

      if (telething > -1 && 
          !(alwaystele && teleon) && !(!alwaystele && cantele))
        stop_telekinesis();
      
      usedsavestate = 1;
      svstcount = -30;
      
      fclose(file);
    } else {
      set_up_input("The file was not found",1,-1,-1);
    }
  }
}


void save_map(int num) {
  char filestr[512];
  sprintf(filestr, "%srooms%sbr%d.mp", support_path, DIRSEP, num);
  file = fopen(filestr, "wb");
  if (file) {
    for (i=0;i<SCR_WIDTH/SPR_SIZE;i++)
      for (j=0;j<(SCR_HEIGHT-150)/SPR_SIZE;j++)
        fwrite(tiles[i][j], sizeof(int), 3, file);
    fclose(file);
  } else {
    set_up_input("The file was not opened",1,-1,-1);
  }
}


void load_map(int num) {
  char filestr[512];
  char mcode;
  //  sprintf(filestr, "rooms%sbr%d.mp", DIRSEP, num);
  sprintf(filestr, RESOURCE_PATH "rooms/maps");
  file = fopen(filestr, "rb");
  if (file) {
    do {
      fread(&mcode, sizeof(char), 1, file);
      for (i=0;i<SCR_WIDTH/SPR_SIZE;i++)
	for (j=0;j<(SCR_HEIGHT-150)/SPR_SIZE;j++)
	  fread(tiles[i][j], sizeof(int), 3, file);
    } while (mcode != num && !feof(file));
    fclose(file);
  } else {
    set_up_input("The file was not found",1,-1,-1);
  }

  sprintf(filestr, "%ssaves%sgame%d.ret", support_path, DIRSEP, gameNum);
  file = fopen(filestr, "rb");
  if (file) {
    fread(&contlvl, sizeof(int), 1, file);
    fclose(file);
    if (contlvl / 20 == mapCode) cancont = 1;
    else cancont = 0;
  } else {
    cancont = 0;
  }
}


void start_map() {
  int minabovegm = UNF;
  nextopenlevel = -1;
  for (i=0; i<20; i++)
    if (levelentry[mapCode][i] < minabovegm && 
        levelentry[mapCode][i] > gamemedals) {
      minabovegm = levelentry[mapCode][i];
      nextopenlevel = i;
    }
  switch_music(1, 0);
  mapselect = -1;
  int curcamx = camx, curcamy = camy;
  camx = 0;
  camy = 0;
  mx += camx-curcamx;
  my += camy-curcamy;
  load_map(mapCode);
}


void check_map_select() {
  if (mx >= 0 && my >= 0) {
    mapselect = tiles[mx/SPR_SIZE][my/SPR_SIZE][0] - MAPTILE;
    if (mapselect + MAPTILE == CLEAR) {
      if (cancont) mapselect = contlvl - mapCode*20;
      else mapselect = -1;
    } else if (mapselect < 0 || 
	       (levelentry[mapCode][mapselect] > gamemedals && mapselect != nextopenlevel && mapCode < 4))
      mapselect = -1;
  }
}


void draw_map_status() {
  display_message(74,15,smfont,"S TO VIEW THE STORY",1);
  display_message(74,30,smfont,"C TO VIEW THE CONTROLS",1);
  display_message(74,45,smfont,"A TO VIEW THE CREDITS",1);
  display_message(SCR_WIDTH/4,500,font,"Evil Corporation",1);
  display_message(SCR_WIDTH/4,530,medfont,wingnames[mapCode],1);
  sprintf(messagestr, "%d", gamemedals);
  display_message(SCR_WIDTH/4+30, 565, font, 0,1);
  apply_sprite(SCR_WIDTH/4-message->w/2-45,565-message->h/2,5,15,
               1,1,spritesheet,screen);
  if (mapselect > -1) {
    apply_surface((SCR_WIDTH-lvlnumbkg->w)/2, 400, lvlnumbkg, screen);
    sprintf(messagestr, "%d", levelnums[mapCode][mapselect]);
    display_message(SCR_WIDTH/2+numofst[levelnums[mapCode][mapselect]-1],422,font,0,0);
    sprintf(messagestr, "Department of %s", deptnames[mapCode][mapselect]);
    int deptnameofst = (mapCode==3||(mapCode==4 && mapselect==10))?30:(mapCode==4?15:0);
    display_message(3*SCR_WIDTH/4,500+deptnameofst,font,0,1);
    if (strcmp(divnames[mapCode][mapselect], "-")) {
      sprintf(messagestr, "%s Division", divnames[mapCode][mapselect]); 
      display_message(3*SCR_WIDTH/4,530+deptnameofst,medfont,0,1);
    }
    if (levelentry[mapCode][mapselect] > gamemedals) {
      sprintf(messagestr, "%d Medallions required for entry", levelentry[mapCode][mapselect]);
      display_message(3*SCR_WIDTH/4,565,medfont,0,1);
    } else {
      if (mapCode < 3) {
	for (i=0; i<6; i++) {
	  if (gotmedals[(mapCode*20+mapselect)*6+i])
	    apply_sprite(3*SCR_WIDTH/4-165+i*60, 547, medalsprx[i],
			 medalspry[i], 1, 1, spritesheet, screen);
	  else
	    apply_sprite(3*SCR_WIDTH/4-165+i*60, 547, 17, 17,
			 1, 1, spritesheet, screen);
	}
      }
      display_message(3*SCR_WIDTH/4,590,smfont,"CLICK TO ENTER",1);
    }
  }
  if (cancont && contlvl > -1) {
    display_message(3*SCR_WIDTH/4, 45,smfont,"CONTINUE LAST PLAY",0);
  }
  if ((mapCode > 0 || (mapCode == 0 && (opencreator || beatlevel[LAST_LEVEL]))) &&
      !(mapCode == 4 && !beatlevel[LAST_LEVEL])) {
    if (mx >= 30 && mx <= 120 && my >= 195 && my <= 285)
      apply_sprite(30, 195, 3, 10, 3, 3, tilesheet, screen);
    else apply_sprite(30, 195, 3, 7, 3, 3, tilesheet, screen);
    display_message(75, 240, smfont, "PREVIOUS MAP",0);
  }
  if ((mapCode < 4 && beatlevel[maplastlevel[mapCode]]) || mapCode == 4) {
    if (mx >= SCR_WIDTH-120 && mx <= SCR_WIDTH-30 && my >= 195 && my <= 285)
      apply_sprite(SCR_WIDTH-120, 195, 0, 10, 3, 3, tilesheet, screen);
    else apply_sprite(SCR_WIDTH-120, 195, 0, 7, 3, 3, tilesheet, screen);
    display_message(SCR_WIDTH-75, 240, smfont, "NEXT MAP",0);
  }
}


void draw_creat(int page) {
  apply_surface(0,0,background,screen);
  display_message(SCR_WIDTH/2, 30,font,"Creator Guide",1);
  for (i=0;i<(page==0?CREAT1LEN:(page==1?CREAT2LEN:CREAT3LEN));i++)
    display_message(SCR_WIDTH/2, 60+25*i, medfont, 
		    (page==0?creat1[i]:(page==1?creat2[i]:creat3[i])),1);
}

void draw_story() {
  apply_surface(0,0,background,screen);
  display_message(SCR_WIDTH/2, 30,font,"Story",1);
  for (i=0;i<STORYLEN;i++)
    display_message(SCR_WIDTH/2, 60+25*i, medfont, story[i],1);
}

void draw_controls() {
  apply_surface(0,0,background,screen);
  display_message(SCR_WIDTH/2, 45,font,"Game Controls",1);
  for (i=0;i<CONTROLLEN;i++)
    display_message(SCR_WIDTH/2, 100+25*i, medfont, controls[i],1);
}

void draw_map() {
  int maptype, addto;
  apply_surface(0,0,mapbkg,screen);
  for (i = 0; i < SCR_WIDTH/SPR_SIZE; i++) {
    for (j = 0; j < 15; j++) {
      if (tiles[i][j][0] == CLEAR) {
        if (cancont) {
          apply_sprite(i*SPR_SIZE, j*SPR_SIZE,
                       (CLEAR-1)%24,(CLEAR-1)/24,
                       1,1,tilesheet,screen);
          apply_sprite(i*SPR_SIZE, j*SPR_SIZE,
                       tiles[i][j][2]%24,22+tiles[i][j][2]/24,
                       1,1,tilesheet,screen);
        }
      } else if (tiles[i][j][0] == GLASS) {
        apply_sprite(i*SPR_SIZE, j*SPR_SIZE,
                     (GLASS-1)%24,(GLASS-1)/24,
                     1,1,tilesheet,screen);
        apply_sprite(i*SPR_SIZE, j*SPR_SIZE,
                     tiles[i][j][2]%24,22+tiles[i][j][2]/24,
                     1,1,tilesheet,screen);
      } else if (tiles[i][j][0] > 0) {
        maptype = tiles[i][j][0] - MAPTILE;
        if (levelentry[mapCode][maptype] <= gamemedals || 
	    maptype == nextopenlevel || mapCode == 4) {
          addto = 0;
          if (mapselect == maptype) addto++;
          if (levelentry[mapCode][maptype] > gamemedals && !(mapCode == 4 && opencreator)) addto+=2;
          apply_sprite(i*SPR_SIZE, j*SPR_SIZE,
                       (tiles[i][j][0]-1)%24,(tiles[i][j][0]-1)/24+addto,
                       1,1,tilesheet,screen);
          if (addto < 2) 
            apply_sprite(i*SPR_SIZE, j*SPR_SIZE,
                         tiles[i][j][2]%24,22+tiles[i][j][2]/24,
                         1,1,tilesheet,screen);
        }
      }
    }
  }
}

// Copies the main savestate file to a backup file.
int copy_save_state(char *sf1, char *sf2) {
  char buf[4096];
  int cread;
  int successcopy = 0;
  FILE *sf1h, *sf2h;
  sf1h = fopen(sf1, "rb");
  sf2h = fopen(sf2, "wb");
  if (sf1h && sf2h) {
    while (!feof(sf1h)) {
      cread = fread(buf, sizeof(char), 4096, sf1h);
      fwrite(buf, sizeof(char), cread, sf2h);
    }
    successcopy = 1;
  }
  if (sf1h) fclose(sf1h);
  if (sf2h) fclose(sf2h);
  return successcopy;
}


void save_state() {
  if (!beret.dead && !fadetime) {
    char filestr[512];
    sprintf(filestr, "%ssaves%sgame%d.sav", support_path, DIRSEP, gameNum);
    if (hassavestate && !loadedbkpsavestate) {
      char filestr2[512];
      sprintf(filestr2, "%sbkp", filestr);
      hasbkpsavestate = copy_save_state(filestr, filestr2);
    }
    loadedbkpsavestate = 0;
    file = fopen(filestr, "wb");
    if (file) {
      fwrite(&beret, sizeof(Thing), 1, file);
      fwrite(&camx, sizeof(int),1,file);
      fwrite(&camy, sizeof(int),1,file);
      fwrite(things, sizeof(Thing), THINGMAX, file);
      for (i=0;i<lvlWidth/SPR_SIZE;i++)
        for (j=0;j<lvlHeight/SPR_SIZE;j++)
          fwrite(tiles[i][j], sizeof(int), 3, file);
      fwrite(particles, sizeof(Particle), PARTICLEMAX, file);
      fwrite(&mfragcount, sizeof(int), 1, file);
      fwrite(gotmfrag, sizeof(int), MFRAGTOTAL, file);
      fwrite(gotmcrn, sizeof(int), 4, file);
      fwrite(&gotmwht, sizeof(int), 1, file);
      fwrite(&gotgmed, sizeof(int), 1, file);
      fwrite(&gotpmed, sizeof(int), 1, file);
      fwrite(&enemdeadhere, sizeof(int), 1, file);
      fwrite(&enemalldead, sizeof(int), 1, file);
      fwrite(&spframes, sizeof(int), 1, file);
      fwrite(&spseconds, sizeof(int), 1, file);
      fwrite(&spminutes, sizeof(int), 1, file);
      fwrite(&telething, sizeof(int), 1, file);
      fwrite(&switchflags, sizeof(int), 1, file);
      fwrite(&mx, sizeof(int), 1, file);
      fwrite(&my, sizeof(int), 1, file);
      fwrite(&gotallfrags, sizeof(int), 1, file);
      fwrite(get_bossvars(), sizeof(int), BOSSMAX, file);
      fwrite(&gravdir, sizeof(int), 1, file);
      int jumpgrace = get_jumpgrace(), runmargin = get_runmargin();
      fwrite(&jumpgrace, sizeof(int), 1, file);
      fwrite(&runmargin, sizeof(int), 1, file);
      
      hassavestate = 1;
      svstcount = 30;
      fclose(file);
      
      save_room_return(trgentrance == -1 ? 0 : 1);
    } else {
      set_up_input("The file was not opened",1,-1,-1);
    }
  }
}


void load_options() {
  char filestr[512];
  sprintf(filestr, "%ssaves%sgame%d.opt", support_path, DIRSEP, gameNum);
  file = fopen(filestr, "rb");
  if (file) {
    if (!fread(&mousecammode, sizeof(int), 1, file))
      mousecammode = 1;
    if (!fread(&runningmode, sizeof(int), 1, file))
      runningmode = 1;
    if (!fread(&deathstateload, sizeof(int), 1, file))
      deathstateload = 0;
    if (!fread(&opencreator, sizeof(int), 1, file))
      opencreator = 0;
    if (!fread(&musicon, sizeof(int), 1, file))
      musicon = 1;
    if (!fread(&guides, sizeof(int), 1, file))
      guides = 0;
    if (!fread(&istophat, sizeof(int), 1, file))
      istophat = 0;
    fclose(file);
  } else {
    mousecammode = 1;
    runningmode = 1;
    deathstateload = 0;
    opencreator = 0;
    musicon = 1;
  }
  if (ingame != 1 && (musicon == 0 || musicon == 3)) {
    Mix_HaltMusic();
  }
}


void load_game_data() {
  load_options();
  char filestr[512];
  sprintf(filestr, "%ssaves%sgame%d.gm", support_path, DIRSEP, gameNum);
  file = fopen(filestr, "rb");
  if (file) {
    fread(gamename, sizeof(char), 25, file);
    fread(gotmedals, sizeof(int), MEDALMAX, file);
    fread(&mapCode, sizeof(int), 1, file);
    fread(&initgamemsg, sizeof(int), 1, file);
    if (!fread(beatlevel, sizeof(int), 100, file)) {
      for (i=0; i<100; i++) beatlevel[i] = 0;
    }
    for (i = 0; i < 100; i++) {
      if (!fread(game_gotfrags[i], sizeof(char), 100, file)) {
	for (j = 0; j < 100; j++) {
	  if (gotmedals[i*6+2]) game_gotfrags[i][j] = 1;
	  else game_gotfrags[i][j] = 0;
	}
      }
      if (!fread(game_gotcorners[i], sizeof(char), 4, file)) {
	for (j = 0; j < 4; j++) {
	  if (gotmedals[i*6+1]) game_gotcorners[i][j] = 1;
	  else game_gotcorners[i][j] = 0;
	}
      }
    }
    for (i = 0; i < 100; i++) {
      if (!fread(game_enemdead[i], sizeof(char), ROOMMAX, file)) {
	for (j = 0; j < ROOMMAX; j++) {
	  if (gotmedals[i*6+3]) game_enemdead[i][j] = 1;
	  else game_enemdead[i][j] = 0;
	}
      }
    }
    fclose(file);
    gamemedals = 0;
    for (i=0; i<MEDALMAX; i++) 
      if (gotmedals[i]) {
        gamemedals++;
      }
    initmapmsg = 1;
  } else {
    strcpy(gamename, "---New---");
    for (i=0; i<MEDALMAX; i++) gotmedals[i] = 0;
    for (i=0; i<100; i++) beatlevel[i] = 0;
    for (i=0; i<100; i++) {
      for (j=0; j<100; j++) game_gotfrags[i][j] = 0;
      for (j=0; j<4; j++) game_gotcorners[i][j] = 0;
      for (j=0; j<ROOMMAX; j++) game_enemdead[i][j] = 0;
    }
    gamemedals = 0;
    mapCode = 0;
    initmapmsg = 0;
    initgamemsg = 0;
  }
}


void save_options() {
  char filestr[512];
  sprintf(filestr, "%ssaves%sgame%d.opt", support_path, DIRSEP, gameNum);
  file = fopen(filestr, "wb");
  if (file) {
    fwrite(&mousecammode, sizeof(int), 1, file);
    fwrite(&runningmode, sizeof(int), 1, file);
    fwrite(&deathstateload, sizeof(int), 1, file);
    fwrite(&opencreator, sizeof(int), 1, file);
    fwrite(&musicon, sizeof(int), 1, file);
    fwrite(&guides, sizeof(int), 1, file);
    fwrite(&istophat, sizeof(int), 1, file);
    fclose(file);
  } else {
    set_up_input("The file was not opened",1,-1,-1);
  }
}


void save_game_data() {
  char filestr[512];
  sprintf(filestr, "%ssaves%sgame%d.gm", support_path, DIRSEP, gameNum);
  file = fopen(filestr, "wb");
  if (file) {
    fwrite(gamename, sizeof(char), 25, file);
    fwrite(gotmedals, sizeof(int), MEDALMAX, file);
    fwrite(&mapCode, sizeof(int), 1, file);
    fwrite(&initgamemsg, sizeof(int), 1, file);
    fwrite(beatlevel, sizeof(int), 100, file);
    for (i = 0; i < 100; i++) {
      fwrite(game_gotfrags[i], sizeof(char), 100, file);
      fwrite(game_gotcorners[i], sizeof(char), 4, file);
    }
    for (i = 0; i < 100; i++) {
      fwrite(game_enemdead[i], sizeof(char), ROOMMAX, file);
    }
    fclose(file);
  } else {
    set_up_input("The file was not opened",1,-1,-1);
  }
}


void start_new_game() {
  char filestr[512];
  sprintf(filestr, "%ssaves%sgame%d.ret", support_path, DIRSEP, gameNum);
  remove(filestr);
  strcpy(gamename, inputstr);
  save_game_data();
}


void delete_game() {
  char filestr[512];
  sprintf(filestr, "%ssaves%sgame%d.gm", support_path, DIRSEP, gameNum);
  remove(filestr);
  sprintf(filestr, "%ssaves%sgame%d.ret", support_path, DIRSEP, gameNum);
  remove(filestr);
  sprintf(filestr, "%ssaves%sgame%d.sav", support_path, DIRSEP, gameNum);
  remove(filestr);
  sprintf(filestr, "%ssaves%sgame%d.savbkp", support_path, DIRSEP, gameNum);
  remove(filestr);
  sprintf(filestr, "%ssaves%sgame%d.opt", support_path, DIRSEP, gameNum);
  remove(filestr);
  load_game_data();
}


void init_leave_select(int getinputset) {
  getinput = getinputset;
  inputlength = -3;
  optselect = 0;
  optmax = 4;
  paused = 0;
}


void init_opt_select() {
  getinput = 15;
  inputlength = -3;
  optselect = 0;
  optmax = 5;
}


void init_sign(int mgnum) {
  msgcode = mgnum;
  getinput = 3;
  inputlength = -1;
}


void use_door() {
  Thing door = things[beret.timer];

  if (door.type == FINISHDOOR) {
    if (spminutes > 0 || spseconds > 0 || spframes > 0) {
      gotgmed = 1;
      spminutes = 0;
      spseconds = 0;
      spframes = 0;
      play_sound(SND_MEDG);
      if (statusbar) {
        make_expl(GMEDPOS+15+camx,30+camy,0,0,GREEN,6,100);
        make_expl(GMEDPOS+15+camx,30+camy,0,0,GRYELLOW,6,40);
      }
    }
    if (!gotpmed && lvlCode != LAST_LEVEL) {
      gotpmed = 1;
      play_sound(SND_MEDP);
      if (statusbar) {
        make_expl(PMEDPOS+15+camx,30+camy,0,0,PURPLE,6,120);
        make_expl(PMEDPOS+15+camx,30+camy,0,0,RED,6,20);
      }
    }
  }
  if (door.type == READSIGN && key2 == DOWN && beret.vx == 0) {
    init_sign(door.dir);
  } else if (door.type == SIGN) {
    if (door.subtype%2 == 0 && beret.x <= 0 && !walkaway) { 
      if (ingame == -1) init_fade(1);
      else go_to_room(door.status, door.dir);
      walkaway = 1;
    }
    if (door.subtype%2 == 1 && beret.x+beret.width>=lvlWidth && !walkaway) {
      if (ingame == -1) init_fade(1);
      else go_to_room(door.status, door.dir);
      walkaway = 2;
    }
  } else if ((door.type == FINISHDOOR || door.type == DOOR) &&
      key2 == DOWN && beret.vx == 0 && fadetime == 0) {
    if (ingame == -1) {
      init_fade(1);
    } else {
      if (door.type == FINISHDOOR) {
        set_up_input("Exit this level?",-14,-2,-1);
      } else if (door.type == DOOR) {
        go_to_room(door.status, door.dir);
      }
    }
  }
}


void begin_game() {
  if (strcmp(gamename, "---New---") == 0) 
    set_up_input("Enter your name:",2,13,-1);
  else {
    char filestr[512];
    sprintf(filestr, "%ssaves%sgame%d.ret", support_path, DIRSEP, gameNum);
    file = fopen(filestr, "rb");
    if (file) {
      cancont = 1;
      initgamemsg = 1;
      fread(&contlvl, sizeof(int), 1, file);
      fclose(file);
      set_up_input("Continue last play?",-15,-2,-1);
      yesno = 1;
    } else {
      cancont = 0;
      init_fade(2);
    }
  }
}


void handle_key_down(SDLKey key) {
  SDLMod mod = SDL_GetModState();

  if ((key == QUITKEY_WIN && (mod & QUITMOD_WIN)) ||
      (key == QUITKEY_LIN && (mod & QUITMOD_LIN)) ||
      (key == QUITKEY_MAC && (mod & QUITMOD_MAC))) {
    save_options();
    quit = 1;
  }
  
  if (ingame == 0) {
    if (key == SDLK_ESCAPE) {
      quit = 1;
      return;
    }
    ingame = 1;
    load_game_data();
    return;
  } else if (key == SDLK_SLASH) {
    musicon = !musicon;
    if (musicon) switch_music(curmusic, 1);
    else Mix_HaltMusic();
    if (ingame != 0 && ingame != 1) save_options();
  } else if ((mod & KMOD_ALT) && key == SDLK_RETURN) {
    fullscreenmode = !fullscreenmode;
    if (fullscreenmode) {
      screen = SDL_SetVideoMode(SCR_WIDTH, SCR_HEIGHT,
				SCR_BPP, SDL_FULLSCREEN);
    } else {
      screen = SDL_SetVideoMode(SCR_WIDTH, SCR_HEIGHT,
				SCR_BPP, SDL_SWSURFACE);
    }
  }

  if (ingame == 4 || ingame == 5) {
    ingame = (ingame == 4 ? ingamereturn : 2);
    return;
  }

  if (ingame == 10 || ingame == 11) {
    ingame++;
    return;
  }

  if (ingame == 12) {
    ingame = -1;
    return;
  }

  if (ingame == 13 && key == SDLK_ESCAPE) {
    init_fade(4);
    return;
  }

  if (ingame == 2 && !getinput) {
    if (key == SDLK_e) {
      if (secretcode % 4 == 0) secretcode++;
    } else if (key == SDLK_v) {
      if (secretcode % 4 == 1) secretcode++;
    } else if (key == SDLK_i) {
      if (secretcode % 4 == 2) secretcode++;
    } else if (key == SDLK_l) {
      if (secretcode % 4 == 3) secretcode++;
      if (secretcode == 12) {
	opencreator = 1;
	play_sound(SND_SWITCHGV);
	save_options();
      }
    } else secretcode = 0;
    if (key == SDLK_ESCAPE) init_leave_select(13);
    else if (key == SDLK_c) {
      ingamereturn = ingame;
      ingame = 4;
      switch_background(2);
    } else if (key == SDLK_s) {
      ingame = 5;
      switch_background(2);
    } else if (key == SDLK_a) {
      init_fade(9);
    }
    return;
  }

  if (ingame == 1 && !getinput) {
    if ((key == SDLK_RIGHT || key == SDLK_d) && gameNum < GAMEMAX) {
      gameNum++;
      load_game_data();
    }
    if ((key == SDLK_LEFT || key == SDLK_a) && gameNum > 1) {
      gameNum--;
      load_game_data();
    }
    if (key == SDLK_RETURN || key == SDLK_SPACE || key == SDLK_LSHIFT ||
        key == SDLK_RSHIFT) {
      begin_game();
      return;
    }
    if (key == SDLK_ESCAPE) {
      ingame = 0;
      return;
    }
    if ((key == SDLK_DELETE || key == SDLK_BACKSPACE) &&
        strcmp(gamename, "---New---")) {
      set_up_input("Delete this game?",-10,-2,-1);
    }
  }

  if (getinput) {
    if (key == SDLK_RETURN || key == SDLK_ESCAPE ||
        ((getinput == 1 || getinput == 3) && 
         (key == SDLK_w || key == SDLK_s || key == SDLK_a || key == SDLK_d ||
         key == SDLK_UP || key == SDLK_DOWN || key == SDLK_LEFT || key == SDLK_RIGHT))) {
      // take care of different reasons for getting input
      resolve_input(key);
    } else if (inputpos < inputlength && ((key >= SDLK_0 && key <= SDLK_9) ||
               (getinput > 0 && key >= SDLK_a && key <= SDLK_z))) {
      if (!(mod & KMOD_SHIFT) || (key >= SDLK_0 && key <= SDLK_9))
        inputstr[inputpos++] = key;
      else inputstr[inputpos++] = key - 'a' + 'A';
      inputstr[inputpos] = '\0';
    } else if (inputlength != -1 && inputpos > 0 && key == SDLK_BACKSPACE) {
      inputstr[--inputpos] = '\0';
      if (inputpos == 0) sprintf(inputstr, "-");
    } else if (inputlength == -2) {
      if (key == SDLK_a || key == SDLK_LEFT) yesno = 0;
      if (key == SDLK_d || key == SDLK_RIGHT) yesno = 1;
    } else if (inputlength == -3) {
      if (key == SDLK_s || key ==SDLK_DOWN) optselect++;
      if (key == SDLK_w || key ==SDLK_UP) optselect--;
      if (optselect >= optmax) optselect = optmax-1;
      if (optselect < 0) optselect = 0;
      if (getinput == 15) {
	if (key == SDLK_a || key == SDLK_LEFT || 
	    key == SDLK_d || key == SDLK_RIGHT) {
	  if (optselect == 0) {
	    if (key == SDLK_a || key == SDLK_LEFT) musicon--;
	    else musicon++;
	    if (musicon == 4) musicon = 0;
	    else if (musicon == -1) musicon = 3;
	    if (musicon == 1 || musicon == 2) switch_music(curmusic, 1);
	    else Mix_HaltMusic();
	  }
	  if (optselect == 1) mousecammode = !mousecammode;
	  if (optselect == 2) runningmode = !runningmode;
	  if (optselect == 3) deathstateload = !deathstateload;
	  if (optselect == 4) statusbar = !statusbar;
	  save_options();
	}
      }
    }
  } else {
    if (key == SDLK_LSHIFT || key == SDLK_RSHIFT) {
      key3 = SHIFT;
    } else if (key == SDLK_ESCAPE && ingame < 0) {
      if (creatormode) init_leave_select(16);
      else init_leave_select(17);
    } 
    
    if (creatormode) {
      // creator keys
      if (key == SDLK_g) {
        crtgridsize++;
        crtgridsize %= 3;
      } else if (key == SDLK_c) {
        switchflags ^= NOCOLLIDEFLAG;
      } else if (key == SDLK_q) {
	ingame = 10;
      } else if (key == SDLK_h) {
        crtgridsnap = !crtgridsnap;
      } else if (key == SDLK_l) {
        crtlinking = !crtlinking;
        hassaved = 0;
      } else if (key == SDLK_p) {
        set_start_pos();
      } else if (key == SDLK_EQUALS) {
        if (curbkg < BKG_MAX-1) switch_background(curbkg+1);
        if (curbkg == 0) switch_background(1);
        hassaved = 0;
      } else if (key == SDLK_MINUS) {
        if (curbkg > 1) switch_background(curbkg-1);
        else switch_background(-1);
        hassaved = 0;
      } else if (key == SDLK_RIGHTBRACKET) {
        if (curmusic < MUSIC_MAX-1) switch_music(curmusic+1, 0);
        hassaved = 0;
      } else if (key == SDLK_LEFTBRACKET) {
        if (curmusic > 0) switch_music(curmusic-1, 0);
        hassaved = 0;
      } else if (key == SDLK_i) {
        crtinventory = !crtinventory;
        crtlinking = 0;
        crtselect = 0;
      } else if (key == SDLK_COMMA) {
        crtplacedir = 0;
      } else if (key == SDLK_PERIOD) {
        crtplacedir = 1;
      } else if (key == SDLK_TAB) {
        crtplacesubtype++;
        crtplacesubtype %= subtype_count(crtplacetype);
      } else if (key == SDLK_o) {
        create_object();
      } else if (key == SDLK_u) {
        create_tiles(crtplacetile);
      } else if (key == SDLK_y) {
        create_tiles(EMPTY);
      } else if (key == SDLK_e) {
        set_up_input("Doors exit to room:", -1, 4, -1);
       } else if (key == SDLK_m) { 
         set_up_input("Set sign message:", -13, 4, -1); 
      } else if (key == SDLK_r) {
	set_up_input("Really clear the room?",-20,-2,-1);
	yesno = 0;
      } else if (key == SDLK_F1) {
        set_up_input("Save room in division:", -2, 2, lvlCode - 79);
      } else if (key == SDLK_F4) {
        set_up_input("Load room in division:", -3, 2, lvlCode - 79);
/*       } else if (key == SDLK_F5) { */
/*         set_up_input("Save map number:", -6, 4, -1); */
/*       } else if (key == SDLK_F8) { */
/*         set_up_input("Load map number:", -7, 4, -1); */
      } else if (key == SDLK_F9) {
        if (hassaved) set_up_input("Set timer minutes:", -8, 2, -1);
        else set_up_input("First save the room", 1, -1, -1);
      } else if (key == SDLK_F12) {
        if (hassaved) {
          spminutes = 0;
          spseconds = 0;
          write_metalevel();
        } else set_up_input("First save the room", 1, -1, -1);
      } else if (key >= SDLK_0 && key <= SDLK_9) {
        if (key3 == SHIFT) crtexit = key - '0';
        else crtentrance = key - '0';
      } else if (key == SDLK_END) {
        if (lvlWidth < 19*SCR_WIDTH) {
          lvlWidth += SCR_WIDTH;
          for (i=0; i< lvlHeight/SPR_SIZE; i++)
            fix_tile_borders((lvlWidth-SCR_WIDTH)/SPR_SIZE-1,i,0);
        }
        hassaved = 0;
      } else if (key == SDLK_HOME) {
        if (lvlWidth > SCR_WIDTH) {
          lvlWidth -= SCR_WIDTH;
          for (i=0; i< lvlHeight/SPR_SIZE; i++)
            fix_tile_borders(lvlWidth/SPR_SIZE-1,i,0);
        }
        fix_camera();
        hassaved = 0;
      } else if (key == SDLK_PAGEDOWN) {
        if (lvlHeight < 25*SCR_HEIGHT) {
          lvlHeight += SCR_HEIGHT;
          for (i=0; i< lvlWidth/SPR_SIZE; i++)
            fix_tile_borders(i,(lvlHeight-SCR_HEIGHT)/SPR_SIZE-1,0);
        }
        hassaved = 0;
      } else if (key == SDLK_PAGEUP) {
        if (lvlHeight > SCR_HEIGHT) {
          lvlHeight -= SCR_HEIGHT;
          for (i=0; i< lvlWidth/SPR_SIZE; i++)
            fix_tile_borders(i,lvlHeight/SPR_SIZE-1,0);
        }
        fix_camera();
        hassaved = 0;
      } else if (key == SDLK_v) {
        paste_objects();
        hassaved = 0;
      } else if (key == SDLK_DELETE || key == SDLK_BACKSPACE) {
        for (i=0; i<THINGMAX; i++) {
          if (selection[i] || telething == i) {
            destroy_thing(&things[i], PIT, things, i);
            things[i].type = NOTYPE;
            selection[i]=0;
          }
        }
        hassaved = 0;
      }
    } else if (ingame == 3 || ingame == -1) {
      if (key == SDLK_F4 || key == SDLK_4) {
	load_state(mod & KMOD_CTRL);
      } else if (!fadetime) {
        
        // standard keys
	if (key == SDLK_F1 || key == SDLK_1) {
          save_state();
        } else if (key == SDLK_p && !fadetime) {
          paused = !paused;
        } else if (key == SDLK_m) {
          mousecammode = !mousecammode;
	  save_options();
        } else if (key == SDLK_g) {
          guides = !guides;
        } else if (ingame == 3 && key == SDLK_ESCAPE) {
          init_leave_select(14);
	  save_options();
        } else if (key == SDLK_i) {
          camykey = UP;
	  freecam = 1;
        } else if (key == SDLK_j) {
          camxkey = LEFT;
	  freecam = 1;
        } else if (key == SDLK_k) {
          camykey = DOWN;
	  freecam = 1;
        } else if (key == SDLK_l) {
          camxkey = RIGHT;
	  freecam = 1;
	} else if (key == SDLK_r) {
	  init_fade(6);
        } else if (key == SDLK_t && !beret.dead && beatlevel[LAST_LEVEL]) {
	  istophat = !istophat;
	  beret.subtype = istophat;
	  float cx = beret.x+beret.width/2, cy = beret.y+beret.height/2;
	  make_expl(cx,cy,0,0,beret.subtype?PURPLE:GREEN,8,150);
	  make_expl(cx,cy,0,0,beret.subtype?GRAY:SKY,6,75);
	  make_expl(cx,cy,0,0,BROWN,7,25);
	}
      }
    }
    
    if (key == SDLK_w || key == SDLK_UP || key == SDLK_SPACE) {
      key2 = UP;
      freecam = 0;
    } else if (key == SDLK_a || key == SDLK_LEFT) {
      remkey1 = key1;
      key1 = LEFT;
      freecam = 0;
    } else if (key == SDLK_s || key == SDLK_DOWN) {
      key2 = DOWN;
      freecam = 0;
    } else if (key == SDLK_d || key == SDLK_RIGHT) {
      remkey1 = key1;
      key1 = RIGHT;
      freecam = 0;
    }
  }
}


void handle_key_up(SDLKey key) {
  if (key == SDLK_w || key == SDLK_UP || key == SDLK_SPACE) {
    if (key2 == UP) key2 = NONE;
  } else if (key == SDLK_a || key == SDLK_LEFT) {
    if (key1 == LEFT) {key1 = remkey1; remkey1 = NONE;}
    if (remkey1 == LEFT) remkey1 = NONE;
  } else if (key == SDLK_s || key == SDLK_DOWN) {
    if (key2 == DOWN) key2 = NONE;
  } else if (key == SDLK_d || key == SDLK_RIGHT) {
    if (key1 == RIGHT) {key1 = remkey1; remkey1 = NONE;};
    if (remkey1 == RIGHT) remkey1 = NONE;
  } else if (key == SDLK_LSHIFT || key == SDLK_RSHIFT) {
    key3 = NONE;
  } else if (key == SDLK_i) {
    if (camykey == UP) camykey = NONE;
  } else if (key == SDLK_j) {
    if (camxkey == LEFT) camxkey = NONE;
  } else if (key == SDLK_k) {
    if (camykey == DOWN) camykey = NONE;
  } else if (key == SDLK_l) {
    if (camxkey == RIGHT) camxkey = NONE;
  }
}


void handle_tele_check() {
  int cursrad;
  canbetelething = -1;
  for (cursrad = 0; cursrad <= CURSORRADIUS; cursrad += CURSORRADIUS) {
    if (canbetelething == -1) {
      for (i=THINGMAX-1; i >= 0; i--) {
        if (things[i].type != NOTYPE && things[i].infront &&
            (creatormode || (things[i].pickup && !things[i].teledelay)) &&
            mx+cursrad >= things[i].x && mx-cursrad < things[i].x+things[i].width &&
            my+cursrad >= things[i].y && my-cursrad < things[i].y+things[i].height) {
          canbetelething = i;
          break;
        }
      }
    } else break;
    if (canbetelething == -1) {
      for (i=THINGMAX-1; i >= 0; i--) {
        if (things[i].type != NOTYPE && !things[i].infront &&
            (creatormode || (things[i].pickup && !things[i].teledelay)) &&
            mx+cursrad >= things[i].x && mx-cursrad < things[i].x+things[i].width &&
            my+cursrad >= things[i].y && my-cursrad < things[i].y+things[i].height) {
          canbetelething = i;
          break;
        }
      }
    } else break;
  }
}


// handle the setup of a link from selected Link Blocks to the
// object that the mouse is over
void set_link() {
  crtlinking = 0;
  for (i=0; i<THINGMAX; i++)
    if (things[i].type == LINKBLOCK && selection[i]) {
      things[i].link = canbetelething;
      if (things[i].subtype % 2 == 1)
        things[i].islinked = canbetelething;
      else if (canbetelething > -1) things[canbetelething].islinked = i;
    }
}


void handle_mouse_down(int x, int y, int button) {
  if (ingame == 0) {
    ingame = 1;
    load_game_data();
    return;
  } else if (ingame == 1 && !getinput) {
    begin_game();
    return;
  }

  if (ingame == 4 || ingame == 5) {
    ingame = (ingame == 4 ? ingamereturn : 2);
    return;
  }

  if (ingame == 10 || ingame == 11) {
    ingame++;
    return;
  }

  if (ingame == 12) {
    ingame = -1;
    return;
  }

  if (getinput) {
    resolve_input(0);
    return;
  }

  if (ingame== 2 && !getinput) {
    if (mapselect > -1 && 
        (button == SDL_BUTTON_LEFT || button == SDL_BUTTON_RIGHT) &&
        (levelentry[mapCode][mapselect] <= gamemedals || (mapCode == 4 && opencreator))) {
      if (my/SPR_SIZE == 1) init_fade(7);
      else if (mapCode == 4 && mapselect == 10) {
	char filestr[512];
	sprintf(filestr, "%ssaves%sgame%d.ret", support_path, DIRSEP, gameNum);
	remove(filestr);
	init_fade(1);
      }
      else init_fade(3);

    }
    if ((mapCode > 0 || (mapCode == 0 && (opencreator || beatlevel[LAST_LEVEL]))) &&
	!(mapCode == 4 && !beatlevel[LAST_LEVEL])) {
      if (mx >= 30 && mx <= 120 && my >= 195 && my <= 285) {
        trgmap = mapCode-1;
	if (trgmap < 0) trgmap = 4;
        init_fade(8);
      }
    }
    if ((mapCode < 4 && beatlevel[maplastlevel[mapCode]]) || mapCode == 4) {
      if (mx >= SCR_WIDTH-120 && mx <= SCR_WIDTH-30 && my >= 195 && my <= 285) {
        trgmap = mapCode+1;
	if (trgmap > 4) trgmap = 0;
        init_fade(8);
      }
    }
    return;
  }

  if (creatormode &&
      (button == SDL_BUTTON_LEFT || button == SDL_BUTTON_RIGHT)) {
    if (crtlinking) {
      set_link();
    } else if (crtinventory) {
      int xtile = x/SPR_SIZE-1, ytile = y/SPR_SIZE-1, tindex;
      if (xtile > -1 && xtile < 24 && ytile > -1 && ytile < 18) {
        if (ytile < 10) {
          tindex = xtile+ytile*24+2;
          if ((tindex < TOPHAT && tindex != STONEY &&
	      tindex != BLOCKSTER && tindex != MATTERLY) || tindex == MATTERFRAG) {
            crtplacetype = tindex;
            crtplacesubtype = 0;
          }
        } else {
          tindex = xtile+(ytile-10)*24;
          if (tindex < TILEMAX) crtplacetile = tindex;
        }
      }
      crtinventory = 0;
    }
  }
  if (!paused && !getinput) {
    if (button == SDL_BUTTON_RIGHT) {
      if (creatormode) {
        crtselect = 1;
        selx = mx;
        sely = my;
      } 
      else guides = !guides;
    } else if (button == SDL_BUTTON_LEFT) {
      cantele = 1;
      teleon = !teleon;
      if (alwaystele && !teleon && telething > -1) {
        stop_telekinesis();
      }
      if (creatormode) clear_selection();
    }
  }
}


void handle_mouse_up(int x, int y, int button) {
  if (button == SDL_BUTTON_LEFT) {
    cantele = 0;
    if (!alwaystele && telething > -1) {
      stop_telekinesis();
    }
  }
  if (button == SDL_BUTTON_RIGHT && creatormode && crtselect) {
    if (key3 != SHIFT) clear_selection();
    crtselect = 0;
    int sell, selr, selt, selb;
    if (selx < mx) {
      sell = selx; selr = mx;
    } else {
      sell = mx; selr = selx;
    }
    if (sely < my) {
      selt = sely; selb = my;
    } else {
      selt = my; selb = sely;
    }
    for (i=0; i<THINGMAX; i++) {
      if (things[i].type != NOTYPE && things[i].x <= selr &&
          things[i].y <= selb && things[i].x+things[i].width > sell &&
          things[i].y+things[i].height > selt)
        selection[i] = 1;
    }
  }
}


void draw_sprites(int front) {
  int args[6];

  for (i=0; i<THINGMAX; i++) {
    if (things[i].type != NOTYPE && 
        ((things[i].infront == front && !things[i].inback) ||
         (things[i].inback && front == -1))) {
      draw_thing(&things[i], args);
      if (args[2] > -1) {
        apply_sprite(args[0]-camx,args[1]-camy,args[2],args[3],
                     args[4],args[5],spritesheet,screen);
	// Handle Top Hat's glowing eyes
	if (things[i].type == TOPHAT) {
	    // If Top Hat is telekineticize-o-rama-ing something
	  if ((get_bossvars())[6] > 0)
	    apply_sprite(args[0]-camx+(things[i].dir?14:5),
			 args[1]-camy+((things[i].jump && things[i].vy>0)?14:13),
			 18,5-things[i].dir,1,1,spritesheet,screen);
	}
      }
      if (creatormode) {
        if (things[i].type == LINKBLOCK && selection[i] &&
            (things[i].link > -1 || crtlinking)) {
          draw_line(screen,things[i].x+things[i].width/2-camx,
                    things[i].y+things[i].height/2-camy, 
                    (crtlinking?mx:(things[things[i].link].x+things[things[i].link].width/2))-camx,
                    (crtlinking?my:(things[things[i].link].y+things[things[i].link].height/2))-camy, 0);
        }
        if (things[i].type == SPIKEBALL)
          apply_sprite(things[i].startx-camx,things[i].starty-camy,
                       19-things[i].subtype, 16+things[i].dir,1,1,
                       spritesheet, screen);
        else if (things[i].type == FAKEBLOCK) {
          things[i].anim = 3;
          draw_thing(&things[i], args);
          apply_sprite(args[0]-camx,args[1]-camy,args[2],args[3],
                       args[4],args[5],spritesheet,screen);
          things[i].anim = 0;
        } else if ((things[i].type == DOOR || things[i].type == SIGN || 
                    things[i].type == READSIGN) && selection[i]) {
            if (things[i].type == READSIGN) sprintf(messagestr, "%d", things[i].dir);
          else sprintf(messagestr, "%d, %d, %d", things[i].timer, things[i].status, things[i].dir);
          message = TTF_RenderText_Blended(smfont, messagestr, textcolor);
          apply_surface(things[i].x-camx, things[i].y-12-camy, message, screen);
          SDL_FreeSurface(message);
        }
        if (selection[i]) {
          apply_sprite(things[i].startx+things[i].width-SPR_SIZE-camx,
                       things[i].starty-camy,
                       19,9,1,1,spritesheet,screen);
          apply_sprite(things[i].startx-camx,
                       things[i].starty+things[i].height-SPR_SIZE-camy,
                       19,10,1,1,spritesheet,screen);
        }
      }
    }
  }
}


void draw_beret() {
  if (creatormode) {
    apply_sprite(startx-5-camx,starty-6-camy,
                 0,0,1,1,spritesheet,screen);
  } else {
    int args[6];
    draw_thing(&beret, args);
    apply_sprite(args[0]-camx,args[1]-camy,args[2],args[3],
                 args[4],args[5],spritesheet,screen);
    if (telething > -1)
      apply_sprite(args[0]-camx+(beret.dir?14:5),
		   args[1]-camy+((beret.jump && beret.vy>0)?(istophat?14:13):(istophat?13:11)),
		   18,5-beret.dir,1,1,spritesheet,screen);
  }
}


// draws a haze showing that the bottom of the screen is
// a deadly pit of doom
void draw_pit() {
  apply_surface(0,lvlHeight-15-camy,pit,screen);
}


void draw_tiles(int infront) {
  int tl = camx/SPR_SIZE, tr = (camx+SCR_WIDTH)/SPR_SIZE;
  int tt = camy/SPR_SIZE, tb = (camy+SCR_HEIGHT)/SPR_SIZE;
  for (i = tl; i <= tr; i++) {
    for (j = tt; j <= tb; j++) {
      if (tiles[i][j][0] > 0 && (tiles[i][j][1] > 0) == infront) {
        apply_sprite((i-tl)*SPR_SIZE-(camx%SPR_SIZE),
                     (j-tt)*SPR_SIZE-(camy%SPR_SIZE),
                     (tiles[i][j][0]-1)%24,(tiles[i][j][0]-1)/24,1,1,
                     tilesheet,screen);
        apply_sprite((i-tl)*SPR_SIZE-(camx%SPR_SIZE),
                     (j-tt)*SPR_SIZE-(camy%SPR_SIZE),
                     tiles[i][j][2]%24,22-(see_through(tiles[i][j][0])?2:0)+
                     tiles[i][j][2]/24,1,1,tilesheet,screen);
      }
    }
  }
  if (creatormode && crtgridsize > 0 && !getinput) {
    for (i = tl; i <= tr; i++) {
      for (j = tt; j <= tb; j++) {
        apply_sprite((i-tl)*SPR_SIZE-(camx%SPR_SIZE),
                     (j-tt)*SPR_SIZE-(camy%SPR_SIZE),
                     19,4+crtgridsize,1,1,
                     spritesheet,screen); 
      }
    }
  }
}


void draw_bkg(int flag) {
  if (curbkg > -1 && !flag) {
    apply_surface(0,0,background,screen);
  } else if (flag) apply_surface(0,0,invbackground,screen);
  else apply_surface(0,0,mapbkg,screen);
}


// determines if one point can see another point
int check_vision(int x1, int y1, int x2, int y2, int *r_tilecx, int *r_tilecy) {
  int tilecx = x1/SPR_SIZE, tilecy = y1/SPR_SIZE;
  int tilemx = x2/SPR_SIZE, tilemy = y2/SPR_SIZE;
  float slope = 1.0*(y2-y1)/(x2-x1);
  
  // y = slope*(x-x1) + y1
  int checked = 0;
  while (!checked) {
    checked = tilecx == tilemx && tilecy == tilemy;

    int tt=tilecy*SPR_SIZE, tb=(tilecy+1)*SPR_SIZE;
    int tl=tilecx*SPR_SIZE, tr=(tilecx+1)*SPR_SIZE;
    
    if (tiles[tilecx][tilecy][0] && ((tiles[tilecx][tilecy][1] == SOLID || tiles[tilecx][tilecy][0] == DARKNESSTILE) &&
				     !(tiles[tilecx][tilecy][0] >= SPIKEU && tiles[tilecx][tilecy][0] <= SPIKEL))) {
      *r_tilecx = tilecx;
      *r_tilecy = tilecy;
      return 0;
    }
    
    if (tilecx == tilemx) {
      tilecy += tilecy<tilemy?1:-1;
    } else {
      float y;
      if (tilecx < tilemx) {
        y = (tr-x1)*slope+y1;
      } else {
        y = (tl-x1)*slope+y1;
      }
      if (y < tt) tilecy--;
      else if (y > tb) tilecy++;
      else tilecx += tilecx<tilemx?1:-1;
      }
  }
  return 1;
}


// set that the last level has been beaten
void set_beat_last_level() {
  beatlevel[LAST_LEVEL] = 1;
}


// note that a fragment has been destroyed
void kill_fragment() {
  gotallfrags = -1;
}


// store that the given object has been collected
void collect_thing(Thing* this) {
  if (this->type == MEDALFRAGMENT) {
    play_sound(SND_COLLECT+rand_to(6));
    if (this->dir > -1) gotmfrag[this->dir] = 1;
    if (this->subtype < 2) {
      mfragcount++;
      if (mfragcount == MFRAGTOTAL) {
	play_sound(SND_MEDB);
	if (statusbar) {
	  make_expl(BMEDPOS+15+camx,30+camy,0,0,AQUA,6,150);
	  make_expl(BMEDPOS+15+camx,30+camy,0,0,BLUE,6,60);
	}
      }
    }
  } else if (this->type == MEDALCORNER) {
    gotmcrn[this->subtype % 4] = 1;
    play_sound(SND_CORNER);
    if (this->subtype < 4) {
      if ((gotmcrn[0] || game_gotcorners[lvlCode][0]) && (gotmcrn[1] || game_gotcorners[lvlCode][1]) &&
	  (gotmcrn[2] || game_gotcorners[lvlCode][2]) && (gotmcrn[3] || game_gotcorners[lvlCode][3])) {
	play_sound(SND_MEDO);
	if (statusbar) {
	  make_expl(OMEDPOS+15+camx,30+camy,0,0,ORANGE,6,150);
	  make_expl(OMEDPOS+15+camx,30+camy,0,0,BROWN,6,60);
	}
      }
    }
  } else if (this->type == WHITEMEDAL) {
    gotmwht = 1;
    play_sound(SND_MEDW);
    if (statusbar && this->subtype == 0) {
      make_expl(WMEDPOS+15+camx,30+camy,0,0,WHITE,6,150);
      make_expl(WMEDPOS+15+camx,30+camy,0,0,YELLOW,6,60);
    }
  }
}


// determine if a point is within telekinesis range of another point
int check_range(int x1, int y1, int x2, int y2, int cx, int cy) {
  int tilecx = 0, tilecy = 0, check_vision_ret = 0;
  check_vision_ret = check_vision(x1, y1, x2, y2, &tilecx, &tilecy);
  return sqrt(f_sqr(x2-cx) + f_sqr(y2-cy)) <= SPR_SIZE*TELERADIUS &&
    check_vision_ret;
}


// helper functions for check_can_see
int get_x_see_coord(Thing this, int which) {
  switch (which) {
  case 0: return this.x;
  case 1: return this.x+this.width/2;
  case 2: return this.x+this.width;
  default : return 0;
  }
}
int get_y_see_coord(Thing this, int which) {
  switch (which) {
  case 0: return this.y;
  case 1: return this.y+this.height/2;
  case 2: return this.y+this.height;
  default : return 0;
  }
}


// determine if the first thing can see the second thing
int check_can_see(Thing t1, Thing t2) {
  int coordi, coordj;
  for (coordi=0; coordi<9; coordi++)
    for (coordj=0; coordj<9; coordj++)
      if (check_range(get_x_see_coord(t1,coordi/3),
                      get_y_see_coord(t1,coordi%3),
                      get_x_see_coord(t2,coordj/3),
                      get_y_see_coord(t2,coordj%3),
                      t1.x+t1.width/2, t1.y+t1.height/2))
        return 1;
  return 0;
}


// necessary functionality for moving objects, checking
// telekinesis bounds
void do_telekinesis() {
  //  int part_x = beret.dir?14:5, part_y = (beret.jump && beret.vy>0)?13:11;
  telestat = 0;
  if (telething > -1) {
    // Make particles stream from Beret's eyes
    /* if (rand_to(5) == 0) */
    /*   create_particle(beret.x+part_x-3+rand_to(3), beret.y+part_y-3+rand_to(3), 0, 0, (rand_to(3) == 0?BLUE:WHITE),  */
    /* 		      3+rand_to(4)); */
    /* if (rand_to(5) == 0) */
    /*   create_particle(beret.x+part_x+2+rand_to(3), beret.y+part_y+(beret.dir?-1:1)-3+rand_to(3), 0, 0,  */
    /* 		      (rand_to(3) == 0?BLUE:WHITE), 3+rand_to(4)); */
    // Do the actual telekinesis
    Thing ttt = things[telething];
    if (!creatormode && (ttt.type == NOTYPE || !ttt.pickup || 
        !check_can_see(beret, ttt))) {
      telestat = 1;
      stop_telekinesis();
    } else if (creatormode || (!things[telething].infront && things[telething].islinked == -1)) {
      float cx = things[telething].x+things[telething].width/2;
      float cy = things[telething].y+things[telething].height/2;
      things[telething].vx = approach(things[telething].vx,
                                      cap_val((mx-cx)*TELEMODIF,TELECAP),
                                      TELEACCEL);
      things[telething].vy = approach(things[telething].vy,
                                      cap_val((my-cy)*TELEMODIF,TELECAP),
                                      TELEACCEL);
      if (things[telething].solid) {
        if ((things[telething].collide[LEFT] && things[telething].vx < 0) ||
            (things[telething].collide[RIGHT] && things[telething].vx > 0))
          things[telething].vx = 0;
        if ((things[telething].collide[UP] && things[telething].vy < 0) ||
            (things[telething].collide[DOWN] && things[telething].vy > 0))
          things[telething].vy = 0;
      }
    }
  } else if (canbetelething > -1) {
    Thing cbtt = things[canbetelething];
    if (!creatormode && !check_can_see(beret, cbtt)) {
      telestat = 1;
    }
  } else if (!creatormode && !check_range(beret.x+beret.width/2, beret.y+beret.height/2, mx, my,
                                          beret.x+beret.width/2, beret.y+beret.height/2)) {
    telestat = 1;
  }
  if (!telestat)
    telestat = telething > -1 ? 0 : (canbetelething > -1 ? 3 : 2);
}


void draw_cursor() {
  apply_sprite(mx-camx-7,my-camy-7,19,telestat,1,1,spritesheet,screen);
}


void draw_inventory() {
  Thing displthing;
  int xpos, ypos;
  int args[6];
  displthing.subtype = 0;
  displthing.dir = 0;
  displthing.status = 0;
  displthing.anim = 0;
  displthing.telething = 0;

  for (i=2; i <= TOPHATSHIELD; i++) {
    if (i == STONEY || i == BLOCKSTER || i == MATTERLY ||
	i == TOPHAT || i == SHIELDGEN ||
	(i >= TYPEMAX && i <= SPIKEBLOCK) || i == TOPHATSHIELD) continue;
    displthing.type = i;
    displthing.x = ((i-2)%24)*SPR_SIZE+SPR_SIZE;
    displthing.y = ((i-2)/24)*SPR_SIZE+SPR_SIZE;
    displthing.startx = displthing.x;
    displthing.starty = displthing.y;
    
    draw_thing(&displthing, args);
    apply_sprite(args[0],args[1],args[2],args[3],
                 1,1,spritesheet,screen);
  }
  
  apply_sprite(30,330,19,7,1,1,spritesheet,screen);
  for (i=1; i < TILEMAX; i++) {
    xpos = (i%24)*SPR_SIZE+SPR_SIZE;
    ypos = (i/24)*SPR_SIZE+SPR_SIZE+SCR_HEIGHT/2;
    apply_sprite(xpos,ypos,(i-1)%24,(i-1)/24,1,1,tilesheet,screen);
  }

  for (i = 0; i < SCR_WIDTH; i+=SPR_SIZE) {
    for (j = 0; j < SCR_HEIGHT; j+=SPR_SIZE) {
      apply_sprite(i,j,19,5,1,1,spritesheet,screen); 
    }
  }
}


void draw_get_input() {
  if (getinput == 3) { // Show message
    apply_surface(SCR_WIDTH/2-msgback->w/2, SCR_HEIGHT/2-msgback->h/2,msgback,screen);
    if(lvlCode < 80){
      for (i=0; i<8; i++) {
        display_message(SCR_WIDTH/2,SCR_HEIGHT/2-msgback->h/2+24+20*i,medfont,msgs[msgcode][i],1);
      }
    } else {
      char filestr[250];
      char msgstr[250];
      sprintf(filestr, "%srooms%ssign%d-%d.txt", support_path, DIRSEP, lvlCode, msgcode);
      msgfile = fopen(filestr, "r");
      if(msgfile){
        for (i=0; i<8; i++) {
          if(fgets(msgstr, sizeof msgstr, msgfile) != NULL && strlen(msgstr) > 0) {
            if(msgstr[strlen(msgstr) - 1] == '\n'){
              msgstr[strlen(msgstr) - 1] = '\0';
            }
            if(strlen(msgstr) > 0)
              display_message(SCR_WIDTH/2,SCR_HEIGHT/2-msgback->h/2+24+20*i,medfont,msgstr,1);
          }
        }
        fclose(msgfile);
      }
    }
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2+msgback->h/2-14,smfont,"PRESS ENTER TO CONTINUE",1);
  } else if (getinput == 14 || getinput == 13 || getinput == 17) { // Get input to go back to map screen
    apply_surface(SCR_WIDTH/2-getinputback->w/2, SCR_HEIGHT/2-getinputback->h/2,getinputback,screen);
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2-getinputback->h/2+20,font,"Return to game",0);
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2-getinputback->h/2+50,font,
		    (getinput == 14 ? "Exit to map" : (getinput == 17 ? "Exit to creator" : "Exit to title")),0);
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2-getinputback->h/2+80,font,"Options",0);
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2-getinputback->h/2+110,font,"Controls",0);
    apply_sprite(SCR_WIDTH/2-125,SCR_HEIGHT/2-getinputback->h/2+20+30*optselect-7,0,15,1,1,spritesheet,screen);
    apply_sprite(SCR_WIDTH/2+110,SCR_HEIGHT/2-getinputback->h/2+20+30*optselect-7,0,15,1,1,spritesheet,screen);
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2+getinputback->h/2-14,smfont,"PRESS ENTER TO CONTINUE",0);
  } else if (getinput == 16) { // Get input in level creator
    apply_surface(SCR_WIDTH/2-getinputback->w/2, SCR_HEIGHT/2-getinputback->h/2,getinputback,screen);
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2-getinputback->h/2+20,font,"Return to creator",0);
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2-getinputback->h/2+50,font,"Test room",0);
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2-getinputback->h/2+80,font,"Exit to map",0);
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2-getinputback->h/2+110,font,"Controls",0);
    apply_sprite(SCR_WIDTH/2-125,SCR_HEIGHT/2-getinputback->h/2+20+30*optselect-7,0,15,1,1,spritesheet,screen);
    apply_sprite(SCR_WIDTH/2+110,SCR_HEIGHT/2-getinputback->h/2+20+30*optselect-7,0,15,1,1,spritesheet,screen);
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2+getinputback->h/2-14,smfont,"PRESS ENTER TO CONTINUE",0);
  } else if (getinput == 15) { // Options screen
    apply_surface(SCR_WIDTH/2-optback->w/2, SCR_HEIGHT/2-optback->h/2,optback,screen);
    display_message(SCR_WIDTH/2-70,SCR_HEIGHT/2-optback->h/2+20,font,"Sound on:",0);
    if (musicon == 0) {
      display_message(SCR_WIDTH/2+110,SCR_HEIGHT/2-optback->h/2+20,font,"No",0);
    } else if (musicon == 1) {
      display_message(SCR_WIDTH/2+110,SCR_HEIGHT/2-optback->h/2+20,font,"Yes",0);
    } else if (musicon == 2) {
      display_message(SCR_WIDTH/2+110,SCR_HEIGHT/2-optback->h/2+20,font,"Music Only",0);
    } else if (musicon == 3) {
      display_message(SCR_WIDTH/2+110,SCR_HEIGHT/2-optback->h/2+20,font,"SFX Only",0);
    }
    display_message(SCR_WIDTH/2-70,SCR_HEIGHT/2-optback->h/2+55,font,"Mouse mode:",0);
    if (mousecammode) {
      display_message(SCR_WIDTH/2+110,SCR_HEIGHT/2-optback->h/2+55,font,"Normal",0);
    } else {
      display_message(SCR_WIDTH/2+110,SCR_HEIGHT/2-optback->h/2+55,font,"Alternate",0);
    }
    display_message(SCR_WIDTH/2-70,SCR_HEIGHT/2-optback->h/2+90,font,"Running mode:",0);
    if (runningmode) {
      display_message(SCR_WIDTH/2+110,SCR_HEIGHT/2-optback->h/2+90,font,"Walk with shift",0);
    } else {
      display_message(SCR_WIDTH/2+110,SCR_HEIGHT/2-optback->h/2+90,font,"Double tap",0);
    }
    display_message(SCR_WIDTH/2-50,SCR_HEIGHT/2-optback->h/2+125,font,"Load state on death:",0);
    if (deathstateload) {
      display_message(SCR_WIDTH/2+110,SCR_HEIGHT/2-optback->h/2+125,font,"Yes",0);
    } else {
      display_message(SCR_WIDTH/2+110,SCR_HEIGHT/2-optback->h/2+125,font,"No",0);
    }
    display_message(SCR_WIDTH/2-50,SCR_HEIGHT/2-optback->h/2+160,font,"Display status bar:",0);
    if (statusbar) {
      display_message(SCR_WIDTH/2+110,SCR_HEIGHT/2-optback->h/2+160,font,"Yes",0);
    } else {
      display_message(SCR_WIDTH/2+110,SCR_HEIGHT/2-optback->h/2+160,font,"No",0);
    }
    apply_sprite(SCR_WIDTH/2-215,SCR_HEIGHT/2-optback->h/2+20+35*optselect-7,0,15,1,1,spritesheet,screen);
    apply_sprite(SCR_WIDTH/2+200,SCR_HEIGHT/2-optback->h/2+20+35*optselect-7,0,15,1,1,spritesheet,screen);
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2+optback->h/2-14,smfont,"PRESS ENTER TO CONTINUE",0);
  } else { // Other
    apply_surface(SCR_WIDTH/2-getinputback->w/2, SCR_HEIGHT/2-90,getinputback,screen);
    display_message(SCR_WIDTH/2, SCR_HEIGHT/2-60, font,getinputstr,0);
    if (inputlength > -1) {
      display_message(SCR_WIDTH/2,SCR_HEIGHT/2-15, font,inputstr,0);
    }
    if (inputlength == -2) {
      display_message(SCR_WIDTH/2-60,SCR_HEIGHT/2-15,font,"No",0);
      if (!yesno) {
        apply_sprite(SCR_WIDTH/2-60-message->w/2-20, SCR_HEIGHT/2-22,0,15,1,1,spritesheet,screen);
        apply_sprite(SCR_WIDTH/2-60+message->w/2+5, SCR_HEIGHT/2-22,0,15,1,1,spritesheet,screen);
      }
      display_message(SCR_WIDTH/2+60,SCR_HEIGHT/2-15,font,"Yes",0);
      if (yesno) {
        apply_sprite(SCR_WIDTH/2+60-message->w/2-20, SCR_HEIGHT/2-22,0,15,1,1,spritesheet,screen);
        apply_sprite(SCR_WIDTH/2+60+message->w/2+5, SCR_HEIGHT/2-22,0,15,1,1,spritesheet,screen);
      }
    }
    display_message(SCR_WIDTH/2,SCR_HEIGHT/2+30,smfont,"PRESS ENTER TO CONTINUE",0);
  }
}


void draw_paused() {
  apply_surface(SCR_WIDTH/2-getinputback->w/2, SCR_HEIGHT/2-75,getinputback,screen);
  message = TTF_RenderText_Blended(font, "Paused", textcolor);
  apply_surface(SCR_WIDTH/2-message->w/2, SCR_HEIGHT/2-30, message, screen);
  SDL_FreeSurface(message);
  message = TTF_RenderText_Blended(smfont, "PRESS P TO CONTINUE", textcolor);
  apply_surface(SCR_WIDTH/2-message->w/2, SCR_HEIGHT/2+15, message, screen);
  SDL_FreeSurface(message);
  message = TTF_RenderText_Blended(font, "Paused", textcolor2);
  apply_surface(SCR_WIDTH/2-message->w/2+TEXTSHADOW, SCR_HEIGHT/2-30+TEXTSHADOW, message, screen);
  SDL_FreeSurface(message);
  message = TTF_RenderText_Blended(smfont, "PRESS P TO CONTINUE", textcolor2);
  apply_surface(SCR_WIDTH/2-message->w/2+TEXTSHADOW, SCR_HEIGHT/2+15+TEXTSHADOW, message, screen);
  SDL_FreeSurface(message);
}


void draw_svst_msg() {
  if (svstcount > 0) {
    svstcount--;
    display_message(150, SCR_HEIGHT-60,font,"Saved State",0);
  } else if (svstcount < 0) {
    svstcount++;
    display_message(150, SCR_HEIGHT-60,font,"Loaded State",0);
  }
}


void draw_status_bar() {

  if (gotmwht || haswmed) apply_sprite(WMEDPOS,15,5,15,1,1,spritesheet,screen);
  else apply_sprite(WMEDPOS,15,17,17,1,1,spritesheet,screen);
  
  if (!hasomed && !((gotmcrn[0] || game_gotcorners[lvlCode][0]) && (gotmcrn[1] || game_gotcorners[lvlCode][1]) &&
		    (gotmcrn[2] || game_gotcorners[lvlCode][2]) && (gotmcrn[3] || game_gotcorners[lvlCode][3]))) {
    apply_sprite(OMEDPOS,15,17,17,1,1,spritesheet,screen);
    for (i=0;i<4;i++)
      if (gotmcrn[i] || game_gotcorners[lvlCode][i])
        apply_sprite(OMEDPOS+(i>0&&i<3?15:0),15+(i>1?15:0),
                     (i<3?4:3),(i<3?8+i:10),1,1,spritesheet,screen);
  }
  else apply_sprite(OMEDPOS,15,5,18,1,1,spritesheet,screen);
  
  if (mfragcount < MFRAGTOTAL && !hasbmed) {
    if (gotallfrags == 1) apply_sprite(BMEDPOS-2,15,18,9,1,1,spritesheet,screen);
    apply_sprite(BMEDPOS+2,17,2,6,1,1,spritesheet,screen);
    apply_sprite(BMEDPOS+18,25,2,6,1,1,spritesheet,screen);
    apply_sprite(BMEDPOS+2,33,2,7,1,1,spritesheet,screen);
    if (gotallfrags == -1) apply_sprite(BMEDPOS-2,15,18,15,1,1,spritesheet,screen);
  } else apply_sprite(BMEDPOS,15,5,19,1,1,spritesheet,screen);
  sprintf(messagestr, "%d", mfragcount);
  message = TTF_RenderText_Blended(font, messagestr, textcolor);
  apply_surface(BMEDPOS+MEDINFODIST,15,message,screen);
  SDL_FreeSurface(message);
  message = TTF_RenderText_Blended(font, messagestr, textcolor2);
  apply_surface(BMEDPOS+MEDINFODIST+TEXTSHADOW,15+TEXTSHADOW,message,screen);
  SDL_FreeSurface(message);
  
  if (!enemalldead && !hasrmed) apply_sprite(RMEDPOS,15,17,17,1,1,spritesheet,screen);
  else apply_sprite(RMEDPOS,15,5,16,1,1,spritesheet,screen);
  if (enemdead[areaCode] || game_enemdead[lvlCode][areaCode]) apply_sprite(RMEDPOS+MEDINFODIST,15,18,8,1,1,spritesheet,screen);
  else if (enemdeadhere) apply_sprite(RMEDPOS+MEDINFODIST,15,19,8,1,1,spritesheet,screen);
  else {
    int cmod = (count/3)%8;
    apply_sprite(RMEDPOS+MEDINFODIST,15,19,(cmod<5?11+cmod:20-cmod),1,1,spritesheet,screen);
  }

  if (hasgmed || gotgmed) apply_sprite(GMEDPOS,15,5,17,1,1,spritesheet,screen);
  else apply_sprite(GMEDPOS,15,17,17,1,1,spritesheet,screen);
  if (!(spframes < 10 && spminutes == 0 && spseconds > 0 && spseconds < 15)) {
    sprintf(messagestr, "%d : %02d", spminutes, spseconds);
    message = TTF_RenderText_Blended(font, messagestr, textcolor);
    apply_surface(GMEDPOS+MEDINFODIST,15,message,screen);
    SDL_FreeSurface(message);
    message = TTF_RenderText_Blended(font, messagestr, textcolor2);
    apply_surface(GMEDPOS+MEDINFODIST+TEXTSHADOW,15+TEXTSHADOW,message,screen);
    SDL_FreeSurface(message);
  }

  if (haspmed || gotpmed) apply_sprite(PMEDPOS,15,3,11,1,1,spritesheet,screen);
  else {
    apply_sprite(PMEDPOS,15,17,17,1,1,spritesheet,screen);
    //    if (usedsavestate) apply_sprite(PMEDPOS,15,19,7,1,1,spritesheet,screen);
  }
}


void draw_guides() {
  int midx = (telesource == -1 ? beret.x+beret.width/2 :
              things[telesource].x+things[telesource].width/2);
  int midy = (telesource == -1 ? beret.y+beret.height/2 :
              things[telesource].y+things[telesource].height/2);
  apply_surface(midx-camx-SPR_SIZE*TELERADIUS,
                midy-camy-SPR_SIZE*TELERADIUS,
                teleguide, screen);      
}


void draw_select_game() {
  apply_surface(SCR_WIDTH/2-gameselect->w/2, 330, gameselect, screen);
  sprintf(messagestr, "Game %d", gameNum);
  display_message(SCR_WIDTH/2, 355, font, 0,0);
  display_message(SCR_WIDTH/2, 400, font, gamename,0);
  display_message(SCR_WIDTH/2, 470, smfont, "PRESS ENTER TO BEGIN",0);
  if (gamemedals > -1) {
    sprintf(messagestr, "%d", gamemedals);
    display_message(SCR_WIDTH/2+30, 435, font, 0,0);
    apply_sprite(SCR_WIDTH/2-message->w/2-35,435-message->h/2,5,15,
                 1,1,spritesheet,screen);
  }
  if (gameNum > 1) apply_sprite(SCR_WIDTH/2-gameselect->w/2+15, 445,
                     19,16,1,1,spritesheet,screen);
  if (gameNum < GAMEMAX) apply_sprite(SCR_WIDTH/2+gameselect->w/2-30, 445,
                     19,17,1,1,spritesheet,screen);
}


void draw_title_screen() {
  apply_surface(0,0,title,screen);
  sprintf(messagestr, "Beret 1.2.1");
  display_message(45, 15, smfont, 0, 1);
}


int square(int x) {
  return x * x;
}


int cube(int x) {
  return x * x * x;
}


int credit_square(int shift, int div) {
  return square(credittime-shift)/div;
}


int credit_cube(int shift, int div) {
  return cube(credittime-shift)/div;
}


void draw_credits() {
  apply_surface(0,0,fades[4],screen);
  apply_surface(0,0,credits,screen);
  // Game By: - Nigel Kilmer
  display_message(300 - credit_square(150,200), 350 - credit_cube(150,4000), font, "Game by:", 0);
  display_message(SCR_WIDTH - 300 + credit_square(170,200), 400 + credit_cube(170,4000), font, "Nigel Kilmer", 0);
  display_message(SCR_WIDTH - 250 + credit_square(170,200), 450 + credit_cube(170,4000), smfont, "(Kiwisauce)", 0);
  /*
  // Design
  display_message(320 - credit_square(350,200), 350 - credit_cube(350,4000), font, "Design:", 0);
  display_message(SCR_WIDTH - 320 + credit_square(370,200), 400 + credit_cube(370,4000), font, "Nigel Kilmer", 0);
  // Programming
  display_message(300 - credit_square(550,200), 370 - credit_cube(550,4000), font, "Programming:", 0);
  display_message(SCR_WIDTH - 300 + credit_square(570,200), 420 + credit_cube(570,4000), font, "Nigel Kilmer", 0);
  // Level Design
  display_message(350 - credit_square(750,200), 440 - credit_cube(750,4000), font, "Level Design:", 0);
  display_message(SCR_WIDTH - 300 + credit_square(770,200), 490 + credit_cube(770,4000), font, "Nigel Kilmer", 0);
  // Art
  display_message(310 - credit_square(950,200), 340 - credit_cube(950,4000), font, "Art:", 0);
  display_message(SCR_WIDTH - 310 + credit_square(970,200), 390 + credit_cube(970,4000), font, "Nigel Kilmer", 0);
  // Music
  display_message(270 - credit_square(1150,200), 320 - credit_cube(1150,4000), font, "Music:", 0);
  display_message(SCR_WIDTH - 400 + credit_square(1170,200), 370 + credit_cube(1170,4000), font, "Nigel Kilmer", 0);
  // Sound
  display_message(330 - credit_square(1350,200), 440 - credit_cube(1350,4000), font, "Sound:", 0);
  display_message(SCR_WIDTH - 350 + credit_square(1370,200), 490 + credit_cube(1370,4000), font, "Nigel Kilmer", 0);
  */
  // Playtesters
  display_message(250 - credit_square(350,200), 350 - credit_cube(350,4000), font, "Thanks to all playtesters!", 0);
  display_message(SCR_WIDTH - 250 - credit_square(500,200), 490 + credit_cube(500,4000), font, "Stefan Roger", 0);
  display_message(SCR_WIDTH - 550 + credit_square(575,200), 400 + credit_cube(575,4000), font, "Bret Sepulveda", 0);
  display_message(SCR_WIDTH - 450 + credit_square(650,200), 320 + credit_cube(650,4000), font, "Kyle Kilmer", 0);
  display_message(SCR_WIDTH - 320 + credit_square(725,200), 280 + credit_cube(725,4000), font, "Nathan Weizenbaum", 0);
  display_message(SCR_WIDTH - 350 + credit_square(800,200), 420 + credit_cube(800,4000), font, "KC Gidewall", 0);
  display_message(SCR_WIDTH - 470 + credit_square(875,200), 300 + credit_cube(875,4000), font, "Alyssa Gidewall", 0);
  display_message(SCR_WIDTH - 450 + credit_square(950,200), 410 + credit_cube(950,4000), font, "Ivan Kozlov", 0);
  display_message(SCR_WIDTH - 350 + credit_square(1025,200), 500 + credit_cube(1025,4000), font, "Daniel Mills", 0);
  display_message(SCR_WIDTH - 250 + credit_square(1100,200), 430 + credit_cube(1100,4000), font, "Raymond Zhang", 0);
  display_message(SCR_WIDTH - 550 + credit_square(1175,200), 290 + credit_cube(1175,4000), font, "Jonathan Kane", 0);
  display_message(SCR_WIDTH - 300 + credit_square(1250,200), 400 + credit_cube(1250,4000), font, "Tristan Pearson", 0);
  display_message(SCR_WIDTH - 460 + credit_square(1325,200), 250 + credit_cube(1325,4000), font, "Beau Pearson", 0);
  // Copyright
  if (credittime < 1500) {
    display_message(300, 420 + credit_square(1500,200), font, "Copyright 2011 Nigel Kilmer", 0);
  } else {
    display_message(300, 420, font, "Copyright 2011 Nigel Kilmer", 0);
    display_message(400, 470, smfont, "Thanks for playing!", 0);
    display_message(400, 500, smfont, "kiwisauce.com", 0);
  }
}


void draw_creator_guides() {
  Thing displthing;
  int args[6];

  if (crtselect) {
    int sell, selr, selt, selb;
    if (selx < mx) {
      sell = selx-camx; selr = mx-camx;
    } else {
      sell = mx-camx; selr = selx-camx;
    }
    if (sely < my) {
      selt = sely-camy; selb = my-camy;
    } else {
      selt = my-camy; selb = sely-camy;
    }
    if (sell < 0) sell = 0;
    if (sell >= SCR_WIDTH) sell = SCR_WIDTH-1;
    if (selt < 0) selt = 0;
    if (selt >= SCR_HEIGHT) selt = SCR_HEIGHT-1;
    if (selr < 0) selr = 0;
    if (selr >= SCR_WIDTH) selr = SCR_WIDTH-1;
    if (selb < 0) selb = 0;
    if (selb >= SCR_HEIGHT) selb = SCR_HEIGHT-1;
    draw_rect(screen, sell, selt, selr, selb, selcolor, 0x50);
  }
  int xoffst=15, yoffst=15;
  if (mx-camx < SCR_WIDTH/4 && my-camy < SCR_HEIGHT/4)
    xoffst = SCR_WIDTH/3;
  apply_sprite(xoffst,yoffst,16,18,4,2,spritesheet,screen);
  if (crtplacetile > 0) 
    apply_sprite(xoffst+15,yoffst+15,(crtplacetile-1)%24,
                 (crtplacetile-1)/24,1,1,tilesheet,screen);
  displthing.type = crtplacetype;
  displthing.subtype = crtplacesubtype;
  displthing.dir = crtplacedir;
  displthing.status = 0;
  displthing.anim = 0;
  displthing.telething = 0;
  displthing.x = xoffst+60;
  displthing.y = yoffst+15;
  displthing.startx = displthing.x;
  displthing.starty = displthing.y;
  draw_thing(&displthing, args);
  apply_sprite(args[0],args[1],args[2],args[3],
               1,1,spritesheet,screen);
  if (crtplacetype == SPIKEBALL)
    apply_sprite(displthing.x,displthing.y,
                 19-crtplacesubtype, 16+crtplacedir,1,1,
                 spritesheet, screen);
  else if (crtplacetype == DOOR || crtplacetype == SIGN || crtplacetype == READSIGN) {
    if (crtplacetype == READSIGN) sprintf(messagestr, "%d", crtmessage);
    else sprintf(messagestr, "%d, %d, %d", crtentrance, crtexit, crtexitroom);
    message = TTF_RenderText_Blended(smfont, messagestr, textcolor);
    apply_surface(displthing.x, displthing.y-12, message, screen);
    SDL_FreeSurface(message);
  }
}


void handle_grid_snap() {
  int gridsize = crtgridsize == 1 ? SPR_SIZE : SPR_SIZE/2;
  int tx, ty, cx, cy;
  for (i=0; i<THINGMAX; i++) {
    if (things[i].type != NOTYPE) {
      if (abs(things[i].vx) < SNAPVEL && !things[i].nomove) {
        tx = things[i].x;
        cx = things[i].x+things[i].width/2;
        if (tx % gridsize < SNAPSIZE)
          things[i].x -= tx % gridsize;
        else if (tx % gridsize >= gridsize - SNAPSIZE)
          things[i].x += gridsize - tx % gridsize;
        else if ((tx+things[i].width) % gridsize < SNAPSIZE)
          things[i].x -= (tx+things[i].width) % gridsize;
        else if (crtgridsize == 1) {
          if ((tx+things[i].width) % gridsize >= gridsize - SNAPSIZE)
            things[i].x += gridsize - (tx+things[i].width) % gridsize;
          else if (cx % gridsize < gridsize/2+SNAPSIZE &&
                   cx % gridsize >= gridsize/2-SNAPSIZE)
            things[i].x += gridsize/2 - cx % gridsize;
        }
      }
      if (abs(things[i].vy) < SNAPVEL && !things[i].nomove) {
        ty = things[i].y;
        cy = things[i].y+things[i].height/2;
        if (ty % gridsize < SNAPSIZE)
          things[i].y -= ty % gridsize;
        else if (ty % gridsize >= gridsize - SNAPSIZE)
          things[i].y += gridsize - ty % gridsize;
        else if ((ty+things[i].height) % gridsize < SNAPSIZE)
          things[i].y -= (ty+things[i].height) % gridsize;
        else if (crtgridsize == 1) {
          if ((ty+things[i].height) % gridsize >= gridsize - SNAPSIZE)
            things[i].y += gridsize - (ty+things[i].height) % gridsize;
          else if (cy % gridsize < gridsize/2+SNAPSIZE &&
                   cy % gridsize >= gridsize/2-SNAPSIZE)
            things[i].y += gridsize/2 - cy % gridsize;
        }
      }
    }
    
    things[i].x = (int)things[i].x;
    things[i].y = (int)things[i].y;
  }
}


void handle_start_pos() {
  for (i=0; i<THINGMAX; i++) {
    if (things[i].type != NOTYPE) {
      things[i].startx = things[i].x;
      things[i].starty = things[i].y;
    }
  }
}


void handle_mvmt() {
  for (i=0;i<THINGMAX;i++) {
    if (things[i].type != NOTYPE && selection[i]) {
      if (key1 == LEFT) things[i].vx = (key3==SHIFT?-1:-8);
      else if (key1 == RIGHT) things[i].vx = (key3==SHIFT?1:8);
      if (key2 == UP) things[i].vy = (key3==SHIFT?-1:-8);
      else if (key2 == DOWN) things[i].vy = (key3==SHIFT?1:8);
    }
  }
}


void update_timer() {
  if (spframes > 0 || spseconds > 0 || spminutes > 0) { 
    spframes--;
    if (spframes < 0) {spseconds--; spframes=59;}
    if (spseconds < 0) {spminutes--; spseconds=59;}
  }
}


void update_fade() {
  if (fadetime > 0) {
    draw_fade();
    fadetime--;
    if (fadetime == 0) resolve_fade();
  }
}


void reincarnate_beret(int bdir, int bspeed) {
  for (i = 0; i < THINGMAX; i++) {
    if (things[i].type == REINCARNATOR && things[i].subtype == 1) {
      make_beret(&beret, BERET, istophat, things[i].x+5,
                 things[i].y+66, 1, things);
      beret.dir = bdir;
      beret.speed = bspeed;
      play_sound(SND_REGEN);
      make_expl(beret.x+beret.width/2,beret.y+beret.height/2,
                0,0,ORANGE,5,125);
      make_expl(beret.x+beret.width/2,beret.y+beret.height/2,
                0,0,WHITE,5,125);
      center_camera();
      deathtime = 0;
      break;
    }
  }
}


int main(int argc, char* argv[]) {

  // Initalize and load necessary files
  if (!init()) {printf("Initialization error\n"); return 1;}
  if (!load_files()) {printf("File load error\n"); return 1;}

  game_init();

  Uint32 curTime, nextTime = SDL_GetTicks();

  // Enter main game loop
  while (!quit) {
    curTime = SDL_GetTicks();
    if (curTime >= nextTime) {
      nextTime = SDL_GetTicks() + (1000 / FPS_LIMIT);

      // Fixes mouse-in-corner bug - no it doesn't, silly
      /* if (SDL_GetAppState() & SDL_APPMOUSEFOCUS) { */
      /* 	if (mouse_visible) { */
      /* 	  SDL_ShowCursor(SDL_DISABLE); */
      /* 	  mouse_visible = 0; */
      /* 	} */
      /* } else { */
      /* 	if (!mouse_visible) { */
      /* 	  SDL_ShowCursor(SDL_ENABLE); */
      /* 	  mouse_visible = 1; */
      /* 	} */
      /* } */
      
      // Check events
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          creatormode = 0;
          clean_up();
          return 0;
        } else if (event.type == SDL_ACTIVEEVENT) {
            inactive = !event.active.gain;
        } else if (event.type == SDL_KEYDOWN) {
          handle_key_down(event.key.keysym.sym);
        } else if (event.type == SDL_KEYUP) {
          handle_key_up(event.key.keysym.sym);
        } else if (event.type == SDL_MOUSEMOTION) {
          mx = event.motion.x+camx;
          my = event.motion.y+camy;
	  // Adjust the mouse position near the edge of the screen to make smashing enemies easier.
	  if (event.motion.x <= 0) mx -= SPR_SIZE * 2;
	  else if (event.motion.x >= SCR_WIDTH - 1) mx += SPR_SIZE * 2;
	  if (event.motion.y <= 0) my -= SPR_SIZE * 2;
	  else if (event.motion.y >= SCR_HEIGHT - 1) my += SPR_SIZE * 2;
	  // Handle option choices
	  if (getinput) {
	    if (inputlength == -2) {
	      yesno = (event.motion.x > SCR_WIDTH / 2);
	    } else if (inputlength == -3 && getinput != 15) {
	      if (optmax == 4) optselect = (event.motion.y - (SCR_HEIGHT/2-getinputback->h/2+10)) / 30;
	      else optselect = (event.motion.y - (SCR_HEIGHT/2-getinputback->h/2+15)) / 40;
	      if (optselect < 0) optselect = 0;
	      else if (optselect >= optmax) optselect = optmax - 1;
	    }
	  }
        } else if (event.type == SDL_MOUSEBUTTONDOWN) {
          handle_mouse_down(event.button.x, event.button.y,
                            event.button.button);
        } else if (event.type == SDL_MOUSEBUTTONUP) {
          handle_mouse_up(event.button.x, event.button.y,
                          event.button.button);
        }
      }

      if (ingame == 4 || ingame == 5 || (ingame >= 10 && ingame <= 12)) {
        telestat = 2;
        if (ingame == 4) draw_controls();
        else if (ingame == 5) draw_story();
	else draw_creat(ingame - 10);
        if (!inactive) draw_cursor();
        if (SDL_Flip(screen) == -1) return 1;
        continue;
      }

      if (ingame == 13) {
	credittime++;
	draw_credits();
	if (credittime >= 1950) init_fade(4);
        if (!inactive) draw_cursor();
        update_fade();
        if (SDL_Flip(screen) == -1) return 1;
	continue;
      }

      if (ingame == 0 || ingame == 1) {
        telestat = 2;
        for (i=0; i < THINGMAX; i++) {
          if (things[i].type != NOTYPE)
            update_thing(&things[i], tiles, things, &beret, 
                         lvlWidth, lvlHeight, i, &gravdir,
                         &switchflags, telething > -1);
	}
        draw_bkg(0);
        draw_pit();
        draw_tiles(0);
        draw_sprites(-1);
        draw_sprites(0);
        draw_sprites(1);
        draw_tiles(1);
        draw_title_screen();
        if (ingame == 1) draw_select_game();
        if (getinput) draw_get_input();
        if (!inactive) draw_cursor();
        update_fade();
        if (SDL_Flip(screen) == -1) return 1;
        continue;
      }

      if (ingame == 2) {
        telestat = 2;
        draw_map();
        draw_map_status();
        if (!getinput && !fadetime) check_map_select();
        if (getinput) draw_get_input();
        if (!inactive) draw_cursor();
        update_fade();
        if (SDL_Flip(screen) == -1) return 1;
        continue;
      }

      if (!crtinventory) handle_tele_check();

      //      int curbx = beret.x, curby = beret.y;

      if (!paused && !creatormode && !getinput) {
        beret.timer = -1;
        if (gotallfrags > -1) gotallfrags = 1;
        update_timer();
        update_particles(particles, gravdir);
        for (i=0; i < THINGMAX; i++) {
          if (things[i].type != NOTYPE && !things[i].infront)
            update_thing(&things[i], tiles, things, &beret, 
                         lvlWidth, lvlHeight, i, &gravdir,
                         &switchflags, telething > -1);
          if (things[i].type == MEDALFRAGMENT && things[i].subtype < 2 && gotallfrags > -1)
            gotallfrags = 0;
        }
        if (!beret.dead) handle_key_input(key1, key2, key3, &beret,
					  tiles, things, lvlWidth,
					  lvlHeight, &gravdir,
					  &switchflags, walkaway,
					  runningmode);
        for (i=0; i < THINGMAX; i++) {
          if (things[i].type != NOTYPE && things[i].infront)
            update_thing(&things[i], tiles, things, &beret, 
                         lvlWidth, lvlHeight, i, &gravdir,
                         &switchflags, telething > -1);
        }
        for (i=0; i < THINGMAX; i++) {
          if (things[i].type != NOTYPE && things[i].dead)
            things[i].type = NOTYPE;
        }
        if (beret.dead) {
          if (telething > -1) stop_telekinesis();
          if (deathtime == DEATHDELAY - 5 && switchflags & REINCARNATE)
            reincarnate_beret(beret.dir, beret.speed);
          else if (deathtime < DEATHDELAY) deathtime++;
          else if (deathstateload && hassavestate) load_state(0);
	  else if (!fadetime) init_fade(6);
        }
        if (beret.ontele == 1) {
          things[telething].teledelay = TELEDELAY;
          stop_telekinesis();
        }
        if (beret.timer > -1 && !beret.jump) use_door();
        start_telekinesis();
        if (!beret.dead) do_telekinesis();
      } else if (creatormode && !getinput) {
        if (!crtinventory) {
          start_telekinesis();
          do_telekinesis();
          handle_mvmt();
          for (i=0; i < THINGMAX; i++) {
            if (things[i].type != NOTYPE)
              update_thing(&things[i], tiles, things, &beret, 
                           lvlWidth, lvlHeight, i, &gravdir,
                           &switchflags, telething > -1);
          }
          if (crtgridsnap && crtgridsize) handle_grid_snap();
          handle_start_pos();
        } else {
          telestat = 2;
        }
      }

      // Update camera
      if (!creatormode) {
        int curcamx = camx, curcamy = camy;
	if (freecam) {
	  if (!paused) {
	    if (camxkey == LEFT) camx -= CAMSCROLL;
	    if (camxkey == RIGHT) camx += CAMSCROLL;
	    if (camykey == UP) camy -= CAMSCROLL;
	    if (camykey == DOWN) camy += CAMSCROLL;
	  }
	} else {
	  int midx = (telesource == -1 ? beret.x+beret.width/2 :
		      things[telesource].x+things[telesource].width/2);
	  int midy = (telesource == -1 ? beret.y+beret.height/2 :
		      things[telesource].y+things[telesource].height/2);
	  int bvy = abs(beret.vy);
	  if (midx < camx + 2*SCR_WIDTH/5)
	    camx = approach(camx, midx-2*SCR_WIDTH/5, MAX(CAMSCROLL*2,bvy));
	  if (midx > camx + 3*SCR_WIDTH/5)
	    camx = approach(camx, midx-3*SCR_WIDTH/5, MAX(CAMSCROLL*2,bvy));
	  if (midy < camy + 2*SCR_HEIGHT/5)
	    camy = approach(camy, midy-2*SCR_HEIGHT/5, MAX(CAMSCROLL*2,bvy));
	  if (midy > camy + 3*SCR_HEIGHT/5)
	    camy = approach(camy, midy-3*SCR_HEIGHT/5, MAX(CAMSCROLL*2,bvy));
	}
        
        fix_camera();
/*         if (!inactive &&  */
/*             ((!mousecammode && (curcamx != camx || curcamy != camy)) ||  */
/*             (mousecammode && (curbx != (int)beret.x || curby != (int)beret.y)))) { */
        if (!inactive && (curcamx != camx || curcamy != camy)) {
/*           if (mousecammode) { */
/*             mx -= curbx - (int)beret.x; */
/*             my -= curby - (int)beret.y; */
/*             SDL_WarpMouse(mx-camx,my-camy); */
/*           } else { */
            if (!mousecammode) {
              int tox = mx-camx, toy = my-camy;
              if (tox < 0) tox = 0;
              if (toy < 0) toy = 0;
              if (tox > SCR_WIDTH-1) tox = SCR_WIDTH-1;
              if (toy > SCR_HEIGHT-1) toy = SCR_HEIGHT-1;
              SDL_WarpMouse(tox,toy);
            } else {
              mx -= curcamx - camx;
              my -= curcamy - camy;
            }
/*           } */
        }
      } else if (!crtinventory && !getinput) {
        int cammove;
        if (mx < camx+CAMSENSE) {
          cammove = (CAMSENSE-(mx-camx))/2; 
          camx -= cammove; mx -= cammove;
        }
        if (mx > camx+SCR_WIDTH-CAMSENSE) {
          cammove = (CAMSENSE-(camx+SCR_WIDTH-mx))/2;
          camx += cammove; mx += cammove;
        }
        if (my < camy+CAMSENSE) {
          cammove = (CAMSENSE-(my-camy))/2;
          camy -= cammove; my -= cammove;
        }
        if (my > camy+SCR_HEIGHT-CAMSENSE) {
          cammove = (CAMSENSE-(camy+SCR_HEIGHT-my))/2; 
          camy += cammove; my +=cammove;
        }
        int dmx = camx, dmy = camy;
        fix_camera();
        dmx = dmx-camx, dmy = dmy-camy;
        mx -= dmx;
        my -= dmy;
      }

      // Draw sprites and background
      if (!creatormode || !crtinventory) {
        draw_bkg(0);
        if (camy+SCR_HEIGHT >= lvlHeight - 30) draw_pit();
        draw_tiles(0);
        draw_sprites(-1);
        draw_sprites(0);
        if (!beret.dead || creatormode) draw_beret();
        draw_sprites(1);
        draw_tiles(1);
      } else {
        draw_bkg(1);
        draw_inventory();
      }

      if (paused) draw_paused();

      // Draw the mouse cursor and guides
      if (!creatormode && guides) draw_guides();
      if (!creatormode && statusbar && lvlCode != LAST_LEVEL)
	draw_status_bar();
      if (!creatormode && svstcount != 0) draw_svst_msg();
      if (creatormode && !crtinventory && !getinput)
        draw_creator_guides();
      if (!creatormode) draw_particles();

      if (getinput) draw_get_input();
      if (!inactive) draw_cursor();

      update_fade();

      // Draw the screen
      if (SDL_Flip(screen) == -1) return 1;

      // update the misc counter
      count++;
    } else {
      SDL_Delay(nextTime - curTime);
    }
  }

  clean_up();
  return 0;
}


// takes necessary action after fading has finished
void resolve_fade() {
  int temp, tempcx, tempcy;

  switch (fadereason) {
  case 1 :
    if (ingame >= 0) {
      hassaved = 0;
      creatormode = 1;
      clear_room();
    } else {
      creatormode = !creatormode;
    }
    ingame = -1;
    switchflags ^= CREATORFLAG;
    if (creatormode) init_creator();
    else init_play(0,1);
    break;
  case 2 :
    ingame = 2;
    creatormode = 0;
    switchflags &= ~CREATORFLAG;
    start_map();
    if (!initmapmsg) {
      init_sign(0);
      initmapmsg = 1;
    }
    break;
  case 3 :
    lvlCode = mapCode*20+mapselect;
    areaCode = 0;
    (get_bossvars())[BOSSMAX-1] = 0;
    trgentrance = -1;
    loaderror = 0;
    read_level();
    if (!loaderror) {
      ingame = 3;
      init_play(0,1);
      if (!initgamemsg) {
        init_sign(1);
        initgamemsg = 1;
      }
    } else {
      load_map(mapCode);
    }
    break;
  case 4:
    game_init();
    break;
  case 5 :
    temp = areaCode;
    areaCode = tempAreaCode;
    tempAreaCode = temp;
    tempcx = camx;
    tempcy = camy;
    read_level();
    camx = tempcx;
    camy = tempcy;
    init_play(1,1);
    break;
  case 6 :
    load_backups();
    break;
  case 7 :
    loaderror = 0;
    load_room_return();
    break;
  case 8 :
    mapCode = trgmap;
    start_map();
    break;
  case 9 :
    credittime = 0;
    ingame = 13;
    switch_music(13, 0);
    break;
  }
  if (fadereason > 0) init_fade(0);
  else walkaway = 0;
}

void resolve_input(SDLKey key) {
  int temp = getinput;
  getinput = 0;
  if ((key == SDLK_RETURN || key == SDLK_SPACE || !key) &&
      inputlength == -2 && !yesno && temp == -15)
    init_fade(2);
  if ((key == SDLK_RETURN || key == SDLK_SPACE || !key) &&
      (inputstr[0] != '-' || (inputlength == -2 && yesno) || inputlength == -3)) {
    switch (temp) {
    case -13 :
      crtmessage = atoi(inputstr);
      if (crtmessage >= MSGMAX) crtmessage = 0;
      break;
    case -15 :
      init_fade(7);
      break;
    case -20 :
      areaCode = -1;
      clear_room();
      break;
    case -21 :
      write_level();
      break;
    case -1 : 
      crtexitroom = atoi(inputstr);
      if (crtexitroom >= ROOMMAX) crtexitroom = 0;
      break;
    case -2 : 
      lvlCode = atoi(inputstr) + 79;
      if (lvlCode >= 80 && lvlCode <= 89)
	set_up_input("Save room number:",-4,2,areaCode);
      else set_up_input("Must be 1 - 10",1,-1,-1);
      break;
    case -3 : 
      lvlCode = atoi(inputstr) + 79;
      if (lvlCode >= 80 && lvlCode <= 89)
	set_up_input("Load room number:",-5,2,-1);
      else set_up_input("Must be 1 - 10",1,-1,-1);
      break;
    case -4 : 
      areaCode = atoi(inputstr);
      if (check_level_exists()) {
	set_up_input("Overwrite existing room?",-21,-2,-1);
	yesno = 0;
	break;
      }
      write_level();
      break;
    case -5 : 
      areaCode = atoi(inputstr);
      read_level();
      break;
    case -6 :
      save_map(atoi(inputstr));
      break;
    case -7 :
      load_map(atoi(inputstr));
      break;
    case -8 :
      spminutes = atoi(inputstr);
      set_up_input("Set timer seconds:", -9, 2, -1);
      break;
    case -9 :
      spseconds = atoi(inputstr);
      if (spseconds >= 60) spseconds = 0;
      write_metalevel();
      break;
    case -10 :
      delete_game();
      break;
    case -11 :
      init_fade(4);
      break;
    case 13 :
    case 14 :
    case 17 :
      // Go back to map screen, title, or creator.
      if (optselect == 1) init_fade(temp == 13 ? 4 : (temp == 17 ? 1 : 2));
      else if (optselect == 2) init_opt_select();
      else if (optselect == 3) {
	if (ingame == 2) switch_background(2);
	ingamereturn = ingame;
	ingame = 4;
      }
      break;
    case 16 :
      if (optselect == 1) init_fade(1);
      else if (optselect == 2) init_fade(2);
      break;
    case -12 :
      init_fade(2);
      break;
    case -14 :
      if (enemdeadhere) enemdead[areaCode] = 1;
      beatlevel[lvlCode] = 1;
      if (lvlCode < LAST_LEVEL) {
	// Check which medals the player collected.
	if (gotmwht && !haswmed) {gotmedals[lvlCode*6] = 1; gamemedals++;}
	if (!hasomed && (gotmcrn[0] || game_gotcorners[lvlCode][0]) && (gotmcrn[1] || game_gotcorners[lvlCode][1]) &&
	    (gotmcrn[2] || game_gotcorners[lvlCode][2]) && (gotmcrn[3] || game_gotcorners[lvlCode][3])) {
	  gotmedals[lvlCode*6+1] = 1; gamemedals++;
	}
	if (!hasbmed && mfragcount == MFRAGTOTAL) {gotmedals[lvlCode*6+2] = 1; gamemedals++;}
	if (!hasrmed && enemalldead) {gotmedals[lvlCode*6+3] = 1; gamemedals++;}
	if (!hasgmed && gotgmed) {gotmedals[lvlCode*6+4] = 1; gamemedals++;}
	if (!haspmed && gotpmed) {gotmedals[lvlCode*6+5] = 1; gamemedals++;}
	// Check which pieces of medals the player collected.
	for (i = 0; i < 4; i++) {
	  if (gotmcrn[i]) game_gotcorners[lvlCode][i] = 1;
	}
	for (i = 0; i < MFRAGTOTAL; i++) {
	  if (gotmfrag[i]) game_gotfrags[lvlCode][i] = 1;
	}
	for (i = 0; i < ROOMMAX; i++) {
	  if (enemdead[i]) game_enemdead[lvlCode][i] = 1;
	}
      }
      save_game_data();
      remove_room_return();
      if (lvlCode != LAST_LEVEL) {
	init_fade(2);
      } else {
	init_fade(9);
      }
      break;
    case 2 :
      start_new_game();
      init_fade(2);
      break;
    }
  }
}

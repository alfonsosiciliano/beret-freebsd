#include <stdlib.h>
#include <math.h>
#include "SDL.h"
#include "thing.h"
#include "tile.h"
#include "physics.h"
#include "game.h"

#define LVLMAX 500
#define THINGMAX 250
#define BOSSMAX 10
#define PARTICLEMAX 2500

#define ACCEL 0.4
#define FRICT 0.4
#define BERETFRICT 0.7
#define LINKACCEL 2
#define LINKSPEED 8
#define TAP_TIME 8
#define GRAVITY 0.4
#define JUMP_SPEED 6
#define WALK_SPEED 2.5
#define RUN_SPEED 4
#define TELEMODIF 0.175
#define TELECAP 17.5
#define TELEACCEL 3
#define SPR_SIZE 30
#define CORNER 6
#define FRICTMOD 0.08
#define ICYMOD 0.15
#define FLOATMOD 0.15
#define SMALLSIZE 20
#define TINYSIZE 10
#define ENEMYCRASH 13
#define RUNMARGIN 9
#define JUMPGRACE 5
#define NOMOVETIME 3
#define MOVERSPEED 7.5
#define MOVERACC 3
#define SWITCH_BOUNCE_ERROR 10
#define PI 3.14159265
#define TURRET_SHOOT_SPEED 13.5
#define BLOCKSTER_SHOOT_SPEED 16
#define BLOCKSTER_SHOOT_VSPEED 3
#define BLOCKSTER_COUNT 14
#define GHOST_DIST 190
#define GHOST_TOO_FAST 7
#define GHOST_CHASE_DIST 300
#define REINC_MARGIN 10
#define REINC_CENTER_OFFST 30
#define TOPHATMAXSUBTYPE 3
#define SHIELDVEL 12
#define SHIELDGENNUM 5

#define GREENFLAG 0x1
#define REDFLAG 0x2
#define CREATORFLAG 0x4
#define NOCOLLIDEFLAG 0x8
#define KEYPRESSED 0x10
#define REINCARNATE 0x20

int leftTime=0, rightTime=0, upTime=0;
int runMargin=0, jumpGrace=0, holdShift=0, shiftRun=0;

ThingNode* thingnodes[THINGMAX+1];
int floorchecked[THINGMAX+1];
int bossvars[BOSSMAX];

int* get_bossvars() {
  return bossvars;
}

int get_jumpgrace() { return jumpGrace; }
void set_jumpgrace(int val) { jumpGrace = val; }
int get_runmargin() { return runMargin; }
void set_runmargin(int val) { runMargin = val; }

void initialize_thingnodes() {
  int i;
  for (i=0; i<THINGMAX+1; i++)
    thingnodes[i] = 0;
}


// Find the closest object of the given type to the given coordinates
// If the given type is NOTYPE, search for the closest pickupable object
int closest_thing(Thing things[THINGMAX], int bcx, int bcy, 
		  int* count, int searchtype) {
  int temp, temp2 = 10000, temp3 = -1, temp4;
  int ocx, ocy, cnt=0;
  for (temp=0; temp<THINGMAX; temp++) {
    if ((things[temp].type == searchtype ||
	 (searchtype == NOTYPE && things[temp].pickup && !things[temp].telething)) &&
	 !things[temp].dead) {
      cnt++;
      if ((!things[temp].telething && things[temp].subtype != 1) ||
	  searchtype != STONEY) {
        ocx = things[temp].x+things[temp].width/2;
        ocy = things[temp].y+things[temp].height/2;
        temp4 = sqrt((ocx-bcx)*(ocx-bcx)+(ocy-bcy)*(ocy-bcy));
        if (temp4 < temp2) {
          temp2 = temp4;
          temp3 = temp;
        }
      }
    }
  }
  if (count) *count = cnt;
  return temp3;
}


void clear_floorchecked() {
  int i;
  for (i=0; i<THINGMAX+1; i++)
    floorchecked[i] = 0;
}


float abs_f(float f) {
  return f>0?f:-f;
}


float pos_f(float f) {
  return f==0?0:(f>0?1:-1);
}


int rand_to(int upper) {
  int randval = upper*(rand()/(1.0*RAND_MAX));
  if (randval == upper) return 0;
  return randval;
}


void update_particles(Particle particles[PARTICLEMAX], int gravdir) {
  int i;
  for (i = 0; i < PARTICLEMAX; i++) {
    if (particles[i].time > 0) {
      particles[i].time--;
      particles[i].x += particles[i].vx;
      particles[i].y += particles[i].vy;
  
      // deal with gravity
      switch (gravdir) {
      case DOWN: particles[i].vy += GRAVITY*0.25; break;
      case LEFT: particles[i].vx -= GRAVITY*0.25; break;
      case RIGHT: particles[i].vx += GRAVITY*0.25; break;
      case UP: particles[i].vy -= GRAVITY*0.25; break;
      }
      
      // deal with friction
      if (gravdir == DOWN || gravdir == UP) 
        particles[i].vx = apply_friction(FRICTMOD, particles[i].vx);
      else
        particles[i].vy = apply_friction(FRICTMOD, particles[i].vy);
    }
  }
}


void make_expl(int x, int y, float vx, float vy, 
               int color, int force, int number) {
  int i = 0;
  float angle, vel;
  for (i = 0; i < number; i++) {
    angle = rand_to(360)*PI/180;
    vel = force+(1.0*(rand_to(51)-25)/25);
    create_particle(x,y,vx+vel*cos(angle),vy+vel*sin(angle),
                    color, 10+rand_to(10));
  }
}


void make_circle(int x, int y, int color1, int color2,
                 int radius, int number) {
  int i = 0;
  float angle;
  for (i = 0; i < number; i++) {
    angle = rand_to(360)*PI/180;
    create_particle(x+(radius+rand_to(4))*cos(angle),
                    y+(radius+rand_to(4))*sin(angle),
                    rand_to(3)*cos(angle),rand_to(3)*sin(angle),
                    (i%2==0)?color1:color2, 3+rand_to(5));
  }
}


void tile_collision(Thing* this, Thing things[250], 
                    int tiles[500][500][3], int xpos,
                    int ypos, int index, int barely) {
  if (!(xpos < 0 || ypos < 0 || xpos >= 500 || ypos >= 500)) {
    int thistile = tiles[xpos][ypos][0];
    if (!barely && thistile == CHOICEONLY) {
      if (this->type == BERET) {
        tiles[xpos][ypos][0] = BERETONLY;
        tiles[xpos][ypos][1] = BERETONLYF;
	play_sound(SND_CHOICEBERET+rand_to(3));
      } else if (this->solid) {
        tiles[xpos][ypos][0] = OBJONLY;
        tiles[xpos][ypos][1] = OBJONLYF;
	play_sound(SND_CHOICEOBJECT+rand_to(3));
      }
    }
    if ((this->type == ANTIMATTER || this->type == ANTISEEKER ||
	 this->type == MATTERLY || this->type == ANTIFECTLING) &&
	thistile != EMPTY && thistile != ANTITILE &&
        !(thistile >= OBJONLY && thistile <= CHOICEONLY) &&
        !(thistile >= MOVERU && thistile <= MOVERL)) {
      tiles[xpos][ypos][0] = EMPTY;
      tiles[xpos][ypos][1] = NOTSOLIDB;
      fix_tile_borders(xpos,ypos,1);
      make_expl(xpos*30+15,ypos*30+15,0,0,PURPLE,5,75);
      if (on_screen(this->x, this->y, this->width, this->height))
	play_sound(SND_ANTIMATTER+rand_to(3));
    }
    if (this->solid && !barely && !this->infront) {
      if (!this->telething) {
        if (thistile == MOVERU) 
          this->vy = approach(this->vy, -MOVERSPEED, MOVERACC);
        else if (thistile == MOVERR)
          this->vx = approach(this->vx, MOVERSPEED, MOVERACC);
        else if (thistile == MOVERD) 
          this->vy = approach(this->vy, MOVERSPEED, MOVERACC);
        else if (thistile == MOVERL)
          this->vx = approach(this->vx, -MOVERSPEED, MOVERACC);
      }
      if (this->animate) {
        if (thistile == MOVERR) this->dir = 1;
        else if (thistile == MOVERL) this->dir = 0;
      }
      if (this->type == PLATFORM) {
	if ((this->subtype == 0 || this->subtype == 1) &&
	    (thistile == MOVERL || thistile == MOVERR)) {
	  this->startx = this->x + this->vx;
	} else if ((this->subtype == 2 || this->subtype == 3) &&
		   (thistile == MOVERU || thistile == MOVERD)) {
	  this->starty = this->y + this->vy;
	}
      }
    }
    if (thistile >= SPIKEU && thistile <= SPIKEL &&
        this->animate && this->solid && barely && this->type != ANTISEEKER &&
        ((thistile == SPIKEU && this->vy >= 0 &&
	  this->y+this->height-CORNER < ypos*SPR_SIZE &&
	  this->x <= (xpos+1)*SPR_SIZE-CORNER && this->x+this->width >= xpos*SPR_SIZE+CORNER) ||
         (thistile == SPIKED && this->vy <= 0 &&
	  this->y+CORNER > (ypos+1)*SPR_SIZE &&
	  this->x <= (xpos+1)*SPR_SIZE-CORNER && this->x+this->width >= xpos*SPR_SIZE+CORNER) ||
         (thistile == SPIKEL && this->vx >= 0 &&
	  this->x+this->width-CORNER < xpos*SPR_SIZE) ||
         (thistile == SPIKER && this->vx <= 0 &&
	  this->x+CORNER > (xpos+1)*SPR_SIZE))) {
      destroy_thing(this, TOUCH, things, index);
    }
    if (thistile == ANTITILE && this->solid && barely && !this->dead &&
        this->type != ANTIMATTER && this->type != ANTISEEKER &&
	this->type != MATTERLY && this->type != ANTIFECTLING &&
	this->type != MATTERFRAG) {
      destroy_thing(this, TOUCH, things, index);
      make_expl(this->x+this->width/2,this->y+this->height/2,0,0,
                PURPLE,5,75);
      if (on_screen(this->x, this->y, this->width, this->height))
	play_sound(SND_ANTIMATTER+rand_to(3));
    }
  }
}


// returns the reverse of the given direction
int rev_dir(int dir) {
  switch (dir) {
  case UP : return DOWN;
  case RIGHT : return LEFT;
  case DOWN : return UP;
  case LEFT : return RIGHT;
  }
  printf("Error: unknown direction %d\n", dir);
  return dir;
}


// traverse the linked list to determine whether this can be stood on
void check_ontele(Thing* this, ThingNode* thisnode, int index) {
  floorchecked[index] = 1;

  ThingNode* curnode;
  for (curnode = thisnode; curnode != 0; curnode = curnode->next) {
    if (floorchecked[curnode->thingindex]) {
      this->ontele = -1;
      break;
    }
    if (curnode->thing->ontele > -1) {
      check_ontele(curnode->thing, thingnodes[curnode->thingindex], 
                   curnode->thingindex);
    }
    if (curnode->thing->ontele == -1) {
      this->ontele = -1;
      break;
    } else if (curnode->thing->ontele == 1)
      this->ontele = 1;
  }

  if (this->type == BERET && 
      (this->ontele == 1 || (thisnode != 0 && this->ontele == 0))) {
    this->jumpdelay = 5;
  }
}


void thing_collision(Thing* this, Thing* other, 
                     int collision, int tindex, int oindex,
                     int *gravdir, Thing things[250], int *switchflags,
                     int isthisthing) {
  int i, temp;
  float angle;
  float cx = this->x+this->width/2, cy = this->y+this->height/2;
  float ocx = other->x+other->width/2, ocy = other->y+other->height/2;

  // do standing on block stuff, but only if it's this thing
  if (this->ontele > -1 && isthisthing && this->solid && other->solid) {
    
    // calculate gravity modifiers
    int tgravmod = *gravdir, ogravmod = *gravdir;
    if (this->islinked == -1 && this->gravmod != NONE)
      tgravmod = this->gravmod;
    if (this->islinked > -1) {
      if (things[this->islinked].gravmod != NONE) tgravmod = things[this->link].gravmod;
      if ((things[this->islinked].type == LINKBLOCK && things[this->islinked].subtype == 2) ||
          (this->type == LINKBLOCK && this->subtype == 3))
        tgravmod = rev_dir(tgravmod);
    }
    if (other->islinked == -1 && other->gravmod != NONE)
      ogravmod = other->gravmod;
    if (other->islinked > -1) {
      if (things[other->islinked].gravmod != NONE) ogravmod = things[other->link].gravmod;
      if ((things[other->islinked].type == LINKBLOCK && things[other->islinked].subtype == 2) ||
          (other->type == LINKBLOCK && other->subtype == 3))
        ogravmod = rev_dir(ogravmod);
    }
    if (collision == tgravmod) {
      if (tgravmod == rev_dir(ogravmod)) {
        this->ontele = -1;
      } else {
        ThingNode* newnode = malloc(sizeof(ThingNode));
        newnode->thing = other;
        newnode->next = thingnodes[tindex];
        newnode->thingindex = oindex;
        thingnodes[tindex] = newnode;
      }
    }
  }    

/*   if (this->telething && collision == UP && other->type == BERET && */
/*       !(*switchflags & KEYPRESSED) && this->vx != 0 && */
/*       abs(this->vx) < MOVEBERETHISPEED ) { */
/*     other->vx = approach(other->vx, cx-ocx, 2); */
/*     other->anim = 4; */
/*   } */

  switch (this->type) {
  case TOPHATSHIELD :
    if (other->type != TOPHAT && !other->dead && other->solid) {
      temp = (other->width+other->height)/4; // avg/2
      if (sqrt((cx-ocx)*(cx-ocx)+(cy-ocy)*(cy-ocy)) <= this->width/2+temp - 10) {
	angle = atan2(ocy-cy, ocx-cx);
	other->vx = cos(angle)*SHIELDVEL;
	other->vy = sin(angle)*SHIELDVEL-12;
	if (abs(other->vx) < 10) {
	  if (ocx > cx) this->vx = 15;
	  else this->vx = -15;
	}
      }
    }
    break;
  case ANTIMATTER : case ANTISEEKER : case MATTERLY : 
    if (other->type != ANTISEEKER && other->type != ANTIMATTER &&
	other->type != MATTERLY && other->type != ANTIFECTLING &&
	!(other->type == MATTERFRAG && this->type == MATTERLY) && 
	other->solid) {
      destroy_thing(other, TOUCH, things, oindex);
      make_expl(ocx,ocy,0,0,PURPLE,6,75);
      if (on_screen(this->x, this->y, this->width, this->height))
	play_sound(SND_ANTIMATTER+rand_to(3));
    }
    break;
  case REINCARNATOR :
    if (this->subtype == 0 && other->type == BERET &&
        ocx > cx - REINC_MARGIN &&
        ocx < cx + REINC_MARGIN &&
        ocy > cy + REINC_CENTER_OFFST - REINC_MARGIN &&
        ocy < cy + REINC_CENTER_OFFST + REINC_MARGIN) {
      *switchflags |= REINCARNATE;
      for (i = 0; i < THINGMAX; i++) {
        if (things[i].type == REINCARNATOR && things[i].subtype == 1) {
          things[i].subtype = 0;
          make_expl(things[i].x+15,things[i].y+30,0,0,WHITE,4,60);
          break;
        }
      }
      this->subtype = 1;
      make_expl(this->x+15,this->y+30,0,0,ORANGE,4,60);
      play_sound(SND_REGENINIT);
    }
    break;
  case CARRIER :
    if (collision == UP && !this->jump &&
        !this->telething && other->solid && !other->infront) {
      other->vx = approach(other->vx, cx-ocx, 2);
      if (other->type == BERET) other->anim = 4;
    }
    break;
  case BERET :
    if (other->deathtouch) destroy_thing(this, TOUCH, things, tindex);
    if (other->type == WHITEMEDAL || other->type == MEDALCORNER ||
        other->type == MEDALFRAGMENT) {
      if (!other->dead) collect_thing(other);
      destroy_thing(other, COLLECT, things, oindex);
    }
    if (!this->jump && (other->type == DOOR || other->type == FINISHDOOR || other->type == READSIGN) &&
        this->x > other->x - this->width/2 && this->x < other->x + other->width - this->width/2 &&
        this->y + this->height/2 > other->y) 
      this->timer = oindex;
    if (!this->jump && other->type == SIGN) this->timer = oindex;
    break;
  case AURAHELPER :
    if (other->animate && other->type != AURADROP && !other->dead) {
      temp = (other->width+other->height)/4; // avg/2
      if (sqrt((cx-ocx)*(cx-ocx)+(cy-ocy)*(cy-ocy)) < this->width/2+temp-4) {
        destroy_thing(other, BOMBED, things, oindex);
        make_expl(ocx,ocy,0,0,RED,6,75);
      }
    }
    break;
  case FIREBALL : 
    if (other->animate && oindex != this->status)
      destroy_thing(other, BOMBED, things, oindex);
    break;
  case SPIKEBLOCK : 
    if (other->animate || 
        (other->type == SPIKEBLOCK && other->vx == 0 && other->vy == 0 &&
         !other->telething)) {
      destroy_thing(other, BOMBED, things, oindex);
      if (other->type == BLOCKSTER) 
        destroy_thing(this, BOMBED, things, tindex);
    }
    break;
  case BOMBHELPER :
    if (other->animate || other->type == BOMB ||
        (other->type >= WOODBLOCK && other->type <= STONEBLOCK && other->subtype == 3)) {
      temp = (other->width+other->height)/4; // avg/2
      if (sqrt((cx-ocx)*(cx-ocx)+(cy-ocy)*(cy-ocy)) <=
          this->width/2+temp) destroy_thing(other, BOMBED, things, oindex);
    } else if (other->type == SOLIDSWITCH || other->type == GRAVITYSWITCH) {
      if (other->timer == 0) {
        if (other->type == SOLIDSWITCH) {
          other->status = !other->status;
          *switchflags ^= (other->subtype?REDFLAG:GREENFLAG);
	  play_sound((this->subtype?SND_SWITCHRD:SND_SWITCHGR)+rand_to(3));
        } else {
          other->anim += (other->subtype ? 1 : -1);
          if (other->anim < UP) other->anim = LEFT;
          if (other->anim > LEFT) other->anim = UP;
          *gravdir = other->anim;
	  play_sound(SND_SWITCHGV+rand_to(3));
        }
        other->timer = 30;
      }
    }
    break;
  case ANTIFECTLING :
    if (other->type != ANTISEEKER && other->type != ANTIMATTER &&
	other->type != MATTERLY && other->type != ANTIFECTLING &&
	other->solid) {
      destroy_thing(other, TOUCH, things, oindex);
      make_expl(ocx,ocy,0,0,PURPLE,6,75);
      if (on_screen(this->x, this->y, this->width, this->height))
	play_sound(SND_ANTIMATTER+rand_to(3));
    }
  case INFECTLING :
    if ((other->type != INFECTLING ||
	 other->subtype != 1) && other->type != ANTIFECTLING &&
        (this->subtype == 1 || other->animate || this->type == ANTIFECTLING) &&
        this->timer == 0 && !other->dead && other->solid &&
	!(other->type == TOPHAT && bossvars[0] < SHIELDGENNUM) &&
	!(other->type == FIREBALL) &&
	!(this->type == ANTIFECTLING && other->type != ANTIMATTER &&
	  other->type != MATTERLY && other->type != ANTISEEKER)) {
      float vx = other->vx, vy = other->vy;
      int startx = other->startx+other->width/2-this->width/2;
      int starty = other->starty+other->height/2-this->height/2;
      int islinked = (other->type == LINKBLOCK &&
                      other->subtype%2==1) ? -1 : other->islinked;
      destroy_thing(other, INFECT, things, oindex);
      if (oindex == 250) {
        other->solid = 0; // to stop Beret from colliding with object
        for (i = 0; i < 250; i++) {
          if (things[i].type == NOTYPE) break;
        }
        if (i < 250) oindex = i;
      }
      if (oindex < 250) {
        make_thing(oindex, this->type, this->subtype,
                   other->x+other->width/2-this->width/2,
                   other->y+other->height/2-this->height/2,
                   0, things);
        things[oindex].vx = vx;
        things[oindex].vy = vy;
        things[oindex].startx = startx;
        things[oindex].starty = starty;
        things[oindex].islinked = islinked;
        if (islinked > -1) things[islinked].link = oindex;
        things[oindex].timer = 6;
        make_expl(things[oindex].x+things[oindex].width/2, 
                  things[oindex].y+things[oindex].height/2,
                  cap_val(vx/8,3), cap_val(vy/8,3),
                  this->type == ANTIFECTLING ? PURPLE : (this->subtype ? WHITE : GRAY),
		  3, 25);
      }
    }
    break;
  }
}


// returns the number of subtypes in the given type
int subtype_count(int type) {
  switch (type) {
  case SMWOODBLOCK: case SMICEBLOCK: case SMSTONEBLOCK:
  case ANNOYINGBLOCK: case INFECTLING: case SPIKEBALL:
  case MEDALFRAGMENT: case ROBOT: case STONEY:
  case HOPPER: case SOLIDSWITCH:
  case GRAVITYSWITCH:
    return 2;
  case BOMB: case FAKEBOMB : case FAKEBLOCK: case DOOR: case MATTERFRAG:
    return 3;
  case WOODBLOCK: case ICEBLOCK: case STONEBLOCK: case GRAVITYBLOCK:
  case LINKBLOCK: case MEDALCORNER: case BIGBLOCK: case VSIGN:
  case PLATFORM: case TELESEEKER: case SIGN: case SOLIDBLOCK:
  case SPIKEBLOCK:
    return 4;
  case WHITEDIE: case BLACKDIE:
    return 6;
  case TOPHAT:
    return TOPHATMAXSUBTYPE;
  }
  return 1;
}


// sets the elements in args to the correct values for
// drawing the given object
// set args[2] = -1 for no drawing
void draw_thing(Thing* this, int args[6]) {
  int temp;
  args[0] = this->x;
  args[1] = this->y;
  args[4] = 1;
  args[5] = 1;

  switch(this->type) {
  case GRAVITYSWITCH :
    args[2] = 6;
    args[3] = 12+4*this->subtype+this->anim;
    break;
  case TURRET :
    args[2] = 10+this->anim;
    args[3] = 13-this->dir;
    break;
  case FIREBALL :
    args[2] = 11;
    args[3] = 5;
    break;
  case BLOCKSTER :
    args[2] = 18;
    args[3] = 10 + (int)this->anim;
    break;
  case SPIKEBLOCK :
    args[2] = 18;
    args[3] = this->subtype;
    break;
  case STONEY :
    if (this->subtype == 2) {
      args[2] = 16+this->dir;
      args[3] = 9;
    } else {
      args[2] = 16+this->anim;
      args[3] = this->subtype?6+this->dir:8;
    }
    break;
  case SOLIDSWITCH :
    args[2] = 6+this->subtype;
    args[3] = this->anim;
    break;
  case SOLIDBLOCK :
    args[2] = 4+this->subtype/2;
    args[3] = this->anim;
    break;
  case PLATFORM :
    if ((this->subtype == 0 && this->starty - this->timer + 30 <= this->y) ||
	(this->subtype == 1 && this->starty + this->timer - 30 >= this->y)) {
      args[1] += rand_to(3) - 1;
    } else if ((this->subtype == 2 && this->startx + this->timer - 30 >= this->x) ||
	       (this->subtype == 3 && this->startx - this->timer + 30 <= this->x)) {
      args[0] += rand_to(3) - 1;
    }
    args[2] = 3;
    args[3] = 12+this->subtype;
    args[4] = (this->subtype > 1 ? 2 : 1);
    break;
  case BIGBLOCK :
    args[2] = (this->subtype == 3 ? 5 : 3);
    args[3] = (this->subtype == 3 ? 6 : 4+this->subtype);
    if (this->subtype > 1) {
      args[4] = 2;
      args[5] = 2;
    }
    break;
  case MEDALFRAGMENT :
    if (this->subtype < 2) {
      args[2] = 2;
      args[3] = 6+this->subtype;
    } else {
      args[2] = 8;
      args[3] = 4+this->subtype;
    }

    break;
  case MEDALCORNER :
    if (this->subtype < 4) {
      args[2] = (this->subtype < 3 ? 4 : 3);
      args[3] = (this->subtype < 3 ? 8+this->subtype : 10);
    } else if (this->subtype == 4) {
      args[2] = 15;
      args[3] = 15;
    } else if (this->subtype == 5) {
      args[2] = 17;
      args[3] = 16;
    } else if (this->subtype == 6) {
      args[2] = 16;
      args[3] = 17;
    } else if (this->subtype == 7) {
      args[2] = 16;
      args[3] = 16;
    }
    break;
  case WHITEMEDAL :
    if (this->subtype == 0) {
      args[2] = 5;
      args[3] = 15;
    } else {
      args[2] = 15;
      args[3] = 14;
    }
    break;
  case ANTIMATTER :
    args[0] -= 2;
    args[2] = 15;
    args[3] = 11;
    break;
  case FAKEBOMB :
    if ((int)this->anim == 0) {
      args[2] = 2;
      args[3] = 2 + this->subtype;
    } else {
      args[2] = 15 + (int)this->anim;
      args[3] = 10 + this->subtype + this->dir*3;
    }
    break;
  case AURADROP :
    args[2] = 9+this->dir*3+this->anim;
    if ((int)this->anim == 3) args[2] -= 2;
    args[3] = 11;
    break;
  case TOPHATSHIELD :
    args[2] = 12;
    args[3] = 14;
    args[4] = 3;
    args[5] = 3;
    break;
  case AURAHELPER :
    args[2] = 11;
    args[3] = 6;
    args[4] = 5;
    args[5] = 5;
    break;
  case BOMBHELPER :
    args[2] = -1;
    break;
  case GHOST :
    if (this->anim >= 3) args[2] = -1;
    else {
      args[2] = 8 + this->dir;
      args[3] = 14 + this->anim;
    }
    break;
  case REINCARNATOR :
    args[2] = 8;
    args[3] = 3 * this->subtype;
    args[5] = 3;
    break;
  case BERET :
    if (this->subtype == 0) {
      args[3] = (int)this->anim%8;
      if (this->jump) {
	args[3] = (this->vy<=0)?8:9;
      }
      args[0] = this->x-6+this->dir;
      args[1] = this->y-6;
      args[2] = 1-this->dir;
      break;
    } // otherwise draw as Top Hat
  case TOPHAT :
    if (!this->jump) {
      args[2] = 7 + (int)this->anim%8;
      args[3] = 18 + this->dir;
    } else {
      args[2] = 15;
      args[3] = (this->vy<=0?16:18) + this->dir;
    }
    args[0] = this->x-6+this->dir;
    args[1] = this->y-6;
    break;
  case SHIELDGEN :
    args[0] = this->x-5;
    args[1] = this->y-5;
    args[3] = 17;
    if ((int)this->anim % 4 == 0) args[2] = 9;
    else if ((int)this->anim == 1 || (int)this->anim == 3) args[2] = 11;
    else if ((int)this->anim == 2) args[2] = 12;
    else if ((int)this->anim == 5 || (int)this->anim == 7) args[2] = 13;
    else args[2] = 14;
    break;
  case WOODBLOCK :
  case SMWOODBLOCK :
  case ICEBLOCK :
  case SMICEBLOCK :
  case STONEBLOCK :
  case SMSTONEBLOCK :
    args[2] = (this->type-WOODBLOCK)%3;
    args[3] = 10+this->subtype+(this->type<SMWOODBLOCK?0:4);
    break;
  case FAKEBLOCK :
    if (this->anim < 1) {
      args[2] = this->subtype;
      args[3] = 10;
    } else {
      args[2] = 12+this->subtype+this->dir*3;
      args[3] = 2+this->anim;
    }
    break;
  case GHOSTBLOCK :
    args[2] = 6;
    args[3] = 9+this->anim;
    break;
  case WEIGHT :
    args[0] -= 2;
    args[2] = 4; 
    args[3] = 13;
    break;
  case BOMB :
    args[2] = 2;
    args[3] = 2+this->subtype;
    break;
  case SIGN :
    args[2] = 7+this->subtype/2;
    args[3] = 10+this->subtype%2;
    break;
  case VSIGN :
    args[2] = 7;
    args[3] = 13+this->subtype;
    break;
  case READSIGN :
    args[2] = 7;
    args[3] = 12;
    break;
  case DOOR :
    if (this->subtype < 2) {
      args[2] = 2;
      args[3] = this->subtype;
    } else {
      args[2] = 8;
      args[3] = 12;
      args[4] = 2;
      args[5] = 2;
    }
    break;
  case FINISHDOOR :
    args[2] = 7;
    args[3] = 8;
    args[4] = 2;
    args[5] = 2;
    break;
  case STICKYBOMB :
    if (this->status == 0) {
      args[2] = 2;
      args[3] = 5;
    } else {
      temp = this->timer < 300 ? 30 : 5;
      if (this->timer % temp < 2) {
        args[2] = 4 + (this->status == CRASHD || this->status == CRASHL);
        args[3] = 11 + (this->status == CRASHU || this->status == CRASHL);
      } else {
        args[2] = 2 + (this->status == CRASHD || this->status == CRASHL);
        args[3] = 8 + (this->status == CRASHU || this->status == CRASHL);
      }
    }
    break;
  case WHITEDIE :
  case BLACKDIE :
    args[2] = this->subtype%3;
    args[3] = 16+2*(this->type-WHITEDIE)+this->subtype/3;
    break;
  case FLOATBLOCK :
    args[2] = 6;
    args[3] = 8;
    break;
  case GRAVITYBLOCK :
    args[2] = 3+this->subtype%2;
    args[3] = 16+this->subtype/2;
    break;
  case ANNOYINGBLOCK :
    args[2] = 7;
    args[3] = 6+this->subtype;
    break;
  case TELEBLOCK :
    args[2] = 5;
    args[3] = 10-this->status/2;
    break;
  case LINKBLOCK :
    args[2] = 3+this->subtype%2;
    args[3] = 18+this->subtype/2;
    break;
  case ANTISEEKER :
    args[2] = 10+this->dir;
    args[3] = 16;
    break;
  case MATTERFRAG :
    args[0] = this->x-1;
    args[1] = this->y-1;
    if ((this->subtype % 3) < 2) {
      args[2] = 10+(this->subtype%3);
      args[3] = 15;
    } else {
      args[2] = 10;
      args[3] = 17;
    }
    break;
  case MATTERLY :
    args[2] = 10+this->dir;
    args[3] = 14;
    break;
  case TELESEEKER :
    args[2] = 9+this->dir;
    if (this->subtype == 2) args[3] = 9;
    else if (this->subtype == 3) args[3] = 10;
    else args[3] = (this->subtype == 1 ? 6 : 3)+this->anim;
    break;
  case ROBOT :
    args[2] = 9+this->dir+3*this->subtype;
    args[3] = this->anim;
    break;
  case HOPPER :
    args[2] = 14+this->dir+2*this->subtype;
    args[3] = this->status/4.2;
    break;
  case CARRIER :
    args[2] = 11;
    args[3] = 3+this->anim;
    break;
  case SPIKEBALL :
    args[2] = 11;
    args[3] = this->anim;
    break;
  case ANTIFECTLING :
    args[2] = 7;
    args[3] = 17;
    break;
  case INFECTLING :
    args[2] = 5;
    args[3] = 13 + this->subtype;
    break;
  }
}


// find an empty spot in the array of things, and return the
// index of that spot or -1 if no spot is found
int find_empty(Thing things[250]) {
  int i;
  for (i=0;i<250;i++)
    if (things[i].type == NOTYPE) return i;
  return -1;
}


// removes this thing, probably with a fancy explosion
void destroy_thing(Thing* this, int method, 
                   Thing things[250], int index) {
  int i, temp;
  float cx = this->x+this->width/2, cy = this->y+this->height/2;
  float vx = cap_val(this->vx/8,3), vy = cap_val(this->vy/8,3);
  
  if (this->link > -1 && this->link < 250) {
    if (this->type == AURADROP && things[this->link].type == AURAHELPER) {
      if (method == PIT) 
	destroy_thing(&things[this->link], PIT, things, this->link);
      else {
	things[this->link].vx = cap_val(this->vx,12);
	things[this->link].vy = cap_val(this->vy,12);
	things[this->link].startx = cx;
	things[this->link].starty = cy;
	things[this->link].status = 1;
      }
    } else if (this->type == LINKBLOCK && (this->subtype == 0 || this->subtype == 2) && 
	       things[this->link].type == PLATFORM) {
      things[this->link].startx = things[this->link].x;
      things[this->link].starty = things[this->link].y;
    }
  }

      

  if (!this->dead) {

    // Sound effects for deaths.
    if (on_screen(this->x, this->y, this->width, this->height) &&
	!(this->type == BOMB || this->type == STICKYBOMB || this->type == FAKEBOMB || this->type == FIREBALL)) {
      if (this->type == GHOST || method == TOUCH || method == BOMBED || (method >= CRASHU && method <= CRASHL))
	play_sound(SND_POP+rand_to(3));
      else if (method == INSIDE) {
	if (this->type == BERET) play_sound(SND_SQUELCH+rand_to(3));
	else play_sound(SND_CRUNCH+rand_to(3));
      } else if (method == COLLECT) play_sound(SND_COLLECT+rand_to(6));
      else if (method == INFECT) play_sound(SND_INFECT);
    }

    if (this->type == TOPHAT) {
      make_thing(THINGMAX-1, DOOR, 2, 585, 900, 0, things);
      make_thing(THINGMAX-2, DOOR, 2, 975, 900, 0, things);
      make_thing(THINGMAX-3, DOOR, 2, 1425, 900, 0, things);
      make_thing(THINGMAX-4, DOOR, 2, 1845, 900, 0, things);
      things[THINGMAX-1].dir = 7;
      things[THINGMAX-2].dir = 7;
      things[THINGMAX-3].dir = 7;
      things[THINGMAX-4].dir = 7;
      make_expl(615,930,0,0,GRAY,10,300);
      make_expl(1005,930,0,0,GRAY,10,300);
      make_expl(1455,930,0,0,GRAY,10,300);
      make_expl(1875,930,0,0,GRAY,10,300);
      set_beat_last_level();
    } else if (this->type == BLOCKSTER) {
      if (bossvars[4] > 25) {
        bossvars[4] -= 75;
        bossvars[1]++;
      }
      if (++bossvars[3] == BLOCKSTER_COUNT) {
        bossvars[BOSSMAX-1] = 1;
        make_thing(THINGMAX-1, FINISHDOOR, 0, 1155,
                   450, 0, things);
        make_expl(1185,480,0,0,WHITE,10,600);
      }
    } else if (this->type == MATTERLY) {
      bossvars[BOSSMAX-1] = 1;
      make_thing(THINGMAX-1, FINISHDOOR, 0, 360, 810, 0, things);
      make_expl(390, 840, 0, 0, WHITE, 10, 600);
    } else if (this->type == STONEY) {
      if (this->subtype != 1) {
        temp = closest_thing(things, cx, cy, &bossvars[1], STONEY);
        bossvars[1]--;
      } else if (this->subtype == 1) {
        temp = closest_thing(things, cx, cy, &bossvars[1], STONEY);
        if (temp > -1) {
          things[temp].subtype = 1;
          things[temp].anim = 2.5;
          things[temp].status = -1;
          bossvars[0] = 0;
        }
      }
      if (bossvars[1] == 0) {
        make_thing(THINGMAX-1, FINISHDOOR, 0, 750, 
                   660, 0, things);
        make_expl(780,690,0,0,WHITE,10,600);
        bossvars[BOSSMAX-1] = 1;
      } else if (bossvars[1] <= 15) {
        for (temp=0; temp<THINGMAX; temp++) {
          if (things[temp].type == STONEY &&
              things[temp].subtype < 2 && temp != index) {
            things[temp].subtype = 2;
            make_expl(things[temp].x+things[temp].width/2,
                      things[temp].y+things[temp].height/2,
                      0,0,RED,5,100);
          }
        }
      }
    }

    if (this->type == MEDALFRAGMENT && method != COLLECT && this->subtype < 2) kill_fragment();
  }

  this->vx = 0;
  this->vy = 0;
  this->nomove = 1;

  if (this->islinked > -1) {
    this->islinked = -1;
  }
  for (i=0;i<250;i++) {
    if (things[i].link == index && things[i].type == LINKBLOCK) {
      things[i].link = -1;
      things[i].islinked = -1;
    }
  }

  if (method != PIT && !this->dead) {
    switch (this->type) {
    case SHIELDGEN :
      bossvars[0]++;
      make_expl(bossvars[1]+45,bossvars[2]+45,0,0,RED,10,150);
      make_expl(bossvars[1]+45,bossvars[2]+45,0,0,PURPLE,8,150);
      make_expl(cx,cy,vx,vy,PURPLE,5,75);
      make_expl(cx,cy,vx,vy,RED,5,10);
      break;
    case TOPHATSHIELD :
      make_expl(cx,cy,vx,vy,PURPLE,10,150);
      make_expl(cx,cy,vx,vy,TRANSPARENT,8,200);
      break;
    case GHOST :
      make_expl(cx,cy,vx,vy,TRANSPARENT,4,75);
      break;
    case REINCARNATOR :
      make_expl(cx,cy,vx,vy,GRYELLOW,5,75);
      make_expl(cx,cy,vx,vy,BROWN,5,75);
      break;
    case GRAVITYSWITCH :
      if (this->subtype == 0) make_expl(cx,cy,vx,vy,BROWN,5,100);
      else make_expl(cx,cy,vx,vy,PURPLE,5,100);
      make_expl(cx,cy,vx,vy,BLUE,5,25);
      break;
    case TURRET :
      make_expl(cx,cy,vx,vy,ORANGE,5,50);
      make_expl(cx,cy,vx,vy,YELLOW,5,50);
      make_expl(cx,cy,vx,vy,GRAY,5,25);
      break;
    case FIREBALL :
      make_expl(cx,cy,vx,vy,ORANGE,1,6);
      make_expl(cx,cy,vx,vy,RED,1,6);
      break;
    case SOLIDSWITCH :
      if (this->subtype == 0) make_expl(cx,cy,vx,vy,GREEN,5,100);
      else make_expl(cx,cy,vx,vy,RED,5,100);
      make_expl(cx,cy,vx,vy,ORANGE,5,25);
      break;
    case SOLIDBLOCK :
      if (this->subtype/2 == 0) make_expl(cx,cy,vx,vy,GREEN,5,100);
      else make_expl(cx,cy,vx,vy,RED,5,100);
      break;
    case PLATFORM :
      switch (this->subtype) {
      case 0: make_expl(cx,cy,vx,vy,PURPLE,4,100); break;
      case 1: make_expl(cx,cy,vx,vy,AQUA,4,100); break;
      case 2: make_expl(cx,cy,vx,vy,GREEN,6,150); break;
      case 3: make_expl(cx,cy,vx,vy,RED,6,150); break;
      }
      break;
    case BERET :
      make_expl(cx,cy,vx,vy,this->subtype?PURPLE:GREEN,8,150);
      make_expl(cx,cy,vx,vy,this->subtype?GRAY:SKY,6,75);
      make_expl(cx,cy,vx,vy,BROWN,7,25);
      break;
    case TOPHAT :
      make_expl(cx,cy,vx,vy,PURPLE,17,300);
      make_expl(cx,cy,vx,vy,GRAY,16,300);
      make_expl(cx,cy,vx,vy,BROWN,15,300);
      break;
    case FAKEBLOCK :
      if (this->subtype == 0) make_expl(cx,cy,vx,vy,BROWN,5,125);
      else if (this->subtype == 1) make_expl(cx,cy,vx,vy,AQUA,5,125);
      else make_expl(cx,cy,vx,vy,GRAY,5,125);
      break;
    case WOODBLOCK :
      make_expl(cx,cy,vx,vy,BROWN,5,125);
      break;
    case BIGBLOCK :
      make_expl(cx,cy,vx,vy,(this->subtype%2==0?GRAY:BROWN),
                (this->subtype>1?8:5),(this->subtype>1?250:125));
      break;
    case ICEBLOCK :
      make_expl(cx,cy,vx,vy,AQUA,5,125);
      break;
    case STONEY :
    case BLACKDIE :
    case STONEBLOCK :
      make_expl(cx,cy,vx,vy,GRAY,5,125);
      break;
    case SMWOODBLOCK :
      make_expl(cx,cy,vx,vy,BROWN,3,100);
      break;
    case SMICEBLOCK :
      make_expl(cx,cy,vx,vy,AQUA,3,75);
      break;
    case WEIGHT :
    case SMSTONEBLOCK :
      make_expl(cx,cy,vx,vy,GRAY,3,100);
      break;
    case WHITEMEDAL :
      make_expl(cx,cy,vx,vy,YELLOW,5,50);
    case WHITEDIE :
      make_expl(cx,cy,vx,vy,WHITE,5,125);
      break;
    case GHOSTBLOCK :
      make_expl(cx,cy,vx,vy,TRANSPARENT,5,125);
      break;
    case MEDALCORNER :
      make_expl(cx,cy,vx,vy,ORANGE,3,100);
      make_expl(cx,cy,vx,vy,BROWN,3,40);
      break;
    case MEDALFRAGMENT :
      make_expl(cx,cy,vx,vy,AQUA,2,20);
      make_expl(cx,cy,vx,vy,BLUE,2,15);
      break;
    case GRAVITYBLOCK :
      make_expl(cx,cy,vx,vy,BLUE,5,25);
    case FLOATBLOCK :
      make_expl(cx,cy,vx,vy,YELLOW,5,100);
      break;
    case BLOCKSTER :
    case ANNOYINGBLOCK :
      make_expl(cx,cy,vx,vy,RED,5,125);
      break;
    case TELEBLOCK :
      make_expl(cx,cy,vx,vy,GREEN,5,40);
      make_expl(cx,cy,vx,vy,PURPLE,5,85);
      break;
    case SPIKEBLOCK :
      make_expl(cx,cy,vx,vy,GRAY,3,25);
      if (this->subtype > 1) make_expl(cx,cy,vx,vy,GRAY,3,75);
      else make_expl(cx,cy,vx,vy,BROWN,3,75);
      break;
    case LINKBLOCK :
      switch (this->subtype) {
      case 0 : make_expl(cx,cy,vx,vy,PINK,5,125); break;
      case 1 : make_expl(cx,cy,vx,vy,GREEN,5,125); break;
      case 2 : make_expl(cx,cy,vx,vy,AQUA,5,125); break;
      case 3 : make_expl(cx,cy,vx,vy,PURPLE,5,125); break;
      }
      break;
    case TELESEEKER :
      if (this->subtype == 0) make_expl(cx,cy,vx,vy,RED,3,75);
      else if (this->subtype == 1) make_expl(cx,cy,vx,vy,GRYELLOW,3,75);
      else if (this->subtype == 2) make_expl(cx,cy,vx,vy,AQUA,3,75);
      else make_expl(cx,cy,vx,vy,PURPLE,3,75);
      break;
    case ROBOT :
      if (this->subtype == 0) make_expl(cx,cy,vx,vy,GRAY,5,125);
      else make_expl(cx,cy,vx,vy,PINK,5,125);
      break;
    case HOPPER :
      if (this->subtype == 0) make_expl(cx,cy,vx,vy,AQUA,3,75);
      else make_expl(cx,cy,vx,vy,YELLOW,3,75);
      break;
    case CARRIER :
      make_expl(cx,cy,vx,vy,YELLOW,3,60);
      make_expl(cx,cy,vx,vy,GRAY,3,15);
      break;
    case SPIKEBALL :
      make_expl(cx,cy,vx,vy,RED,5,65);
      make_expl(cx,cy,vx,vy,ORANGE,5,65);
      break;
    case AURADROP :
      make_expl(cx,cy,vx,vy,RED,5,125);
      break;
    case INFECTLING :
      make_expl(cx,cy,vx,vy,this->subtype?WHITE:GRAY,3,100);
      break;
    case ANTIFECTLING :
      make_expl(cx,cy,vx,vy,PURPLE,3,100);
      break;
    case MATTERLY : case ANTISEEKER : case MATTERFRAG : case ANTIMATTER :
      make_expl(cx,cy,vx,vy,PURPLE,4,80);
      break;
    case FAKEBOMB :
    case STICKYBOMB :
    case BOMB :
      temp = (this->type == BOMB || this->type == FAKEBOMB) ? 
        (this->subtype == 0 ? ORANGE :
         (this->subtype == 1 ? RED : GRYELLOW)) : BLUE;
      if (temp == GRYELLOW) play_sound(SND_BOOM+2);
      else if (temp == RED) play_sound(SND_BOOM+1);
      else play_sound(SND_BOOM);
      if (method == TOUCH)
        make_expl(cx,cy,vx,vy,temp,4,115);
      else if ((temp = find_empty(things)) > -1)
        make_thing(temp, BOMBHELPER, 
                   (this->type == BOMB || this->type == FAKEBOMB) ?
                   this->subtype : 3, this->x, this->y,0,things);
      break;
    }
  }

  this->dead = 1;
  if (this->animate && this->type != BERET) check_room_dead();
}


void make_beret(Thing* this, int type, int subtype,
                int x, int y, int dir, Thing things[250]) {
  this->type = type;
  this->subtype = subtype;
  this->x = x;
  this->y = y;
  this->startx = x;
  this->starty = y;
  this->stickx = 0;
  this->sticky = 0;
  this->vx = 0;
  this->vy = 0;
  this->jump = 0;
  this->dead = 0;
  this->timer = -1;
  this->status = 0;
  this->link = -1;
  this->islinked = -1;
  this->anim = 0;
  this->telething = 0;
  this->icy = 0;
  this->gravmod = DOWN;
  this->floating = 0;
  this->solid = 1;
  this->nomove = 1;
  this->nohmove = 0;
  this->novmove = 0;
  this->infront = 0;
  this->inback = 0;
  this->width = 20;
  this->height = 24;
  this->pickup = 0;
  this->dir = dir;
  this->speed = WALK_SPEED;
  this->crashspeed = -1;
  this->animate = 1;
}


// returns the value for the object parameter for objects
// of the given type
int get_param(int type, int dir, int link) {
  int param = 0;
  if (type >= TELESEEKER && type < TYPEMAX) param = dir;
  else {
    switch (type) {
    case LINKBLOCK : param = link; break;
    }
  }
  return param;
}


// copies the given Thing into the given array at the given index
// puts it into an empty spot if index = -1
// returns the index at which the Thing can be found
int copy_thing(Thing* this, Thing things[250], int index) {
  int param = get_param(this->type, this->dir, this->link);

  if (index == -1) index = find_empty(things);
  if (index > -1) {
    make_thing(index, this->type, this->subtype,
               this->x, this->y, param, things);
    things[index].islinked = this->islinked;
    if (this->type == DOOR || this->type == SIGN ||
        this->type == READSIGN || this->type == MEDALFRAGMENT) {
      things[index].dir = this->dir;
      things[index].timer = this->timer;
      things[index].status = this->status;
    }
  }
  return index;
}


void make_thing(int index, int type, int subtype,
                int x, int y, int param, Thing things[250]) {
  Thing* this = &things[index];

  this->type = type;
  this->subtype = subtype;
  this->x = x;
  this->y = y;
  this->startx = x;
  this->starty = y;
  this->stickx = 0;
  this->sticky = 0;
  this->vx = 0;
  this->vy = 0;
  this->speed = 0;
  this->jump = 0;
  this->dir = 0;
  this->dead = 0;
  this->timer = 0;
  this->status = 0;
  this->link = -1;
  this->anim = 0;
  this->telething = 0;
  this->width = 30;
  this->height = 30;
  this->pickup = 1;
  this->icy = 0;
  this->gravmod = NONE;
  this->floating = 0;
  this->solid = 1;
  this->deathtouch = 0;
  this->nomove = 1;
  this->nohmove = 0;
  this->novmove = 0;
  this->infront = 0;
  this->inback = 0;
  this->crashspeed = -1;
  this->animate = 0;

  switch (type) {
  case SHIELDGEN :
    this->width = 20;
    this->height = 20;
    this->floating = 1;
    this->animate = 1;
    this->deathtouch = 1;
    break;
  case SOLIDBLOCK :
    this->infront = 1;
    if (this->subtype%2 == 0) this->anim = 5;
    else this->anim = 0;
    this->solid = 0;
  case GRAVITYSWITCH :
  case SOLIDSWITCH :
    this->pickup = 0;
    this->floating = 1;
    break;
  case PLATFORM :
    this->pickup = 0;
    this->floating = 1;
    if (this->subtype > 1) {this->width = 60;this->height = 17;}
    else this->height = 15;
    break;
  case BIGBLOCK :
    if (subtype > 1) {
      this->width = 60;
      this->height = 60;
    }
    if (subtype % 2 == 0) this->pickup = 0;
    break;
  case SMSTONEBLOCK :
    this->width = 15;
    this->height = 15;
    this->pickup = 0;
    break;
  case SMICEBLOCK :
    this->icy = 1;
  case INFECTLING :
  case ANTIFECTLING :
  case SMWOODBLOCK :
    this->width = 15;
    this->height = 15;
    break;
  case WEIGHT :
    this->height = 16;
    this->width = 26;
    break;
  case ICEBLOCK :
    this->icy = 1;
    break;
  case STONEBLOCK :
    this->pickup = 0;
    break;
  case FLOATBLOCK :
    this->floating = 1;
    break;
  case GRAVITYBLOCK :
    this->gravmod = subtype;
    break;
  case ANNOYINGBLOCK :
    this->floating = 1;
    this->pickup = 0;
    this->nohmove = subtype;
    this->novmove = !subtype;
    break;
  case TELEBLOCK :
    this->floating = 1;
    this->pickup = 0;
    this->solid = 0;
    this->infront = 1;
    break;
  case LINKBLOCK : // set param = index of link
    this->link = param;
    if (subtype % 2 == 0) {
      if (param > -1) things[param].islinked = index;
    } else {
      this->islinked = param;
    }
    break;
  case MEDALFRAGMENT :
    this->width = 10;
    this->height = 10;
    this->dir = -1;
    break;
  case MEDALCORNER :
    this->width = 15;
    this->height = 15;
  case WHITEMEDAL :
    this->floating = 1;
    this->solid = 0;
    this->pickup = 0;
    this->inback = 1;
    break;
  case FAKEBLOCK :
    if (subtype == 2) this->pickup = 0;
    else if (subtype == 1) this->icy = 1;
    this->deathtouch = 1;
    this->animate = 1;
    break;
  case TOPHAT :
    this->width = 20;
    this->height = 24;
    this->pickup = 0;
    this->animate = 1;
    this->deathtouch = 1;
    break;
  case REINCARNATOR :
    this->pickup = 0;
    this->floating = 1;
    this->inback = 1;
    this->solid = 0;
    this->height = 90;
    break;
  case GHOST :
    this->solid = 0;
    this->floating = 1;
    this->deathtouch = 1;
    this->crashspeed = ENEMYCRASH;
    this->width = 16;
    this->height = 16;
    this->animate = 1;
    break;
  case TELESEEKER : case ANTISEEKER :
    this->floating = 1;
    this->deathtouch = 1;
    this->width = 16;
    this->height = 19;
    this->animate = 1;
    break;
  case AURADROP : case ROBOT :
    this->deathtouch = 1;
    this->crashspeed = ENEMYCRASH;
  case TURRET :
    this->animate = 1;
    break;
  case HOPPER :
    this->deathtouch = 1;
    this->crashspeed = ENEMYCRASH;
    this->animate = 1;
    this->width = 15;
    this->height = 19;
    break;
  case CARRIER :
    this->width = 15;
    this->height = 18;
    break;
  case SPIKEBLOCK :
    this->deathtouch = 1;
    this->pickup = this->subtype < 2;
    break;
  case MATTERLY :
  case STONEY :
  case BLOCKSTER :
    if (bossvars[BOSSMAX-1]) {
      this->type = NOTYPE;
    } else {
      this->deathtouch = 1;
      this->animate = 1;
      this->pickup = (this->type == STONEY);
      this->floating = (this->type != BLOCKSTER);
    }
    break;
  case MATTERFRAG :
    this->width = 10;
    this->height = 10;
    this->deathtouch = 1;
    break;
  case SPIKEBALL :
    this->deathtouch = 1;
    this->floating = 1;
    this->crashspeed = ENEMYCRASH;
    this->animate = 1;
    break;
  case STICKYBOMB :
    this->width = 15;
    this->height = 15;
    break;
  case FAKEBOMB :
    this->deathtouch = 1;
    this->animate = 1;
  case BOMB :
    this->width = 18;
    this->height = 18;
    this->crashspeed = 8;
    break;
  case ANTIMATTER :
    this->width = 18;
    this->height = 15;
    break;
  case FINISHDOOR :
    if (bossvars[BOSSMAX-1] || subtype == 0 || x == 0) {
      this->width = 60;
      this->height = 60;
    } else {
      this->type = NOTYPE;
      break;
    }
  case SIGN :
  case VSIGN :
  case READSIGN :
  case DOOR :
    this->floating = 1;
    this->solid = 0;
    this->inback = 1;
    this->pickup = 0;
    if (this->type == DOOR && this->subtype == 2) {
      this->width = 60;
      this->height = 60;
    }
    break;
  case FIREBALL :
    this->width = 12;
    this->height = 12;
    this->pickup = 0;
    this->dir = param;
    this->vx = this->dir ? TURRET_SHOOT_SPEED : -TURRET_SHOOT_SPEED;
    this->crashspeed = 0;
    break;
  case TOPHATSHIELD :
    this->width = SPR_SIZE*3;
    this->height = SPR_SIZE*3;
    this->solid = 0;
    this->floating = 1;
    this->infront = 1;
    this->pickup = 0;
    break;
  case AURAHELPER :
    this->width = SPR_SIZE*5;
    this->height = SPR_SIZE*5;
    this->solid = 0;
    this->floating = 1;
    this->infront = 1;
    this->pickup = 0;
    break;
  case BOMBHELPER :
    this->width = this->subtype == 3 ? 15 : 18;
    this->height = this->subtype == 3 ? 15 : 18;
    this->solid = 0;
    this->floating = 1;
    this->pickup = 0;
    break;
  }

  if (this->animate) {
    this->gravmod = DOWN;
    this->dir = param;
  }
}


void fix_vel(Thing* this) {
  if ((this->collide[LEFT] && this->vx < 0) ||
      (this->collide[RIGHT] && this->vx > 0)) this->vx = 0;
  if ((this->collide[UP] && this->vy < 0) ||
      (this->collide[DOWN] && this->vy > 0)) this->vy = 0;
}


float cap_val(float val, float cap) {
  if (val < -cap) return -cap;
  if (val > cap) return cap;
  return val;
}


void move_fake(Thing *this, int anims, float bounceh, 
               int bcx, int bcy, int tcx, int tcy) {
  if (abs(bcx-tcx) < 9*SPR_SIZE && abs(bcy-tcy) < 6*SPR_SIZE &&
      !this->telething && this->islinked == -1 && 
      !(bcx <= tcx && this->collide[LEFT]) && 
      !(bcx >= tcx && this->collide[RIGHT])) {
    this->deathtouch = 1;
    this->anim = approach(this->anim, anims, 0.3);
    if (this->anim < anims && this->anim >= anims - 0.3) {
      play_sound(SND_FAKE+rand_to(2));
    }
    this->dir = bcx > tcx;
    if (abs(bcx-tcx) > 6) this->vx = approach(this->vx, cap_val(bcx-tcx, 8), 0.75);
    if (!this->jump && abs(this->vx) < 10 && this->timer == 0) {
      this->vy = -bounceh;
      this->timer = 10;
    }
    fix_vel(this);
  } else {
    this->deathtouch = 0;
    this->anim = approach(this->anim, 0, 0.3);
  }
  if (this->timer > 0) this->timer--;
}


void move_bnf(Thing* this, int updown, float speed, float accel) {
  int a = updown?UP:LEFT, b = updown?DOWN:RIGHT;
  if (this->dir == 0) {
    if (!updown) this->vx = approach(this->vx, -speed, accel);
    else this->vy = approach(this->vy, -speed, accel);
    if (this->collide[a] && !this->collide[b] &&
        this->timer == 0) {
      this->dir = 1;
      this->timer = 10;
    }
  } else {
    if (!updown) this->vx = approach(this->vx, speed, accel);
    else this->vy = approach(this->vy, speed, accel);
    if (this->collide[b] && !this->collide[a] &&
        this->timer == 0) {
      this->dir = 0;
      this->timer = 10;
    }
  }
  if (this->timer > 0) this->timer--;
}


void special_thing(Thing* this, Thing things[250], 
                   Thing* beret, int *gravdir, int teleused,
                   int index, int *switchflags) {
  int temp, temp2, temp3;
  float angle;
  int bcx = beret->x+beret->width/2, bcy = beret->y+beret->height/2;
  int tcx = this->x+this->width/2, tcy = this->y+this->height/2;

  switch (this->type) {
  case SHIELDGEN :
    this->anim += 0.4;
    if (this->anim >= 8) this->anim = 0;
    break;
  case TOPHATSHIELD :
    this->x = bossvars[1];
    this->y = bossvars[2];
    if (bossvars[0] == SHIELDGENNUM) {
      destroy_thing(this, TOUCH, things, index);
    }
    break;
  case TURRET :
    if (++this->status >= 12) this->status = 0;
    this->anim = this->status/2;
    if ((this->status == 0 || this->status == 6) &&
        (temp = find_empty(things)) > -1) {
      if (on_screen(this->x, this->y, this->width, this->height))
	play_sound(SND_TURRET+rand_to(3));
      make_thing(temp, FIREBALL, 0, 
		 this->x+(this->dir?this->width-2:-10),
		 this->y+(this->status==6?16:6),
		 this->dir,things);
      make_expl(this->x+(this->dir?this->width-7:7),
		this->y+(this->status == 6?this->height-4:4),
		this->dir?3:-3,0,RED,2,10);
      things[temp].status = index;
      if (*gravdir == DOWN) things[temp].vy = -1;
      if (*gravdir == UP) things[temp].vy = 1;
    }
    break;
  case FIREBALL :
    if (this->vx == 0 && this->vy == 0) {
      destroy_thing(this, PIT, things, index);
    }
    break;
  case PLATFORM :
    if (this->collide[UP]) {
      // status = 2 means we should ignore the collision for one frame
      // status = 1 means the platform has been activated
      // status = 0 means everything is fine and dandy
      if (this->status == 2) this->status = 0;
      else this->status = 1;
    } else if (this->status == 1 && this->dir == 0) {
      this->status = 0;
      this->timer += 30;
      this->dir = 12;
      if (on_screen(this->x, this->y, this->width, this->height))
	play_sound(SND_PLATFORM+rand_to(3));
    }
    temp = this->startx;
    temp2 = this->starty;
    if (this->dir > 0) this->dir--;
    if (this->subtype == 0) temp2 -= this->timer;
    else if (this->subtype == 1) temp2 += this->timer;
    else if (this->subtype == 2) temp += this->timer;
    else if (this->subtype == 3) temp -= this->timer;
    if (this->islinked == -1) {
      this->vx = approach(this->vx, cap_val(temp-this->x,2.5),1.1);
      this->vy = approach(this->vy, cap_val(temp2-this->y,2.5),1.1);
    }
    break;
  case SOLIDBLOCK :
    this->status = (*switchflags&(this->subtype/2==0?GREENFLAG:REDFLAG));
    if (this->subtype%2 == 1) this->status = !this->status;
    if (!this->status && this->anim < 5) this->anim += 0.75;
    else if (this->status && this->anim > 0) this->anim -= 0.75;
    this->solid = (this->anim <= 2);
    break;
  case GRAVITYSWITCH :
    this->anim = *gravdir;
    if (this->islinked == -1) {
      this->vx = approach(this->vx, cap_val(this->startx-this->x,4),1.1);
      this->vy = approach(this->vy, cap_val(this->starty-this->y,4),1.1);
    }
    if (this->timer > 0) this->timer--;
    break;
  case SOLIDSWITCH :
    this->status = (*switchflags&(this->subtype==0?GREENFLAG:REDFLAG));
    if (this->islinked == -1) {
      this->vx = approach(this->vx, cap_val(this->startx-this->x,4),1.1);
      this->vy = approach(this->vy, cap_val(this->starty-this->y,4),1.1);
    }
    if (this->status && this->anim < 5) this->anim += 0.75;
    else if (!this->status && this->anim > 0) this->anim -= 0.75;
    if (this->timer > 0) this->timer--;
    break;
  case WHITEMEDAL : case MEDALCORNER :
    if (((this->timer+index*13) % 30) == 0) this->vy += 1;
    else if (((this->timer+index*13) % 30) == 15) this->vy -= 1;
    this->timer++;
    break;
  case SPIKEBLOCK :
    this->timer++;
    if (this->timer == 300) destroy_thing(this, BOMBED, things, index);
    break;
  case MATTERFRAG :
    if (this->subtype >= 3 && bossvars[2] == 0 && bossvars[BOSSMAX-1] == 0) {
      angle = (index*6+things[bossvars[1]].timer % 90) * PI/45;
      temp3 = 60 + 10*cos(things[bossvars[1]].timer % 360 * PI/180);
      this->vx = 
	approach(this->vx, cap_val(things[bossvars[1]].x+9+cos(angle)*temp3-
				   this->x, 6), 0.75);
      this->vy = 
	approach(this->vy, cap_val(things[bossvars[1]].y+9+sin(angle)*temp3-
				   this->y, 6), 0.75);
    }
    break;
  case TOPHAT :
    switch (this->subtype) {
    case 2 :
      if (this->status == 0) {
	if ((temp = find_empty(things)) > -1) {
	  make_thing(temp,TOPHATSHIELD,0,this->x-30,this->y-30,0,things);
	  this->status = 1;
	}
      }
      bossvars[1] = this->x-37;
      bossvars[2] = this->y-37;
      if (bossvars[4] > 0) {
	bossvars[4]--;
	if (bossvars[4] == 20) bossvars[6] = 0;
      }
      if (bossvars[6] == 0 && bossvars[4] == 0 &&
	  (temp = closest_thing(things, tcx, tcy, 0, NOTYPE)) > -1) {
	bossvars[6] = temp+1;
	bossvars[4] = 40;
      }
      bossvars[7] = bcx;
      bossvars[8] = bcy;
      if (abs(bcy-tcy) < SPR_SIZE*8)
	this->vx = approach(this->vx, cap_val(bcx-tcx, RUN_SPEED), 1.5);
      if (bcx > tcx) this->dir = 1;
      else this->dir = 0;
      if (((this->collide[LEFT] && this->dir == 0) ||
	   (this->collide[RIGHT] && this->dir == 1)) && !this->jump) {
	if (bossvars[3] == 0) {
	  this->vy = -8.25;
	  this->anim = 4;
	  bossvars[3] = 5;
	} else bossvars[3]--;
      }
      break;
    case 1 :
      if (this->x > 600) this->vx = approach(this->vx, -WALK_SPEED, 1.5);
      if (bossvars[0] == 0 && bcx > this->x) {
	bossvars[6] = 1;
	bossvars[7] = this->x + 150;
	bossvars[8] = this->y - 150;
	bossvars[0] = 1;
	bossvars[1] = 100;
	this->dir = 1;
	break;
      }
      if (bossvars[0] == 1) {
	if (bossvars[1] > 0) bossvars[1]--;
	else {
	  this->vx = approach(this->vx, -WALK_SPEED, 1.5);
	  this->dir = 0;
	  if (this->x < 225 && !this->jump)
	    destroy_thing(this, PIT, things, index);
	}
      }
      break;
    case 0 :
      if (bossvars[0] == 0) {
	if (this->x > 680) {
	  this->vx = approach(this->vx, -RUN_SPEED, 1.5);
	  this->dir = 0;
	} else {
	  if (!this->jump) {
	    if (bossvars[1] == 0) {
	      this->vy = -JUMP_SPEED;
	      this->anim = 4;
	      bossvars[1] = 5;
	      bossvars[2] = 0;
	    } else bossvars[1]--;
	  }
	}
      } else {
	if (this->x < 765) {
	  this->vx = approach(this->vx, RUN_SPEED, 1.5);
	  this->dir = 1;
	} else {
	  if (!this->jump) {
	    if (bossvars[1] == 0) {
	      this->vy = -JUMP_SPEED;
	      this->anim = 4;
	      bossvars[1] = 5;
	      bossvars[2] = 0;
	    } else bossvars[1]--;
	  }
	}
      }
      if (bossvars[3] > 0) bossvars[3]--;
      if (bossvars[2] == 0 && bossvars[3] == 0) {
	temp = closest_thing(things, bcx, bcy, 0, SOLIDBLOCK);
	if ((things[temp].subtype < 2) == bossvars[0]) {
	  bossvars[2] = 1;
	  bossvars[3] = 20;
	  bossvars[0] = !bossvars[0];
	}
      }
      break;
    }
    // animation
    if (this->vx != 0) {
      this->anim += abs_f(this->vx)/6;
    } else if ((int)this->anim%4 != 0) {
      this->anim += .3;
    }
    // bossvars[6-8] are used for Top Hat's telekinesis:
    // bossvars[6]: index of telething + 1
    // bossvars[7]: x-pos of telekinesis
    // bossvars[8]: y-pos of telekinesis
    if (bossvars[6] > 0) {
      if (check_can_see(*this, things[bossvars[6]-1]) &&
	  things[bossvars[6]-1].type != NOTYPE &&
	  !things[bossvars[6]-1].telething) {
	things[bossvars[6]-1].vx = approach(things[bossvars[6]-1].vx,
					    cap_val((bossvars[7]-things[bossvars[6]-1].x+
						     things[bossvars[6]-1].width/2)*
						    TELEMODIF,TELECAP), TELEACCEL);
	things[bossvars[6]-1].vy = approach(things[bossvars[6]-1].vy,
					    cap_val((bossvars[8]-things[bossvars[6]-1].y+
						     things[bossvars[6]-1].height/2)*
						    TELEMODIF,TELECAP), TELEACCEL);
	if (things[bossvars[6]-1].solid) {
	  if ((things[bossvars[6]-1].collide[LEFT] && things[bossvars[6]-1].vx < 0) ||
	      (things[bossvars[6]-1].collide[RIGHT] && things[bossvars[6]-1].vx > 0))
	    things[bossvars[6]-1].vx = 0;
	  if ((things[bossvars[6]-1].collide[UP] && things[bossvars[6]-1].vy < 0) ||
	      (things[bossvars[6]-1].collide[DOWN] && things[bossvars[6]-1].vy > 0))
	    things[bossvars[6]-1].vy = 0;
	}
      } else {
	bossvars[6] = 0;
      }
    }
    break;
  case MATTERLY :
    if (this->timer % 250 == 150)
      bossvars[2] = 1;
    else if (this->timer % 250 == 0) bossvars[2] = 0;
    if (bossvars[0] == 0) bossvars[1] = index;
    if (bossvars[0] < 20 && this->timer % 10 == 0) {
      bossvars[0]++;
      if ((temp = find_empty(things)) > -1) {
	angle = (temp*6 + this->timer % 90) * PI/45;
	temp3 = 60;
	make_thing(temp, MATTERFRAG, bossvars[0] % 3 + 3,
		   this->x+9+cos(angle)*temp3,
		   this->y+9+sin(angle)*temp3, 0, things);
	make_expl(this->x+9+cos(angle)*temp3,this->y+9+sin(angle)*temp3,
		  0,0,PURPLE,3,20);
      } else bossvars[0] = 30;
    }
    if (this->timer % 250 > 200 && this->timer % 250 < 220) {
      this->vx = approach(this->vx, cap_val(beret->x-this->x,6),0.75);
      this->vy = approach(this->vy, cap_val(beret->y-this->y,6),0.75);
      this->dir = beret->x > this->x;
    } else if (this->timer % 30 == 0) this->vy += 1.5;
    else if (this->timer % 30 == 15) this->vy -= 1.5;
    this->timer++;
    break;
  case BLOCKSTER :
    if (bossvars[0] == 1) {
      bossvars[0] = bossvars[4] + ((int)(beret->x) % 100);
      bossvars[2] = bossvars[1];
    } else if (bossvars[0] == 0 && beret->x > this->x - 450) {
      bossvars[0] = 1700;
      bossvars[4] = 1500;
      bossvars[1] = 2;
    }
    if (bossvars[2] > 0) {
      if (bossvars[3] > 11 || (this->status == 0 &&
        (this->timer + index) % 3 == 0)) {
        bossvars[2]--;
        this->status = 1;
        this->dir = 0;
        // Pseudorandomize this variable.
        this->timer += bossvars[2] + beret->x + beret->y;
      } else {
        this->timer--;
      }
    }
    if (this->status == 1) {
      // Get the subtype of the spiked block, also pseudorandomly.
      temp2 = ((this->timer+(int)(beret->x))%2) + 
        ((this->timer%3 == 0 || bossvars[3] > 11) ? 0 : 2);
      if (this->anim < 4) {
	this->anim += 0.3;
	if (bossvars[7] == 0) {
	  play_sound(SND_FAKE);
	  bossvars[7] = 1;
	}
      } else {
        if (this->dir < 35) {
          this->dir++;
          if (this->dir % 6 == 0) 
            make_expl(this->x-15, this->y+15, 0, 0,
                      (temp2 < 2 ? WHITE : RED), 4, 40);
        } else {
          make_expl(this->x-15, this->y+15, 0, 0, PURPLE, 5, 75);
          if ((temp = find_empty(things)) > -1) {
	    play_sound(SND_REGENINIT);
            make_thing(temp, SPIKEBLOCK, temp2,
                       this->x-30,this->y,0,things);
            things[temp].vx = -BLOCKSTER_SHOOT_SPEED;
            things[temp].vy = -BLOCKSTER_SHOOT_VSPEED;
          }
          this->status = 0;
	  bossvars[7] = 0;
        }
      }
    } else if (this->anim > 0) this->anim -= 0.3;
    if (bossvars[0] > 0) bossvars[0]--;
    break;
  case STONEY :
    if (this->subtype == 2) {
      this->vx = approach(this->vx, 3*pos_f(bcx - tcx), 0.4);
      this->vy = approach(this->vy, 3*pos_f(bcy - tcy), 0.4);
      if (tcx > bcx) this->dir = 0;
      else this->dir = 1;
    } else if (this->subtype == 1) {
      if (bossvars[0] >= 35 && bossvars[0] < 75) {
      
        this->vx = approach(this->vx, 6*pos_f(bcx - tcx), 0.6);
        this->vy = approach(this->vy, 6*pos_f(bcy - tcy), 0.6);
        if (tcx > bcx) this->dir = 0;
        else this->dir = 1;
      
      } else if (bossvars[0] >= 150 && this->status == 0) {
        this->status = 1;
      }

      bossvars[0]++;

      if (this->telething) this->status = 1;
      if (this->status == 1) {
        if (this->anim >= 2.5) {
          
          temp3 = closest_thing(things, bcx, bcy, 0, STONEY);
          if (temp3 > -1) {
            bossvars[0] = 0;
            things[temp3].subtype = 1;
            things[temp3].anim = 2.5;
            things[temp3].status = -1;
            this->subtype = 0;
            this->status = 0;
            this->anim = index%2;
          }
        } else this->anim += 0.3;
      } else if (this->status == -1) {
        if (this->anim <= 0) this->status = 0;
        else this->anim -= 0.3;
	if (this->anim < 0) this->anim = 0;
      }
    } else {
      this->anim = index%2;
      this->vx = approach(this->vx, 0.3*pos_f(bcx - tcx), 0.18);
      this->vy = approach(this->vy, 0.3*pos_f(bcy - tcy), 0.18);
    }
    if (((this->timer+index*17) % 30) == 0) this->vy += 1.3;
    else if (((this->timer+index*17) % 30) == 15) this->vy -= 1.3;
    fix_vel(this);
    this->timer++;
    break;
  case GHOST :
    temp3 = sqrt(this->vx*this->vx + this->vy*this->vy);
    if (temp3 >= GHOST_TOO_FAST) {
      this->anim = 0;
      this->solid = 1;
    } else {
      this->solid = 0;
      temp = sqrt((bcx-tcx)*(bcx-tcx) + (bcy-tcy)*(bcy-tcy));
      if (temp < GHOST_CHASE_DIST) {
        temp2 = bcx-tcx;
        this->vx = approach(this->vx, 
                            (abs_f(temp2) > 12 ? 1.5*pos_f(temp2) : 0) +
                            ((this->timer + index * 17) % 40 < 20 ? 1 : -1),
                            0.6);
        temp2 = beret->y - tcy;
        this->vy = approach(this->vy, 
                            (abs_f(temp2) > 12 ? 2.5*pos_f(temp2) : 0) + 
                            ((this->timer + index * 13) % 40 < 20 ? 1 : -1),
                            0.6);
        if (bcx > tcx) this->dir = 1;
        else this->dir = 0;
      }
      if (temp >= GHOST_DIST) this->anim = 3;
      else this->anim = 2-((GHOST_DIST-temp)/30);
      if (this->anim < 0) this->anim = 0;
    }
    this->timer++;
    break;
  case ANTISEEKER :
      this->vx = approach(this->vx, 3.5*pos_f(bcx - tcx), 0.5);
      this->vy = approach(this->vy, 3.5*pos_f(bcy - tcy), 0.5);
      if (tcx > bcx) this->dir = 0;
      else this->dir = 1;
    break;
  case TELESEEKER :
    if (this->subtype == 3) {
      this->vx = approach(this->vx, 3.5*pos_f(tcx - bcx), 0.5);
      this->vy = approach(this->vy, 3.5*pos_f(tcy - bcy), 0.5);
      if (bcx > tcx) this->dir = 0;
      else this->dir = 1;
    } else if ((teleused && this->subtype == 0) || 
        (!teleused && this->subtype == 1) ||
        this->subtype == 2) {
      this->anim = 2;
      float dx = bcx - tcx;
      float dy = bcy - tcy;
      if (abs(dx) > 6) this->vx = approach(this->vx, 3.5*pos_f(dx), 0.5);
      if (abs(dy) > 6) this->vy = approach(this->vy, 3.5*pos_f(dy), 0.5);
      if (bcx > tcx) this->dir = 1;
      else if (bcx < tcx) this->dir = 0;
    } else {
      int blinkcheck = (this->timer+index*13)%75;
      if (blinkcheck < 4) this->anim = 2;
      else if (blinkcheck < 8 || blinkcheck > 70) this->anim = 1;
      else this->anim = 0;
      if (((this->timer+index*17) % 30) == 0) this->vy += 2;
      else if (((this->timer+index*17) % 30) == 15) this->vy -= 2;
    }
    fix_vel(this);
    this->timer++;
    break;
  case FAKEBOMB :
    move_fake(this, 2, 8, bcx, bcy, tcx, tcy);
    break;
  case FAKEBLOCK :
    move_fake(this, 3, 2.3, bcx, bcy, tcx, tcy);
    break;
  case GHOSTBLOCK :
    if (abs(this->vx) < 2 && abs(this->vy) < 2) {
      this->solid = 1;
      this->anim = approach(this->anim, 0, 0.3);
    } else {
      this->solid = 0;
      this->anim = approach(this->anim, 2, 0.3);
    }
    break;
  case SPIKEBALL :
    if (this->status == 0) this->anim += 0.3;
    else this->anim -= 0.3;
    if (this->anim <= 0) this->status = 0;
    else if (this->anim >= 2.4) this->status = 1;
    if (!this->telething) {
      if (this->subtype == 0) move_bnf(this, 0, 4, 0.3);
      else move_bnf(this, 1, 4, 0.3);
    } else this->timer = 0;
    break;
  case CARRIER :
    this->anim += 0.3;
    if (this->anim >= 2) this->anim = 0;
    if (!this->jump && !this->telething) move_bnf(this, 0, 4.5, 1.5);
    else this->timer = 0;
    break;
  case ANTIFECTLING :
  case INFECTLING :
    if (this->timer > 0) this->timer--;
    break;
  case STICKYBOMB :
    if (this->status > 0) {
      if (this->link != -1) {
        Thing *stickto = this->link == -2 ?
          beret : &things[this->link];
        if (stickto->dead || stickto->type == NOTYPE || !stickto->solid ||
            stickto->type == INFECTLING || stickto->type == STICKYBOMB)
            destroy_thing(this, BOMBED, things, index);
        this->x = stickto->x+this->stickx;
        this->y = stickto->y+this->sticky;
      } else {
        this->x = this->stickx;
        this->y = this->sticky;
      }
      if (this->timer == 375) destroy_thing(this, BOMBED, things, index);
      this->timer++;
      temp = this->timer < 300 ? 30 : 5;
      if (this->timer % temp == 0) play_sound(SND_TICK+rand_to(3));
    }
    break;
  case AURAHELPER :
    if (this->status == 0) {
      if (things[this->link].type == AURADROP) {
        this->x = things[this->link].x-60;
        this->y = things[this->link].y-60;
      }
    } else {
      this->vx = approach(this->vx, cap_val(this->startx-(this->x+this->width/2),6),1);
      this->vy = approach(this->vy, cap_val(this->starty-(this->y+this->height/2),6),1);
    }
    break;
  case BOMBHELPER :
    if (this->timer == (this->subtype == 0 ? 8 :
                        (this->subtype == 1 ? 18 :
                         (this->subtype == 2 ? 30 : 8))))
      destroy_thing(this, PIT, things, index);
    this->x -= 6;
    this->y -= 6;
    this->width += 12;
    this->height += 12;
    temp = (this->subtype == 0 ? ORANGE :
             (this->subtype == 1 ? RED :
              (this->subtype == 2 ? GRYELLOW : BLUE)));
    temp2 = (this->subtype == 0 ? BROWN :
             (this->subtype == 1 ? YELLOW :
              (this->subtype == 2 ? GREEN : AQUA)));
    if (this->timer % 3 < 2) 
      make_circle(tcx, tcy, temp, temp2, this->width/2, 200);
    this->timer++;
    break;
  case AURADROP :
    if (this->status == 0 && (temp = find_empty(things)) > -1) {
      make_thing(temp, AURAHELPER,0,this->x-60,this->y-60,0,things);
      things[temp].link = index;
      this->status = 1;
      this->link = temp;
    }
    if (!this->jump && !this->telething) {
      move_bnf(this, 0, 2.5, 1);
    } else this->timer = 0;
    this->anim += 0.2;
    if (this->anim >= 4) this->anim = 0;
    fix_vel(this);
    break;
  case ROBOT :
    if (!this->jump && !this->telething) {
      move_bnf(this, 0, this->subtype?4.5:2.3, 1);
    } else this->timer = 0;
    this->anim += 0.3;
    if (this->anim >= 3) this->anim = 0;
    fix_vel(this);
    break;
  case HOPPER :
    if (bcx > tcx) this->dir = 1;
    else if (bcx < tcx) this->dir = 0;
    if (!this->telething && this->islinked == -1) {
      if (!this->jump) {
        if (this->status >= 12 && !this->jumpdelay) {
          this->status = 0;
          this->vy = (this->subtype?-8.5:-5.25);
          this->jump = 1;
	  if (on_screen(this->x, this->y, this->width, this->height))
	    play_sound(SND_HOP+rand_to(3));
          this->vx = (this->dir?1:-1)*(this->subtype?4:3.25);
        } else if (++this->status > 12) this->status = 0;
      } else if (this->vx == 0 && this->vy <= 0) {
        this->vx = (this->dir?2.5:-2.5);
      }
      fix_vel(this);
    }
    break;
  case ANNOYINGBLOCK :
    if (this->subtype == 0) {
      this->vx = approach(this->vx, cap_val(bcx - tcx,7.5), 2.5);
    } else {
      this->vy = approach(this->vy, cap_val(bcy - tcy,7.5), 2.5);
    }
    fix_vel(this);
    break;
  case TELEBLOCK :
    if (teleused) {
      if (this->status < 5) this->status++;
      this->solid = 1;
    } else {
      if (this->status > 0) this->status--;
      this->solid = 0;
    }
    break;
  case LINKBLOCK :
    if (things[this->link].type == NOTYPE) this->link = -1;
    if (this->link > -1) {
      switch (this->subtype) {
      case 0 :
        things[this->link].vx=approach(things[this->link].vx,
                                       cap_val(things[this->link].startx+this->x
                                               -this->startx-things[this->link].x,LINKSPEED),LINKACCEL);
        things[this->link].vy=approach(things[this->link].vy,
                                       cap_val(things[this->link].starty+this->y
                                               -this->starty-things[this->link].y,LINKSPEED),LINKACCEL);
        fix_vel(&things[this->link]);
        break;
      case 1 :
        if (things[this->link].type != NOTYPE) {
          this->vx=approach(this->vx, cap_val(-things[this->link].startx-this->x
                                              +this->startx+things[this->link].x,LINKSPEED), LINKACCEL);
          this->vy=approach(this->vy, cap_val(-things[this->link].starty-this->y
                                              +this->starty+things[this->link].y,LINKSPEED), LINKACCEL);
          fix_vel(this);
        } else {
          this->islinked = -1;
        }
        break;
      case 2 :
        things[this->link].vx=approach(things[this->link].vx,
                                       cap_val(things[this->link].startx-this->x
                                               +this->startx-things[this->link].x,LINKSPEED),LINKACCEL);
        things[this->link].vy=approach(things[this->link].vy,
                                       cap_val(things[this->link].starty-this->y
                                               +this->starty-things[this->link].y,LINKSPEED),LINKACCEL);
        fix_vel(&things[this->link]);
        break;
      case 3 :
        if (things[this->link].type != NOTYPE) {
          this->vx=approach(this->vx,cap_val(things[this->link].startx-this->x
                                             +this->startx-things[this->link].x,LINKSPEED),LINKACCEL);
          this->vy=approach(this->vy,cap_val(things[this->link].starty-this->y
                                             +this->starty-things[this->link].y,LINKSPEED),LINKACCEL);
          fix_vel(this);
        } else {
          this->islinked = -1;
        }
        break;
      }
    }
    break;
  }
}


void check_crash(Thing* this, Thing* other, int oindex, float vel,
                 int crashtype, Thing things[250], int *gravdir,
                 int *switchflags, int thisindex, int playsound) {
  if (!(*switchflags & CREATORFLAG)) {
    if (this->crashspeed > -1 && vel > this->crashspeed) {
      destroy_thing(this, crashtype, things, thisindex);
    }

    // Sound effect handling for collisions.
    if (playsound && vel > 8 && !(other && other->type == FIREBALL) &&
	on_screen(this->x, this->y, this->width, this->height) &&
	((crashtype == CRASHU && this->vy < 0 &&
	  abs_f(this->vy) > abs_f(this->vx)) ||
	 (crashtype == CRASHD && this->vy > 0 &&
	  abs_f(this->vy) > abs_f(this->vx)) ||
	 (crashtype == CRASHL && this->vx < 0 &&
	  abs_f(this->vx) > abs_f(this->vy)) ||
	 (crashtype == CRASHR && this->vx > 0 &&
	  abs_f(this->vx) > abs_f(this->vy)))) {
      if (this->type == WOODBLOCK || this->type == SMWOODBLOCK ||
	  (this->type >= WHITEDIE && this->type <= LINKBLOCK) ||
	  (this->type == FAKEBLOCK && this->subtype == 0)) play_sound(SND_KNOCK+rand_to(3));
      else if (this->type == ICEBLOCK || this->type == STONEBLOCK ||
	       this->type == SMICEBLOCK || this->type == SMSTONEBLOCK ||
	       (this->type == FAKEBLOCK && this->subtype > 0))
	play_sound(SND_CLINK+rand_to(3));
    }

    switch (this->type) {
    case SOLIDSWITCH :
      if (vel > 2.3 && this->timer == 0 && (!other || (other->type != GHOST &&
                                                       other->type != ANTIMATTER))) {
        this->status = !this->status;
        *switchflags ^= (this->subtype?REDFLAG:GREENFLAG);
	play_sound((this->subtype?SND_SWITCHRD:SND_SWITCHGR)+rand_to(3));
        if (!this->collide[(crashtype-CRASHU+2) % 4] &&
            abs_f(this->x - this->startx) < SWITCH_BOUNCE_ERROR &&
            abs_f(this->y - this->starty) < SWITCH_BOUNCE_ERROR &&
            other) {
          if (crashtype == CRASHU || crashtype == CRASHD)
            this->vy = cap_val(other->vy, 6);
          else this->vx = cap_val(other->vx, 6);
        }
        this->timer = 22;
      }
      break;
    case GRAVITYSWITCH :
      if (vel > 2.3 && this->timer == 0 && (!other || other->type != GHOST)) {
        this->anim += (this->subtype ? 1 : -1);
        if (this->anim < UP) this->anim = LEFT;
        if (this->anim > LEFT) this->anim = UP;
        *gravdir = this->anim;
	play_sound(SND_SWITCHGV+rand_to(3));
        if (!this->collide[(crashtype-CRASHU+2) % 4] &&
            abs_f(this->x - this->startx) < SWITCH_BOUNCE_ERROR &&
            abs_f(this->y - this->starty) < SWITCH_BOUNCE_ERROR &&
            other) {
          if (crashtype == CRASHU || crashtype == CRASHD)
            this->vy = cap_val(other->vy, 6);
          else this->vx = cap_val(other->vx, 6);
        }
        this->timer = 22;
      }
      break;
    case STICKYBOMB :
      if (this->status == 0 && vel > 2.6) {
        this->islinked = -1;
        this->solid = 0;
        this->floating = 1;
        this->pickup = 0;
        this->infront = 1;
        this->status = crashtype;
	play_sound(SND_STICK);
        if (other != NULL) {
          this->stickx = this->x-other->x;
          this->sticky = this->y-other->y;
          if (other->type == BERET) this->link = -2;
          else this->link = oindex;
        } else {
          this->stickx = this->x;
          this->sticky = this->y;
        }
      }
      break;
    case PLATFORM :
      // status = 2 means a clip that shouldn't activate the platform was detected
      if (this->status == 0 && crashtype == CRASHU && other && other->vy < 0) this->status = 2;
      break;
    }
  }
}


void collide_sides(Thing* this, int lvlWidth, int lvlHeight) {
  float thisl = this->x, thist = this->y;
  int thisw = this->width, thish = this->height;
  float thisr = thisl+thisw, thisb = thist+thish;
  if (thisl <= 0) {
    this->collide[LEFT] = 1;
    this->colltarget[LEFT] = 0;
  }
  if (thisr >= lvlWidth) {
    this->collide[RIGHT] = 1;
    this->colltarget[RIGHT] = lvlWidth-thisw;
  }
  if (thist <= 0) {
    this->collide[UP] = 1;
    this->colltarget[UP] = -1;
  }
  if (thisb >= lvlHeight) {
    this->collide[DOWN] = 1;
    this->colltarget[DOWN] = lvlHeight-thish;
  }
}


void update_thing(Thing* this, int tiles[500][500][3],
                  Thing things[250], Thing* beret,
                  int lvlWidth, int lvlHeight, int index,
                  int *gravdir, int *switchflags, int teleused) {
  int i, j;

  // move in small, bite-sized steps
  float greater = abs(this->vx) > abs(this->vy) ? this->vx : this->vy;
  int stepsize = abs(greater)/CORNER + 1;
  if (this->width < SMALLSIZE || this->height < SMALLSIZE) stepsize *= 2;
  for (i = 0; i < stepsize; i++) {
    this->x += this->vx/stepsize;
    this->y += this->vy/stepsize;
    if (this->vx == 0 && this->vy == 0) this->nomove = 1;
    else this->nomove = 0;
    if (!(*switchflags & CREATORFLAG && *switchflags & NOCOLLIDEFLAG)) {
      get_collisions(this, tiles, things, beret, thingnodes, 
                     lvlWidth, lvlHeight, index, gravdir, switchflags);
      apply_collisions(this, gravdir, things, switchflags, index);
    } else {
      for (j = 0; j < 4; j++) {
        this->collide[j] = 0;
      }
      collide_sides(this, lvlWidth, lvlHeight);
      apply_collisions(this, gravdir, things, switchflags, index);
    }
  }

  if (!(*switchflags & CREATORFLAG)) {
    special_thing(this, things, beret, gravdir, teleused,
                  index, switchflags);
    
    if (this->ontele > -1) {
      clear_floorchecked();
      check_ontele(this, thingnodes[index], index);
    }

    if (this->jumpdelay > 0) this->jumpdelay--;
    
    if (this->islinked > -1 && this->type != LINKBLOCK && 
        (things[this->islinked].link != index || things[this->islinked].type != LINKBLOCK))
      this->islinked = -1;
    
    if (this->teledelay > 0) this->teledelay--;
    
    // deal with gravity
    int mygravdir = *gravdir;
    if (this->islinked == -1 && this->gravmod != NONE)
      mygravdir = this->gravmod;
    if (!this->collide[mygravdir] && !this->floating && 
        !this->telething && this->islinked == -1) {
      switch (mygravdir) {
      case DOWN: this->vy += GRAVITY; break;
      case LEFT: this->vx -= GRAVITY; break;
      case RIGHT: this->vx += GRAVITY; break;
      case UP: this->vy -= GRAVITY; break;
      }
    }
    
    // deal with friction
    if (!this->telething && !this->nomove && this->islinked == -1) {
      if (mygravdir == DOWN || mygravdir == UP || this->floating) 
        this->vx = apply_friction((!this->collide[mygravdir] && !this->floating)?FRICTMOD:
                                  (this->icy?ICYMOD:(this->floating?FLOATMOD:FRICT)), this->vx);
      if (mygravdir == LEFT || mygravdir == RIGHT || this->floating)
        this->vy = apply_friction((!this->collide[mygravdir] && !this->floating)?FRICTMOD:
                                  (this->icy?ICYMOD:(this->floating?FLOATMOD:FRICT)), this->vy);
    }
  } else {
    this->vx = apply_friction(2, this->vx);
    this->vy = apply_friction(2, this->vy);
  }
}


// update Beret's status
void handle_key_input(int key1, int key2, int key3,
                      Thing* this, int tiles[500][500][3],
                      Thing things[250], int lvlWidth, int lvlHeight,
                      int *gravdir, int *switchflags, int walkaway,
		      int runningmode) {
  int moving = 0;
  int i = 0;

  // set the flag to say whether the user is pressing an important key
  if (key1 != NONE || key2 != NONE) *switchflags |= KEYPRESSED;
  else *switchflags &= ~KEYPRESSED;

  // move in small, bite-sized steps
  float greater = abs(this->vx) > abs(this->vy) ? this->vx : this->vy;
  int stepsize = abs(greater)/CORNER + 1;
  for (i = 0; i < stepsize; i++) {
    this->x += this->vx/stepsize;
    this->y += this->vy/stepsize;
    if (this->vx == 0 && this->vy == 0) this->nomove = 1;
    else this->nomove =0;
    if (!walkaway) {
      get_collisions(this, tiles, things, 0, thingnodes, lvlWidth, lvlHeight,
                     250, gravdir, switchflags);
      apply_collisions(this, gravdir, things, switchflags, -1);
    }
  }

  if (this->ontele > -1) {
    clear_floorchecked();
    check_ontele(this, thingnodes[250], 250);
  }

  // detect double tapping or shift for running
  if (runningmode) {
    runMargin = 0;
    leftTime = 0;
    rightTime = 0;
    if (key3 == SHIFT) {
      if (!holdShift) shiftRun = !shiftRun;
      holdShift = 1;
      this->speed = WALK_SPEED;
    } else {
      holdShift = 0;
      this->speed = RUN_SPEED;
    }
    //    if (shiftRun) {
    //      this->speed = RUN_SPEED;
    //    } else {
    //      this->speed = WALK_SPEED;
    //    }
  } else {
    holdShift = 0;
    shiftRun = 0;
    if (key1 == LEFT) {
      if (leftTime > 0 && leftTime < TAP_TIME) {
	this->speed = RUN_SPEED;
	runMargin = RUNMARGIN;
      }
      leftTime = 0;
    } else {
      if (this->vx >= -WALK_SPEED && this->dir == 0) {
	if (runMargin > 0) runMargin--;
	else this->speed = WALK_SPEED;
      }
      leftTime++;
    } 
    if (key1 == RIGHT) {
      if (rightTime > 0 && rightTime < TAP_TIME) {
	this->speed = RUN_SPEED;
	runMargin = RUNMARGIN;
      }
      rightTime = 0;
    } else {
      if (this->vx <= WALK_SPEED && this->dir == 1) {
	if (runMargin > 0) runMargin--;
	else this->speed = WALK_SPEED;
      }
      rightTime++;
    }
  }

  if (walkaway == 1) {
    this->vx = approach(this->vx, -this->speed, ACCEL);
    return;
  } else if (walkaway == 2) {
    this->vx = approach(this->vx, this->speed, ACCEL);
    return;
  }

  // deal with jumping
  if (key2 == UP) {
    upTime++;
  } else {
    upTime = 0;
  }
  int jumpcheck = upTime < 6 && !this->jumpdelay &&
    !this->status && !this->jump;
  if (this->jump) {
    if (this->vy < 0) {
      if (key2 == UP) {
        this->vy += GRAVITY*0.5;
      } else {
        this->vy += GRAVITY*1.75;
      }
    } else {
      this->vy += GRAVITY;
    }
  }
  if (jumpcheck) jumpGrace = JUMPGRACE;
  if (key2 == UP && !this->collide[UP] && (jumpcheck || jumpGrace > 0)) {
    this->vy = -JUMP_SPEED;
    play_sound(SND_JUMP+rand_to(3));
    this->jump = 1;
    this->anim = 4;
    upTime = 6;
    jumpGrace = 0;
  }
  if (this->jumpdelay > 0) this->jumpdelay--;
  if (jumpGrace > 0) jumpGrace--;

  // handle movement
  if (key1 == LEFT) {
    if (this->vx > -this->speed) {
      this->vx = approach(this->vx, -this->speed, ACCEL);
      moving = 1;
    }
    this->dir = 0;
  } else if (key1 == RIGHT) {
    if (this->vx < this->speed) {
      this->vx = approach(this->vx, this->speed, ACCEL);
      moving = 1;
    }
    this->dir = 1;
  }

  // deal with animation
  if (this->vx != 0) {
    float anim_chg = abs_f(this->vx)/6;
    this->anim += anim_chg;
    int anim_int = (int)this->anim;
    if (anim_int % 4 == 0 && (this->anim - anim_int) < anim_chg && !this->jump)
      play_sound(SND_STEP+rand_to(3));
  } else if ((int)this->anim % 4 != 0) {
    this->anim += .3;
  }
  
  // apply friction if not walking and on floor
  if (!moving) {
    this->vx = apply_friction(BERETFRICT*(this->jump?FRICTMOD:1), this->vx);
  }
}

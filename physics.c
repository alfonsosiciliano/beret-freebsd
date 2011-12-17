#include "SDL.h"
#include "tile.h"
#include "thing.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>

#define LVLMAX 500

#define THISBLOCK i*30
#define NEXTBLOCK (i+1)*30 
#define THISLAYER j*30
#define NEXTLAYER (j+1)*30

#define ACCEL 0.4
#define FRICT 0.4
#define TAP_TIME 8
#define GRAVITY 0.4
#define WALK_SPEED 2.5
#define RUN_SPEED 4
#define BOUNCESPEED 4.5
#define SPR_SIZE 30
#define CORNER 6
#define JUMPDELAY 2
#define BOUNCE 0.3
#define CEILINGBOUNCE 0.18
#define SMALLSIZE 20
#define PITMARGIN 0
#define INSIDESPEED 5

#define CREATORFLAG 0x4

float apply_friction(float f, float target) {
  if (target < 0) {
    return (target > -f) ? 0 : target + f;
  } else {
    return (target < f) ? 0 : target - f;
  }
}


int rev_col(int c) {
  switch (c) {
  case UP : return DOWN;
  case LEFT : return RIGHT;
  case DOWN : return UP;
  case RIGHT : return LEFT;
  default : return c;
  }
}


float trunc_f(float f) {
  return ((int) f*100) * 0.01;
}


// checks for collisions between two square things
// returns 1 if the thing is inside something else and needs to die
int check_thing(Thing* this, Thing* other, int index, int thisindex,
                 int *gravdir, Thing things[250], int *switchflags) {
  float thisl = trunc_f(this->x), thist = trunc_f(this->y);
  int thisw = this->width, thish = this->height;
  float thisr = thisl+thisw, thisb = thist+thish;
  float bl = trunc_f(other->x), bt = trunc_f(other->y);
  int bw = other->width, bh = other->height;
  float br = bl+bw, bb = bt+bh;
  int collision = NONE;
  float cvel;

  int mustdie = 0;

  // set up some helper variables
  int collect = (this->type == BERET && other->type == MEDALFRAGMENT);
  int cornersize = CORNER, inssize = 8, hcornersize = CORNER;
  if (thisw < SMALLSIZE || thish < SMALLSIZE ||
      bw < SMALLSIZE || bh < SMALLSIZE) cornersize = 2*CORNER/3;
  if (thisw <= 10 || thish <= 10 ||
      bw <= 10 || bh <= 10) {
    cornersize = 3;
    inssize = 11;
  }
  if (this->infront || other->infront) inssize-=6;
  if ((this->vx != 0 || this->vy != 0) && (other->vx != 0 || other->vy != 0))
    inssize--;
  if (this->vx == 0 && this->vy == 0 && other->vx == 0 && other->vy == 0)
    inssize-=4;
  if (inssize < 0) inssize = 0;
  if (this->type == BERET && this->vx != 0 && !this->jump) hcornersize = CORNER/2;
  else hcornersize = cornersize;

  // check for collisions
  if (thisr-cornersize > bl+inssize && thisl+cornersize < br-inssize-1 &&
      thisb-cornersize > bt+inssize-2 && thist+cornersize < bb-inssize+1 &&
      ((thisw >= SMALLSIZE && thish >= SMALLSIZE && bw >= SMALLSIZE &&
        bh >= SMALLSIZE) || (this->vx < INSIDESPEED && this->vy < INSIDESPEED &&
                             other->vx < INSIDESPEED && other->vy < INSIDESPEED))) {
    if (!(*switchflags & CREATORFLAG) && 
        !((this->type == BERET && other->type == MEDALFRAGMENT) ||
          (this->type == MEDALFRAGMENT && other->type == BERET)) &&
        !this->dead && !other->dead && this->solid && other->solid) {
      if (!this->infront && other->crashspeed != 0 && other->type != GHOST &&
	  !(this->type == MATTERLY && other->type == MATTERFRAG))
        mustdie = 1;
      if (!other->infront && this->crashspeed != 0 && this->type != GHOST &&
	  !(other->type == MATTERLY && this->type == MATTERFRAG))
        destroy_thing(other, INSIDE, things, index);
      if (this->crashspeed == 0) {
        int crashdir = 
          (abs_f(this->vx) > abs_f(this->vy) ?
           (this->vx > 0 ? CRASHL : CRASHR) :
           (this->vy > 0 ? CRASHU : CRASHD));
        cvel = (crashdir == CRASHU || crashdir == CRASHD ?
                abs_f(this->vy) : abs_f(this->vx));
        check_crash(other, this, thisindex, cvel, crashdir,
                    things, gravdir, switchflags, index, 1);
      } else if (other->crashspeed == 0) {
        int crashdir = 
          (abs_f(other->vx) > abs_f(other->vy) ?
           (other->vx > 0 ? CRASHL : CRASHR) :
           (other->vy > 0 ? CRASHU : CRASHD));
        cvel = (crashdir == CRASHU || crashdir == CRASHD ?
                abs_f(other->vy) : abs_f(other->vx));
        check_crash(this, other, index, cvel, crashdir,
                    things, gravdir, switchflags, thisindex, 1);
      }
    }
    collision = IN;
  } else if (thisb >= bt && thisb <= bt+bh/2 &&
      thisr >= bl+hcornersize && thisl <= br-hcornersize &&
      (!this->collide[DOWN] || bt-thish <= this->colltarget[DOWN])) {
    if (!collect && other->solid) this->colltarget[DOWN] = bt-thish;
    collision = DOWN;
    if (this->solid && other->solid) {
      cvel = this->vy;
      check_crash(other, this, thisindex, cvel, CRASHU,
                  things, gravdir, switchflags, index, 1);
      check_crash(this, other, index, cvel, CRASHD,
                  things, gravdir, switchflags, thisindex, 0);
    }
  } else if (thist <= bb && thist >= bb-bh/2 &&
      thisr >= bl+cornersize && thisl <= br-cornersize &&
      (!this->collide[UP] || bb >= this->colltarget[UP])) {
    if (!collect && other->solid) this->colltarget[UP] = bb;
    collision = UP;
    if (this->solid && other->solid) {
      cvel = -this->vy;
      check_crash(other, this, thisindex, cvel, CRASHD,
                  things, gravdir, switchflags, index, 1);
      check_crash(this, other, index, cvel, CRASHU,
                  things, gravdir, switchflags, thisindex, 0);
    }
  } else if (thisr >= bl && thisr <= bl+bw/2 &&
      thisb >= bt+cornersize && thist <= bb-cornersize &&
      (!this->collide[RIGHT] || bl-thisw <= this->colltarget[RIGHT])) {
    if (!collect && other->solid) this->colltarget[RIGHT] = bl-thisw;
    collision = RIGHT;
    if (this->solid && other->solid) {
      cvel = this->vx;
      check_crash(other, this, thisindex, cvel, CRASHL,
                  things, gravdir, switchflags, index, 1);
      check_crash(this, other, index, cvel, CRASHR,
                  things, gravdir, switchflags, thisindex, 0);
    }
  } else if (thisl <= br && thisl >= br-bw/2 &&
      thisb >= bt+cornersize && thist <= bb-cornersize &&
      (!this->collide[LEFT] || br >= this->colltarget[LEFT])) {
    if (!collect && other->solid) this->colltarget[LEFT] = br;
    collision = LEFT;
    if (this->solid && other->solid) {
      cvel = -this->vx;
      check_crash(other, this, thisindex, cvel, CRASHR,
                  things, gravdir, switchflags, index, 1);
      check_crash(this, other, index, cvel, CRASHL,
                  things, gravdir, switchflags, thisindex, 0);
    }
  }

  if (collision != NONE) {
    if (!(*switchflags & CREATORFLAG)) {
      thing_collision(this, other, collision, thisindex, index,
                      gravdir, things, switchflags, 1);
      thing_collision(other, this, rev_col(collision), index,
                      thisindex, gravdir, things, switchflags, 0);
    }
    if (collision != IN && this->solid && other->solid &&
        !collect && !other->dead)
      this->collide[collision] = 1;
  }

  return mustdie;
}


int tile_collide(int tiles[500][500][3], int i, int j, int collidetype) {
  return tiles[i][j][0] && 
    (tiles[i][j][1] == SOLID || tiles[i][j][1] == SOLIDF ||
     tiles[i][j][1] == collidetype);
}


// check for collisions with tiles and objects, storing results
// of the check into the object's collide field
void get_collisions(Thing* this, int tiles[500][500][3],
                    Thing things[250], Thing* beret, 
                    ThingNode* thingnodes[251],
                    int lvlWidth, int lvlHeight,
                    int nocheck, int *gravdir, int *switchflags) {
  int i, j;
  float thisl = this->x, thist = this->y;
  int thisw = this->width, thish = this->height;
  float thisr = thisl+thisw, thisb = thist+thish;
  int tl = (thisl-1)/SPR_SIZE, tt = (thist-1)/SPR_SIZE;
  int tr = thisr/SPR_SIZE, tb = thisb/SPR_SIZE;

  int mustdie = 0; // is set if this is found to be inside something

  ThingNode* tempnode;
  ThingNode* curnode = thingnodes[nocheck];
  if (this->islinked > -1) {
    tempnode = malloc(sizeof(ThingNode));
    tempnode->thing = &things[this->islinked];
    tempnode->next = 0;
    tempnode->thingindex = this->islinked;
    thingnodes[nocheck] = tempnode;
  } else thingnodes[nocheck] = 0;
  while (curnode != 0) {
    tempnode = curnode;
    curnode = curnode->next;
    free(tempnode);
  }
  

  for (i = 0; i < 4; i++) {
    this->collide[i] = 0;
  }

  // determine status of this thing for ontele
  if (this->floating || 
      (this->islinked > -1 && things[this->islinked].floating))
    this->ontele = -1;
  else if (this->telething || 
           (this->islinked > -1 && things[this->islinked].telething))
    this->ontele = 1;
  else this->ontele = 0;

  // collisions with level boundaries
  if (this->type < TYPEMAX || this->crashspeed == 0) {
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
  }
  if (*switchflags & CREATORFLAG) {
    if (thisb >= lvlHeight) {
      this->collide[DOWN] = 1;
      this->colltarget[DOWN] = lvlHeight-thish;
    }
  } else {
      if (thist >= lvlHeight+PITMARGIN) {
      destroy_thing(this, PIT, things, nocheck);
    }
  }

  // collisions with tiles
  
  if (!(*switchflags & CREATORFLAG)) {
    for (i = thisl/SPR_SIZE; i <= (thisr-1)/SPR_SIZE; i++) {
      for (j = thist/SPR_SIZE; j <= (thisb-1)/SPR_SIZE; j++) {
        tile_collision(this, things, tiles, i, j, nocheck,0);
      }
    }
    tile_collision(this, things, tiles, (thisl-1)/SPR_SIZE,
                   thist/SPR_SIZE, nocheck,1);
    tile_collision(this, things, tiles, thisr/SPR_SIZE,
                   thist/SPR_SIZE, nocheck,1);
    tile_collision(this, things, tiles, thisl/SPR_SIZE,
                   (thist-1)/SPR_SIZE, nocheck,1);
    tile_collision(this, things, tiles, thisl/SPR_SIZE,
                   thisb/SPR_SIZE, nocheck,1);
    tile_collision(this, things, tiles, (thisl-1)/SPR_SIZE,
                   (thisb-1)/SPR_SIZE, nocheck,1);
    tile_collision(this, things, tiles, thisr/SPR_SIZE,
                   (thisb-1)/SPR_SIZE, nocheck,1);
    tile_collision(this, things, tiles, (thisr-1)/SPR_SIZE,
                   (thist-1)/SPR_SIZE, nocheck,1);
    tile_collision(this, things, tiles, (thisr-1)/SPR_SIZE,
                   thisb/SPR_SIZE, nocheck,1);
  }

  if (this->solid) {

    int cornersize = CORNER, hcornersize = CORNER;
    if (thisw <= 10 || thish <= 10) cornersize = CORNER/3;
    if (this->type == BERET && this->vx != 0 && !this->jump) hcornersize = CORNER/2;
    else hcornersize = cornersize;

    int collidetype = (this->type == BERET ? OBJONLYF : BERETONLYF);
    
    if (tile_collide(tiles,(thisl+thisw/2)/SPR_SIZE,(thist+thish/2)/SPR_SIZE,collidetype))
      mustdie = 1;

    // down collisions
    if (tb < LVLMAX) {
      for (i = tl; i <= tr; i++) {
        if (tile_collide(tiles,i,tb,collidetype) && thisb <= tb*SPR_SIZE+SPR_SIZE/2 && (tb == 0 || !tile_collide(tiles,i,tb-1,collidetype))) {
          if ((thisl < NEXTBLOCK-hcornersize && thisr > THISBLOCK+hcornersize) ||
              ((i == 0 || !tile_collide(tiles,i-1,tb,collidetype)) &&
               thisr > THISBLOCK+hcornersize && thisl < THISBLOCK-hcornersize) ||
              ((i == LVLMAX-1 || !tile_collide(tiles,i+1,tb,collidetype)) &&
               thisr > NEXTBLOCK+hcornersize && thisl < NEXTBLOCK-hcornersize)) {
            this->collide[DOWN] = 1;
            this->colltarget[DOWN] = tb*SPR_SIZE-thish;
          }
        }
      }
    }
    
    // up collisions
    if (tt >= 0) {
      for (i = tl; i <= tr; i++) {
        if (tile_collide(tiles,i,tt,collidetype) && thist >= (tt+1)*SPR_SIZE-SPR_SIZE/2 && (tt == LVLMAX-1 || !tile_collide(tiles,i,tt+1,collidetype))) {
          if ((thisl < NEXTBLOCK-cornersize && thisr > THISBLOCK+cornersize) ||
              ((i == 0 || !tile_collide(tiles,i-1,tt,collidetype)) &&
               thisr > THISBLOCK+cornersize && thisl < THISBLOCK-cornersize) ||
              ((i == LVLMAX-1 || !tile_collide(tiles,i+1,tt,collidetype)) &&
               thisr > NEXTBLOCK+cornersize && thisl < NEXTBLOCK-cornersize)) {
            this->collide[UP] = 1;
            this->colltarget[UP] = (tt+1)*SPR_SIZE;
          }
        }
      }
    }
    
    // left collisions
    if (tl >= 0) {             
      for (i = tt; i <= tb; i++) {
        if (tile_collide(tiles,tl,i,collidetype) && thisl >= (tl+1)*SPR_SIZE-SPR_SIZE/2 && (tl == LVLMAX-1 || !tile_collide(tiles,tl+1,i,collidetype))) {
          if ((thist < NEXTBLOCK-cornersize && thisb > THISBLOCK+cornersize) ||
              ((i == 0 || !tile_collide(tiles,tl,i-1,collidetype)) &&
               thisb > THISBLOCK+cornersize && thist < THISBLOCK-cornersize) ||
              ((i == LVLMAX-1 || !tile_collide(tiles,tl,i+1,collidetype)) &&
               thisb > NEXTBLOCK+cornersize && thist < NEXTBLOCK-cornersize)) {
            this->collide[LEFT] = 1;
            this->colltarget[LEFT] = (tl+1)*SPR_SIZE;
          }
        }
      }
    }
    
    // right collisions
    if (tr < LVLMAX) {
      for (i = tt; i <= tb; i++) {
        if (tile_collide(tiles,tr,i,collidetype) && thisr <= tr*SPR_SIZE+SPR_SIZE/2 && (tr == 0 || !tile_collide(tiles,tr-1,i,collidetype))) {
          if ((thist < NEXTBLOCK-cornersize && thisb > THISBLOCK+cornersize) ||
              ((i == 0 || !tile_collide(tiles,tr,i-1,collidetype)) &&
               thisb > THISBLOCK+cornersize && thist < THISBLOCK-cornersize) ||
              ((i == LVLMAX-1 || !tile_collide(tiles,tr,i+1,collidetype)) &&
               thisb > NEXTBLOCK+cornersize && thist < NEXTBLOCK-cornersize)) {
            this->collide[RIGHT] = 1;
            this->colltarget[RIGHT] = tr*SPR_SIZE-thisw;
          }
        }
      }
    }
  }

  if (this->collide[this->gravmod==NONE?*gravdir:this->gravmod])
    this->ontele = -1;

  // collisions with beret, things
  if (!(*switchflags & CREATORFLAG) && beret && !beret->dead) {
    if (check_thing(this, beret, 250, nocheck,
                    gravdir, things, switchflags)) mustdie = 1;
  }
  for (i=0; i<250; i++) {
    if (things[i].type != NOTYPE && i != nocheck) {
      if (check_thing(this, &things[i], i, nocheck, 
                      gravdir, things, switchflags)) mustdie = 1;
    }
  }

  if (this->link > -1 && this->type == LINKBLOCK &&
      things[this->link].ontele == 0 && this->subtype % 2 == 0) {
    things[this->link].ontele = this->ontele;
  }
  
  if (mustdie) destroy_thing(this, INSIDE, things, nocheck);
}


float approach(float cur, float trg, float inc) {
  if (cur < trg) {
    return trg - cur < inc ? trg : cur + inc;
  } else {
    return cur - trg < inc ? trg : cur - inc;
  }
}


int bounce_type(int type) {
  return !(type == BERET || type == HOPPER || type == SOLIDSWITCH ||
           type == PLATFORM || type == TOPHAT);
}


// apply the effects of the detected collisions on the given thing
void apply_collisions(Thing* this, int *gravdir, Thing things[250],
                      int *switchflags, int index) {

  if (!this->nomove) {
    if (!this->novmove || (*switchflags & CREATORFLAG)) {
      if (this->collide[UP] && this->collide[DOWN]) {
        this->vy = 0;
        float avgtarget =
          (this->colltarget[UP]+this->colltarget[DOWN])/2;
        this->y = avgtarget;
      } else {
        if (this->collide[UP]) {
          if (this->vy < 0) {
            check_crash(this, NULL, -1, -this->vy, CRASHU,
                        things, gravdir, switchflags, index, 1);
            if (!(*switchflags & CREATORFLAG) && this->vy < -BOUNCESPEED && 
                !this->telething && this->islinked == -1 &&
                bounce_type(this->type))
              this->vy *= -BOUNCE;
            else
              this->vy = 0;
          }
          this->y = this->colltarget[UP];
        }
        
        if (this->collide[DOWN]) {
          check_crash(this, NULL, -1, this->vy, CRASHD,
                      things, gravdir, switchflags, index, 1);
          if (!(*switchflags & CREATORFLAG) && 
              this->vy > BOUNCESPEED && !this->telething && 
              this->islinked == -1 && bounce_type(this->type))
            this->vy *= -BOUNCE;
          else if (this->vy > 0)
            this->vy = 0;
          this->y = this->colltarget[DOWN];
        }
      }
    }

    if (!this->nohmove || (*switchflags & CREATORFLAG)) {
      if (this->collide[LEFT] && this->collide[RIGHT]) {
        this->vx = 0;
        float avgtarget =
          (this->colltarget[LEFT]+this->colltarget[RIGHT])/2;
        this->x = approach(this->x, avgtarget, CORNER);
      } else {
        if (this->collide[LEFT]) {
          if (this->vx < 2)
            this->x = approach(this->x, this->colltarget[LEFT], CORNER);
          if (this->vx < 0) {
            check_crash(this, NULL, -1, -this->vx, CRASHL,
                        things, gravdir, switchflags, index, 1);
            if (!(*switchflags & CREATORFLAG) && this->vx < -BOUNCESPEED &&
                !this->telething && this->islinked == -1)
              this->vx *= -BOUNCE;
            else this->vx = 0;
          }
        }
        
        if (this->collide[RIGHT]) {
          if (this->vx > -2)
            this->x = approach(this->x, this->colltarget[RIGHT], CORNER);
          if (this->vx > 0) {
            check_crash(this, NULL, -1, this->vx, CRASHR,
                        things, gravdir, switchflags, index, 1);
            if (!(*switchflags & CREATORFLAG) && this->vx > BOUNCESPEED &&
                !this->telething && this->islinked == -1)
              this->vx *= -BOUNCE;
            else this->vx = 0;
          }
        }
      }
    }
  }

  if (this->collide[DOWN]) {
    if (this->jump == 1 && this->jumpdelay < JUMPDELAY)
      this->jumpdelay = JUMPDELAY;
    this->jump = 0;
  } else {
    this->jump = 1;
  }
}

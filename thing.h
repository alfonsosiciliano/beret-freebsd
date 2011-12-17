enum deathtypes {
  INSIDE, PIT, TOUCH, CRASHU, CRASHR, CRASHD, CRASHL,
  BOMBED, COLLECT, INFECT
};

enum colors {
  RED, PINK, PURPLE, BLUE, AQUA, SKY, GREEN, GRYELLOW,
  YELLOW, WHITE, BROWN, GRAY, ORANGE, TRANSPARENT, NOCOLOR
};

enum sounds {
  SND_KNOCK=0, SND_CLINK=3, SND_STEP=8, SND_JUMP=11,
  SND_COLLECT=16, SND_BOOM=22, SND_TICK=25,
  SND_HOP=28, SND_INFECT=31, 
  SND_SWITCHGR=34, SND_SWITCHRD=37, SND_SWITCHGV=40,
  SND_CHOICEBERET=49, SND_CHOICEOBJECT=52,
  SND_TURRET=55, SND_PLATFORM=58, SND_STICK=61, SND_ANTIMATTER=62,
  SND_FAKE=65, SND_POP=69, SND_CRUNCH=72, SND_SQUELCH=75, SND_REGEN=78, SND_REGENINIT=79,
  SND_MEDW, SND_MEDO, SND_MEDB, SND_MEDR, SND_MEDG, SND_MEDP, SND_CORNER,
  SOUND_MAX
};

enum types {
  NOTYPE, BERET, WOODBLOCK, ICEBLOCK, STONEBLOCK, SMWOODBLOCK, SMICEBLOCK,
  SMSTONEBLOCK, WHITEDIE, BLACKDIE, BIGBLOCK, GRAVITYBLOCK, ANNOYINGBLOCK,
  TELEBLOCK, LINKBLOCK, INFECTLING, WEIGHT, BOMB, STICKYBOMB, 
  DOOR, FINISHDOOR, SIGN, READSIGN, 
  MEDALFRAGMENT, MEDALCORNER, WHITEMEDAL, 
  TELESEEKER, ROBOT, SPIKEBALL, HOPPER, CARRIER, VSIGN, 
  SOLIDSWITCH, SOLIDBLOCK, PLATFORM, FLOATBLOCK, GHOSTBLOCK,
  FAKEBLOCK, AURADROP, STONEY, GRAVITYSWITCH, TURRET, GHOST,
  REINCARNATOR, ANTIMATTER, FAKEBOMB, BLOCKSTER, MATTERLY,
  ANTISEEKER, ANTIFECTLING, TOPHAT, SHIELDGEN,
  TYPEMAX, BOMBHELPER, AURAHELPER, FIREBALL, SPIKEBLOCK, 
  MATTERFRAG, TOPHATSHIELD
};

typedef struct thing {
  float x, y, vx, vy, stickx, sticky, startx, starty;
  int speed, jump, jumpdelay, teledelay;
  int width, height;
  int dir;
  int timer, status;
  int link, islinked;
  float anim;

  int telething, ontele;
  int dead;
  int collide[4];
  int colltarget[4];

  int type, subtype;
  int pickup, icy, gravmod, floating, solid, deathtouch, crashspeed;
  int nomove, nohmove, novmove, infront, inback, animate;
} Thing;

typedef struct thingnode {
  Thing* thing;
  struct thingnode *next;
  int thingindex;
} ThingNode;

typedef struct particle {
  float x, y, vx, vy;
  int color, time;
} Particle;

int get_jumpgrace(void);
void set_jumpgrace(int);
int get_runmargin(void);
void set_runmargin(int);

void initialize_thingnodes();

void handle_key_input(int, int, int, Thing*, int [500][500][3],
                      Thing [250], int, int, int*, int*, int, int);

void draw_thing(Thing*, int [6]);

void make_thing(int, int, int, int, int, int, Thing [250]);

void make_beret(Thing*, int, int, int, int, int, Thing [250]);

void destroy_thing(Thing*, int, Thing [250], int);

void update_thing(Thing*, int [500][500][3], Thing [250], 
                  Thing*, int, int, int, int*, int*, int);

void update_particles(Particle [2500], int);

void thing_collision(Thing*, Thing*, int, int, int, int*, Thing [250], int*, int);

void tile_collision(Thing*, Thing [250], int [500][500][3], int, int, int, int);

void check_crash(Thing*, Thing*, int, float, int, Thing [250], int*, int*, int, int);

float cap_val(float, float);

int copy_thing(Thing*, Thing [250], int);

int find_empty(Thing [250]);

int subtype_count(int);

int get_param(int, int, int);

void make_expl(int, int, float, float, int, int, int);

int* get_bossvars(void);

float abs_f(float);

int rand_to(int);

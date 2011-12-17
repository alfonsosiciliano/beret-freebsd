void create_particle(int, int, float, float, int, int);

void collect_thing(Thing*);

void play_sound(int);

int on_screen(int, int, int, int);

void check_room_dead(void);

void read_level(void);
void kill_fragment(void);
void load_map(int);
void fix_tile_borders(int, int, int);
int check_can_see(Thing, Thing);
void set_beat_last_level(void);

#ifndef GAME_H
#define GAME_H

#define SCREEN_W     640
#define SCREEN_H     480
#define SCREEN_DEPTH 0

#define TILE_SIZE 100 /* Size of tiles (walls, floors, etc.) in pixels. */

#define ENTITY_W (TILE_SIZE / 2) /* Width and height of player */
#define ENTITY_H (TILE_SIZE / 2) /* and zombie rects in pixels. */

#define GOODIE_W (TILE_SIZE / 4) /* Width and height of goodie */
#define GOODIE_H (TILE_SIZE / 4) /* rect in pixels. */

#define LEVEL_W 40 /* Width and height of */
#define LEVEL_H 30 /* the level in tiles. */

/* Path for data files. Relative path by default, this can be set during
 * compilation and can be changed at run-time by supplying the '-d' option. */
#ifndef DATADIR
#define DATADIR "data"
#endif

/* Boolean definitions and type used for convenience. */
#define true    1
#define false   0

typedef _Bool bool;

/* 
 * Shuts down SDL and exits cleanly, optionally emitting an error message
 * if code != 0. Returns code to the system.
 */
int game_terminate(int code);

struct game_data {
	/* Various bookkeeping variables for the game. */
	char *datadir;
	int num_levels;
	int screen_w, screen_h;

	/* In order to scroll our level, we first paint everything to
	 * 'world', then we copy whatever is in the 'camera' rect to
	 * our screen via 'SDL_BlitSurface()'. */
	SDL_Surface *world;
	SDL_Surface *screen;
	SDL_Joystick *joystick;

	/* This array represents our level, and is automatically populated
	 * by the level_generate function. Different characters correspond
	 * to different tiles. */
	char level[LEVEL_H][LEVEL_W];

	/* This array keeps track of all wall tiles on the level. */
	SDL_Rect wall[LEVEL_H][LEVEL_W];

	/* Camera acts as a viewport which follows the player around and draws
	 * the level around the player according to the 'level' array. */
	SDL_Rect camera;

	int cur_level;		/* Current level in game. */
	Sint32 score;		/* Game score for the current session. */
	Uint32 score_scale;	/* The score in which we will gain our next life. */

	Uint8 time;		/* Time remaining for this stage in seconds. */
	bool level_cleared;	/* If this is true, we skip to the next level. */
	Uint32 delta_time;	/* Time elapsed between frames. */

	struct pc {
		SDL_Rect rect;	/* Persistent rect for the player character. */
		SDL_Surface *bg;	/* Background surface for redrawing etc. */
		int iso_x, iso_y;	/* Location on map according to isometric projection. */

		bool dead;		/* Are we dead? */
		int lives;		/* Number of retries for the current session. */
		int dir_x, dir_y;	/* Direction of player on the X / Y axis. */
	} player;

	int num_zombies;	/* Number of zombies in the level. */

	struct npc {
		SDL_Rect rect;	/* Persistent rect for the zombies. */
		SDL_Surface *bg;	/* Background surface for redrawing etc. */
		int iso_x, iso_y;	/* Location on map according to isometric projection. */

		struct node { int x, y; } path[64];
		int num_nodes;		/* Number of nodes in path. */
		int dest_x, dest_y;	/* Destination on the X / Y axis. */
	} zombie[16];

	int num_goodies;	/* Number of goodies in the level. */
	
	struct prize {
		SDL_Rect rect;	/* Persistent rect for the goodies in the level. */
		SDL_Surface *bg;	/* Background surface for redrawing etc. */
		int iso_x, iso_y;	/* Location on map according to isometric projection. */
	} goodie[16];

	struct {
		SDL_Surface *font;
		SDL_Surface *level;
		SDL_Surface *player;
		SDL_Surface *zombie;
		SDL_Surface *goodie;
	} graphics;	/* Keeps track of graphics used throughout the game */

	/* TEMP */
	Uint32 black;
	Uint32 white;
	Uint32 red;
	Uint32 blue;
	Uint32 brown;
	Uint32 yellow;
};

#endif

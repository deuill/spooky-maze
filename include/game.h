#ifndef GAME_H
#define GAME_H

#define SCREEN_W     640
#define SCREEN_H     480
#define SCREEN_DEPTH 0

#define TILE_SIZE 50 /* Size of tiles (walls, floors, etc.) in pixels. */
#define ENTITY_SIZE (TILE_SIZE - (TILE_SIZE / 4)) /* Size of player and zombie rects in pixels. */

#define LEVEL_W 40 /* Width and height */
#define LEVEL_H 30 /* of the level in tiles. */

/* Change DATADIR if you plan on installing the game with the binary and
 * data folder in seperate locations. Empty (relative path) by default. */
#define DATADIR ""

/* terminate: Shuts down SDL and exits cleanly, optionally emitting an
 *            error message if code != 0. Returns code to the system.
 */
int terminate(int code);

typedef struct {
	/* In order to scroll our level, we first paint everything to
	 * 'world', then we copy whatever is in the 'camera' rect to
	 * our screen via 'SDL_BlitSurface()'. */
	SDL_Surface *world;
	SDL_Surface *screen;
	SDL_Joystick *joystick;

	/* This array represents our level, and is automatically populated
	 * by the generate_level function. Different characters correspond
	 * to different tiles. */
	char level[LEVEL_H][LEVEL_W];

	/* This array keeps track of all wall tiles on the level. */
	SDL_Rect wall[LEVEL_H][LEVEL_W];

	/* Camera acts as a viewport which follows the player around and draws
	 * the level around the player according to the 'level' array. */
	SDL_Rect camera;

	int dead;		/* Are we dead? */
	int lives;		/* Number of retries for the current session. */
	int cur_level;		/* Current level in game. */
	Sint32 score;		/* Game score for the current session. */
	Uint32 score_scale;	/* The score in which we will gain our next life. */

	Uint8 time;		/* Time remaining for this stage in seconds. */
	int level_cleared;	/* If this is true, we skip to the next level. */
	Uint32 delta_time;	/* Time elapsed between frames. */

	int dir_x, dir_y;	/* Direction of player on the X / Y axis. */
	SDL_Rect player;	/* Persistent rect for the player character. */

	int num_zombies;	/* Number of zombies in the level. */
	struct npc {
		struct node { int x, y; } path[64];
		int dest_x, dest_y;	/* Destination on the X / Y axis. */
		int num_nodes;		/* Number of nodes in path. */
		SDL_Rect rect;	/* Persistent rect for the zombies. */
	} zombie[16];

	int num_goodies;	/* Number of goodies in the level. */
	SDL_Rect goodie[16];	/* Persistent rect for the goodies in the level. */

	struct {
		SDL_Surface *font;
	} graphics;	/* Keeps track of graphics used throughout the game */

	/* TEMP */
	Uint32 black;
	Uint32 white;
	Uint32 red;
	Uint32 blue;
	Uint32 brown;
	Uint32 yellow;
} game_data;

#endif

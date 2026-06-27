#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>
#include <stdio.h>
#include <string.h>
#include "game.h"
#include "shared.h"
#include "utility.h"
#include "scene_game.h"
#include "scene_menu.h"
#include "pacman_obj.h"
#include "ghost.h"
#include "map.h"


// Upper bound on ghosts (Hard mode). The active count `ghost_count` is chosen
// per difficulty at the start of each round (see init()).
#define MAX_GHOST_NUM 4
// How long the "GAME OVER" screen lingers before returning to the menu,
// measured in game_update ticks (~128/sec).
#define GAME_OVER_DELAY 256
/* global variables*/
extern const uint32_t GAME_TICK_CD;
extern uint32_t GAME_TICK;
extern ALLEGRO_TIMER* game_tick_timer;
int game_main_Score = 0;
bool game_over = false;

/* Internal variables*/
static ALLEGRO_TIMER* power_up_timer;
static int power_up_duration = 10;       // seconds; set per difficulty in init()
static Pacman* pman;
static Map* basic_map;
static Ghost** ghosts;
static int ghost_count = 0;              // active ghosts this round (per difficulty)
static int game_over_counter = 0;
bool debug_mode = false;
bool cheat_mode = false;

/* Declare static function prototypes */
static void init(void);
static void step(void);
static void checkItem(void);
static void status_update(void);
static void update(void);
static void draw(void);
static void printinfo(void);
static void destroy(void);
static void on_key_down(int key_code);
static void on_mouse_down(int button, int x, int y, int dz);
static void render_init_screen(void);
static void draw_hitboxes(void);

static void init(void) {
	game_over = false;
	game_over_counter = 0;
	game_main_Score = 0;
	// Apply the difficulty chosen on the menu: ghost count, ghost speed
	// (via the shared global read by ghost_create) and power-up length.
	switch (game_difficulty) {
	case DIFF_EASY:
		ghost_count = 1; ghost_base_speed = 1; power_up_duration = 12;
		break;
	case DIFF_HARD:
		ghost_count = 4; ghost_base_speed = 4; power_up_duration = 5;
		break;
	case DIFF_NORMAL:
	default:
		ghost_count = 2; ghost_base_speed = 2; power_up_duration = 8;
		break;
	}
	// create map
	basic_map = create_map(NULL);
	// [TODO]
	// Create map from .txt file and design your own map !!
	// basic_map = create_map("Assets/map_nthu.txt");
	if (!basic_map) {
		game_abort("error on creating map");
	}	
	// create pacman
	pman = pacman_create();
	if (!pman) {
		game_abort("error on creating pacamn\n");
	}
	
	// allocate ghost memory
	// [HACKATHON 2-1]
	// Allocate dynamic memory for the ghosts array.
	else {
		ghosts = (Ghost**)malloc(sizeof(Ghost*) * ghost_count);
		if (!ghosts)
			game_abort("error allocating ghosts array\n");
		// [HACKATHON 2-2] create the ghosts; type i picks colour + cage spot.
		for (int i = 0; i < ghost_count; i++) {
			ghosts[i] = ghost_create(i);
			if (!ghosts[i])
				game_abort("error creating ghost\n");
		}
	}
	GAME_TICK = 0;

	render_init_screen();
	power_up_timer = al_create_timer(1.0f); // 1 tick / sec
	if (!power_up_timer)
		game_abort("Error on create timer\n");
	return ;
}

static void step(void) {
	// Decrement toward 0 without underflowing the unsigned counter when a
	// speed change leaves moveCD smaller than speed (e.g. an eaten ghost).
	if (pman->objData.moveCD >= (uint32_t)pman->speed)
		pman->objData.moveCD -= pman->speed;
	else
		pman->objData.moveCD = 0;
	for (int i = 0; i < ghost_count; i++) {
		// important for movement
		if (ghosts[i]->objData.moveCD >= (uint32_t)ghosts[i]->speed)
			ghosts[i]->objData.moveCD -= ghosts[i]->speed;
		else
			ghosts[i]->objData.moveCD = 0;
	}
}
static void checkItem(void) {
	int Grid_x = pman->objData.Coord.x, Grid_y = pman->objData.Coord.y;
	if (Grid_y >= basic_map->row_num - 1 || Grid_y <= 0 || Grid_x >= basic_map->col_num - 1 || Grid_x <= 0)
		return;
	// [HACKATHON 1-3]
	// Eat whatever item sits on Pacman's current cell and score it.
	switch (basic_map->map[Grid_y][Grid_x])
	{
	case '.':
		pacman_eatItem(pman, '.');
		game_main_Score += 10;
		// [HACKATHON 1-4] erase the eaten item (only items, never a wall block).
		basic_map->map[Grid_y][Grid_x] = ' ';
		break;
	case 'P':
		pacman_eatItem(pman, 'P');
		game_main_Score += 50;
		basic_map->map[Grid_y][Grid_x] = ' ';
		// Power-up: frighten the free-roaming ghosts, then start the countdown.
		pman->powerUp = true;
		for (int i = 0; i < ghost_count; i++)
			ghost_toggle_FLEE(ghosts[i], true);
		al_set_timer_count(power_up_timer, 0);
		al_start_timer(power_up_timer);
		break;
	default:
		break;
	}
}
static void status_update(void) {
	RecArea pacmanArea = getDrawArea(pman->objData, GAME_TICK_CD);
	for (int i = 0; i < ghost_count; i++) {
		if (ghosts[i]->status == GO_IN)
			continue;
		RecArea ghostArea = getDrawArea(ghosts[i]->objData, GAME_TICK_CD);
		if (cheat_mode || !RecAreaOverlap(pacmanArea, ghostArea))
			continue;
		if (ghosts[i]->status == FLEE) {
			// Pacman eats a frightened ghost; it heads back to the room.
			ghost_collided(ghosts[i]);
			game_main_Score += 200;
		}
		else {
			game_log("collide with ghost\n");
			pacman_die();
			game_over = true;
			break;
		}
	}
}

static void update(void) {

	if (game_over) {
		// Hold on the death screen briefly, then return to the menu.
		if (++game_over_counter > GAME_OVER_DELAY)
			game_change_scene(scene_menu_create());
		return;
	}

	step();
	checkItem();
	// End power-up mode once the timer has run for power_up_duration seconds.
	if (pman->powerUp && al_get_timer_count(power_up_timer) >= power_up_duration) {
		pman->powerUp = false;
		for (int i = 0; i < ghost_count; i++)
			ghost_toggle_FLEE(ghosts[i], false);
		al_stop_timer(power_up_timer);
	}
	status_update();
	pacman_move(pman, basic_map);
	for (int i = 0; i < ghost_count; i++) 
		ghosts[i]->move_script(ghosts[i], basic_map, pman);
}

static void draw(void) {

	al_clear_to_color(al_map_rgb(0, 0, 0));

	
	//	Draw scoreboard.
	al_draw_textf(menuFont, al_map_rgb(255, 255, 0),
		map_offset_x, 12, ALLEGRO_ALIGN_LEFT, "SCORE  %d", game_main_Score);

	draw_map(basic_map);

	pacman_draw(pman);
	if (game_over) {
		al_draw_text(menuFont, al_map_rgb(255, 0, 0),
			SCREEN_W / 2, SCREEN_H / 2 - 20, ALLEGRO_ALIGN_CENTER, "GAME OVER");
		return;
	}
	// no drawing below when game over
	for (int i = 0; i < ghost_count; i++)
		ghost_draw(ghosts[i]);
	
	//debugging mode
	if (debug_mode) {
		draw_hitboxes();
	}

}

static void draw_hitboxes(void) {
	RecArea pmanHB = getDrawArea(pman->objData, GAME_TICK_CD);
	al_draw_rectangle(
		pmanHB.x, pmanHB.y,
		pmanHB.x + pmanHB.w, pmanHB.y + pmanHB.h,
		al_map_rgb_f(1.0, 0.0, 0.0), 2
	);

	for (int i = 0; i < ghost_count; i++) {
		RecArea ghostHB = getDrawArea(ghosts[i]->objData, GAME_TICK_CD);
		al_draw_rectangle(
			ghostHB.x, ghostHB.y,
			ghostHB.x + ghostHB.w, ghostHB.y + ghostHB.h,
			al_map_rgb_f(1.0, 0.0, 0.0), 2
		);
	}

}

static void printinfo(void) {
	game_log("pacman:\n");
	game_log("coord: %d, %d\n", pman->objData.Coord.x, pman->objData.Coord.y);
	game_log("PreMove: %d\n", pman->objData.preMove);
	game_log("NextTryMove: %d\n", pman->objData.nextTryMove);
	game_log("Speed: %f\n", pman->speed);
}


static void destroy(void) {
	// Free everything allocated in init() so replays don't leak.
	if (ghosts) {
		for (int i = 0; i < ghost_count; i++)
			ghost_destory(ghosts[i]);
		free(ghosts);
		ghosts = NULL;
	}
	pacman_destory(pman);
	pman = NULL;
	delete_map(basic_map);
	basic_map = NULL;
	if (power_up_timer) {
		al_destroy_timer(power_up_timer);
		power_up_timer = NULL;
	}
}

static void on_key_down(int key_code) {
	switch (key_code)
	{
		// [HACKATHON 1-1]
		// Map WASD to Pacman's next move direction.
		case ALLEGRO_KEY_W:
			pacman_NextMove(pman, UP);
			break;
		case ALLEGRO_KEY_A:
			pacman_NextMove(pman, LEFT);
			break;
		case ALLEGRO_KEY_S:
			pacman_NextMove(pman, DOWN);
			break;
		case ALLEGRO_KEY_D:
			pacman_NextMove(pman, RIGHT);
			break;
		case ALLEGRO_KEY_C:
			cheat_mode = !cheat_mode;
			if (cheat_mode)
				printf("cheat mode on\n");
			else
				printf("cheat mode off\n");
			break;
	default:
		break;
	}

}

static void on_mouse_down(int button, int x, int y, int dz) {
	(void)button;
	(void)x;
	(void)y;
	(void)dz;
	// nothing here

}

static void render_init_screen(void) {
	al_clear_to_color(al_map_rgb(0, 0, 0));

	draw_map(basic_map);
	pacman_draw(pman);
	for (int i = 0; i < ghost_count; i++) {
		ghost_draw(ghosts[i]);
	}

	al_draw_text(
		menuFont,
		al_map_rgb(255, 255, 0),
		400, 400,
		ALLEGRO_ALIGN_CENTER,
		"READY!"
	);

	al_flip_display();
	al_rest(2.0);

}
// Functions without 'static', 'extern' prefixes is just a normal
// function, they can be accessed by other files using 'extern'.
// Define your normal function prototypes below.

// The only function that is shared across files.
Scene scene_main_create(void) {
	Scene scene;
	memset(&scene, 0, sizeof(Scene));
	scene.name = "Start";
	scene.initialize = &init;
	scene.update = &update;
	scene.draw = &draw;
	scene.destroy = &destroy;
	scene.on_key_down = &on_key_down;
	scene.on_mouse_down = &on_mouse_down;
	// TODO: Register more event callback functions such as keyboard, mouse, ...
	game_log("Start scene created");
	return scene;
}

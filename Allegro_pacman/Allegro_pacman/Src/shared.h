// [shared.h]
// you should put shared variables between files in this header.

#ifndef SCENE_SHARED_H
#define SCENE_SHARED_H
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_audio.h>

// TODO: Shared constant variable (without initializing value).

// TODO: More shared resources or data that needed to be accessed
// across different scenes. initialize the value in 'shared.c'.;
;
extern ALLEGRO_FONT* font_pirulen_32;
extern ALLEGRO_FONT* font_pirulen_24;
extern ALLEGRO_SAMPLE* themeMusic;
extern ALLEGRO_SAMPLE* PACMAN_MOVESOUND;
extern ALLEGRO_SAMPLE* PACMAN_DEATH_SOUND;
extern ALLEGRO_FONT* menuFont;
extern int fontSize;
extern float music_volume;
extern float effect_volume;
extern bool gameDone;

// Difficulty chosen on the menu; drives ghost count, ghost speed and power-up
// length in the game scene. game_difficulty persists between rounds.
typedef enum { DIFF_EASY = 0, DIFF_NORMAL = 1, DIFF_HARD = 2 } Difficulty;
extern Difficulty game_difficulty;
extern int ghost_base_speed;

// Initialize shared variables and resources.
// Allows the game to perform any initialization it needs before
// starting to run.
void shared_init(void);
// Free shared variables and resources.
void shared_destroy(void);

#endif
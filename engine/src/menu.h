/*
	This file is part of SHMUP.

    SHMUP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    SHMUP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/    
/*
 *  menu.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-06-19.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_MENU
#define DE_MENU

#include "math.h"
#include "texture.h"
#include "fx.h"
#include "sounds.h"
#include "commands.h"

typedef void (*buttonAction)(void* tag)  ;
typedef void (*buttonUpdate)()  ;

typedef struct button_t
{
	xf_colorless_sprite_t upVertices[4];
	xf_colorless_sprite_t downVertices[4];
	
	touch_t* touch;
	vec2short_t textPos;
	
	char* text;
	float font_size;
	buttonAction action;
	buttonUpdate update;
	void* tag;
	char actionTriggered;
	
} menu_button_t;

typedef struct menu_text_t
{
	vec2short_t textPos;
	float font_size;
	char* text;
	uchar centerStyle;
} menu_text_t;

typedef struct menu_image_t
{
	xf_colorless_sprite_t vertices[4];
} menu_image_t;

#define MAX_NUM_MENU_TEXTS 16
#define MAX_NUM_MENU_BUTTONS 16
#define MAX_NUM_MENU_IMAGES 4


typedef struct menu_screen_t
{
	
	
	char numTexts;
	menu_text_t texts[MAX_NUM_MENU_TEXTS];
	
	char numButtons;
	menu_button_t buttons[MAX_NUM_MENU_BUTTONS];
	touch_t touches[MAX_NUM_MENU_BUTTONS];
	
	char numImages;
	menu_image_t images[MAX_NUM_MENU_IMAGES];
	
	float alpha;

} menu_screen_t;

#define MENU_NONE -1
#define MENU_HOME 0
#define MENU_CREDITS 1
#define MENU_ABOUT 2
#define MENU_MULTIPLAYER 3
#define MENU_GAMEOVER 4
#define MENU_REPLAY 5
#define MENU_OTHERS 6
#define MENU_SELECT_DIFFICULTY 7

void MENU_Init(void);
void MENU_Set(signed char menuId);
signed char MENU_Get(void);
void MENU_Render(void);
void MENU_HandleTouches(void);
void MENU_FreeRessources(void);
touch_t* MENU_GetCurrentButtonTouches(void);
int MENU_GetNumButtonsTouches(void);
char* MENU_GetMultiplayerTextLine(int i);
#define FADING_IN_TIME_PER_MS (1/1000.0f)



#endif
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
    along with SHMUP.  If not, see <http://www.gnu.org/licenses/>.
*/    
/*
 *  event.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-02-11.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_EVENTS
#define DE_EVENTS

#include "globals.h"
#include "math.h"
#include "enemy.h"

#define EV_ROOT				0x0
#define EV_ATTACH_PLAYER	0x1
#define EV_DETACH_PLAYER	0x2
#define EV_SPAWN_ENEMY		0x3
#define EV_DISPLAY_STATS	0x4
#define EV_MASK_STATS		0x5
#define EV_SHOW_PROLOG		0x6
#define EV_SHOW_EPILOG		0x7
#define EV_REQUEST_SCENE	0x8
#define EV_SPAWN_TEXT		0x9
#define EV_STOP_PLAYBACK	0xA
#define EV_REQUEST_MENU		0xB
#define EV_AUTOPILOT_PL		0xC
#define EV_SAVE_SCORE		0xD
#define EV_LIMITED_EVENT	0xE
#define EV_CLEAR_TITLE      0xF

typedef struct event_t
{
	int time;
	ushort type;
	struct event_t* next;
	void* payload;
	
} event_t ;

typedef struct event_spawnEnemy_payload_t
{
	uchar type;
	
	float xAxisRot;
	float yAxisRot;
	float zAxisRot;
	
	vec2_t startPosition;
	vec2_t endPosition;
	vec2_t controlPoint;
	
	uchar mouvementPatternType;
	
	int ttl;
	uchar subType ;
	
	float parameters[NUM_ENEMY_PARAMETERS];
	
} event_spawnEnemy_payload_t ;

typedef struct event_title_payload_t
{
	int duration;
} event_title_payload_t;

typedef struct event_text_payload_t
{
	int duration;
	char text[256];
	float size;
	vec2short_t ss_start_pos;
	vec2short_t ss_end_pos;
} event_text_payload_t;

typedef struct event_req_scene_t
{
	int sceneId;
} event_req_scene_t;

typedef struct event_req_menu_t
{
	int menuId;
} event_req_menu_t;

void EV_InitForScene(void);
void EV_ReadEnemiesEvents(void);
void EV_ReadTextsEvents(void);
void EV_Update(void);
void EV_CleanAllRemainingEvents(void);
void EV_AddEvent(event_t* event);
event_t*  EV_GetNextEvent(void);
extern event_t events;

#endif
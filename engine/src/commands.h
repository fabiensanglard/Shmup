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
 *  commands.h
 *  dEngine
 *
 *  Created by fabien sanglard on 09-10-09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */



#ifndef DE_COMMAND
#define DE_COMMAND

#include "globals.h"
#include "math.h"

#include "player.h"



#define BUTTON_MOVE  0
#define BUTTON_FIRE  1
#define BUTTON_GHOST 2


#define BUTTON_FIRE_PRESSED 0x1
#define BUTTON_GHOST_PRESSED 0x2
#define BUTTON_MOVE_PRESSED 0x4

#define NET_RTM_COMMAND 3
#define NET_RTM_ABS_UPDATE 4
#define NET_RTM_DIED 5

typedef struct command_t
{
	uchar type;
	uint time; 
	vec2_t delta;
	uchar buttons;
	uchar playerId;
	
} command_t;

typedef struct xf_textureless_sprite_t
{
	vec2short_t pos;
	uchar color[4];	
} xf_textureless_sprite_t;

typedef struct touch_t
{
	float dist[2];
	short iphone_coo_SysPos[2];
	float iphone_size;
	char down;
} touch_t;

void COM_ResetTouchesBuffer(void);
void COM_Init(void);
void COM_StartScene(void);
void COM_EndtScene(void);
void COM_Update(void) ;
void COM_StopRecording(void);
void COM_InitPlayback(char* filename);

#define COMMAND_BUFFER_SIZE 16
typedef struct command_buffer_t
{
	uchar numCommands;
	command_t cmds[COMMAND_BUFFER_SIZE];
} command_buffer_t;

extern command_buffer_t commandsBuffers[MAX_NUM_PLAYERS];

extern command_t toSend;


extern touch_t touches[NUM_BUTTONS];

#define NUM_CIRCLES 3
#define NUM_VERTICES_PER_CIRCLES 12
//extern  ushort controlIndices[NUM_CIRCLES*NUM_VERTICES_PER_CIRCLES*6] ;
extern ushort controlIndices[16] ;
extern ushort controlNumIndices;
#define NUM_CONTROL_VERTICES (NUM_CIRCLES*NUM_VERTICES_PER_CIRCLES)
extern xf_textureless_sprite_t controlVertices[NUM_CONTROL_VERTICES];
extern command_t lastLocalCommand;


void COM_ClearBuffers(void);


#endif
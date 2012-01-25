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
 *  enemy.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-02-09.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_ENEMY
#define DE_ENEMY


#include "globals.h"
#include "math.h"
#include "entities.h"




// Enemy types
#define ENEMY_HAB		0
#define ENEMY_FHT		1
#define ENEMY_LEE		2
#define ENEMY_SHAB		3
#define ENEMY_LOFB		4
#define ENEMY_THA		5


// Enemy types
#define ENEMY_SUBTYPE_NORMAL 0
#define ENEMY_SUBTYPE_HAAARD 1
#define ENEMY_SUBTYPE_IMPOSSIBLE 2
#define ENEMY_SUBTYPE_WEAK 3

#define MAX_NUM_ENEMIES 64

#define MVMT_CIRCLE 0
#define MVMT_STRAIGHT 1
#define MVMT_X_SIN 2

struct enemy_t ;

typedef void (*updateFunction_t)(struct enemy_t*)  ;

#define NUM_ENEMY_PARAMETERS 5

typedef struct enemy_t
{
	unsigned int uniqueId;
	
	short ss_boudaries[4];
	
	short energy;
	
	vec2_t ss_position;	
	
	entity_t entity;
	
	updateFunction_t updateFunction;
	

	
	ushort type;
	
	
	//float spawnZAxisRot;
	float spawn_X_AxisRot;
	float spawn_Y_AxisRot;
	float spawn_Z_AxisRot;
	vec2_t spawn_startPosition;
	vec2_t spawn_endPosition;
	vec2_t spawn_controlPoint;
	
	ushort timeCounter;
	int ttl;
	float fttl;
	
	struct enemy_t* next;
	struct enemy_t* prev;
	
	uchar shouldFlicker;
	
	int lastTimeFired;
	
	uchar state;
	
	float parameters[NUM_ENEMY_PARAMETERS];
	
	uchar score;
} enemy_t ;

typedef void (*stateFunction)(enemy_t* enemy);

extern char* enemyTypePath[] ;
extern updateFunction_t enemyTypeUpdateFct[];
extern ushort enemyTypeEnergy[];
extern uint enemyScore[];

void ENE_Mem_Init(void);
void ENE_Precache(void);
void ENE_Update(void);
void ENE_Reset(void);

enemy_t* ENE_Get(void);
void ENE_Release(enemy_t* enemy);
void ENE_ReleaseAll(void);
void ENE_AttachToCamera(matrix_t globalMatrix);
enemy_t* ENE_GetFirstEnemy(void);
int ENE_GetNumEnemies(void);
void ENE_UpdateSSBoundaries(enemy_t* enemy);

void Spawn_EntityParticules(vec2_t ss_position);

extern void updateDEVIL(enemy_t* enemy);
extern void updateMINE(enemy_t* enemy);


#define FX_ENEMY_WHITE_ORIGIN_U (64.0f/128.0f * SHRT_MAX)
#define FX_ENEMY_WHITE_ORIGIN_V (0/128.0f * SHRT_MAX)
#define FX_ENEMY_WHITE_WIDTH (16/128.0f * SHRT_MAX)
#define FX_ENEMY_WHITE_HEIGHT (16/128.0f * SHRT_MAX)
#define WHITE_PRE_BULLET_SIZE (LEE_BULLET_SIZE  )

#endif
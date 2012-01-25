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
 *  enemy_particules.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-10-04.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */


#ifndef DE_ENEMY_ARTICULES
#define DE_ENEMY_ARTICULES

#include "math.h"
#include "fx.h"

#define MAX_NUM_ENEMY_PARTICULES 512
#define MAX_NUM_ENEMY_PARTICULES_INDICES (MAX_NUM_ENEMY_PARTICULES*4*6)
typedef struct enemy_part_t
{
	short ss_boudaries[4];
	short ss_starting_boudaries[4];
	
	vec2short_t posDiff;
	int ttl;
	float originalTTL;
	ushort text[4][2];
} enemy_part_t ;


void ENPAR_Init(void);
void ENPAR_Reset(void);
enemy_part_t* ENPAR_GetNextParticule(void);
void ENPAR_Update(void);

typedef struct enemy_particule_lib_t
{
	enemy_part_t particules[MAX_NUM_ENEMY_PARTICULES];
	int numParticules;
	
	xf_colorless_sprite_t ss_vertices[4*MAX_NUM_ENEMY_PARTICULES];
	ushort num_indices;
	ushort indices[MAX_NUM_ENEMY_PARTICULES_INDICES];
	
} enemy_particule_lib_t ;

extern enemy_particule_lib_t partLib;






//Enemy fxs
#define MAX_NUM_ENEMY_FX 64
typedef struct enemy_fx_lib_t
{
	xf_sprite_t ss_vertices[4*MAX_NUM_ENEMY_FX];
	ushort num_vertices;
	ushort num_indices;
	ushort indices[MAX_NUM_ENEMY_FX*6];
} enemy_fx_lib_t;

extern enemy_fx_lib_t enFxLib;

void ENPAR_StartEnemyFX(void);

#endif
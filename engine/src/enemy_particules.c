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
 *  enemy_particules.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-10-04.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "enemy_particules.h"
#include "timer.h"


enemy_part_t dumpParticule;

/*

 Particules and FX vertices are as follow:

 0   3
 1   2
  
*/
enemy_particule_lib_t partLib;
enemy_fx_lib_t enFxLib;

void ENPAR_Init(void)
{
	int i;
	int numVertices=0;
	
	for (i=0 ; i < MAX_NUM_ENEMY_PARTICULES_INDICES ; ) {
		partLib.indices[i+0] = numVertices + 0;
		partLib.indices[i+1] = numVertices + 1;
		partLib.indices[i+2] = numVertices + 2;
		partLib.indices[i+3] = numVertices + 0;
		partLib.indices[i+4] = numVertices + 2;
		partLib.indices[i+5] = numVertices + 3;
		
		i+=6;
		numVertices+=4;
	}
	
	numVertices=0;
	for (i=0; i < MAX_NUM_ENEMY_FX ; ) 
	{
		enFxLib.indices[i+0] = numVertices + 0;
		enFxLib.indices[i+1] = numVertices + 1;
		enFxLib.indices[i+2] = numVertices + 2;
		enFxLib.indices[i+3] = numVertices + 0;
		enFxLib.indices[i+4] = numVertices + 2;
		enFxLib.indices[i+5] = numVertices + 3;
		
		i+=6;
		numVertices+=4;
	}
	
	
	
	for (i=0; i < 4*MAX_NUM_ENEMY_FX ; i++) 
	{
		enFxLib.ss_vertices[i].color[R] = 
		enFxLib.ss_vertices[i].color[G] = 
		enFxLib.ss_vertices[i].color[B] = 
		enFxLib.ss_vertices[i].color[A] = 255; 
	}
	
	
}

void ENPAR_Reset(void)
{
	int i;

	
	for (i=0; i < MAX_NUM_ENEMY_PARTICULES; i++) 
	{
		partLib.particules[i].ttl = 0;
	}
	
	partLib.numParticules = 0;
	
	//Also need to generate the indices array used for rendition
	partLib.num_indices = 0;
	
	
	//Also take care of the fx part:
	enFxLib.num_indices = 0;
}

enemy_part_t* ENPAR_GetNextParticule(void)
{
	if ( partLib.numParticules == MAX_NUM_ENEMY_PARTICULES)
		return &dumpParticule;
	
	partLib.num_indices += 6;
	
	return &partLib.particules[partLib.numParticules++];
}



void ENPAR_Update(void)
{
	int i;
	float interpolation;
	vec2short_t delta;
	xf_colorless_sprite_t* sprite;

	enemy_part_t* currParticule;
	
	partLib.num_indices = 0;
	sprite = partLib.ss_vertices;
	
	for (i=0; i < partLib.numParticules; i++) 
	{
		while (partLib.particules[i].ttl <=0 )
		{
			//Need to remove the particule from the list by flipping with the last one
			if ( partLib.numParticules > 0)
			{
				partLib.particules[i] = partLib.particules[partLib.numParticules-1];
				partLib.numParticules--;
			}
			else 
			{
				return;
			}
		}
		
		currParticule = &partLib.particules[i];
		currParticule->ttl -= timediff;
		
		
		
		//Update vertices position normaly with speed
		interpolation = 1- currParticule->ttl / currParticule->originalTTL ;
		
		//printf("interpolation = %.2f\n",interpolation );
		
		delta[X] = partLib.particules[i].posDiff[X] * interpolation;
		delta[Y] = partLib.particules[i].posDiff[Y] * interpolation;
		
		//Update ss_boundaries
		currParticule->ss_boudaries[UP] = currParticule->ss_starting_boudaries[UP] + delta[Y];
		currParticule->ss_boudaries[DOWN] = currParticule->ss_starting_boudaries[DOWN] + delta[Y];
		currParticule->ss_boudaries[LEFT] = currParticule->ss_starting_boudaries[LEFT] + delta[X];
		currParticule->ss_boudaries[RIGHT] = currParticule->ss_starting_boudaries[RIGHT] + delta[X];
		
		//Populate drawing array
		sprite->pos[Y] = currParticule->ss_boudaries[UP] ;
		sprite->pos[X] = currParticule->ss_boudaries[LEFT] ;
		sprite->text[U] = currParticule->text[0][U];
		sprite->text[V] = currParticule->text[0][V];
		sprite++;

		sprite->pos[Y] = currParticule->ss_boudaries[DOWN] ;
		sprite->pos[X] = currParticule->ss_boudaries[LEFT] ;
		sprite->text[U] = currParticule->text[1][U];
		sprite->text[V] = currParticule->text[1][V];
		sprite++;

		
		sprite->pos[Y] = currParticule->ss_boudaries[DOWN] ;
		sprite->pos[X] = currParticule->ss_boudaries[RIGHT] ;
		sprite->text[U] = currParticule->text[2][U];
		sprite->text[V] = currParticule->text[2][V];
		sprite++;

		
		sprite->pos[Y] = currParticule->ss_boudaries[UP] ;
		sprite->pos[X] = currParticule->ss_boudaries[RIGHT] ;
		sprite->text[U] = currParticule->text[3][U];
		sprite->text[V] = currParticule->text[3][V];
		sprite++;
		
		/*
		printf("part %d UP=%d DOWN=%d LEFT=%d RIGHT=%d\n",
			   i,
			   currParticule->ss_boudaries[UP],
			   currParticule->ss_boudaries[DOWN],
			   currParticule->ss_boudaries[LEFT],
			   currParticule->ss_boudaries[RIGHT]);
		*/
		partLib.num_indices += 6;
		
		
	}
	
}

void ENPAR_StartEnemyFX(void)
{
	enFxLib.num_indices = 0;
	enFxLib.num_vertices = 0;
}




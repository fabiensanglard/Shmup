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
 *  shab.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-10-25.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "shab.h"

#include "lofb.h"
#include "globals.h"
#include "timer.h"
#include "enemy_particules.h"
#include "sounds.h"

#define STATE_SHAB_SPAWNING			0
#define STATE_SHAB_HELLING			1
#define STATE_SHAB_LEAVING			2

//#define SHAB_TTL  (enemy->)
//7000.0f
#define SHAB_TTR  700.0f
#define SHAB_TIME_FIRING (enemy->ttl - SHAB_TTR - SHAB_TTR )
#define SHAB_TIME_FIRING_LIMIT (enemy->ttl - SHAB_TTR)

#define SHAB_TEXT_BULLET_U (80/128.0f*SHRT_MAX)
#define SHAB_TEXT_BULLET_V (0/128.0f*SHRT_MAX)
#define SHAB_TEXT_BULLET_WIDTH (16/128.0f*SHRT_MAX)
#define SHAB_TEXT_BULLET_HEIGHT (16/128.0f*SHRT_MAX)
#define SHAB_BULLET_TTL 2000
#define SHAB_BULLET_DISTANCE_TTL (2.2f)
#define SHAB_BULLET_SIZE 0.055f

#define SHAB_FIRE_DELAY_MS 100
//#define SHAB_FULL_CIRCLES_HELL_PER_TTL 2
//#define SHAB_NUM_HELICES 4
#define SHAB_PARTICULE_IMPLOSION_RATION 0.7


void updateSHABSpawning(enemy_t* enemy)
{
	float f;
	
	f = enemy->timeCounter / SHAB_TTR ;
	
	if (f>1)
		f= 1;
	
	enemy->ss_position[X] = enemy->spawn_startPosition[X] + f*(enemy->spawn_endPosition[X] - enemy->spawn_startPosition[X]);
	enemy->ss_position[Y] = enemy->spawn_startPosition[Y] + f*(enemy->spawn_endPosition[Y] - enemy->spawn_startPosition[Y]);
	
	//As a devil is seen from above, the roll is actually a rotatiom around Y.
	enemy->entity.zAxisRot = (1-f)*enemy->spawn_Z_AxisRot;
	
	
	
	if (enemy->timeCounter >= SHAB_TTR)
	{
		enemy->entity.zAxisRot = 0 ;
		enemy->state = STATE_SHAB_HELLING;
	}
}



void emitSHABBullet(enemy_t* enemy,float angle)
{
	enemy_part_t* bullet;
	//float tmp ;
	//float cosAngle;
	//float sinAngle;
	
	bullet = ENPAR_GetNextParticule();
	
	bullet->ttl = SHAB_BULLET_TTL;
	bullet->originalTTL = SHAB_BULLET_TTL;
	
	
	// 0 3
	// 1 2
	
	bullet->ss_boudaries[UP] = bullet->ss_starting_boudaries[UP] = enemy->ss_position[Y] * SS_H + SHAB_BULLET_SIZE /2 * SS_H;
	bullet->ss_boudaries[DOWN] = bullet->ss_starting_boudaries[DOWN] = enemy->ss_position[Y] * SS_H - SHAB_BULLET_SIZE /2 * SS_H;
	bullet->ss_boudaries[LEFT] = bullet->ss_starting_boudaries[LEFT] = enemy->ss_position[X] * SS_W  - SHAB_BULLET_SIZE /2 *SS_H/(float)SS_W * SS_W;
	bullet->ss_boudaries[RIGHT] = bullet->ss_starting_boudaries[RIGHT] = enemy->ss_position[X] * SS_W  + SHAB_BULLET_SIZE /2 *SS_H/(float)SS_W * SS_W;
	
	bullet->text[0][U] = SHAB_TEXT_BULLET_U;
	bullet->text[0][V] = SHAB_TEXT_BULLET_V;
	
	bullet->text[1][U] = SHAB_TEXT_BULLET_U;
	bullet->text[1][V] = SHAB_TEXT_BULLET_V + SHAB_TEXT_BULLET_HEIGHT;
	
	bullet->text[2][U] = SHAB_TEXT_BULLET_U + SHAB_TEXT_BULLET_WIDTH ;
	bullet->text[2][V] = SHAB_TEXT_BULLET_V + SHAB_TEXT_BULLET_HEIGHT;
	
	bullet->text[3][U] = SHAB_TEXT_BULLET_U + SHAB_TEXT_BULLET_WIDTH ;
	bullet->text[3][V] = SHAB_TEXT_BULLET_V;
	
	
	//Roate diff by angle
	//tmp = bullet->posDiff[X];
	//cosAngle = cosf(angle);
	//sinAngle = sinf(angle);
	//printf("[emitSHABBullet] angle=%.2f\n",angle);
	
	bullet->posDiff[X] = cosf(angle)*SHAB_BULLET_DISTANCE_TTL*SS_H;//bullet->posDiff[X] * cosAngle - bullet->posDiff[Y] *  sinAngle; 
	bullet->posDiff[Y] = sinf(angle)*SHAB_BULLET_DISTANCE_TTL*SS_H;//tmp                * sinAngle + bullet->posDiff[Y] *  cosAngle;
	
	//printf("[emitSHABBullet] posDiffX=%d posDiffY=%d\n",bullet->posDiff[X],bullet->posDiff[Y]);
}


void updateSHABHelling(enemy_t* enemy)
{
	//Pour hell
	float f;
	float angle;
	vec2_t random ;
	short ss_boundaries[4];
	xf_sprite_t* sprite;
	float step;
	float a1,a2,i;
	
	f = (enemy->timeCounter - enemy->ttl) / (float)SHAB_TIME_FIRING;
	
	if ((enemy->timeCounter > SHAB_TTR+ 2000) && (simulationTime -  enemy->lastTimeFired) >= SHAB_FIRE_DELAY_MS)
	{
		
		angle = enemy->parameters[PARAMETER_SHAB_FIRING_ROT_ANGLE]  * f;
		
		a1 = angle + enemy->parameters[PARAMETER_SHAB_FIRING_ANGLE1];
		a2 = angle + enemy->parameters[PARAMETER_SHAB_FIRING_ANGLE2];
		
		step = ( a2-a1)/enemy->parameters[PARAMETER_SHAB_FIRING_NUM_THREAD];
		
		//printf("a1=%.2f, a2=%.2f step=%.2f.\n",a1,a2,step);
		
		for(angle = a1,i=0; 
			i <= enemy->parameters[PARAMETER_SHAB_FIRING_NUM_THREAD] ; 
			angle += step,i++)
		{
			emitSHABBullet(enemy,angle);
			SND_PlaySound(SND_ENEMY_SHOT);
			//printf("emitSHABBullet(%.2f).\n",angle);
		}
		enemy->lastTimeFired = simulationTime;
		
		
		
		
	}
	
	enemy->ss_position[X] += 0.002*cosf( (enemy->uniqueId + enemy->timeCounter / enemy->fttl) * 4 * 2 * M_PI);
	enemy->ss_position[Y] += 0.002*sinf( (enemy->uniqueId + enemy->timeCounter / enemy->fttl) * 4 * 2 * M_PI);
	
	
	//Retina persistence Flash simulator
	if (simulationTime -  enemy->lastTimeFired < SHAB_FIRE_DELAY_MS)
	{
		f = (simulationTime -  enemy->lastTimeFired) / (float)SHAB_FIRE_DELAY_MS;
		
		sprite = &enFxLib.ss_vertices[enFxLib.num_vertices];
		
		//Output a white ball 
		ss_boundaries[UP]    =   enemy->ss_position[Y] * SS_H + f *SHAB_BULLET_SIZE  * SS_H;
		ss_boundaries[DOWN]  =   enemy->ss_position[Y] * SS_H - f *SHAB_BULLET_SIZE  * SS_H;
		ss_boundaries[LEFT]  =   enemy->ss_position[X] * SS_W - f *SHAB_BULLET_SIZE  * SS_H;
		ss_boundaries[RIGHT] =   enemy->ss_position[X] * SS_W + f *SHAB_BULLET_SIZE  * SS_H;
		
		
		sprite->pos[Y] =  ss_boundaries[UP];
		sprite->pos[X] =  ss_boundaries[LEFT];
		sprite->text[U] = FX_ENEMY_WHITE_ORIGIN_U ;
		sprite->text[V] = FX_ENEMY_WHITE_ORIGIN_V ;
		sprite->color[A] = 255*f;
		sprite++;
		
		sprite->pos[Y] =  ss_boundaries[DOWN];
		sprite->pos[X] =  ss_boundaries[LEFT];
		sprite->text[U] = FX_ENEMY_WHITE_ORIGIN_U ;
		sprite->text[V] = FX_ENEMY_WHITE_ORIGIN_V + FX_ENEMY_WHITE_HEIGHT;
		sprite->color[A] = 255*f;
		sprite++;
		
		
		sprite->pos[Y] =  ss_boundaries[DOWN];
		sprite->pos[X] =  ss_boundaries[RIGHT];
		sprite->text[U] = FX_ENEMY_WHITE_ORIGIN_U + FX_ENEMY_WHITE_WIDTH;
		sprite->text[V] = FX_ENEMY_WHITE_ORIGIN_V + FX_ENEMY_WHITE_HEIGHT;
		sprite->color[A] = 255*f;
		sprite++;
		
		
		sprite->pos[Y] =  ss_boundaries[UP];
		sprite->pos[X] =  ss_boundaries[RIGHT];
		sprite->text[U] = FX_ENEMY_WHITE_ORIGIN_U + FX_ENEMY_WHITE_WIDTH;
		sprite->text[V] = FX_ENEMY_WHITE_ORIGIN_V ;
		sprite->color[A] = 255*f;		
		//sprite++;
		
		enFxLib.num_vertices += 4 ;
		enFxLib.num_indices  += 6 ;
	}
	//f = (simulationTime -  enemy->lastTimeFired) / (float)SHAB_FIRE_DELAY_MS;
	
	//printf("%.2f\n",f);
	
	//return;
	
	
	
	
	
	random[X] = (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1);
	random[Y] = (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1);
	normalize2(random);
	FX_GetParticule(enemy->ss_position,random		,0.02*PARTICULE_SIZE_GLOBAL*SHAB_PARTICULE_IMPLOSION_RATION	,PARTICULE_TRAVEL_DIST*0.6*rand()/(float)RAND_MAX,PARTICULE_TYPE_IMPLOSION, PARTICULE_COLOR_WHITE,3.0f);
	
	
	if (enemy->timeCounter >= SHAB_TIME_FIRING_LIMIT)
		enemy->state = STATE_SHAB_LEAVING;
	
}

void updateSHABLeaving(enemy_t* enemy)
{
	float f;
	
	f = (enemy->timeCounter - SHAB_TIME_FIRING_LIMIT) / SHAB_TTR ;
	
	if (f>1)
		f= 1;
	
	enemy->ss_position[X] = enemy->spawn_endPosition[X] + f*(enemy->spawn_startPosition[X] - enemy->spawn_endPosition[X]);
	enemy->ss_position[Y] = enemy->spawn_endPosition[Y] + f*(enemy->spawn_startPosition[Y] - enemy->spawn_endPosition[Y]);
	
	//As a devil is seen from above, the roll is actually a rotatiom around Y.
	enemy->entity.zAxisRot = f*enemy->spawn_Z_AxisRot ;
	
	
	
	if (enemy->timeCounter >= enemy->ttl)
		ENE_Release(enemy);
	
}


stateFunction shab_states[] = {updateSHABSpawning,updateSHABHelling,updateSHABLeaving};

void updateSHAB(enemy_t* enemy)
{
	shab_states[enemy->state](enemy);
}
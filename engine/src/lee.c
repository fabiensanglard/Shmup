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
 *  lee.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-10-19.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "lee.h"
#include "timer.h"
#include "enemy_particules.h"
#include "globals.h"
#include "player.h"
#include "sounds.h"


//#define LEE_TTL 5000
#define LEE_APP_TIME 1000
#define LEE_UNSPAWNING_START_TIME (enemy->ttl-1000)



#define LEE_BULLET_SIZE 0.07f
#define LEE_BULLET_TTL 2700
#define LEE_BULLET_DISTANCE_TTL (-2)

#define LEE_TEXT_BULLET_U (80/128.0f*SHRT_MAX)
#define LEE_TEXT_BULLET_V (0/128.0f*SHRT_MAX)
#define LEE_TEXT_BULLET_WIDTH (16/128.0f*SHRT_MAX)
#define LEE_TEXT_BULLET_HEIGHT (16/128.0f*SHRT_MAX)
#define LEE_PARTICULE_IMPLOSION_RATION 0.8
#define LEE_PARTICULE_EXPLOSION_RATION 0.7

#define LEE_PRE_PREPARE_BULLET_TOTAL_TIME 1000.0f
#define LEE_PRE_PREPARE_BULLET_GROWING_LIMIT 0.7f
//#define LEE_PRE_PREPARE_BULLET_SHRINK_LIMIT 1.0f

/*
void updateSpawning(enemy_t* enemy);
void updateThinking(enemy_t* enemy);
void updatePrepareBullet(enemy_t* enemy);
void updateShoot(enemy_t* enemy);
void updateUnspawning(enemy_t* enemy);
*/

 #define STATE_LEE_SPAWNING			0
 #define STATE_LEE_THINKING			1
 #define STATE_LEE_PREPARE_BULLET	2
 #define STATE_LEE_SHOOT				3
 #define STATE_LEE_UNSPAWNING		4
 
void updateSpawning(enemy_t* enemy)
{
	float f;
	float oneMinusF;
	f = MIN(1,enemy->timeCounter / (float)(LEE_APP_TIME)) ;
	oneMinusF = 1 -f;
	
	enemy->entity.zAxisRot = enemy->spawn_Z_AxisRot* oneMinusF;
	
	enemy->ss_position[X] = oneMinusF*oneMinusF * enemy->spawn_startPosition[X] + 2*oneMinusF*f*enemy->spawn_controlPoint[X]+ f*f*enemy->spawn_endPosition[X];
	enemy->ss_position[Y] = oneMinusF*oneMinusF * enemy->spawn_startPosition[Y] + 2*oneMinusF*f*enemy->spawn_controlPoint[Y]+f*f*enemy->spawn_endPosition[Y];
	
	
	if (enemy->timeCounter >  LEE_APP_TIME)
		enemy->state = STATE_LEE_THINKING;
	
}

void updateThinking(enemy_t* enemy)
{
	/*
	if (enemy->lastTimeFired + LEE_MS_BETWEEN_FIRE <= simulationTime)
	{
		enemy->state = STATE_LEE_SHOOT;
		return;
	}
	*/
	
	if (enemy->timeCounter >= LEE_UNSPAWNING_START_TIME)
	{
		enemy->state = STATE_LEE_UNSPAWNING;
		return;
	}
	
	enemy->state = STATE_LEE_PREPARE_BULLET;
}


void emitBullet(enemy_t* enemy)
{
	enemy_part_t* bullet;
	vec2_t playerDirection ;
	
	
	enemy->lastTimeFired = simulationTime;
	bullet = ENPAR_GetNextParticule();
	
	bullet->ttl = LEE_BULLET_TTL;
	bullet->originalTTL = LEE_BULLET_TTL;
	
	
	// 0 3
	// 1 2

	bullet->ss_boudaries[UP] = bullet->ss_starting_boudaries[UP] = enemy->ss_position[Y] * SS_H + LEE_BULLET_SIZE /2 * SS_H;
	bullet->ss_boudaries[DOWN] = bullet->ss_starting_boudaries[DOWN] = enemy->ss_position[Y] * SS_H - LEE_BULLET_SIZE /2 * SS_H;
	bullet->ss_boudaries[LEFT] = bullet->ss_starting_boudaries[LEFT] = enemy->ss_position[X] * SS_W  - LEE_BULLET_SIZE /2 *SS_H/(float)SS_W * SS_W;
	bullet->ss_boudaries[RIGHT] = bullet->ss_starting_boudaries[RIGHT] = enemy->ss_position[X] * SS_W  + LEE_BULLET_SIZE /2 *SS_H/(float)SS_W * SS_W;
	
	bullet->text[0][U] = LEE_TEXT_BULLET_U;
	bullet->text[0][V] = LEE_TEXT_BULLET_V;
	
	bullet->text[1][U] = LEE_TEXT_BULLET_U;
	bullet->text[1][V] = LEE_TEXT_BULLET_V + LEE_TEXT_BULLET_HEIGHT;
	
	bullet->text[2][U] = LEE_TEXT_BULLET_U + LEE_TEXT_BULLET_WIDTH ;
	bullet->text[2][V] = LEE_TEXT_BULLET_V + LEE_TEXT_BULLET_HEIGHT;
	
	bullet->text[3][U] = LEE_TEXT_BULLET_U + LEE_TEXT_BULLET_WIDTH ;
	bullet->text[3][V] = LEE_TEXT_BULLET_V;
	
	if (enemy->parameters[PARAMETER_LEE_FIRING_TYPE] == LEE_FIRING_TYPE_TARGET_PLAYER)
	{
		playerDirection[X] = enemy->ss_position[X] - players[0].ss_position[X];
		playerDirection[Y] = enemy->ss_position[Y] - players[0].ss_position[Y];
		
		normalize2(playerDirection);
		
		bullet->posDiff[X] = playerDirection[X] *enemy->parameters[PARAMETER_LEE_BULLET_SPEED_FACTOR]*LEE_BULLET_DISTANCE_TTL * SS_W;
		bullet->posDiff[Y] = playerDirection[Y] *enemy->parameters[PARAMETER_LEE_BULLET_SPEED_FACTOR]*LEE_BULLET_DISTANCE_TTL * SS_H;
		
	}
	else 
	if (enemy->parameters[PARAMETER_LEE_FIRING_TYPE] == LEE_FIRING_TYPE_DOWN)
	{
		
		bullet->posDiff[X] = LEE_BULLET_DISTANCE_TTL *SS_H*enemy->parameters[PARAMETER_LEE_BULLET_SPEED_FACTOR]*cosf(enemy->entity.yAxisRot+M_PI/2);
		bullet->posDiff[Y] = LEE_BULLET_DISTANCE_TTL *SS_H*enemy->parameters[PARAMETER_LEE_BULLET_SPEED_FACTOR]*sinf(enemy->entity.yAxisRot+M_PI/2);
	}
	else 
	if (enemy->parameters[PARAMETER_LEE_FIRING_TYPE] == LEE_FIRING_TYPE_NO_FIRE)
	{
			
		bullet->posDiff[X] = enemy->parameters[PARAMETER_LEE_BULLET_SPEED_FACTOR]*-SS_W;
		bullet->posDiff[Y] = enemy->parameters[PARAMETER_LEE_BULLET_SPEED_FACTOR]*LEE_BULLET_DISTANCE_TTL*SS_H;
	}
	
	
	SND_PlaySound(SND_ENEMY_SHOT);
}




void updatePrepareBullet(enemy_t* enemy)
{
	float f;
	xf_sprite_t* sprite;
	short ss_boundaries[4];
	vec2_t random ;
	
	f = (simulationTime - enemy->lastTimeFired) / LEE_PRE_PREPARE_BULLET_TOTAL_TIME;
	
	
	if (f >= 1.0f)
		enemy->state  = STATE_LEE_SHOOT;
	
	//printf("f=%.2f\n",f);
	
	// Prepare bullet has X phases:
	// - Growing white ball. Alpha 0 -> 255
	// - White ball shrinks and behind can be seen the actual bullet
	// - Only bullet can be seen
	// - After this a bullet is emitted and particules are emitted as well.
	
	
	//Ouput an growing progessively white stuff
	sprite = &enFxLib.ss_vertices[enFxLib.num_vertices];
	
	
	// GROWING PHASE
	if (f < LEE_PRE_PREPARE_BULLET_GROWING_LIMIT)
	{
		//from [0,LEE_PRE_PREPARE_BULLET_GROWING_LIMIT] to [0,1]
		f = f/LEE_PRE_PREPARE_BULLET_GROWING_LIMIT ;
		
		
		
		ss_boundaries[UP]    =   enemy->ss_position[Y] * SS_H + f *WHITE_PRE_BULLET_SIZE /2 * SS_H;
		ss_boundaries[DOWN]  =   enemy->ss_position[Y] * SS_H - f *WHITE_PRE_BULLET_SIZE /2 * SS_H;
		
		ss_boundaries[LEFT]  =   enemy->ss_position[X] * SS_W - f *WHITE_PRE_BULLET_SIZE /2 * SS_H;
		ss_boundaries[RIGHT] =   enemy->ss_position[X] * SS_W + f *WHITE_PRE_BULLET_SIZE /2 * SS_H;
		
		//printf("enemy->ss_position[X] = %h , enemy->ss_position[Y] = %h\n",);
		//printf("U=%d L=%d R=%d D=%d\n",ss_boundaries[UP],ss_boundaries[LEFT],ss_boundaries[RIGHT],ss_boundaries[DOWN]);
		
		//f *= 255;
		
		sprite->pos[Y] =  ss_boundaries[UP];
		sprite->pos[X] =  ss_boundaries[LEFT];
		sprite->text[U] = FX_ENEMY_WHITE_ORIGIN_U ;
		sprite->text[V] = FX_ENEMY_WHITE_ORIGIN_V ;
		sprite->color[A] = 255;
		sprite++;

		sprite->pos[Y] =  ss_boundaries[DOWN];
		sprite->pos[X] =  ss_boundaries[LEFT];
		sprite->text[U] = FX_ENEMY_WHITE_ORIGIN_U ;
		sprite->text[V] = FX_ENEMY_WHITE_ORIGIN_V + FX_ENEMY_WHITE_HEIGHT;
		sprite->color[A] = 255;
		sprite++;

		
		sprite->pos[Y] =  ss_boundaries[DOWN];
		sprite->pos[X] =  ss_boundaries[RIGHT];
		sprite->text[U] = FX_ENEMY_WHITE_ORIGIN_U + FX_ENEMY_WHITE_WIDTH;
		sprite->text[V] = FX_ENEMY_WHITE_ORIGIN_V + FX_ENEMY_WHITE_HEIGHT;
		sprite->color[A] = 255;
		sprite++;

		
		sprite->pos[Y] =  ss_boundaries[UP];
		sprite->pos[X] =  ss_boundaries[RIGHT];
		sprite->text[U] = FX_ENEMY_WHITE_ORIGIN_U + FX_ENEMY_WHITE_WIDTH;
		sprite->text[V] = FX_ENEMY_WHITE_ORIGIN_V ;
		sprite->color[A] = 255;		
		sprite++;
		
		enFxLib.num_vertices += 4 ;
		enFxLib.num_indices  += 6 ;
		
		
		
		random[X] = (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1);
		random[Y] = (rand() - RAND_MAX ) / (float)RAND_MAX;
		normalize2(random);
		FX_GetParticule(enemy->ss_position,random		,0.02*PARTICULE_SIZE_GLOBAL*LEE_PARTICULE_IMPLOSION_RATION	,PARTICULE_TRAVEL_DIST/2.5f*rand()/(float)RAND_MAX,PARTICULE_TYPE_IMPLOSION, PARTICULE_COLOR_WHITE,3);
		
	}
	else 
	{
		//from [LEE_PRE_PREPARE_BULLET_GROWING_LIMIT,1] to [0,1]
		f = (f-LEE_PRE_PREPARE_BULLET_GROWING_LIMIT)/(1.0f-LEE_PRE_PREPARE_BULLET_GROWING_LIMIT);
		
		//Output a fake bullet
		ss_boundaries[UP]    =   enemy->ss_position[Y] * SS_H + (LEE_BULLET_SIZE+f*(WHITE_PRE_BULLET_SIZE-LEE_BULLET_SIZE)) /2 * SS_H;
		ss_boundaries[DOWN]  =   enemy->ss_position[Y] * SS_H - (LEE_BULLET_SIZE+f*(WHITE_PRE_BULLET_SIZE-LEE_BULLET_SIZE)) /2 * SS_H;
		ss_boundaries[LEFT]  =   enemy->ss_position[X] * SS_W - (LEE_BULLET_SIZE+f*(WHITE_PRE_BULLET_SIZE-LEE_BULLET_SIZE)) /2 * SS_H;
		ss_boundaries[RIGHT] =   enemy->ss_position[X] * SS_W + (LEE_BULLET_SIZE+f*(WHITE_PRE_BULLET_SIZE-LEE_BULLET_SIZE)) /2 * SS_H;
		
		
		sprite->pos[Y] =  ss_boundaries[UP];
		sprite->pos[X] =  ss_boundaries[LEFT];
		sprite->text[U] = LEE_TEXT_BULLET_U ;
		sprite->text[V] = LEE_TEXT_BULLET_V ;
		sprite->color[A] = 255;
		sprite++;
		
		sprite->pos[Y] =  ss_boundaries[DOWN];
		sprite->pos[X] =  ss_boundaries[LEFT];
		sprite->text[U] = LEE_TEXT_BULLET_U ;
		sprite->text[V] = LEE_TEXT_BULLET_V + LEE_TEXT_BULLET_HEIGHT;
		sprite->color[A] = 255;
		sprite++;
		
		
		sprite->pos[Y] =  ss_boundaries[DOWN];
		sprite->pos[X] =  ss_boundaries[RIGHT];
		sprite->text[U] = LEE_TEXT_BULLET_U + LEE_TEXT_BULLET_WIDTH;
		sprite->text[V] = LEE_TEXT_BULLET_V + LEE_TEXT_BULLET_HEIGHT;
		sprite->color[A] = 255;
		sprite++;
		
		
		sprite->pos[Y] =  ss_boundaries[UP];
		sprite->pos[X] =  ss_boundaries[RIGHT];
		sprite->text[U] = LEE_TEXT_BULLET_U + LEE_TEXT_BULLET_WIDTH;
		sprite->text[V] = LEE_TEXT_BULLET_V ;
		sprite->color[A] = 255;		
		sprite++;
		
		
		
		
		f = 1-f;
		
		//Output a white ball 
		ss_boundaries[UP]    =   enemy->ss_position[Y] * SS_H + f *WHITE_PRE_BULLET_SIZE /2 * SS_H;
		ss_boundaries[DOWN]  =   enemy->ss_position[Y] * SS_H - f *WHITE_PRE_BULLET_SIZE /2 * SS_H;
		
		ss_boundaries[LEFT]  =   enemy->ss_position[X] * SS_W - f *WHITE_PRE_BULLET_SIZE /2 * SS_H;
		ss_boundaries[RIGHT] =   enemy->ss_position[X] * SS_W + f *WHITE_PRE_BULLET_SIZE /2 * SS_H;

		
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
		sprite++;
		
		enFxLib.num_vertices += 8 ;
		enFxLib.num_indices  += 12 ;
	}

	
}

void updateShoot(enemy_t* enemy)
{
	
	emitBullet(enemy);
	
	//Missing emit particules
	
	enemy->state = STATE_LEE_THINKING;
}

void updateUnspawning(enemy_t* enemy)
{
	enemy->ss_position[Y] += timediff * 0.007;
	enemy->entity.zAxisRot += timediff * 0.007;
	
	if (enemy->timeCounter >= enemy->ttl)
		ENE_Release(enemy);
}
	


stateFunction lee_states[] = {updateSpawning,updateThinking,updatePrepareBullet,updateShoot,updateUnspawning};

#define LEE_NUM_ROTATION 1
void updateLEECircle(enemy_t* enemy)
{
	float f;
	float angle;
	//float cosAngle;
	//float sinAngle;
	
	
	f = enemy->timeCounter / enemy->fttl ;
	
	if (f > 0.85f)
	{
		enemy->ss_position[X] += (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1) * 0.1 ;
		enemy->ss_position[Y] += (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1) * 0.1 ;
		FX_GetExplosion(enemy->ss_position,IMPACT_TYPE_YELLOW,1,0);
		Spawn_EntityParticules(enemy->ss_position);
		FX_GetSmoke(enemy->ss_position, 0.3, 0.3);
		enemy->ss_position[X] += (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1) * 0.2 ;
		enemy->ss_position[Y] += (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1) * 0.2 ;
		FX_GetSmoke(enemy->ss_position, 0.3, 0.3);
		SND_PlaySound(SND_EXPLOSION);
		ENE_Release(enemy);
		return;
	}
	
	if (f > 0.3 && simulationTime - enemy->lastTimeFired > enemy->parameters[PARAMETER_LEE_FIRE_FREQUENCY])
	{
		emitBullet(enemy);
	}
	
	angle = enemy->parameters[PARAMETER_LEE_START_ANGLE]  +f * LEE_NUM_ROTATION * 2.0f * M_PI;
	
	enemy->entity.yAxisRot = angle;
	
	//cosAngle = cosf(angle);
	//sinAngle = sinf(angle);
	f -= 1;
	enemy->ss_position[X] = f * cosf(angle+ M_PI+ M_PI/2) * 1.3f* SS_H / SS_W;
	enemy->ss_position[Y] = f * sinf(angle+ M_PI+ M_PI/2) * 1.3f;
	
	//printf("enemy id:%d, angleparam=%.2f\n",enemy->parameters[PARAMETER_LEE_START_ANGLE]);
	//printf("enemy id:%d, angle=%.2f ss_pos[X]=%.2f,ss_pos[Y]=%.2f\n",enemy->uniqueId,enemy->parameters[PARAMETER_LEE_START_ANGLE]+angle,enemy->ss_position[X],enemy->ss_position[Y]);
	//enemy->ss_position[X] = (1-f)*  enemy->spawn_startPosition[X] ;
	//enemy->ss_position[Y] = (1-f)*  enemy->spawn_startPosition[Y] ;
	
	//printf("ene pso = %.2f, %.2f.\n",enemy->ss_position[X],enemy->ss_position[Y]);
	
	
	
	
	
	
}

void updateLEE(enemy_t* enemy)
{
	switch (enemy->entity.mouvementPatternType) 
	{
		case MVMT_CIRCLE: enemy->updateFunction = updateLEECircle; updateLEECircle(enemy); break;
		default: break;
	}
	
	enemy->ss_position[X] += 0.002*cosf( (enemy->uniqueId + enemy->timeCounter / enemy->fttl) * 4 * 2 * M_PI);
	enemy->ss_position[Y] += 0.002*sinf( (enemy->uniqueId + enemy->timeCounter / enemy->fttl) * 4 * 2 * M_PI);
	
	lee_states[enemy->state](enemy);
}




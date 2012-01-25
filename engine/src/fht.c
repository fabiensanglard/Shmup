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
 *  fht.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-11-03.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "fht.h"
#include "fx.h"
#include "sounds.h"

//#define FHT_TTL  6000.0f
#define FHT_NUM_ROTATION 3
//cos (MINE_ROTATION_SPEED_RAD_MS)
//#define MINE_ROTATION_COS 0.999980262
//sin (MINE_ROTATION_SPEED_RAD_MS)
//#define MINE_ROTATION_SIN 0.00628295866

void updateXSin(enemy_t* enemy)
{
	float f;
	float oneMinusF;
	
	f = enemy->timeCounter / enemy->fttl ;	
	oneMinusF = 1 -f;
	
	enemy->entity.yAxisRot = f * FHT_NUM_ROTATION*1.5 * 2.0f * M_PI;
	
	//enemy->ss_position[X] = oneMinusF*oneMinusF * enemy->spawn_startPosition[X] + 2*oneMinusF*f*enemy->spawn_controlPoint[X]+ f*f*enemy->spawn_endPosition[X];
	enemy->ss_position[X] = enemy->parameters[PARAMETER_FHT_X_POS] +  enemy->parameters[PARAMETER_FHT_X_WIDTH]*cos(enemy->spawn_startPosition[X]*M_PI/2 + f * M_PI  * 2*2);
	enemy->ss_position[Y] = oneMinusF*oneMinusF * enemy->spawn_startPosition[Y] + 2*oneMinusF*f*enemy->spawn_controlPoint[Y]+f*f*enemy->spawn_endPosition[Y];
	
	if (enemy->timeCounter > enemy->ttl)
		ENE_Release(enemy);

}

void updateStraight(enemy_t* enemy)
{
	float f;
	float oneMinusF;
	
	
	
	f = enemy->timeCounter / enemy->fttl ;	
	oneMinusF = 1 -f;
	
	enemy->entity.yAxisRot = f * FHT_NUM_ROTATION * 2.0f * M_PI;
	
	//printf("f=%.2f\n",f);
	
	enemy->ss_position[X] = oneMinusF*oneMinusF * enemy->spawn_startPosition[X] + 2*oneMinusF*f*enemy->spawn_controlPoint[X]+ f*f*enemy->spawn_endPosition[X];
	enemy->ss_position[Y] = oneMinusF*oneMinusF * enemy->spawn_startPosition[Y] + 2*oneMinusF*f*enemy->spawn_controlPoint[Y]+f*f*enemy->spawn_endPosition[Y];
	
	if (enemy->timeCounter > enemy->ttl)
		ENE_Release(enemy);

}


void updateCircle(enemy_t* enemy)
{
	float f;
	float angle;
	//float cosAngle;
	//float sinAngle;
	
	
	f = enemy->timeCounter / enemy->fttl ;
	
	angle = f * FHT_NUM_ROTATION * 2.0f * M_PI;
	
	enemy->entity.yAxisRot = angle;
	
	//cosAngle = cosf(angle);
	//sinAngle = sinf(angle);
	f -= 1;
	enemy->ss_position[X] = f * cosf(enemy->spawn_startPosition[X]+angle) * 1.3f* SS_H / SS_W;
	enemy->ss_position[Y] = f * sinf(enemy->spawn_startPosition[X]+angle) * 1.3f;
	
	//enemy->ss_position[X] = (1-f)*  enemy->spawn_startPosition[X] ;
	//enemy->ss_position[Y] = (1-f)*  enemy->spawn_startPosition[Y] ;
	
	//printf("ene pso = %.2f, %.2f.\n",enemy->ss_position[X],enemy->ss_position[Y]);
	
	
	
	
	if (enemy->timeCounter > enemy->ttl)
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
	}
	
}

void updateFHT(enemy_t* enemy)
{
	switch (enemy->entity.mouvementPatternType) 
	{
		case MVMT_CIRCLE: enemy->updateFunction = updateCircle; break;
		case MVMT_STRAIGHT: enemy->updateFunction = updateStraight; break;
		case MVMT_X_SIN: enemy->updateFunction = updateXSin; break;
		default: break;
	}
	
	enemy->updateFunction(enemy);
}
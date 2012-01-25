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
 *  enemy.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-02-09.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "globals.h"
#include "enemy.h"
#include "camera.h"
#include "timer.h"
#include "event.h"
#include "renderer.h"
#include "sounds.h"
#include "enemy_particules.h"
#include "dEngine.h"
#include "player.h"
#include "lee.h"
#include "lofb.h"
#include "shab.h"
#include "fht.h"
#include "tha.h"

//Warning this matrix is declared as row major: <-- Shit !! This line was actually useful 4 month later !!!! You are good fab !!!
/*
 1	0	0	0
 0	0  -1	0
 0   1	0	0
 0	0	0	1
 */
matrix_t enemyFromAboveRotation = 
{1 , 0  , 0 , 0,
	0 , 0  , 1 , 0,
	0 , -1 , 0 , 0,
	0 , 0  , 0 , 1,} ; 

enemy_t rootEnemy;

enemy_t enemies[MAX_NUM_ENEMIES];

uchar numFreeEnemies;
enemy_t* freeEnemies[MAX_NUM_ENEMIES];




void ENE_Mem_Init(void)
{
	int i;
	
	for (i=0; i < MAX_NUM_ENEMIES; i++) 
		freeEnemies[i] = &enemies[i];

	numFreeEnemies = MAX_NUM_ENEMIES;
	
	
	rootEnemy.next = NULL;
	rootEnemy.prev = NULL;
}


void ENE_Reset(void)
{
	memset(enemies,0,sizeof(enemies));
	memset(&rootEnemy,0,sizeof(rootEnemy));
	ENE_Mem_Init();
}


enemy_t dummyEnemy;
int uniqueIdGenerator=0;
enemy_t* ENE_Get(void)
{
	enemy_t* enemy;
	
	if (numFreeEnemies == 0)
	{
		printf("Enemy pool exhausted (%d). Aborting.\n",MAX_NUM_ENEMIES);
		return &dummyEnemy;
	}
	
	numFreeEnemies--;
	enemy = freeEnemies[numFreeEnemies];
	
	//Insert after rootEnemy
	enemy->next = rootEnemy.next;
	
	if (enemy->next != NULL)
		enemy->next->prev = enemy;
	
	enemy->uniqueId = uniqueIdGenerator++;
	enemy->entity.uid = enemy->uniqueId;
	enemy->state = 0;
	
	enemy->prev = &rootEnemy;
	rootEnemy.next = enemy;
	
	
	
	return enemy;
}

int ENE_GetNumEnemies(void)
{
	return MAX_NUM_ENEMIES - numFreeEnemies ;
}

void ENE_Release(enemy_t* enemy)
{
	//No Need to free enemy/entities ressources 
	// Even enemy->entity.indices is not allocated because enemies are fullDraw and GPU resident
	enemy->uniqueId = 0;
	enemy->entity.uid = 0;
	
	freeEnemies[numFreeEnemies] = enemy;
	numFreeEnemies++;
	
	if (enemy->prev != NULL)
		enemy->prev->next = enemy->next;
	
	if (enemy->next != NULL)
		enemy->next->prev = enemy->prev;
	

	
}


enemy_t* ENE_GetFirstEnemy(void)
{
	return rootEnemy.next;
}


void ENE_ReleaseAll(void)
{
	enemy_t* enemy;
	enemy_t* toRelease;
	
	enemy = ENE_GetFirstEnemy();
	
	while (enemy != NULL) 
	{
		toRelease = enemy;
		enemy = enemy->next;
		ENE_Release(toRelease);
	}
}



entity_t dummy;

void ENE_Precache(void)
{
	event_t* precacheEvent;
	event_spawnEnemy_payload_t* eventEnemyPayload;
	
	engine.playerStats.numEnemies=0;
	
	precacheEvent = &events;
	
	while (precacheEvent != NULL)
	{
		if (precacheEvent->type == EV_SPAWN_ENEMY)
		{
			engine.playerStats.numEnemies++;
			//printf("precache t=%denemy count %f.\n",precacheEvent->time,engine.playerStats.numEnemies);
			eventEnemyPayload = precacheEvent->payload;
			//printf("Precaching entity: %s.\n",enemyTypePath[eventEnemyPayload->type]);
			ENT_LoadEntity(&dummy, enemyTypePath[eventEnemyPayload->type],ENT_FULL_DRAW);
		}
		precacheEvent = precacheEvent->next;
	}
	

}


void ENE_AttachToCamera(matrix_t globalMatrix)
{
	vec4_t ws_enemyPos;
	vec4_t ss_enemyPos;
	enemy_t* enemy;
	
	enemy = ENE_GetFirstEnemy();
	
	while (enemy != NULL)
	{
		//enemy->entity.matrix[14] += -0.24f * timediff ;
		
		ws_enemyPos[X] = enemy->entity.matrix[12];
		ws_enemyPos[Y] = enemy->entity.matrix[13];
		ws_enemyPos[Z] = enemy->entity.matrix[14];
		ws_enemyPos[W] = 1;
		
		
		
		matrix_multiplyVertexByMatrix(ws_enemyPos,globalMatrix,ss_enemyPos);
		
		
		enemy->ss_position[X] = ss_enemyPos[X] / ss_enemyPos[W] ;
		enemy->ss_position[Y] = ss_enemyPos[Y] / ss_enemyPos[W] ;
		
		enemy= enemy->next;
	}
	
}

void ENE_UpdateSSBoundaries(enemy_t* enemy)
{
	enemy->ss_boudaries[UP]   =  (enemy->ss_position[Y] + 0.1)*SS_H ;
	enemy->ss_boudaries[DOWN] =  (enemy->ss_position[Y] - 0.1)*SS_H;
	enemy->ss_boudaries[LEFT] =  (enemy->ss_position[X] - 0.1)*SS_W;
	enemy->ss_boudaries[RIGHT]=  (enemy->ss_position[X] + 0.1)*SS_W;
	
}


void ENE_Update(void)
{
	
	enemy_t* enemy;
	entity_t* entity;
	matrix_t tmp;
	//matrix_t tmp2;
	
	/*
	matrix_t rollMatrix;
	matrix_t yawMatrix;
	matrix_t pitchMatrix;
*/
	matrix_t eulerMatrix;
	
	vec3_t translationForwardTransform;
	vec3_t translationRightTransform;
	vec3_t translationUpTransform;
	vec3_t translationTransform;	
	
	
	eulerMatrix[3] = 0;
	eulerMatrix[7] = 0;
	eulerMatrix[11] = 0;
	eulerMatrix[15] = 1;
	
	enemy = ENE_GetFirstEnemy();
	
	if (!entitiesAttachedToCamera)
	{
		while (enemy != NULL)
		{
			entity = &enemy->entity;
			if (!entitiesAttachedToCamera)
			{
				//memcpy(entity->matrix,enemyFromAboveRotation,16*sizeof(float));
				entity->matrix[14] += -0.24f * timediff ;
				enemy = enemy->next;
			}	
		}
	}
	
	else 
	{
		while (enemy != NULL)
		{
			entity = &enemy->entity;

			enemy->updateFunction(enemy);		
		
			//Update ss_boundaries for collision detection
			ENE_UpdateSSBoundaries(enemy);
		
		
			eulerMatrix[0] = cosf(entity->yAxisRot) * cosf(entity->zAxisRot) - sinf(entity->yAxisRot)*sinf(entity->xAxisRot)*sinf(entity->zAxisRot);
			eulerMatrix[1] = sinf(entity->yAxisRot) * cosf(entity->zAxisRot) + cosf(entity->yAxisRot)*sinf(entity->xAxisRot)*sinf(entity->zAxisRot);
			eulerMatrix[2] = -cosf(entity->xAxisRot) * sinf(entity->zAxisRot) ;
		
			eulerMatrix[4] = -sinf(entity->yAxisRot) * cosf(entity->xAxisRot) ;
			eulerMatrix[5] = cosf(entity->yAxisRot) * cosf(entity->xAxisRot) ;
			eulerMatrix[6] = sinf(entity->xAxisRot);
		
		
			eulerMatrix[8] = cosf(entity->yAxisRot) * sinf(entity->zAxisRot) + sinf(entity->yAxisRot)*sinf(entity->xAxisRot)*cosf(entity->zAxisRot);
			eulerMatrix[9] = sinf(entity->yAxisRot) * sinf(entity->zAxisRot) - cosf(entity->yAxisRot)*sinf(entity->xAxisRot)*cosf(entity->zAxisRot);
			eulerMatrix[10] = cosf(entity->xAxisRot) * cosf(entity->zAxisRot) ;
		
		
			// cameraInvRot * Rz * Rx * Ry * fromAbove
			matrix_multiply(eulerMatrix, enemyFromAboveRotation, tmp);
			matrix_multiply(cameraInvRot,tmp,entity->matrix);
				
		
			//Translation part
			vectorScale(camera.forward,distanceZFromCamera,translationForwardTransform);
			vectorScale(camera.right, enemy->ss_position[X] * widthAtDistance ,translationRightTransform);
			vectorScale(camera.up   , enemy->ss_position[Y] * heightAtDistance,translationUpTransform);
	
			vectorCopy(camera.position,									translationTransform) ;
			vectorAdd(translationTransform,translationForwardTransform,	translationTransform) ;
			vectorAdd(translationTransform,translationRightTransform,	translationTransform) ;
			vectorAdd(translationTransform,translationUpTransform,		translationTransform) ;
	
	
			entity->matrix[12] = translationTransform[X] ;
			entity->matrix[13] = translationTransform[Y] ;
			entity->matrix[14] = translationTransform[Z] ;
		
			enemy->timeCounter += timediff;
		
			enemy = enemy->next;
		}	
		
	}
}


#define DEVIL_TTR  700.0f
#define DEVIL_TIME_ONSCREEN (DEVIL_TTR + 4000)
void updateHAB(enemy_t* enemy)
{
	float f;
	if (enemy->timeCounter < DEVIL_TTR)
	{
		
		f = enemy->timeCounter / DEVIL_TTR ;
	
		f= MIN(f,1);
	
		enemy->ss_position[X] = enemy->spawn_startPosition[X] + f*(enemy->spawn_endPosition[X] - enemy->spawn_startPosition[X]);
		enemy->ss_position[Y] = enemy->spawn_startPosition[Y] + f*(enemy->spawn_endPosition[Y] - enemy->spawn_startPosition[Y]);
	
		//As a devil is seen from above, the roll is actually a rotatiom around Y.
		enemy->entity.zAxisRot = (1-f)*enemy->entity.zAxisRot;
	
		
		return;
	}
	
	
	
	
	//ON screen and battle
	enemy->entity.zAxisRot = 0 ;
	
	
	
	if (enemy->timeCounter > DEVIL_TIME_ONSCREEN)
	{
		f = (enemy->timeCounter - DEVIL_TIME_ONSCREEN) / DEVIL_TTR ;
		
		f= MIN(f,1);
		
		enemy->ss_position[X] = enemy->spawn_endPosition[X] + f*(enemy->spawn_startPosition[X] - enemy->spawn_endPosition[X]);
		enemy->ss_position[Y] = enemy->spawn_endPosition[Y] + f*(enemy->spawn_startPosition[Y] - enemy->spawn_endPosition[Y]);
		
		//As a devil is seen from above, the roll is actually a rotatiom around Y.
		enemy->entity.zAxisRot = f*enemy->entity.zAxisRot ;

	
		
	}
	
	if (enemy->timeCounter >= enemy->ttl)
		ENE_Release(enemy);
	
	
}






char* enemyTypePath[] = 
{
	"data/models/enemies/hab.obj.md5mesh",
	"data/models/enemies/fht.obj.md5mesh",
	"data/models/enemies/lee.obj.md5mesh",
	"data/models/enemies/shab.obj.md5mesh",
	"data/models/enemies/lofb.obj.md5mesh",
	"data/models/enemies/tha.obj.md5mesh",
};


updateFunction_t enemyTypeUpdateFct[] =
{
	updateHAB,
	updateFHT,
	updateLEE,
	updateSHAB,
	updateLOFB,
	updateTHA
};


ushort enemyTypeEnergy[] = 
{
	10,
	1,
	12,
	30,
	200,
	30
};

uint enemyScore[] =
{
	10000,
	2000,
	3000,
	10000,
	200000,
	10000
};

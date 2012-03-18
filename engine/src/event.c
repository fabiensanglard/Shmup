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
 *  event.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-02-11.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include <limits.h>
#include "event.h"
#include "timer.h"
#include "math.h"
#include "lexer.h"
#include "player.h"
#include "enemy.h"
#include "dEngine.h"
#include "titles.h"
#include "text.h"
#include "menu.h"
#include "camera.h"
#include "lee.h"
#include "fht.h"
#include "shab.h"
#include "tha.h"
#include "native_services.h"
#include "enemy_particules.h"

void EV_StopPlayback(event_t* event)
{	
	engine.playback.play = 0;
}


void EV_RootEvent(event_t* event)
{

}
	
void EV_AttachToCamera(event_t* event)
{
	matrix_t viewMatrix;
	matrix_t projectionMatrix;
	matrix_t globalMatrix;
	vec3_t vLookat;
	
	
	gluPerspective(camera.fov, camera.aspect,camera.zNear, camera.zFar, projectionMatrix);
	vectorAdd(camera.position,camera.forward,vLookat);	
	gluLookAt(camera.position, vLookat, camera.up, viewMatrix);
	matrix_multiply(projectionMatrix, viewMatrix, globalMatrix);
	
	Log_Printf("[EV_AttachToCamera]\n");
	P_AttachToCamera(globalMatrix);
	ENE_AttachToCamera(globalMatrix);
	
	engine.controlVisible = 1;
}

void EV_DetachCamera(event_t* event)
{
	P_DetachToCamera();
	
	engine.controlVisible = 0;
}



void EV_SpawnEnemy(event_t* event)
{
	// enemyPayload;
	event_spawnEnemy_payload_t* eventPayload;
	enemy_t* enemy;
	
	
	
	eventPayload = (event_spawnEnemy_payload_t*)event->payload;

	//spawn a devil
	enemy = ENE_Get();
	
	
	
	ENT_LoadEntity(&enemy->entity, enemyTypePath[eventPayload->type],ENT_FULL_DRAW);

	enemy->type = eventPayload->type  ;
	enemy->timeCounter = 0;

	
	vector2Copy(eventPayload->startPosition,enemy->ss_position);
	ENE_UpdateSSBoundaries(enemy);
	
	vector2Copy(eventPayload->startPosition,enemy->spawn_startPosition);
	vector2Copy(eventPayload->endPosition,enemy->spawn_endPosition);
	vector2Copy(eventPayload->controlPoint,enemy->spawn_controlPoint);
	enemy->spawn_Z_AxisRot = eventPayload->zAxisRot;
	
	
	//Log_Printf("Angle = %.2f\n",enemy->spawn_angle);
	

	enemy->entity.xAxisRot = eventPayload->xAxisRot;
	enemy->entity.yAxisRot = eventPayload->yAxisRot;
	enemy->entity.zAxisRot = eventPayload->zAxisRot;

	
	enemy->updateFunction = enemyTypeUpdateFct[enemy->type];
	enemy->entity.mouvementPatternType = eventPayload->mouvementPatternType;
	
	enemy->energy = enemyTypeEnergy[enemy->type] ;
	enemy->score = enemyScore[enemy->type] << engine.difficultyLevel;
	enemy->shouldFlicker = 0;
	
	//Enemy are spawned outside the screen always !!
	enemy->ss_position[X] = 2;
	enemy->ss_position[Y] = 2;
	
	enemy->ss_boudaries[UP] = SS_H;
	enemy->ss_boudaries[DOWN] = SS_H;
	enemy->ss_boudaries[RIGHT] = SS_W;
	enemy->ss_boudaries[LEFT] = SS_W;
	
	matrixLoadIdentity(enemy->entity.matrix);
	enemy->entity.matrix[12]=0;
	enemy->entity.matrix[13]=110;
	enemy->entity.matrix[14]=-412;
	
	enemy->ttl = eventPayload->ttl;
	enemy->fttl = eventPayload->ttl; 
	
	memcpy(enemy->parameters,eventPayload->parameters,sizeof(enemy->parameters));
	
	switch (eventPayload->subType) {
		case ENEMY_SUBTYPE_NORMAL:
			enemy->entity.color[R] = 1;
			enemy->entity.color[G] = 1;
			enemy->entity.color[B] = 1;
			enemy->entity.color[A] = 1;	
			break;

		case ENEMY_SUBTYPE_HAAARD:
			enemy->entity.color[R] = 0.85f;
			enemy->entity.color[G] = 0.85f;
			enemy->entity.color[B] = 1.0f;
			enemy->entity.color[A] = 1;	
			enemy->energy *= 5;
			break;

		case ENEMY_SUBTYPE_IMPOSSIBLE:
			enemy->entity.color[R] = 0.2f;
			enemy->entity.color[G] = 0.2f;
			enemy->entity.color[B] = 0.2f;
			enemy->entity.color[A] = 1;	
			enemy->energy *= 40;
			break;
		case ENEMY_SUBTYPE_WEAK:	
			enemy->entity.color[R] = 1;
			enemy->entity.color[G] = 1;
			enemy->entity.color[B] = 1;
			enemy->entity.color[A] = 1;	
			enemy->energy = 1;

			break;
		default:
			break;
	}
	

	
}

void EV_SpawnText(event_t* event)
{
	event_text_payload_t* payload;
	
	
	payload = (event_text_payload_t*)event->payload;
	
	DYN_TEXT_AddText(payload->ss_start_pos, payload->ss_end_pos, payload->duration,payload->size, payload->text);
	
}

void EV_DisplayStats(event_t* event)
{
	
}

void EV_ShowProlog(event_t* event)
{
	//Log_Printf("EV_ShowProlog()\n");
	event_title_payload_t* pl;
	pl = event->payload;
	TITLE_Show_prolog(pl->duration);
}

void EV_ShowEpilog(event_t* event)
{
	//Log_Printf("EV_ShowEpilog()\n");
	event_title_payload_t* pl;
	pl = event->payload;
	TITLE_Show_epilog(pl->duration);
}

void EV_MaskStats(event_t* event)
{
	
}

void EV_RequestScene(event_t* event)
{
	
	event_req_scene_t* payload;
	
	payload = event->payload;
	
	
	
	dEngine_RequireSceneId(payload->sceneId);
	
	Log_Printf("[EV_RequestScene] engine.requiredSceneId =%d.\n",engine.requiredSceneId );
}

void EV_RequestMenu(event_t* event)
{
	
	event_req_menu_t* payload;
	
	payload = event->payload;
	
	MENU_Set(payload->menuId);
    
   
}

void EV_AutoPilotPls(event_t* event)
{
	int i;
	
	
	for (i=0; i < numPlayers; i++) 
	{
		players[i].autopilot.enabled = 1;
		players[i].autopilot.timeCounter  = PLAYER_ENDLEVEL_REPLACMENT;
		players[i].autopilot.originalTime = PLAYER_ENDLEVEL_REPLACMENT;
		players[i].autopilot.end_ss_position[X] = (i-0.5)*2*0.3;
		
		players[i].autopilot.end_ss_position[Y] = -0.3;
		//Log_Printf("player %d end_ss_position[%.2f,%.2f]\n",players[i].autopilot.end_ss_position[X],players[i].autopilot.end_ss_position[Y]);
		players[i].autopilot.diff_ss_position[X] = players[i].ss_position[X] - players[i].autopilot.end_ss_position[X];
		players[i].autopilot.diff_ss_position[Y] = players[i].ss_position[Y] - players[i].autopilot.end_ss_position[Y];
		
	}
	
}

void EV_SaveScore(event_t* event)
{
	Native_UploadScore(players[controlledPlayer].score);
}


void EV_LimitedEdition_Action(event_t* event)
{
	enemy_t* enemy;
	event_t* toDelete;
	vec2short_t ss_start_pos;
	vec2short_t ss_end_pos;
	event_req_scene_t* payloadScene;
	event_req_menu_t* payloadMenu;
	
	
	
	
	// Destroy all enemies
	enemy = ENE_GetFirstEnemy();
	while (enemy) 
	{
		// spawn an explosion
		FX_GetExplosion(enemy->ss_position,IMPACT_TYPE_YELLOW,1,0);
		//enemy->ss_position[X] += (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1) * 0.1 ;
		//enemy->ss_position[Y] += (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1) * 0.1 ;
		//FX_GetExplosion(enemy);
		
		
		
		Spawn_EntityParticules(enemy->ss_position);
		// spwan smoke
		
		FX_GetSmoke(enemy->ss_position, 0.3, 0.3);
		enemy->ss_position[X] += (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1) * 0.1 ;
		enemy->ss_position[Y] += (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1) * 0.1 ;
		FX_GetSmoke(enemy->ss_position, 0.2, 0.2);
		
		ENE_Release(enemy);
		
		SND_PlaySound(SND_EXPLOSION);
		
		enemy= enemy->next;
	}
	
	
	// Remove all bullets
	partLib.numParticules = 0;
	
	// Autopilot & Move to position
	EV_AutoPilotPls(event);
	
	 
	 
	/// TEXT VERIFIED GOOD
	// Display text
	ss_start_pos[X] = ss_end_pos[X] = 0 ;
	ss_end_pos[Y] = ss_start_pos[Y] = 150;
	DYN_TEXT_AddText(ss_start_pos, ss_end_pos, 10000,2.5f,"Thanks for trying:");

	ss_start_pos[X] = ss_end_pos[X] = 0 ;
	ss_end_pos[Y] = ss_start_pos[Y] = 80;
	DYN_TEXT_AddText(ss_start_pos, ss_end_pos, 10000,2.5f,"\"Shmup Lite\"");
	
	
	ss_start_pos[X] = ss_end_pos[X] = 0 ;
	ss_end_pos[Y] = ss_start_pos[Y] = -10;
	DYN_TEXT_AddText(ss_start_pos, ss_end_pos, 10000,2.5f,"Check out the full version.");
	
	
	
	
	//Add a return to main menu even set at simulationTime+10000
	payloadScene = calloc(1, sizeof(event_req_scene_t));
	payloadScene->sceneId = 0;
	event = calloc(1, sizeof(event_t));
	event->time = simulationTime+10000;
	event->type = EV_REQUEST_SCENE;
	event->payload = payloadScene;
	EV_AddEvent(event);
	
	payloadMenu = calloc(1, sizeof(event_req_menu_t));
	payloadMenu->menuId = MENU_HOME;
	event = calloc(1, sizeof(event_t));
	event->time = simulationTime+10000;
	event->type = EV_REQUEST_MENU;
	event->payload = payloadMenu;
	EV_AddEvent(event);	
	
	
	
	//Remove all futur enemy spawning events.
	event = EV_GetNextEvent();
	
	while (event != NULL) 
	{			
		while (event->next != NULL && event->next->type == EV_SPAWN_ENEMY)
		{
			//Log_Printf("[dEngine_JumpInTime] Cleaning EV_SPAWN_ENEMY events t=%d.\n",event->next->time);
			
			toDelete = event->next;
			event->next = event->next->next;
			
			free(toDelete->payload);
			free(toDelete);
		}
		
		
		event = event->next;
	}
	 
}

void EV_ClearTitle(event_t* event){
    TITLE_Clear();
}

typedef void (*eventProcessor_ft)(event_t*) ;


event_t events;
event_t* nextEvent = NULL;

eventProcessor_ft eventToFunction[32] = 
{
	EV_RootEvent,
	EV_AttachToCamera,
	EV_DetachCamera,
	EV_SpawnEnemy,
	EV_DisplayStats,
	EV_MaskStats,
	EV_ShowProlog,
	EV_ShowEpilog,
	EV_RequestScene,
	EV_SpawnText,
	EV_StopPlayback,
	EV_RequestMenu,
	EV_AutoPilotPls,
	EV_SaveScore,
	EV_LimitedEdition_Action,
    EV_ClearTitle
};



void EV_InitForScene(void)
{
	events.time = 0;
	events.next = NULL;
	events.type = EV_ROOT;
	events.payload = NULL;
	
	nextEvent = &events;
	
	Log_Printf("EV_InitForScene\n");
}

event_t*  EV_GetNextEvent(void)
{
	return nextEvent;
}
void EV_AddEvent(event_t* event)
{
	event_t* cEvent;
	event_t* tmp;
	
	cEvent = nextEvent;
	
	//Search
	while (cEvent->next != NULL && cEvent->next->time <= event->time) {
		cEvent = cEvent->next;
	}
	
	
	// Insert
	if (cEvent->next == NULL)
		cEvent->next = event;
	else 
	{
		tmp = cEvent->next;
		cEvent->next = event;
		event->next = tmp;
	}

}

void EV_Update(void)
{
	event_t* toDelete;
	
	//if (nextEvent != NULL)
	//	Log_Printf("next event t=%d.\n",nextEvent->time);
	
	while (nextEvent != NULL && nextEvent->time < simulationTime) 
	{
		//Log_Printf("Triggering event t=%d type: %d.\n",nextEvent->time,nextEvent->type);
		eventToFunction[nextEvent->type](nextEvent);
		
		toDelete = nextEvent;
		
		nextEvent = nextEvent->next;
		
		
		
		if (toDelete != &events)
		{
		   free(toDelete->payload);
		   free(toDelete);
		}
		 
		//? Freeing nextevent ?
	}
}

void EV_CleanAllRemainingEvents(void)
{
	event_t* toDelete;
	
	while (nextEvent != NULL) 
	{
		toDelete = nextEvent;
		nextEvent = nextEvent->next;
		
		if (toDelete != &events)
		{
			free(toDelete->payload);
			free(toDelete);
		}
	}
	
	
}

void EV_ReadEnemiesEvents(void)
{
	event_t*                     event;
	event_spawnEnemy_payload_t*  eventPayload;
	int                          at;
	int                          numEnemies;
	int                          enemyType;
	int                          i;
	int                          time=0;
	float                        ttl;
	float                        percentageInvulnerable;
	float                        angleoffset;
	uchar                        defaultSubType;
	
	nextEvent = &events;

	LE_readToken() ; //{

	LE_readToken(); 	//at or }
	while (LE_hasMoreData() && strcmp(LE_getCurrentToken(), "}")) 
	{
		if (!strcmp("settime", LE_getCurrentToken()))
		{
			time = LE_readReal();
			//Log_Printf("settime=%d.\n",time);
		}
		else
		if (!strcmp("addtime", LE_getCurrentToken())) 
		{
			time += LE_readReal();
		}
		if (!strcmp("setttl", LE_getCurrentToken()))
		{
			ttl = LE_readReal();
			//Log_Printf("ttl=%.2f.\n",ttl);
		}		
		else if (!strcmp("at", LE_getCurrentToken()))
		{
			at = time + LE_readReal();
		
			//Log_Printf("Fount enemy at %d.\n",at);
		
			LE_readToken();
		

			//at  50000 spawnEnemyWave circle enemyNum 16 enemyType 1
			if (!strcmp("spawnEnemyWave", LE_getCurrentToken()))
			{
				LE_readToken();
			
				if (!strcmp("circle", LE_getCurrentToken()))
				{
					//enemyNum
					LE_readToken();
					numEnemies= LE_readReal();
					//Log_Printf("Fount %d enemies.\n",numEnemies);
				
					//enemyType
					LE_readToken();
					enemyType = LE_readReal();
					//Log_Printf("Fount enemyType %d.\n",enemyType);
				
					//percentageInvulnerable
					LE_readToken();
					percentageInvulnerable = LE_readReal()/100.0f;
					
					LE_readToken();
					angleoffset  = LE_readReal() * 2*M_PI/360 ;
					
					LE_readToken();
					defaultSubType = LE_readReal();
					
					for(i=0 ; i < numEnemies ; i++)
					{
						event = calloc(1, sizeof(event_t));
						event->time = at;
						event->type = EV_SPAWN_ENEMY;
						eventPayload = calloc(1, sizeof(event_spawnEnemy_payload_t));
						event->payload = eventPayload;
						eventPayload->type = enemyType;
						//eventPayload->zAxisRot = 2*3.1415/numEnemies * i;
						
						eventPayload->xAxisRot=0;
						eventPayload->yAxisRot=0;
						eventPayload->zAxisRot=0;
						
						//This is ugly
						eventPayload->startPosition[X] = (angleoffset+2*M_PI)/numEnemies*i;
						//eventPayload->startPosition[X] = 1.3 * cosf(2*M_PI/numEnemies*i);//*SS_H/(float)SS_W;
						//eventPayload->startPosition[Y] = 1.3 * sinf(2*M_PI/numEnemies*i); 
						
						eventPayload->mouvementPatternType = MVMT_CIRCLE;
						
						eventPayload->ttl = ttl;
						
						
						eventPayload->subType = (i/(float)numEnemies < percentageInvulnerable)? ENEMY_SUBTYPE_IMPOSSIBLE : defaultSubType ;
						
						EV_AddEvent(event);
					}
				}
			}	
			else
			//at 0 spawnEnemy enemyType 3 startPos -1 -1 endPos -0.5 0.5 controlPoint -1 1 initialRoll 90
			if (!strcmp("spawnEnemy", LE_getCurrentToken()))
			{
				event = calloc(1, sizeof(event_t));
				event->time = at;
				event->type = EV_SPAWN_ENEMY;
				eventPayload = calloc(1, sizeof(event_spawnEnemy_payload_t));
				event->payload = eventPayload;
				
				eventPayload->ttl =  ttl;
				
				//mouvement
				LE_readToken();
				eventPayload->mouvementPatternType = LE_readReal();
				
				
				 
				 
				 
				switch (eventPayload->mouvementPatternType) {
					case MVMT_X_SIN:
						LE_readToken();
						eventPayload->parameters[PARAMETER_FHT_X_POS] = LE_readReal();
						
						LE_readToken();
						eventPayload->parameters[PARAMETER_FHT_X_WIDTH] = LE_readReal();
						break;
						
					case MVMT_CIRCLE:
						//startAngle 0    fireFrequency 100
						LE_readToken();
						eventPayload->parameters[PARAMETER_LEE_START_ANGLE] = 2*M_PI/360 * LE_readReal();
					//	Log_Printf("eventPayload->parameters[PARAMETER_LEE_START_ANGLE]=%.2f\n",eventPayload->parameters[PARAMETER_LEE_START_ANGLE]);
						LE_readToken();
						eventPayload->parameters[PARAMETER_LEE_FIRE_FREQUENCY] = LE_readReal();
						
						
						
					default:
						break;
				}
				
				
				//enemyType
				LE_readToken();
				eventPayload->type = LE_readReal();
				
				
				//startPos
				LE_readToken(); 
				eventPayload->startPosition[X] = LE_readReal();
				eventPayload->startPosition[Y] = LE_readReal();

				//endPos
				LE_readToken();
				eventPayload->endPosition[X] = LE_readReal();
				eventPayload->endPosition[Y] = LE_readReal();

				//controlPoint
				LE_readToken();
				eventPayload->controlPoint[X] = LE_readReal();
				eventPayload->controlPoint[Y] = LE_readReal();
				
				//Initial roll
				LE_readToken();
				eventPayload->zAxisRot = 2*M_PI/360 *  LE_readReal();

				//Initial pitch
				LE_readToken();
				eventPayload->xAxisRot = 2*M_PI/360 *  LE_readReal();

				
				//Initial yaw
				LE_readToken();
				eventPayload->yAxisRot = 2*M_PI/360 *  LE_readReal();
				
				//subType
				LE_readToken();
				eventPayload->subType = LE_readReal();
				
				
				
				switch (eventPayload->type) {
					case ENEMY_LEE:
						eventPayload->parameters[PARAMETER_LEE_FIRING_TYPE] = LE_readReal();
						
						LE_readToken();						
						eventPayload->parameters[PARAMETER_LEE_BULLET_SPEED_FACTOR]= LE_readReal();												
						break;
						
					case ENEMY_SHAB:
						LE_readToken();
						eventPayload->parameters[PARAMETER_SHAB_FIRING_ANGLE1]    = 2*M_PI/360 *LE_readReal();
						LE_readToken();
						eventPayload->parameters[PARAMETER_SHAB_FIRING_ANGLE2]    = 2*M_PI/360 *LE_readReal();
						LE_readToken();
						eventPayload->parameters[PARAMETER_SHAB_FIRING_NUM_THREAD]= LE_readReal();						
						LE_readToken();
						eventPayload->parameters[PARAMETER_SHAB_FIRING_ROT_ANGLE] = 2*M_PI/360 *LE_readReal();												
						break;
						
					case ENEMY_THA :
						//fireDirection
						LE_readToken();
						eventPayload->parameters[PARAMETER_THA_FIRING_DIRECTION] = LE_readReal();
						
						//firingTime
						LE_readToken();
						eventPayload->parameters[PARAMETER_THA_FIRING_TIME] = LE_readReal();
						
					default:
						break;
				}
				
				
				
				
				
				EV_AddEvent(event);
			}
			//Log_Printf("t=%d enemyType=%d\n",event->time,eventPayload->type);
		}
		LE_readToken(); 
	}
	

}


	
void EV_ReadTextsEvents(void)
{
	event_t* event;
	event_text_payload_t* payload;
	int currentTime=0;
	
	LE_readToken() ; //{
	
	LE_readToken(); 	//at or }
	while (LE_hasMoreData() && strcmp(LE_getCurrentToken(), "}")) 
	{
		if (!strcmp("at", LE_getCurrentToken()))
		{
			event = (event_t*)calloc(1, sizeof(event_t));
			event->time = currentTime + LE_readReal();
			event->type = EV_SPAWN_TEXT;
			payload = (event_text_payload_t*)calloc(1, sizeof(event_text_payload_t));
			event->payload = payload;
			
			//at 0000 display -Welcome_To_"Shump"_tutorial-	 size 2	for 2000 starting 0   0 ending  0 0
			LE_readToken(); //display
			LE_readToken();
			strReplace(LE_getCurrentToken(),'_',' ');
			strcpy(payload->text,LE_getCurrentToken());
			
			
			LE_readToken();	// size
			payload->size = LE_readReal();
			
			LE_readToken();	// for
			payload->duration = LE_readReal();
			
			LE_readToken(); // starting
			payload->ss_start_pos[X] = LE_readReal() ;
			payload->ss_start_pos[Y] = LE_readReal() ;
			
			LE_readToken(); // direction
			payload->ss_end_pos[X] = LE_readReal() ;
			payload->ss_end_pos[Y] = LE_readReal() ;
			
			
			
			
			//Acquired event
		//	Log_Printf("[EV_ReadTextsEvents] at %d: %s\n",event->time,payload->text);
			EV_AddEvent(event);
		}
		else
		if (!strcmp("settime",LE_getCurrentToken())){
			currentTime = LE_readReal();
		}

		LE_readToken(); 
	}
}
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
 *  world.c
 *  dEngine
 *
 *  Created by fabien sanglard on 24/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "world.h"
#include "renderer.h"
#include "timer.h"
#include "dEngine.h"
#include "camera.h"
#include "timer.h"
#include "filesystem.h"
#include "lexer.h"
#include "player.h"
#include "event.h"
#include "titles.h"

light_t light;



entity_t map[MAX_NUM_ENTITIES];
uchar num_map_entities;
int numBackgroundEntities=0;

void World_ReadMatrix(matrix_t target)
{
	int i,j ;
	
	LE_readToken(); //{
	for(i=0 ; i < 4 ; i++)
		for(j=0 ; j < 4 ; j++)
			target[j*4+i] = LE_readReal();
	LE_readToken(); //}
	
	
}

void World_ClearWorldMap(void)
{
	int i;
	
	for (i=0; i < MAX_NUM_ENTITIES;  i++) 
	{
		// map entities are marked as PARTIAL_DRAW and hence have indices
		free(map[i].indices);
		map[i].indices = 0;
	}
	
	num_map_entities=0;
}

void World_ReadMD5s(matrix_t currentMatrix)
{
	entity_t* currentEntity;
	
	
	LE_readToken(); // {
	LE_readToken();
	while (strcmp(LE_getCurrentToken(), "}"))
	{
		if (!strcmp("model",LE_getCurrentToken()))
		{
			LE_readToken();
			
			if (num_map_entities+1 == MAX_NUM_ENTITIES)
			{
				printf("Too many entities in the map.\n");
				exit(0);
			}
			
			currentEntity = &map[num_map_entities];
			
			if (!ENT_LoadEntity(currentEntity,LE_getCurrentToken(),ENT_PARTIAL_DRAW) )
			{
				printf("[World_ReadMD5s] Could not load entity: %s.\n",LE_getCurrentToken());
			}
			else
			{				
				matrixCopy(currentMatrix, currentEntity->matrix);
	
				currentEntity->uid = num_map_entities;
				
				
				ENT_GenerateWorldSpaceBBox(currentEntity);
				
				num_map_entities++ ;
			}
		}
		
		LE_readToken();	
	}	
}

void World_Loadmap(char* mapFileName)
{
	filehandle_t* mapFile ;
	matrix_t currentMatrix;
	
	mapFile = FS_OpenFile(mapFileName, "rt");
	
	if (!mapFile)
	{
		printf("Map file '%s' cannot be opened.\n",mapFileName);
		return;
	}
	
	printf("[World_Loadmap] Found map: '%s'.\n",mapFileName);
	
	LE_pushLexer();
	LE_init(mapFile);
	
	while(LE_hasMoreData())
	{
		LE_readToken();
		
		if (!strcmp(LE_getCurrentToken(), "matrix"))
		{
			World_ReadMatrix(currentMatrix);
			//printf("Read matrix:\n");
			//matrix_print(currentMatrix);
		}
		else if (!strcmp(LE_getCurrentToken(), "numBackgroundEntities"))
		{
			numBackgroundEntities = LE_readReal();
		}
		else if (!strcmp(LE_getCurrentToken(), "entities"))
		{
			LE_readToken(); // OBJ or MD5 ?
			
			if (!strcmp("MD5",LE_getCurrentToken()))
			{
				World_ReadMD5s(currentMatrix);
				//ENT_DumpEntityCache();
				//printf("Loaded MD5:%s.\n",LE_getCurrentToken());
			}
		}
	}
	
	LE_popLexer();
	
	printf("[World_Loadmap] Loaded map with: %d entities.\n",num_map_entities);
}



void World_OpenScene(char* filename)
{
	filehandle_t*	sceneFile;
	uchar			currentPlayerId;
	int				i,j;
	event_t* event;
	
	
	event_title_payload_t* titleEventPayload;
	event_req_scene_t* ev_requestAct_payload;
	event_req_menu_t* ev_requestMenu_payload;
	camera.pathFilename[0] = 0;
	
	sceneFile = FS_OpenFile(filename, "rt");
	
	if (!sceneFile)
	{
		printf("Could not find scene file: %s.\n",filename);
		return;
	}
	
	printf("[World_OpenScene] Found scene: '%s'.\n",filename);
	
	LE_pushLexer();
	LE_init(sceneFile);
	
	//Init events
	
	
	while (LE_hasMoreData()) 
	{
		LE_readToken();
		
		if (!strcmp("map", LE_getCurrentToken()))
		{
			LE_readToken();	//{
			LE_readToken();
			while (strcmp("}", LE_getCurrentToken()))
			{
				if (!strcmp("filename", LE_getCurrentToken()))
				{
					LE_readToken();
					num_map_entities = 0;
					World_Loadmap(LE_getCurrentToken());
				}
				
				LE_readToken();
			}
		}
		/*
		 uchar props;
		 //FOG enabled/diabled via props
		 vec3_t fogColor;
		 float fogDensity; //Only used for non linear fogs
		 uint fogStartAt;
		 uint fogStopAt;
		 
		 
		 enabled 1
		 density 0.2
		 startAt 100.0
		 endAt   350.0
		 color: { 0.7 , 0.7 , 0.9, 1.0 };  
		 
		 */
		else
		if (!strcmp("fog", LE_getCurrentToken()))
		{
			LE_readToken();	//{
			LE_readToken();
			while (strcmp("}", LE_getCurrentToken()))
			{
				if (!strcmp("enabled", LE_getCurrentToken()))
				{
					/*
					if (0 == LE_readReal())
					{
						renderer.props &= ~PROP_FOG ;
					}
					else {
						renderer.props |= PROP_FOG;
					}
					 */
					engine.fogEnabled = LE_readReal() ;

				}
				else 
				if (!strcmp("density", LE_getCurrentToken()))
				{
					renderer.fogDensity = LE_readReal();
				}
				else 
				if (!strcmp("startAt", LE_getCurrentToken()))
				{
					renderer.fogStartAt = LE_readReal();
				}
				else 
				if (!strcmp("endAt", LE_getCurrentToken()))
				{
					renderer.fogStopAt = LE_readReal();
				}
				else 
				if (!strcmp("color", LE_getCurrentToken()))
				{
					renderer.fogColor[0] = LE_readReal();
					renderer.fogColor[1] = LE_readReal();
					renderer.fogColor[2] = LE_readReal();
					renderer.fogColor[3] = 1.0f;
					
				}
				
				
				
					LE_readToken();
			}
		}
		else
		if (!strcmp("player", LE_getCurrentToken()))
		{
			
			LE_readToken();	//{
			LE_readToken();
			while (strcmp("}", LE_getCurrentToken()))
			{
				if (!strcmp("id", LE_getCurrentToken()))
				{
					currentPlayerId = LE_readReal();
				}
				else 
				if (!strcmp("matrix", LE_getCurrentToken()))
				{
					for(i=0 ; i < 4 ; i++)
					{
						for(j=0 ; j < 4 ; j++)
						{
							players[currentPlayerId].entity.matrix[j*4+i] = LE_readReal();
							
						}
					}
									}
				
				LE_readToken();
			}
		}
		else 
		if (!strcmp("light", LE_getCurrentToken()))
		{
			LE_readToken(); // {
			LE_readToken();
			while (strcmp("}", LE_getCurrentToken()))
			{
				if (!strcmp("position", LE_getCurrentToken()))
				{
					light.position[0] = LE_readReal();
					light.position[1] = LE_readReal();
					light.position[2] = LE_readReal();
					light.position[3] = LE_readReal();
				}else 
				if (!strcmp("lookAt", LE_getCurrentToken()))
				{
					light.lookAt[0] = LE_readReal();
					light.lookAt[1] = LE_readReal();
					light.lookAt[2] = LE_readReal();
				}else 
				if (!strcmp("upVector", LE_getCurrentToken()))
				{
					light.upVector[0] = LE_readReal();
					light.upVector[1] = LE_readReal();
					light.upVector[2] = LE_readReal();
				}else 
				if (!strcmp("fov", LE_getCurrentToken()))
				{
					light.fov = LE_readReal();
				}else 
				if (!strcmp("ambientColor", LE_getCurrentToken()))
				{
					light.ambient[0] = LE_readReal();
					light.ambient[1] = LE_readReal();
					light.ambient[2] = LE_readReal();
				}else 
				if (!strcmp("diffuseColor", LE_getCurrentToken()))
				{
					light.diffuse[0] = LE_readReal();
					light.diffuse[1] = LE_readReal();
					light.diffuse[2] = LE_readReal();
				}else 
				if (!strcmp("speculaColor", LE_getCurrentToken()))
				{
					light.specula[0] = LE_readReal();
					light.specula[1] = LE_readReal();
					light.specula[2] = LE_readReal();
				}else 
				if (!strcmp("constantAttenuation", LE_getCurrentToken()))
				{
					light.constantAttenuation = LE_readReal();
				}else 
				if (!strcmp("linearAttenuation", LE_getCurrentToken()))
				{
					light.linearAttenuation = LE_readReal();
				}
				else 
					if (!strcmp("attachedToCamera", LE_getCurrentToken()))
					{
						LE_readToken();
						light.attachedToCamera = !strcmp("1", LE_getCurrentToken());
					}
				else 
				if (!strcmp("enabled", LE_getCurrentToken()))
				{
					light.enabled = LE_readReal();
				}
					
				

				LE_readToken();
				//printf("lgiht token=%s\n",LE_getCurrentToken());
			}
		}
		else 
		if (!strcmp("camera", LE_getCurrentToken()))
		{
			LE_readToken(); // {
			LE_readToken();
			while (strcmp("}", LE_getCurrentToken()))
			{
				if (!strcmp("path", LE_getCurrentToken()))
				{
					LE_readToken();
					camera.pathFilename[0] = 0;
					strcpy(camera.pathFilename, LE_getCurrentToken());

				}
				else 
				if (!strcmp("fov", LE_getCurrentToken()))
				{
					camera.fov = LE_readReal();	
					//printf("[Camera] fov=%.2f\n",camera.fov);
				}
				else 
				if (!strcmp("zNear", LE_getCurrentToken()))
				{
					camera.zNear = LE_readReal();	
					//printf("[Camera] zNear=%.2f\n",camera.zNear);
				}
				else 
				if (!strcmp("zFar", LE_getCurrentToken()))
				{
					camera.zFar = LE_readReal();
					//printf("[Camera] zFar=%.2f\n",camera.zFar);
				}
				else 
				if (!strcmp("attachAt", LE_getCurrentToken()))
				{
					event = calloc(1, sizeof(event_t));
					event->time = LE_readReal();
					event->type = EV_ATTACH_PLAYER;
					
					EV_AddEvent(event);
				}
				else 
				if (!strcmp("detachAt", LE_getCurrentToken()))
				{
					event = calloc(1, sizeof(event_t));
					event->type = EV_DETACH_PLAYER;
					event->time = LE_readReal();
					
					EV_AddEvent(event);
				}
				
				LE_readToken();
			}
			camera.aspect = 320/(float)480;
		}
		else 
		if (!strcmp("enemies", LE_getCurrentToken()))
		{
			EV_ReadEnemiesEvents();
		}
		else 
		if (!strcmp("events", LE_getCurrentToken()))
		{
			LE_readToken(); // {
			LE_readToken();
			while (strcmp("}", LE_getCurrentToken()))
			{
				if (!strcmp("at", LE_getCurrentToken()))
				{
					event = calloc(1, sizeof(event_t));
					event->time = LE_readReal();
					
					LE_readToken();
					
					//at 45000 finishAct
					//at 45000 setPlayback play 0
					if (!strcmp("finishAct", LE_getCurrentToken()))
					{
						event->type = EV_REQUEST_SCENE;
						ev_requestAct_payload = (event_req_scene_t*)calloc(1, sizeof(event_req_scene_t));
						LE_readToken(); // nextAct
						ev_requestAct_payload->sceneId = LE_readReal() ;
						event->payload = ev_requestAct_payload;
					}
					else
					if (!strcmp("stopPlayback", LE_getCurrentToken()))
					{
						event->type = EV_STOP_PLAYBACK;					
					}
					else
					if (!strcmp("setMenu", LE_getCurrentToken()))
					{
						event->type = EV_REQUEST_MENU;	
						ev_requestMenu_payload = (event_req_menu_t* )calloc(1, sizeof(event_req_menu_t));
						ev_requestMenu_payload->menuId = LE_readReal();
						event->payload = ev_requestMenu_payload;
					}
					else
					if (!strcmp("uploadScore", LE_getCurrentToken()))
					{
							event->type = EV_SAVE_SCORE;	
						event->payload = 0;
					}
					
					
					
					EV_AddEvent(event);
				}
				LE_readToken();
			}
		}
		else
		if (!strcmp("texts_events", LE_getCurrentToken()))
		{
			EV_ReadTextsEvents();
		}
		else
		if (!strcmp("music", LE_getCurrentToken()))
		{
			LE_readToken(); // {
			LE_readToken();
			while (strcmp("}", LE_getCurrentToken()))
			{
				if (!strcmp("trackname", LE_getCurrentToken()))
				{
					LE_readToken();
					
					
					engine.musicFilename[0] = '\0';
					strcat(engine.musicFilename, FS_Gamedir());
					strcat(engine.musicFilename,"/");
					strcat(engine.musicFilename, LE_getCurrentToken());
				}
				else
				if (!strcmp("startMusicAt", LE_getCurrentToken()))
				{
					engine.musicStartAt = LE_readReal();
				}	
				LE_readToken();
			}
			
		}
		else
		if (!strcmp("playback", LE_getCurrentToken()))
		{
			LE_readToken(); // {
			LE_readToken();
			while (strcmp("}", LE_getCurrentToken()))
			{
				
				if (!strcmp("filename", LE_getCurrentToken()))
				{
					
					LE_readToken();
					engine.playback.filename[0] = '\0';
					strcpy(engine.playback.filename, LE_getCurrentToken());
				
				} 
				else 
				if (!strcmp("record", LE_getCurrentToken()))
				{
					
					engine.playback.record = LE_readReal();
				}
				if (!strcmp("play", LE_getCurrentToken()))
				{
					
					engine.playback.play = LE_readReal();
				}
				
				


				LE_readToken();
			}
				
			
		}
		else
		if (!strcmp("defaultMenu", LE_getCurrentToken()))
		{
			engine.scenes[engine.sceneId].defaultMenuId = LE_readReal();
		}
		else 
		if (!strcmp("showFingers", LE_getCurrentToken()))
		{
			engine.showFingers = LE_readReal();
		}
		else
		if (!strcmp("title", LE_getCurrentToken()))
		{
			LE_readToken(); // {
			LE_readToken();
			while (strcmp("}", LE_getCurrentToken()))
			{
				if (!strcmp("titleName", LE_getCurrentToken()))
				{
					LE_readToken();
					titleTexture.path = calloc(strlen(LE_getCurrentToken())+1, sizeof(char));
					strcpy(titleTexture.path,LE_getCurrentToken());
					TEX_MakeStaticAvailable(&titleTexture);
				//	printf("titleTexture.textureId=%d\n",titleTexture.textureId);
				}
				else					
				if (!strcmp("prolog", LE_getCurrentToken()))
				{
					LE_readToken();	//start 
					event = calloc(1, sizeof(event_t));
					event->time = LE_readReal();
					LE_readToken();	//duration();  
					titleEventPayload = calloc(1, sizeof(event_title_payload_t));
							  
					event->payload = titleEventPayload;			   
					titleEventPayload->duration = LE_readReal();
					
					event->type = EV_SHOW_PROLOG;
								   
					EV_AddEvent(event);
					
					
				}
				else if (!strcmp("epilog", LE_getCurrentToken()))
				{
					LE_readToken();	//start 
					event = calloc(1, sizeof(event_t));
					event->time = LE_readReal();
					LE_readToken();	//duration();  
					titleEventPayload = calloc(1, sizeof(event_title_payload_t));
					
					event->payload = titleEventPayload;			   
					titleEventPayload->duration = LE_readReal();
					
					event->type = EV_SHOW_EPILOG;
					
					EV_AddEvent(event);		
					
				}
				//at 130000 movePlayersToDefautlSSLocation
				else if (!strcmp("at", LE_getCurrentToken()))
				{
					event = calloc(1, sizeof(event_t));
					event->time = LE_readReal();
					event->type = EV_AUTOPILOT_PL;
					//movePlayersToDefautlSSLocation
					LE_readToken();
					
					EV_AddEvent(event);	
					
					
				}
				 
				LE_readToken();
			}
		}
	}

	LE_popLexer();
}


void World_Update(void)
{
	//simulationTime = 25000;
	if (light.attachedToCamera)
	{
		//light.position = camera.position + camera.right * 5;
		vectorCopy(camera.up,light.position);
		vectorScale(light.position,-20,light.position);
		vectorAdd(camera.position,light.position,light.position);
		
		//light.lookAt = camera.forward * 10;
		vectorCopy(camera.forward,light.lookAt);
		vectorScale(light.lookAt,100,light.lookAt);
		
		vectorCopy(camera.up,light.upVector);
		
		
	}
	else 
	{
		//light.position[0] = cos (simulationTime*0.001f)*900	;
		//light.position[2] = sin (simulationTime*0.001f)*900	;

	}

	
}
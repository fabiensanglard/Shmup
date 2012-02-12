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
 *  main.c
 *  dEngine
 *
 *  Created by fabien sanglard on 09/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "dEngine.h"

#include "globals.h"
#include "math.h"
#include "camera.h"
#include "renderer.h"
#include "renderer_fixed.h"
#include "renderer_progr.h"
#include "filesystem.h"
#include "timer.h"
#include "world.h"
#include "material.h"
#include "commands.h"
#include "collisions.h"
#include "player.h"
#include "enemy.h"
#include "sounds.h"
#include "music.h"
#include "world.h"
#include "lexer.h"
#include "event.h"
#include "fx.h"
#include "vis.h"
#include "netchannel.h"
#include "menu.h"
#include "stats.h"
#include "titles.h"
#include "enemy_particules.h"
#include "text.h"
#include "event.h"

engine_info_t engine;



char* screenShotDirectory = "/Users/fabiensanglard/Pictures/dEngine/";


void dEngine_ReadConfig(void)
{
	filehandle_t* config;
	int currentSceneId=0;
	
    
    
	config = FS_OpenFile("data/config.cfg", "rt");
	FS_UploadToRAM(config);
	
	if (!config)
	{
		Log_Printf("Configuration file: data/config.cfg not found");
		exit(0);
	}
	
	//renderer.resolution = 1;

	LE_pushLexer();
	LE_init(config);
	
	while (LE_hasMoreData()) 
	{
		LE_readToken();

		if (!strcmp("scenes", LE_getCurrentToken()))
		{
			LE_readToken(); // {
			LE_readToken(); // numScenes
			while (LE_hasMoreData() && strcmp("}", LE_getCurrentToken()))
			{
				if (!strcmp("numScenes", LE_getCurrentToken()))
				{
					engine.numScenes = LE_readReal();
				}
				else 
				if (!strcmp("scene", LE_getCurrentToken()))
				{
					currentSceneId = LE_readReal();
					
					LE_readToken(); //The name of the scene, here only to help developer to keep track of config.cfg
					strReplace(LE_getCurrentToken(), '_', ' ');
					strcpy(engine.scenes[currentSceneId].name, LE_getCurrentToken());
					
					LE_readToken();
					strcpy(engine.scenes[currentSceneId].path, LE_getCurrentToken());
					
					Log_Printf("Read scene %d, name %s, path %s\n",currentSceneId,engine.scenes[currentSceneId].name,engine.scenes[currentSceneId].path);
					
					
				}
				
				LE_readToken();
				
			}
		}
		else if (!strcmp("fx", LE_getCurrentToken()))
		{
			LE_readToken(); //{
			while (LE_hasMoreData() && strcmp("}", LE_getCurrentToken()))
			{
				LE_readToken();
				
				if (!strcmp("impactTextureName", LE_getCurrentToken()))
				{
					LE_readToken();
	//			explosionTexture.path = calloc(strlen(LE_getCurrentToken())+1, sizeof(char));
					strcpy(explosionTexture.path, LE_getCurrentToken());
				}
				else 
				if (!strcmp("smokeTextureName", LE_getCurrentToken()))
				{
					LE_readToken();
	//				smokeTexture.path = calloc(strlen(LE_getCurrentToken())+1, sizeof(char));
					strcpy(smokeTexture.path, LE_getCurrentToken());
				}
				else 
				if (!strcmp("ghostTextureName", LE_getCurrentToken()))
				{
					LE_readToken();
	//				ghostTexture.path = calloc(strlen(LE_getCurrentToken())+1, sizeof(char));
					strcpy(ghostTexture.path, LE_getCurrentToken());
				}
			}
		}
		/*
		else if (!strcmp("video", LE_getCurrentToken()))
		{
			
			LE_readToken(); // {
			LE_readToken();
			while (strcmp("}", LE_getCurrentToken()))
			{
				if (!strcmp("record", LE_getCurrentToken()))
				{
					engine.recordVideo = LE_readReal();
				}
				LE_readToken();
			}
		}
		*/
		else if (!strcmp("players", LE_getCurrentToken()))
		{
			LE_readToken();	//{
			LE_readToken();
			while (LE_hasMoreData() && strcmp("}", LE_getCurrentToken()))
			{
				if (!strcmp("model1", LE_getCurrentToken()))
				{
					LE_readToken();
					strcpy(players[1].modelPath, LE_getCurrentToken());
				}
				else 
				if (!strcmp("model0", LE_getCurrentToken()))
				{
					LE_readToken();
					strcpy(players[0].modelPath, LE_getCurrentToken());
				}
				else 
					if (!strcmp("bulletTextureName", LE_getCurrentToken()))
				{
					LE_readToken();
//					bulletConfig.bulletTexture.path = calloc(strlen(LE_getCurrentToken())+1, sizeof(char));
					strcpy(bulletConfig.bulletTexture.path, LE_getCurrentToken());
				}
				else 
				if (!strcmp("ttl", LE_getCurrentToken()))
				{
					bulletConfig.ttl = LE_readReal();
				}
				else 
				if (!strcmp("heightRatio", LE_getCurrentToken()))
				{
					bulletConfig.heightRatio = LE_readReal();
				}
				else 
				if (!strcmp("widthRatio", LE_getCurrentToken()))
				{
					bulletConfig.widthRatio = LE_readReal();
				}
				else 
				if (!strcmp("screenSpaceXDeltaRatio", LE_getCurrentToken()))
				{
					bulletConfig.screenSpaceXDeltaRatio = LE_readReal();
				}
				else 
				if (!strcmp("screenSpaceYDeltaRatio", LE_getCurrentToken()))
				{
					bulletConfig.screenSpaceYDeltaRatio = LE_readReal();
				}
				else 
				if (!strcmp("flashHeightRatio", LE_getCurrentToken()))
				{
					bulletConfig.flashHeightRatio = LE_readReal();
				}
				else 
				if (!strcmp("flashWidthRatio", LE_getCurrentToken()))
				{
					bulletConfig.flashWidthRatio = LE_readReal();
				}
				else 
				if (!strcmp("flashScreenSpaceXDeltaRatio", LE_getCurrentToken()))
				{
					bulletConfig.flashScreenSpaceXDeltaRatio = LE_readReal();
				}
				else 
				if (!strcmp("flashScreenSpaceYDeltaRatio", LE_getCurrentToken()))
				{
					bulletConfig.flashScreenSpaceYDeltaRatio = LE_readReal();
				}
				
				LE_readToken();
			}
			
		}
		
	}
	
	
	LE_popLexer();
	FS_CloseFile(config);
	
	
}

uchar* screenShotBuffer;
void dEngine_INIT_ScreenshotBuffer(void)
{
	screenShotBuffer = calloc(renderer.glBuffersDimensions[WIDTH]*renderer.glBuffersDimensions[HEIGHT], 4);
}

void dEngine_WriteScreenshot(char* directory)
{
	
	int i;//,j;
	filehandle_t* pScreenshot;
	char fullPath[256];
	
	uchar tga_header[18];
	uchar* pixel;
	uchar tmpChannel;
	
	/*
	char num[64];
	sprintf(num, "scene%05d_t=%05d",engine.sceneId, simulationTime);
	memset(fullPath, 256, sizeof(char));
	strcat(fullPath,directory);
	strcat(fullPath,num);
	strcat(fullPath,".tga");
	*/
	fullPath[0] = '\0';
	sprintf(fullPath,"%sscene%05d_t=%05d.tga",directory,engine.sceneId, simulationTime);
	
	
	
	
	SCR_GetColorBuffer(screenShotBuffer);
	
	pScreenshot = FS_OpenFile(fullPath, "wb");
	
	memset(tga_header, 0, 18);
	tga_header[2] = 2;
	tga_header[12] = (renderer.glBuffersDimensions[WIDTH] & 0x00FF);
	tga_header[13] = (renderer.glBuffersDimensions[WIDTH]  & 0xFF00) / 256; // /256
	tga_header[14] = (renderer.glBuffersDimensions[HEIGHT]  & 0x00FF) ;
	tga_header[15] =(renderer.glBuffersDimensions[HEIGHT] & 0xFF00) / 256;
	tga_header[16] = 32 ;
	
	
	
	FS_Write(&tga_header, 18, sizeof(uchar), pScreenshot);
	
	// RGB > BGR
	pixel = screenShotBuffer;
	for(i=0 ; i < renderer.glBuffersDimensions[WIDTH] * renderer.glBuffersDimensions[HEIGHT] ; i++)
	{
		tmpChannel = pixel[0];
		pixel[0] = pixel[2];
		pixel[2] = tmpChannel;
		
		pixel += 4;
	}
	
	
	FS_Write(screenShotBuffer, renderer.glBuffersDimensions[WIDTH] * renderer.glBuffersDimensions[HEIGHT], 4 * sizeof(uchar), pScreenshot);
	
	FS_CloseFile(pScreenshot);
	
}

void dEngine_Init(void) 
{
	FS_InitFilesystem();
    Log_Printf("dEngine Initialization...\n");
    
	ENT_InitCacheSystem();
	TEXT_InitCacheSystem();
	MAT_InitCacheSystem();

#ifdef GENERATE_VIDEO	
	dEngine_INIT_ScreenshotBuffer();
#endif
	
	engine.sceneId = -1;
	dEngine_RequireSceneId(0);
	
	//engine.recordVideo = 0;
	
	engine.musicFilename[0] = '\0';
	engine.musicStartAt = 0;
	engine.playback.filename[0] = '\0';
	
	engine.soundEnabled = 1;
	engine.musicEnabled = 1;
	engine.gameCenterEnabled = 0;
	
	ENPAR_Init();
	
	

	dEngine_ReadConfig();

	SCR_Init();
	
	
	
	
	MATLIB_LoadLibraries();
	

	
	COLL_InitUnitCube();
	
	
	ENE_Mem_Init();
	
	SND_Init();
	
	
	MENU_Init();
	
	
}

void dEngine_InitDisplaySystem(uchar rendererType)
{
	SCR_BindMethods(rendererType);
	
	P_InitPlayers();
	FX_InitMem();
	
	TITLE_AllocRessources();
	COM_Init();
	
	P_CreatePointerCoordinates();
	
	engine.menuVisible = 0;
	MENU_Set(MENU_HOME);
}

void dEngine_LoadScene(int sceneId)
{
	event_t* ev;
	
	COM_StopRecording();

	engine.showFingers=0;
	engine.controlVisible=0;
	
	// Reset variables state (only usefull when we are not loading the first level
	EV_InitForScene();
	
	P_ResetPlayers();
	ENE_Reset();
	ENPAR_Reset();
	DYN_TEXT_Init();
	ENE_ReleaseAll();
	
	COM_ResetTouchesBuffer();
	
	numBackgroundEntities=0;
	
	engine.sceneId = sceneId;
	
		
	engine.musicStartAt= 0;
	
	// Now actually start loading things
	World_OpenScene(engine.scenes[engine.sceneId].path);
	
	MENU_Set(engine.scenes[engine.sceneId].defaultMenuId);
	
	COM_StartScene();
	
	
	
	
	ENE_Precache();
	
	if (engine.musicFilename[0] != '\0')
	{
		SND_InitSoundTrack(engine.musicFilename,engine.musicStartAt);
		SND_StartSoundTrack();
	}
	
	
	
	
	CAM_LoadPath();
	CAM_StartPlaying();
	
#ifdef GENERATE_VIDEO
		Timer_ForceTimeIncrement(16);
#endif	
	
	Timer_resetTime();
	
	//In single player we start right away, in multiplayer the go is given by the netchannel
	if (engine.mode == DE_MODE_SINGLEPLAYER)
		Timer_Resume();
		
	VIS_Update();
	
	if (engine.sceneId == 1 && engine.licenseType == LICENSE_LIMITED)
	{
		ev = calloc(1, sizeof(event_t));
		ev->time = 130000;
		ev->type = EV_LIMITED_EVENT;
		EV_AddEvent(ev);
	}
}

void dEngine_FreeSceneRessources(void)
{
    
	//A LOT A LOT OF THINGS TO FREE HERE !!!!
	// Scenes
	EV_CleanAllRemainingEvents();
	
	// Enemies
	
	// Camera path
	CAM_ClearAllRemainingCameraVS();
	
	
	
	TEXT_ClearTextureLibrary();

	//Debug traces only
	//ENT_DumpEntityCache();
	ENT_ClearModelsLibrary();
	
	//event object and camerapath object are freed on the fly
	World_ClearWorldMap();
	
	COM_ClearBuffers();
	
	TITLE_FreeRessources();
}

int timeJumpCounter =0;//;
int timeJumpTarget = 19250;
void dEngine_JumpInTime(void)
{
	event_t* event;
	event_t* toDelete;
	int i;
	
	if (timeJumpCounter >0)
	{
		//Pause the timer
		Timer_Pause();
		renderer.enabled=0;
		
		//Remove all futur enemy spawning events.
		event = EV_GetNextEvent();
		
		while (event != NULL) 
		{			
			while (event->next != NULL && event->next->time <= timeJumpTarget && event->next->type == EV_SPAWN_ENEMY)
			{
				//Log_Printf("[dEngine_JumpInTime] Cleaning EV_SPAWN_ENEMY events t=%d.\n",event->next->time);
				
				toDelete = event->next;
				event->next = event->next->next;
				
				free(toDelete->payload);
				free(toDelete);
			}
			
			
			event = event->next;
		}
		
	//	Log_Printf("[dEngine_JumpInTime] events cleaned.\n");
		
		while (simulationTime < timeJumpTarget) 
		{
			//Move one step ahead
			timediff = 16; //We are going to simulate by slice of 16ms until we reached the jumpTime.
			simulationTime+=timediff;
			
			
			//Play all frames
		//	Log_Printf("[dEngine_JumpInTime] t=%d.\n",simulationTime);
			dEngine_HostFrame();
		}
		
		for (i=0; i < numPlayers; i++) {
			players[i].showPointer = 0;
		}
		
		//Resume the timer
		Timer_Resume();
		renderer.enabled = 1;
		timeJumpCounter--;
	}
}

void dEngine_RequireSceneId(int sceneId)
{
	engine.requiredSceneId = sceneId ;
}

void dEngine_CheckState(void)
{
	
	if (engine.requiredSceneId == engine.sceneId)
		return;
	
	SND_StopSoundTrack();
	
	dEngine_FreeSceneRessources();
	
	dEngine_LoadScene(engine.requiredSceneId);
	
	
	engine.sceneId = engine.requiredSceneId;
	
	
	dEngine_JumpInTime();
	
	
}

void dEngine_HostFrame(void)
{
	// Load a new scene/menu if needed
	dEngine_CheckState();
	
	
	Timer_tick();
	
	
	diverSpriteLib.numVertices=0;
	diverSpriteLib.numIndices=0;
	
#ifdef GENERATE_VIDEO	
	if (MENU_Get() == 0 && simulationTime > 57000 && !MENU_GetCurrentButtonTouches()[0].down)
		MENU_GetCurrentButtonTouches()[0].down = 1 ;
	
	SND_UpdateRecord();
#endif	
	
	NET_Setup();
	
	NET_Receive();
	COM_Update();
	NET_Send();
	
	
	
	
	//Init the enemy FX system 
	ENPAR_StartEnemyFX();
	

	
	EV_Update();
	TITLE_Update();
	CAM_Update();
	DYN_TEXT_Update();
	//NET_Update();
	
	//Check collisions.
    COLL_CheckEnemies();
    COLL_CheckPlayers();
	
	//Update world
    World_Update();
	P_Update();
	ENE_Update();
	ENPAR_Update();
	FX_UpdateExplosions();
	FX_UpdateParticules();
	FX_UpdateSmoke();

	

	//Rendition
	P_PrepareBulletSprites();
	P_PrepareGhostSprites();
	FX_PrepareSmokeSprites();
	P_PreparePointerSprites();
	
	SCR_RenderFrame();
	
	
	
	if (engine.menuVisible)
		MENU_HandleTouches();
	
#ifdef GENERATE_VIDEO
		dEngine_WriteScreenshot(screenShotDirectory);
#endif
	
}






void dEngine_Pause(void)
{
	//Timer_Pause();
	
	
	title_mode = MODE_UNKNOWN;
	
	NET_Free();
	COM_StopRecording();
	engine.playback.play = 0;
	
	
	
	engine.menuVisible = 0;
	MENU_Set(MENU_HOME);
	SND_StopSoundTrack();
	SND_FinalizeRecord();
	//dEngine_RequireSceneId(0);
	engine.sceneId = -1;
}


void dEngine_Resume(void)
{
	//Timer_Resume();
	//unsigned int mode = 0x27F;
	//asm volatile ("fldcw %0" : : "m" (*&mode));
	
	
	dEngine_RequireSceneId(0);
	P_ResetPlayers();
	COM_ResetTouchesBuffer();
}


void dEngine_GoToNextScene(void)
{	
	if (engine.mode == DE_MODE_MULTIPLAYER && NET_IsRunning())
		NET_OnNextLevelLoad();
	else
		dEngine_RequireSceneId(engine.sceneId + 1  % engine.numScenes);
	
	
}






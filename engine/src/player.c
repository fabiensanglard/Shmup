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
 *  player.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-02-02.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "player.h"
#include "renderer.h"
#include "camera.h"
#include "timer.h"
#include "limits.h"
#include "enemy.h"
#include <math.h>
#include "sounds.h"
#include "menu.h"
#include "netchannel.h"
#include "dEngine.h"
#include "event.h"
#include "enemy_particules.h"
#include "renderer.h"
#include "native_services.h"

unsigned char numPlayerRespawn[] = {6,3,1};

#define SHOW_POINTER_DURATION 5000

uchar numPlayers;
uchar controlledPlayer;
player_t players[2];

diverSpriteLib_t diverSpriteLib;

uchar entitiesAttachedToCamera=0;

player_bullet_config_t bulletConfig;

//MAX_NUM_PLAYERS
xf_colorless_sprite_t pBulletVertices[(MAX_PLAYER_BULLETS*2*4+2*4)*MAX_NUM_PLAYERS]; //+2*4 to include the flash sprite

int numPBulletsIndices=0;
unsigned short bulletIndices[(MAX_PLAYER_BULLETS*2 * 6+ 2*6)*MAX_NUM_PLAYERS];

int numBulletSpriteVertices;


texture_t ghostTexture;

#define POINTER_TEXT_PATH "data/titles/divers.png"
texture_t pointersTexture;

void P_AttachToCamera(matrix_t globalMatrix)
{
	
	
	vec4_t ws_playerPos;
	vec4_t ss_playerPos;
	int i;
	entity_t* playerEntity;
	plan_t cameraFront;
	player_t* player;
			   
	
	//Need to check the screen space position
	
	for( i =0 ; i <numPlayers ; i++)
	{
		player = &players[i] ;
		playerEntity = &player->entity;
		
		ws_playerPos[X] = playerEntity->matrix[12];
		ws_playerPos[Y] = playerEntity->matrix[13];
		ws_playerPos[Z] = playerEntity->matrix[14];
		ws_playerPos[W] = 1;
		
		matrix_multiplyVertexByMatrix(ws_playerPos,globalMatrix,ss_playerPos);
		
		ss_playerPos[X] /= ss_playerPos[W] ;
		ss_playerPos[Y] /= ss_playerPos[W] ;
		
		
		player->ss_position[X] = ss_playerPos[X] ;
		player->ss_position[Y] = ss_playerPos[Y] ;
		
		//ViewPort
		//player->ss_position[X] *= renderWidth;
		//player->ss_position[Y] *= renderHeight;
		
		//printf("[P_AttachToCamera] player[%d] ssPos[X]=%.2f, ssPos[Y]=%.2f\n",i,player->ss_position[X],player->ss_position[Y]);
		
		players[i].showPointer = SHOW_POINTER_DURATION ;
	}
	
	//Also need to generate enemies ss_coordinates
	
	
	
	
	//Need to init distanceFromCamera, pixelWidthAtDistance and pixelHeightAtDistance
	cameraFront.normal[X] = camera.forward[X];
	cameraFront.normal[Y] = camera.forward[Y];
	cameraFront.normal[Z] = camera.forward[Z];
	cameraFront.d = - DotProduct(cameraFront.normal,camera.position);
					   
	distanceZFromCamera = DotProduct(cameraFront.normal,ws_playerPos) + cameraFront.d;

	distanceZFromCamera = fabsf(distanceZFromCamera);
//	if (distanceZFromCamera < 0)
//		distanceZFromCamera *= -1;
	
	heightAtDistance = tanf(camera.fov * DEG_TO_RAD / 2.0) ;
	heightAtDistance *= distanceZFromCamera ;
	//pixelHeightAtDistance /=renderHeight; 
	widthAtDistance = heightAtDistance * camera.aspect;
	
	//printf("distanceZFromCamera=%.2f.\n",distanceZFromCamera);
	//printf("pixelHeightAtDistance=%.2f.\n",heightAtDistance);
	//printf("pixelWidthAtDistance=%.2f.\n",widthAtDistance);

	entitiesAttachedToCamera= 1;
}

void P_DetachToCamera(void)
{
	int i;
	
	entitiesAttachedToCamera= 0;
	
	for (i=0; i < numPlayers; i++) {
		players[i].autopilot.enabled = 0;
		players[i].invulnerableFor = 0;
		players[i].shouldDraw = 1;
	}
}

void P_ResetPlayer(int i)
{
	ghost_t* ghost;
	int j,k;
	player_t* player;
	
	player = &players[i];
	
	
	player->invulFlickering = 0;
	player->invulnerableFor = 0;
	player->shouldDraw = 1;
	
	
	player->nextBulletFireTime = 0 ;
	player->nextGhostFireTime = 0;
	
	player->lastBulletType = i;
	
	player->firingUpTo = 0;
	
	player->showPointer = 0;
	player->autopilot.enabled = 0;
	
	for (j=0; j < MAX_PLAYER_BULLETS; j++) 
	{
		player->bullets[j].energy = BULLET_DEFAULT_ENERGY;
		player->bullets[j].expirationTime = 0;
	}
	
	
	for (k=0; k < GHOSTS_NUM; k++) 
	{
		ghost = &player->ghosts[k];
		ghost->timeCounter = GHOST_TTL_MS+1;
	}
	
	
	//Reset stats
	engine.playerStats.bulletsFired[i] = 0;
	engine.playerStats.bulletsHit[i] = 0;
	engine.playerStats.enemyDestroyed[i] = 0;
}

void PL_ResetPlayersScore(void)
{
	int i;
	
	for (i=0; i < MAX_NUM_PLAYERS; i++) 
	{
		players[i].score = 0;
	}
}

void P_LoadPlayer(int playerIdToLoad)
{
	
	entity_t*	currentEntity ;
	player_t* player;
	
	
	

	player = &players[playerIdToLoad];
	player->playerId = playerIdToLoad;
	currentEntity = &players[playerIdToLoad].entity ;
	currentEntity->model = (md5_mesh_t*)calloc(1,sizeof(md5_mesh_t)) ;
	ENT_LoadEntity(currentEntity,players[playerIdToLoad].modelPath,ENT_FULL_DRAW);
	

	currentEntity->model->memStatic = 1;
	currentEntity->material->textures[TEXTURE_DIFFUSE].memStatic= 1;
	currentEntity->material->textures[TEXTURE_BUMP].memStatic= 1;
	currentEntity->material->textures[TEXTURE_SPECULAR].memStatic= 1;
	
	//We need to mark everything in this entity as DO_NOT_FREE
	P_ResetPlayer(playerIdToLoad);
}



void P_ResetPlayers(void)
{
	int i;
	
	for (i=0; i < MAX_NUM_PLAYERS ; i++)
		P_ResetPlayer(i);
	
	entitiesAttachedToCamera = 0;
	engine.playerStats.numEnemies = 0;
}





#define PLAYER_LIVE_COUNT_HEIGHT 0.12
#define PLAYER_LIVE_COUNT_WIDTH -0.12
#define PLAYER_LIVE_COUNT_SPACING -0.05
#define PLAYER_LIVE_COUNT_START_X  0.96f
#define PLAYER_LIVE_COUNT_START_Y  1.48f


#define PLAYER_LIVE_COUNT_TEXT_START_U (0 / 256*SHRT_MAX)
#define PLAYER_LIVE_COUNT_TEXT_START_V (88  / 128.0f*SHRT_MAX)
#define PLAYER_LIVE_COUNT_TEXT_WIDTH (48/256.0f*SHRT_MAX)
#define PLAYER_LIVE_COUNT_TEXT_HEIGHT (40/128.0f*SHRT_MAX)


#define SCORE_POS_X -300
#define SCORE_POS_Y 460
#define SCORE_FONT_SIZE 2.7f
#define SCORE_FORMAT "SCORE:%7u"
void P_InitPlayers(void)
{
	int j;

	
	numPlayers = 1;
	controlledPlayer = 0;
	
	P_LoadPlayer(0);
	P_LoadPlayer(1);
	
			
	
	
	
	//Loading bulletSprites
	TEX_MakeStaticAvailable(&bulletConfig.bulletTexture);
	
	TEX_MakeStaticAvailable(&ghostTexture);
	
	
//	pointersTexture.path = calloc(sizeof(char), strlen(POINTER_TEXT_PATH)+1);
	strcpy(pointersTexture.path,POINTER_TEXT_PATH);
	TEX_MakeStaticAvailable(&pointersTexture);
	
	bulletConfig.distPerLifepsan = SS_H*2  ; 	
	
	
	bulletConfig.halfHeight = SS_H*2 * bulletConfig.heightRatio / 2 ;
	bulletConfig.halfWidth = bulletConfig.halfHeight * bulletConfig.widthRatio ;

	bulletConfig.ss_deltaX = SS_W * bulletConfig.screenSpaceXDeltaRatio;
	bulletConfig.ss_deltaY = SS_H * bulletConfig.screenSpaceYDeltaRatio;
	
	bulletConfig.flashScreenSpaceXDelta = 2*SS_W * bulletConfig.flashScreenSpaceXDeltaRatio;
	bulletConfig.flashScreenSpaceYDelta = 2*SS_H* bulletConfig.flashScreenSpaceYDeltaRatio;
	
	bulletConfig.msBetweenBullets = (2*SS_H*bulletConfig.heightRatio) * (bulletConfig.ttl/(2.0f*SS_H));

	bulletConfig.flashHeight =		(SS_H			      * bulletConfig.flashHeightRatio);
	bulletConfig.flashHalfWidth  = (bulletConfig.flashHeight  * bulletConfig.flashWidthRatio);
	
	
	
	//Also prepare bullets indices
	numBulletSpriteVertices = 0 ;
	
	for (j=0; j < (MAX_PLAYER_BULLETS*2 * 6 + 2*6); j+=6,numBulletSpriteVertices+=4) 
	{
		bulletIndices[j+0] = numBulletSpriteVertices+0;
		bulletIndices[j+1] = numBulletSpriteVertices+1;
		bulletIndices[j+2] = numBulletSpriteVertices+3;
		bulletIndices[j+3] = numBulletSpriteVertices+3;
		bulletIndices[j+4] = numBulletSpriteVertices+1;
		bulletIndices[j+5] = numBulletSpriteVertices+2;
	}
	numBulletSpriteVertices=0;

	
	
	
	//Init player life count sprites
	 
	 
	 //Prepare the number of remaining lives sprites here

	
	 
	 
	
}





void P_FireBullet(player_t* player,float deltaX, float deltaY)
{
	
	vec2_t spawningPos ;
	
	bullet_t* bullet;
		
	bullet = &player->bullets[player->numBullets];
	
	bullet->expirationTime = simulationTime + bulletConfig.ttl ;
	
	
	spawningPos[X] = player->ss_position[X]*SS_W + deltaX;
	spawningPos[Y] = player->ss_position[Y]*SS_H + deltaY;
	
	bullet->spawnedY = spawningPos[Y];
	
	bullet->spawnedTime = simulationTime;
	
	//Generate ss_boudaries
	bullet->ss_boudaries[UP]   =  spawningPos[Y] + bulletConfig.halfHeight;
	bullet->ss_boudaries[DOWN] =  spawningPos[Y] - bulletConfig.halfHeight;
	bullet->ss_boudaries[LEFT] =  spawningPos[X] - bulletConfig.halfWidth;
	bullet->ss_boudaries[RIGHT]=  spawningPos[X] + bulletConfig.halfWidth;
	
	bullet->energy = BULLET_DEFAULT_ENERGY;
	
	bullet->type = player->lastBulletType++ ;
	player->lastBulletType &= 3;
	
	player->numBullets = (MAX_PLAYER_BULLETS-1) & (++player->numBullets);
	
	player->firingUpTo = simulationTime+ bulletConfig.msBetweenBullets;
	 
}

void P_FireOneBullet(player_t* player)
{
	if ( player->nextBulletFireTime > simulationTime)
		return;
	
	SND_PlaySound(SND_PLASMA);
	
	P_FireBullet(player,0,0);
	
	player->nextBulletFireTime = simulationTime + bulletConfig.msBetweenBullets ;
	player->lastBulletType = ++player->lastBulletType ;
	player->lastBulletType = player->lastBulletType % 2;
	
}

void P_FireTwoBullet(player_t* player)
{
	if ( player->nextBulletFireTime > simulationTime)
		return;
	
	engine.playerStats.bulletsFired[player->playerId] +=2;
	
	SND_PlaySound(SND_PLASMA);
	
	P_FireBullet(player,bulletConfig.ss_deltaX ,bulletConfig.ss_deltaY);
	P_FireBullet(player,-bulletConfig.ss_deltaX,bulletConfig.ss_deltaY);
	
	player->nextBulletFireTime = simulationTime + bulletConfig.msBetweenBullets ;
	player->lastBulletType = ++player->lastBulletType ;
	player->lastBulletType = player->lastBulletType % 2;
	
}

//Warning this matrix is declared as row major: <-- Shit !! This line was actually useful 4 month later !!!! You are good fab !!!
/*
		1	0	0	0
		0	0  -1	0
		0   1	0	0
		0	0	0	1
 */
matrix_t fromAboveRotation = {1 , 0  , 0 , 0,
							  0 , 0  , 1 , 0,
							  0 , -1 , 0 , 0,
							  0 , 0  , 0 , 1,} ; 

void P_Update(void)
{
	int i,j;
	entity_t* playerEntity;
	player_t* player;
	bullet_t* bullet;
	
	matrix_t viewMatrix;
	
	vec3_t translationForwardTransform;
	vec3_t translationRightTransform;
	vec3_t translationUpTransform;
	vec3_t translationTransform;
	
	vec3_t vLookat;
	
	
	float t;
	short bulletHeight;
	
		
	vectorAdd(camera.position,camera.forward,vLookat);	
	gluLookAt(camera.position, vLookat, camera.up, viewMatrix);
	
	// Building transpose of camera rotation
	cameraInvRot[0] = viewMatrix[0]; 	cameraInvRot[4] = viewMatrix[1]; 	cameraInvRot[8] = viewMatrix[2];	cameraInvRot[12] = 0;
	cameraInvRot[1] = viewMatrix[4];	cameraInvRot[5] = viewMatrix[5];	cameraInvRot[9] = viewMatrix[6];	cameraInvRot[13] = 0;
	cameraInvRot[2] = viewMatrix[8];	cameraInvRot[6] = viewMatrix[9];	cameraInvRot[10] = viewMatrix[10];	cameraInvRot[14] = 0;
	cameraInvRot[3] = 0;				cameraInvRot[7] = 0;				cameraInvRot[11] = 0;				cameraInvRot[15] = 1;

	
	//players[0].ss_position[Y] +=0.01 ;
	//players[1].ss_position[X] +=0.01 ;
	
	
	
	
	//Need to update entity matrix to stick to camera
	for( i =0 ; i <numPlayers ; i++)
	{
		player = &players[i] ;
		playerEntity = &player->entity;
		
		if (entitiesAttachedToCamera) 
		{
			players[i].showPointer -= timediff;
			players[i].score += timediff >> 1 << engine.difficultyLevel;
			
			if (player->autopilot.enabled)
			{
				t = player->autopilot.timeCounter/player->autopilot.originalTime;
				
				//printf("t=%.2f\n",t);
				//printf("player->autopilot.timeCounter=%d\n",player->autopilot.timeCounter);
				//printf("player->autopilot.enabled =%d\n",(player->autopilot.timeCounter >0));
				t *= t;   
				player->ss_position[X] = player->autopilot.end_ss_position[X] + t * player->autopilot.diff_ss_position[X];
				player->ss_position[Y] = player->autopilot.end_ss_position[Y] + t * player->autopilot.diff_ss_position[Y];
				P_UpdateSSBoundaries(i);
				
				player->autopilot.timeCounter -= timediff;
				//printf("player->autopilot.timeCounter=%d.\n",player->autopilot.timeCounter);
				player->autopilot.enabled = (player->autopilot.timeCounter > 0) ;
				
				
			}
			
			
			if (players[i].invulnerableFor > 0)
			{
				players[i].invulFlickering += timediff;
				players[i].shouldDraw =players[i].invulFlickering & 128 ; //Fickering every 128ms
				
				players[i].invulnerableFor -= timediff;
				if (players[i].invulnerableFor <= 0)
						players[i].shouldDraw = 1;
			}
			
			
			//UPDATE MATRIX
			
			//Rotation part
			matrix_multiply(cameraInvRot, fromAboveRotation, playerEntity->matrix);
			
			//Translation part
			vectorScale(camera.forward,distanceZFromCamera,translationForwardTransform);
			vectorScale(camera.right, player->ss_position[X] * widthAtDistance ,translationRightTransform);
			vectorScale(camera.up   , player->ss_position[Y] * heightAtDistance,translationUpTransform);
			
			
			
			vectorCopy(camera.position,									translationTransform) ;
			vectorAdd(translationTransform,translationForwardTransform,	translationTransform) ;
			vectorAdd(translationTransform,translationRightTransform,	translationTransform) ;
			vectorAdd(translationTransform,translationUpTransform,		translationTransform) ;
			
			
			playerEntity->matrix[12] = translationTransform[X] ;
			playerEntity->matrix[13] = translationTransform[Y] ;
			playerEntity->matrix[14] = translationTransform[Z] ;
			
			
			//END UPDATE MATRIX	
			
			//Also update bullets
			for(j=0; j < MAX_PLAYER_BULLETS ; j++)
			{
				if (player->bullets[j].expirationTime < simulationTime )
					continue;
				
				bullet = &player->bullets[j];
				
				t = (simulationTime - bullet->spawnedTime)/ (float)bulletConfig.ttl ;
				
				bulletHeight = bullet->spawnedY +  t* (bulletConfig.distPerLifepsan);
				
				
				bullet->ss_boudaries[UP] = bulletHeight + bulletConfig.halfHeight;
				bullet->ss_boudaries[DOWN] = bulletHeight - bulletConfig.halfHeight ;
			}
			
			
			// Update ghosts
			P_UpdateGhosts(player);
			
			
			
			
		}
		else 
		{
			//Ship is not yet behaving as in 2D screenspace, we need to have it follow a predetermined path
			
			//Update translation
			//playerEntity->matrix[12] += translationTransform[X] ;
			//playerEntity->matrix[13] += translationTransform[Y] ;
			
			//playerEntity->matrix[14] = -0.24f * simulationTime + players[i].spawnWorldPosition[Z] ;
			playerEntity->matrix[14] += -0.24f * timediff ;
			//240 units per 1000ms
			
			//printf("t= %d: tdiff:%d p pos=%.2f,%.2f,%.2f.\n",simulationTime,timediff,playerEntity->matrix[12],playerEntity->matrix[13],playerEntity->matrix[14]);
			//Update roll if necessary
			
		}

				
		
	}

	
	
	
}

#define NUM_INDICE_POINTER_PER_PLAYER 18
#define NUM_VERTICE_POINTER_PER_PLAYER 10
vec2short_t pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER * MAX_NUM_PLAYERS];
xf_colorless_sprite_t pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER * MAX_NUM_PLAYERS];
ushort pointerSprIndices[NUM_INDICE_POINTER_PER_PLAYER*MAX_NUM_PLAYERS] = { 1, 0, 2,  0, 2, 3,  4, 5, 6,    5, 6, 7,    6,7,8   , 8, 7, 9,       
								11,10,12, 10,12,13, 14,15,16,   15,16,17,   16,17,18   ,18,17,19
							    };
ushort  numSprIndices;

void P_CreatePointerCoordinates(void)
{
	vec2short_t start,end;
	vec2_t dir;

	/*
	 
	 
	 
	 
	 0_______________3
	 |              |
	 |   Circle     |
	 |              |
	 |         5    |
	 |      4 / \   |
	 |        \  \  |		 
	 |         \  \ |        
	 -----------\----        
	 1           \  2         
	              \  \        
	               \  \       
	                \  \      
	                 \  \7____________________________9
	                  \________________________________|
	                   6                              8	 
	 
	 
	 */
	
	
	// Round circle over player 1 and player 2
	//0  3
	//1  2
	pointerSprVertices[0].text[X] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+0].text[X] = 0;
	pointerSprVertices[0].text[Y] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+0].text[Y] = 0;
	pointerSprVertices[1].text[X] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+1].text[X] = 0;
	pointerSprVertices[1].text[Y] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+1].text[Y] = 64/(float)128*SHRT_MAX;
	pointerSprVertices[2].text[X] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+2].text[X] = 64/(float)256*SHRT_MAX;
	pointerSprVertices[2].text[Y] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+2].text[Y] = 64/(float)128*SHRT_MAX;
	pointerSprVertices[3].text[X] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+3].text[X] = 64/(float)256*SHRT_MAX;
	pointerSprVertices[3].text[Y] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+3].text[Y] = 0;
	
	pointerdeltaSprVertices[0][X] = pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+0][X]= -0.2 * SS_W;
	pointerdeltaSprVertices[0][Y] = pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+0][Y]=  0.2 * 0.6666 * SS_H;
	pointerdeltaSprVertices[1][X] = pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+1][X]= -0.2 * SS_W;
	pointerdeltaSprVertices[1][Y] = pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+1][Y]= -0.2 * 0.6666* SS_H;
	pointerdeltaSprVertices[2][X] = pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+2][X]=  0.2 * SS_W;
	pointerdeltaSprVertices[2][Y] = pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+2][Y]= -0.2 * 0.6666* SS_H;
	pointerdeltaSprVertices[3][X] = pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+3][X]=  0.2 * SS_W;
	pointerdeltaSprVertices[3][Y] = pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+3][Y]=  0.2 * 0.6666* SS_H;
	
	

#define UNDERLINE_WIDTH 0.008f
	//Underline part 1
	//0  3
	//1  2	
	
	start[X] = 0;
	start[Y] = 0;
	end[X] = 0.3 * SS_W;
	end[Y] =-0.2 * 0.6666* SS_H;
	dir[X] = end[X] - start[X];
	dir[Y] = end[Y] - start[Y];
	normalize2(dir);
	
	pointerdeltaSprVertices[4][X]=   dir[Y] * SS_W * UNDERLINE_WIDTH;
	pointerdeltaSprVertices[4][Y]=  - dir[X] * SS_H * UNDERLINE_WIDTH;
	pointerdeltaSprVertices[5][X]= -  dir[Y] * SS_W * UNDERLINE_WIDTH;
	pointerdeltaSprVertices[5][Y]=  dir[X] * SS_H * UNDERLINE_WIDTH;
	pointerdeltaSprVertices[6][X]=  0.3 * SS_W		        + dir[Y] * SS_W * UNDERLINE_WIDTH;
	pointerdeltaSprVertices[6][Y]= -0.2 * 0.6666* SS_H     - dir[X] * SS_H * UNDERLINE_WIDTH;
	pointerdeltaSprVertices[7][X]=  0.3 * SS_W				- dir[Y] * SS_W * UNDERLINE_WIDTH;
	pointerdeltaSprVertices[7][Y]=  -0.2 * 0.6666* SS_H   + dir[X] * SS_H * UNDERLINE_WIDTH;
	
	//Player2 is different
	pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+4][X]= -pointerdeltaSprVertices[4][X];
	pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+4][Y]= -pointerdeltaSprVertices[4][Y];
	pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+5][X]= -pointerdeltaSprVertices[5][X];
	pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+5][Y]= -pointerdeltaSprVertices[5][Y];
	pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+6][X]= -pointerdeltaSprVertices[6][X];
	pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+6][Y]= -pointerdeltaSprVertices[6][Y];
	pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+7][X]= -pointerdeltaSprVertices[7][X];
	pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+7][Y]= -pointerdeltaSprVertices[7][Y];
	
	pointerSprVertices[4].text[X] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+4].text[X] = 59/(float)256*SHRT_MAX ;
	pointerSprVertices[4].text[Y] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+4].text[Y] =  36 /(float)128*SHRT_MAX ;
	pointerSprVertices[7].text[X] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+7].text[X] =  59/(float)256*SHRT_MAX ; 
	pointerSprVertices[7].text[Y] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+7].text[Y] =  37 /(float)128*SHRT_MAX ; 
	pointerSprVertices[5].text[X] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+5].text[X] =  60/(float)256*SHRT_MAX ; 
	pointerSprVertices[5].text[Y] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+5].text[Y] =  37 /(float)128*SHRT_MAX ;
	pointerSprVertices[6].text[X] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+6].text[X] =  60/(float)256*SHRT_MAX ; 
	pointerSprVertices[6].text[Y] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+6].text[Y] =  36 /(float)128*SHRT_MAX ;
	
	
	//Underline part 2 
	//0  3
	//1  2	
	pointerdeltaSprVertices[8][X] = pointerdeltaSprVertices[6][X] + 0.43 * SS_W;
	pointerdeltaSprVertices[8][Y] = pointerdeltaSprVertices[6][Y];
	pointerdeltaSprVertices[9][X] = pointerdeltaSprVertices[8][X];
	pointerdeltaSprVertices[9][Y] = pointerdeltaSprVertices[7][Y];

	pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+8][X]=  -pointerdeltaSprVertices[8][X];
	pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+8][Y]=  -pointerdeltaSprVertices[8][Y];
	pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+9][X]=  -pointerdeltaSprVertices[9][X];
	pointerdeltaSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+9][Y]=  -pointerdeltaSprVertices[9][Y];
	
	
	
	pointerSprVertices[8].text[X] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+8].text[X] =  60/(float)256*SHRT_MAX ; 
	pointerSprVertices[8].text[Y] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+8].text[Y] =  37 /(float)128*SHRT_MAX ;

	pointerSprVertices[9].text[X] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+9].text[X] =  60/(float)256*SHRT_MAX ; 
	pointerSprVertices[9].text[Y] = pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER+9].text[Y] =  36 /(float)128*SHRT_MAX ;

	
}

// Update position of the pointer to be above the player's ship
void P_PreparePointerSprites(void)
{
	int i,j;
	
	numSprIndices = 0;
	
	for (i=0; i < numPlayers; i++) 
	{
		if (players[i].showPointer <=0)
			continue;
		
		//Only need to update the vertices coordiantes as the texture one have been generated already.
		for (j=0;  j < NUM_VERTICE_POINTER_PER_PLAYER; j++) {
			pointerSprVertices[j+i*NUM_VERTICE_POINTER_PER_PLAYER].pos[X] = pointerdeltaSprVertices[j+i*NUM_VERTICE_POINTER_PER_PLAYER][X] + players[i].ss_position[X] * SS_W ;
			pointerSprVertices[j+i*NUM_VERTICE_POINTER_PER_PLAYER].pos[Y] = pointerdeltaSprVertices[j+i*NUM_VERTICE_POINTER_PER_PLAYER][Y] + players[i].ss_position[Y] * SS_H ;

		}
		numSprIndices+= 24;
		
	}
}

char* playersNames[2] = {"Player 1","Player 2"};
float playerDelta[2][2] = {
	/*p1*/{110,-28},
	/*p2*/{-200,98}
};

char stringScore[64];
void PL_RenderPlayerPointers(void)
{
	xf_colorless_sprite_t* spriteVertice;
	int i;
	
	if (!entitiesAttachedToCamera)
		return;
	
	spriteVertice = &diverSpriteLib.vertices[diverSpriteLib.numVertices];
	

	for (i=0; i < players[controlledPlayer].respawnCounter; i++) 
	{
		spriteVertice->pos[X] = (PLAYER_LIVE_COUNT_START_X + (PLAYER_LIVE_COUNT_SPACING+ PLAYER_LIVE_COUNT_WIDTH) * i) * SS_W;
		spriteVertice->pos[Y] = (PLAYER_LIVE_COUNT_START_Y ) * SS_W;
		spriteVertice->text[U] = PLAYER_LIVE_COUNT_TEXT_START_U;
		spriteVertice->text[V] = PLAYER_LIVE_COUNT_TEXT_START_V;
		//spriteVertice->color[R] =  spriteVertice->color[G] =  spriteVertice->color[B] =  spriteVertice->color[A] = 255; 
		spriteVertice++;
		
		spriteVertice->pos[X] = (PLAYER_LIVE_COUNT_START_X + (PLAYER_LIVE_COUNT_SPACING+ PLAYER_LIVE_COUNT_WIDTH) * i ) * SS_W;
		spriteVertice->pos[Y] = (PLAYER_LIVE_COUNT_START_Y - PLAYER_LIVE_COUNT_HEIGHT) * SS_W;
		spriteVertice->text[U] = PLAYER_LIVE_COUNT_TEXT_START_U;
		spriteVertice->text[V] = PLAYER_LIVE_COUNT_TEXT_START_V + PLAYER_LIVE_COUNT_TEXT_HEIGHT;
		// spriteVertice->color[R] =  spriteVertice->color[G] =  spriteVertice->color[B] =  spriteVertice->color[A] = 255;
		spriteVertice++;
		
		
		spriteVertice->pos[X] = (PLAYER_LIVE_COUNT_START_X + (PLAYER_LIVE_COUNT_SPACING+ PLAYER_LIVE_COUNT_WIDTH) * i + PLAYER_LIVE_COUNT_WIDTH) * SS_W;
		spriteVertice->pos[Y] = (PLAYER_LIVE_COUNT_START_Y - PLAYER_LIVE_COUNT_HEIGHT) * SS_W;
		spriteVertice->text[U] = PLAYER_LIVE_COUNT_TEXT_START_U + PLAYER_LIVE_COUNT_TEXT_WIDTH;
		spriteVertice->text[V] = PLAYER_LIVE_COUNT_TEXT_START_V + PLAYER_LIVE_COUNT_TEXT_HEIGHT;
		// spriteVertice->color[R] =  spriteVertice->color[G] =  spriteVertice->color[B] =  spriteVertice->color[A] = 255;
		spriteVertice++;
		
		
		spriteVertice->pos[X] = (PLAYER_LIVE_COUNT_START_X + (PLAYER_LIVE_COUNT_SPACING+ PLAYER_LIVE_COUNT_WIDTH) * i + PLAYER_LIVE_COUNT_WIDTH) * SS_W;
		spriteVertice->pos[Y] = (PLAYER_LIVE_COUNT_START_Y ) * SS_W;
		spriteVertice->text[U] = PLAYER_LIVE_COUNT_TEXT_START_U + PLAYER_LIVE_COUNT_TEXT_WIDTH;
		spriteVertice->text[V] = PLAYER_LIVE_COUNT_TEXT_START_V;
		// spriteVertice->color[R] =  spriteVertice->color[G] =  spriteVertice->color[B] =  spriteVertice->color[A] = 255;
		spriteVertice++;
		
		diverSpriteLib.numVertices+= 4;
		diverSpriteLib.numIndices+=6;
	}
	
	renderer.SetTexture(pointersTexture.textureId);
	
	renderer.RenderColorlessSprites(diverSpriteLib.vertices,diverSpriteLib.numIndices,enFxLib.indices);
	
	
	
	//Pointers
	for (i=0 ; i < numPlayers ; i++)
	{
		if (players[i].showPointer <= 0)
			continue;
		
		renderer.RenderColorlessSprites(pointerSprVertices,NUM_INDICE_POINTER_PER_PLAYER,pointerSprIndices+i*NUM_INDICE_POINTER_PER_PLAYER);
	
		
	}
	
	for (i=0 ; i < numPlayers ; i++)
	{
		if (players[i].showPointer <= 0)
			continue;
		
		SCR_StartConvertText();
		SCR_ConvertTextToVertices(playersNames[i],2.2f,players[i].ss_boudaries[LEFT]+playerDelta[i][X],players[i].ss_boudaries[DOWN]+playerDelta[i][Y],TEXT_NOT_CENTERED);
		SCR_RenderText();
	}
	

	
	//Also render highscore
	sprintf(stringScore,SCORE_FORMAT,players[controlledPlayer].score);
	SCR_StartConvertText();
	SCR_ConvertTextToVertices(stringScore,SCORE_FONT_SIZE,SCORE_POS_X,SCORE_POS_Y,TEXT_NOT_CENTERED);
	SCR_RenderText();
	
}


void P_PrepareBulletSprites(void)
{
	int i,j;
	short flashY;
	short leftFlashX;
	short rightFlashX;
	float flashInterpolation;
	
	xf_colorless_sprite_t* bulSprite;
	
	bullet_t* bullet;
	player_t* player;
	
	bulSprite = pBulletVertices;
	numPBulletsIndices = 0;
	
	
	for(i=0 ; i < numPlayers ; i++)
	{
		player = &players[i] ;
		
		//Check if the player is currently firing and spawn a flash if so.
		if (player->firingUpTo >= simulationTime)
		{
			
			flashInterpolation = 1- (player->firingUpTo - simulationTime) / (float)bulletConfig.msBetweenBullets;
			
			flashY =	player->ss_position[Y] * SS_H + bulletConfig.flashScreenSpaceYDelta ;
			leftFlashX =player->ss_position[X] * SS_W  - bulletConfig.flashScreenSpaceXDelta ; 
			rightFlashX=player->ss_position[X] * SS_W  + bulletConfig.flashScreenSpaceXDelta ; 
			
			bulSprite->pos[X] = leftFlashX - bulletConfig.flashHalfWidth  ;
			bulSprite->pos[Y] = flashY  ;
			bulSprite->text[X] = (80.0f/128*SHRT_MAX) + i*(24.0f/128*SHRT_MAX);
			bulSprite->text[Y] = (64.0f/128*SHRT_MAX) +  (32.0f/128*SHRT_MAX) + player->lastBulletType* (32.0f/128*SHRT_MAX);
			bulSprite++;
			
			bulSprite->pos[X] = leftFlashX - bulletConfig.flashHalfWidth  ;
			bulSprite->pos[Y] = flashY + bulletConfig.flashHeight * flashInterpolation ;
			bulSprite->text[X] = (80.0f/128*SHRT_MAX) + i*(24.0f/128*SHRT_MAX);
			bulSprite->text[Y] = (64.0f/128*SHRT_MAX) + player->lastBulletType* (32.0f/128*SHRT_MAX);
			bulSprite++;
			
			
			bulSprite->pos[X] = leftFlashX + bulletConfig.flashHalfWidth  ;
			bulSprite->pos[Y] = flashY + bulletConfig.flashHeight * flashInterpolation;
			bulSprite->text[X] = (80.0f/128*SHRT_MAX) + i*(24.0f/128*SHRT_MAX) + (24.0f/128*SHRT_MAX);
			bulSprite->text[Y] = (64.0f/128*SHRT_MAX) + player->lastBulletType* (32.0f/128*SHRT_MAX);
			bulSprite++;
			
			bulSprite->pos[X] = leftFlashX + bulletConfig.flashHalfWidth  ;
			bulSprite->pos[Y] = flashY  ;
			bulSprite->text[X] = (80.0f/128*SHRT_MAX) + i*(24.0f/128*SHRT_MAX) + (24.0f/128*SHRT_MAX);
			bulSprite->text[Y] = (64.0f/128*SHRT_MAX) + (32.0f/128*SHRT_MAX) + player->lastBulletType* (32.0f/128*SHRT_MAX);
			bulSprite++;
	
						
			
			
			
			bulSprite->pos[X] = rightFlashX - bulletConfig.flashHalfWidth  ;
			bulSprite->pos[Y] = flashY   ;
			bulSprite->text[X] = (80.0f/128*SHRT_MAX) + i*(24.0f/128*SHRT_MAX);
			bulSprite->text[Y] = (64.0f/128*SHRT_MAX) +  (32.0f/128*SHRT_MAX) + player->lastBulletType* (32.0f/128*SHRT_MAX);
			bulSprite++;
			
			bulSprite->pos[X] = rightFlashX - bulletConfig.flashHalfWidth  ;
			bulSprite->pos[Y] = flashY + bulletConfig.flashHeight * flashInterpolation  ;
			bulSprite->text[X] = (80.0f/128*SHRT_MAX) + i*(24.0f/128*SHRT_MAX);
			bulSprite->text[Y] = (64.0f/128*SHRT_MAX) + player->lastBulletType* (32.0f/128*SHRT_MAX);
			bulSprite++;
			
			
			bulSprite->pos[X] = rightFlashX + bulletConfig.flashHalfWidth  ;
			bulSprite->pos[Y] = flashY + bulletConfig.flashHeight * flashInterpolation ;
			bulSprite->text[X] = (80.0f/128*SHRT_MAX) + i*(24.0f/128*SHRT_MAX) + (24.0f/128*SHRT_MAX);
			bulSprite->text[Y] = (64.0f/128*SHRT_MAX) + player->lastBulletType* (32.0f/128*SHRT_MAX);
			bulSprite++;
			
			bulSprite->pos[X] = rightFlashX + bulletConfig.flashHalfWidth  ;
			bulSprite->pos[Y] = flashY  ;
			bulSprite->text[X] = (80.0f/128*SHRT_MAX) + i*(24.0f/128*SHRT_MAX) + (24.0f/128*SHRT_MAX);
			bulSprite->text[Y] = (64.0f/128*SHRT_MAX) + (32.0f/128*SHRT_MAX) + player->lastBulletType* (32.0f/128*SHRT_MAX);
			bulSprite++;
			
			numPBulletsIndices += 12;
			
			
		}
		
		// Check if there is any bullets to render.
		for(j=0 ; j < MAX_PLAYER_BULLETS ; j++)
		{
			bullet = &player->bullets[j];
			
			
			if (bullet->expirationTime < simulationTime)
				continue;
			
			bullet++;
			bullet->type = bullet->type & 3;
			
			bulSprite->pos[X] = bullet->ss_boudaries[LEFT];
			bulSprite->pos[Y] = bullet->ss_boudaries[DOWN];
			bulSprite->text[X] = i*(16.0f/128*SHRT_MAX) ;
			bulSprite->text[Y] = bullet->type*(32.0f/128*SHRT_MAX) + 32.0f/128*SHRT_MAX;
			bulSprite++;
			
			bulSprite->pos[X] = bullet->ss_boudaries[LEFT];
			bulSprite->pos[Y] = bullet->ss_boudaries[UP];
			bulSprite->text[X] = i*(16.0f/128*SHRT_MAX);
			bulSprite->text[Y] = bullet->type*(32.0f/128*SHRT_MAX) ;
			bulSprite++;
			
					
			bulSprite->pos[X] = bullet->ss_boudaries[RIGHT];
			bulSprite->pos[Y] = bullet->ss_boudaries[UP];
			bulSprite->text[X] = i*(16.0f/128*SHRT_MAX) + 16.0f/128*SHRT_MAX;
			bulSprite->text[Y] = bullet->type*(32.0f/128*SHRT_MAX);
			bulSprite++;
			
			bulSprite->pos[X] = bullet->ss_boudaries[RIGHT];
			bulSprite->pos[Y] = bullet->ss_boudaries[DOWN];
			bulSprite->text[X] = i*(16.0f/128*SHRT_MAX) + (16.0f/128*SHRT_MAX);
			bulSprite->text[Y] = bullet->type*(32.0f/128*SHRT_MAX)+32.0f/128*SHRT_MAX;
			bulSprite++;
			
			
			/*
			bulletIndices[numPBulletsIndices]   = numBulletSpriteVertices+0;
			bulletIndices[numPBulletsIndices+1] = numBulletSpriteVertices+1;
			bulletIndices[numPBulletsIndices+2] = numBulletSpriteVertices+3;
			bulletIndices[numPBulletsIndices+3] = numBulletSpriteVertices+3;
			bulletIndices[numPBulletsIndices+4] = numBulletSpriteVertices+1;
			bulletIndices[numPBulletsIndices+5] = numBulletSpriteVertices+2;
			*/
			
			numPBulletsIndices += 6;
			//numBulletSpriteVertices += 4;
		}	
	}
}



float ghostDirection[GHOSTS_NUM][2] = 
{
	{0.5,0.86},
	{0.7,0.7},
	{0.86,0.5},
	{0.96,0.25},
	
	{-0.5,0.86},
	{-0.7,0.7},
	{-0.86,0.5},
	{-0.96,0.25},	
};


#define GHOST_DEF_ROT 0.25
float ghostDefaultRotation[GHOSTS_NUM] = 
{
	GHOST_DEF_ROT*1.2f,
	GHOST_DEF_ROT*1.1f,
	GHOST_DEF_ROT,
	GHOST_DEF_ROT*1.9f,
	
	-GHOST_DEF_ROT*1.2f,
	-GHOST_DEF_ROT*1.1f,
	-GHOST_DEF_ROT,
	-GHOST_DEF_ROT*1.9f,	
};



void P_FireGhosts(player_t* player)
{
	int i;
	ghost_t* ghost;
	
	
	if (simulationTime < player->nextGhostFireTime)
		return;
	
	engine.playerStats.bulletsFired[player->playerId] +=8;
	
	SND_PlaySound(SND_GHOST_LAUNCH);
	
	player->nextGhostFireTime = simulationTime + MS_BETWEEN_GHOST;
	
	for (i=0; i < GHOSTS_NUM; i++) 
	{
		
		ghost  = &player->ghosts[i];
	
		ghost->target = 0;
		ghost->lastTimeSimulated = simulationTime;	
		ghost->timeCounter = 0;
		ghost->energy = 5;
		ghost->head = 0;
		ghost->startVertexArray = 0;
		ghost->lengthVertexArray = 0;
	
		vector2Copy(player->ss_position,ghost->ss_position);
		
		ghost->short_ss_position[X] = ghost->ss_position[X] * SS_W;
		ghost->short_ss_position[Y] = ghost->ss_position[Y] * SS_H;		
	
		vector2Copy(ghostDirection[i],ghost->ss_direction);
		
	}
	
}

void P_UpdateGhosts(player_t* player)
{
	ghost_t* ghost;
	vec2_t ss_normal;
	enemy_t* target = 0;
	float tmpXdirection;
	int i;
	float ghostRotation;
	vec2_t vecEnemy;
	
	target = ENE_GetFirstEnemy();
	
	for (i=0; i < GHOSTS_NUM; i++) 
	{
		
		ghost = &player->ghosts[i];
	
		if (ghost->timeCounter >= GHOST_TTL_MS)
			return;
	
		ghost->timeCounter += timediff;
	
		// No retarget|| (ghost->target != 0 && ghost->target->energy <= 0)))
		if (ghost->timeCounter > GHOST_FREE_TIME_MS && ghost->timeCounter < GHOST_AUTO_AIM_TIME_LIMIT_MS && ghost->target == NULL )
		{
			if (target != NULL)
			{
				while (1) 
				{
					//Only pick target visible on screen
					if (fabs(target->ss_boudaries[UP]) < SS_H || fabs(target->ss_boudaries[DOWN])  > SS_H)
					{
						ghost->target = target;
						ghost->targetUniqueId = target->uniqueId;
						break; 
					}
					
					//No next, aborting target search
					if (target->next == NULL)
						break;  
					
					target = target->next;
				}
					   
					
				//printf("ghost target type=%d , ss_pos[%.2f,%.2f].\n",ghost->target->type,target->ss_position[X],target->ss_position[Y]);
				if (target->next != NULL)
					target = target->next;
			}
		}
		
		//Need to simulate by step GHOST_DELTA_T_MS starting last simulated time	
		while (ghost->lastTimeSimulated + GHOST_DELTA_T_MS < simulationTime) 
		{
		
			ss_normal[X] = - ghost->ss_direction[Y] * GHOST_HALFWIDTH;
			ss_normal[Y] =   ghost->ss_direction[X] * GHOST_HALFWIDTH;
		
			//Write left pos in vertex array
			ghost->wayPoints[ghost->head].pos[X] = (ghost->ss_position[X] + ss_normal[X] ) * SS_W;
			ghost->wayPoints[ghost->head].pos[Y] = (ghost->ss_position[Y] + ss_normal[Y] ) * SS_H;
			ghost->head++;
		
			//Write right pos in vertex array
			ghost->wayPoints[ghost->head].pos[X] = (ghost->ss_position[X] - ss_normal[X] ) * SS_W;
			ghost->wayPoints[ghost->head].pos[Y] = (ghost->ss_position[Y] - ss_normal[Y] ) * SS_H;
			ghost->head++;
		
			/*
			if(ghost->target != 0)
			{
				printf("t= %d ghost pos= [%.2f,%.2f].\n",ghost->lastTimeSimulated,ghost->ss_position[X],ghost->ss_position[Y]);
				printf("t= %d ghost dire= [%.2f,%.2f].\n",ghost->lastTimeSimulated,ghost->ss_direction[X],ghost->ss_direction[Y]);
				printf("t= %d target pos= [%.2f,%.2f].\n",ghost->lastTimeSimulated,ghost->target->ss_position[X],ghost->target->ss_position[Y]);
			}
			*/
			//UPDATE POSITION
			ghost->ss_position[X] += GHOST_DELTA_T_MS * GHOST_SPEED_SS_MS * ghost->ss_direction[X];
			ghost->ss_position[Y] += GHOST_DELTA_T_MS * GHOST_SPEED_SS_MS * ghost->ss_direction[Y];

			
			if (ghost->energy > 0 )
			{
			
				//UPDATE DIRECTION (make sure direction remain normalized
				if (ghost->timeCounter < GHOST_FREE_TIME_MS )
				{
					tmpXdirection = ghost->ss_direction[X];
					ghost->ss_direction[X] =  cosf(GHOST_ROT_RAD_PER_DELTA*ghostDefaultRotation[i])*ghost->ss_direction[X]    - sinf(GHOST_ROT_RAD_PER_DELTA*ghostDefaultRotation[i])*ghost->ss_direction[Y];
					ghost->ss_direction[Y] =  sinf(GHOST_ROT_RAD_PER_DELTA*ghostDefaultRotation[i])*tmpXdirection			+ cosf(GHOST_ROT_RAD_PER_DELTA*ghostDefaultRotation[i])*ghost->ss_direction[Y];
				}
				else 
				if (ghost->timeCounter < GHOST_AUTO_AIM_TIME_LIMIT_MS)
				{
					if (ghost->target != 0 && ghost->targetUniqueId == ghost->target->uniqueId && ghost->target->energy > 0 )
					{
					
						//c=a-b
						vector2Subtract(ghost->target->ss_position,ghost->ss_position,vecEnemy);
						//Auto aim
						ghostRotation =  atan2(
											   vecEnemy[X]*-ghost->ss_direction[Y] + vecEnemy[Y]*ghost->ss_direction[X],
												vecEnemy[X]* ghost->ss_direction[X] + vecEnemy[Y]*ghost->ss_direction[Y]  
												 
												);					
						//ghostRotation -= atan2(ghost->ss_direction[Y],ghost->ss_direction[X]);
						
						if (ghostRotation >= 0)
						{
							if (ghostRotation > GHOST_ROT_RAD_PER_DELTA*2 )
								ghostRotation = GHOST_ROT_RAD_PER_DELTA;
						}
						else 
						{
							if (ghostRotation < -(GHOST_ROT_RAD_PER_DELTA*2) )
								ghostRotation = -(GHOST_ROT_RAD_PER_DELTA*2);
						}
					
						//printf("t= %d, gid=%d rotation = %.2f\n",ghost->lastTimeSimulated,i,ghostRotation);
					
					
						tmpXdirection = ghost->ss_direction[X];
						ghost->ss_direction[X] =  cosf(ghostRotation)*ghost->ss_direction[X] - sinf(ghostRotation)*ghost->ss_direction[Y];
						ghost->ss_direction[Y] =  sinf(ghostRotation)*tmpXdirection			 + cosf(ghostRotation)*ghost->ss_direction[Y];
					
					}
				}
				else 
				{
					// Increase speed slowly
					// Also increase tail length
				}
			}

			//	printf("Simulated ghost t=%d x=%.2f y=%.2f.\n",ghost->lastTimeSimulated,ghost->ss_position[X],ghost->ss_position[Y]);
			ghost->lastTimeSimulated += GHOST_DELTA_T_MS;
		
		}
		
		ghost->short_ss_position[X] = ghost->ss_position[X] * SS_W;
		ghost->short_ss_position[Y] = ghost->ss_position[Y] * SS_H;
	}
	
}

void P_PrepareGhostSprites(void)
{
	player_t*		player;
	ghost_t*		ghost;
	int i,j;
	short textureY;
	short stepUShort;
	

	xf_colorless_sprite_t* vertex;
	
	for (i=0; i < numPlayers; i++) 
	{
		player = &players[i];
		
		for (j=0; j < GHOSTS_NUM; j++) 
		{
			ghost = &player->ghosts[j];
		
			if (ghost->timeCounter >= GHOST_TTL_MS)
				continue;
		
			if (ghost->head  - GHOST_TAIL_VERTICES < 0)
			{
				ghost->startVertexArray = 0;
				ghost->lengthVertexArray = ghost->head   ;
			}
			else 
			{
				ghost->startVertexArray = (ghost->head - GHOST_TAIL_VERTICES) ;
				ghost->lengthVertexArray = GHOST_TAIL_VERTICES;
			}
		
			if (ghost->lengthVertexArray == 0)
				continue;
		
		

			stepUShort = SHRT_MAX/(ghost->lengthVertexArray/2);
			
		
			//printf("Ghost has %d vertices stepUShort=%d , stepUByte=%d.\n",ghost->lengthVertexArray,stepUShort,stepUByte);
		
			textureY = SHRT_MAX;
			
		
			vertex = &ghost->wayPoints[ghost->startVertexArray];
			while (vertex < &ghost->wayPoints[ghost->head]) 
			{
			
				//Need to update texture coordinate
				vertex->text[X] = i * SHRT_MAX/2; 
				vertex->text[Y] = textureY;
				vertex++;
			
				//Need to update texture coordinate
				vertex->text[X] = i * SHRT_MAX/2 + SHRT_MAX/2; 
				vertex->text[Y] = textureY;
				vertex++;

				textureY -= stepUShort;
			
			}
		}
	}
		
	
}



void P_Die(uchar playerId)
{

	command_t t;
	event_t* event;
	event_req_menu_t* eventReqMenu;
	event_req_scene_t* eventReqScene;
	// Player collided with the enemy
	// Spawn explosin, smoke and particules
	FX_GetExplosion(players[playerId].ss_position,IMPACT_TYPE_YELLOW,1,0);
	Spawn_EntityParticules(players[playerId].ss_position);
	FX_GetSmoke(players[playerId].ss_position, 0.3, 0.3);
	
	SND_PlaySound(SND_EXPLOSION);
	
	players[playerId].respawnCounter--;

	
	
	
	//NET_Update peer that we died
	if (engine.mode == DE_MODE_MULTIPLAYER && playerId == controlledPlayer)
	{
		t.time = simulationTime;
		t.type = NET_RTM_DIED;
		t.playerId = playerId;
		Net_SendDie(&t);
	}
	
	players[playerId].invulnerableFor = PLAYER_INVUL_TIME_MS;
	
	
	if (players[playerId].respawnCounter >= 0)
	{
		//printf("RESPAWN branch lives=%d\n",players[playerId].lives);
		players[playerId].invulFlickering = 0;
		
		
		// Set player's position out of screen
		players[playerId].ss_position[X] = 0.5f*(playerId-0.5f)*2.f;
		players[playerId].ss_position[Y] = -1.4;
		
		
		players[playerId].autopilot.enabled = 1;
		
		players[playerId].autopilot.end_ss_position[X] = 0.5f*(playerId-0.5f)*2.f;
		players[playerId].autopilot.end_ss_position[Y] = -0.0f;
		
		players[playerId].autopilot.diff_ss_position[X] = players[playerId].ss_position[X] - players[playerId].autopilot.end_ss_position[X];
		players[playerId].autopilot.diff_ss_position[Y] = players[playerId].ss_position[Y] - players[playerId].autopilot.end_ss_position[Y];
		
		players[playerId].autopilot.timeCounter = PLAYER_RESPAWN_REPLACMENT;
		players[playerId].autopilot.originalTime = PLAYER_RESPAWN_REPLACMENT;
		
		players[playerId].showPointer = SHOW_POINTER_DURATION;
		
	}
	else 
	{
		//printf("RIP branch lives=%d\n",players[playerId].lives);
		// Set player's position out of screen
		players[playerId].ss_position[X] = 0.5f*(playerId-0.5f)*2.f;
		players[playerId].ss_position[Y] = -1.4;
		
		
		players[playerId].autopilot.enabled = 1;
		
		players[playerId].autopilot.end_ss_position[X] = 0.5f*(playerId-0.5f)*2.f;
		players[playerId].autopilot.end_ss_position[Y] = -1.4f;
		
		players[playerId].autopilot.diff_ss_position[X] = 0;
		players[playerId].autopilot.diff_ss_position[Y] = 0;
		
		players[playerId].autopilot.timeCounter = 2000000;
		players[playerId].shouldDraw = 0;
		
		
		if (((numPlayers == 1) && (playerId == controlledPlayer))     ||
			((numPlayers == 2) && (players[0].respawnCounter < 0 && players[1].respawnCounter < 0))
           )
		{
			MENU_Set(MENU_GAMEOVER);
			Native_UploadScore(players[controlledPlayer].score);
			
			players[playerId].invulnerableFor = 500000;
			
			//Request scene 0 and menu 0 for within 3 seconds from now
			event = calloc(1, sizeof(event_t));
			event->type = EV_REQUEST_MENU;
			event->time = simulationTime + 5000;
			eventReqMenu = (event_req_menu_t*)calloc(1,sizeof(event_req_menu_t));
			eventReqMenu->menuId = MENU_HOME;
			event->payload = eventReqMenu;
			EV_AddEvent(event);
			
			event= calloc(1, sizeof(event_t));
			event->type = EV_REQUEST_SCENE;
			event->time = simulationTime + 5000;
			eventReqScene = (event_req_scene_t*)calloc(1,sizeof(event_req_scene_t));
			eventReqScene->sceneId = 0;
			event->payload = eventReqScene;
			EV_AddEvent(event);
			
			
		}
		//printf("players[playerId].autopilot.timeCounter=%d\n",players[playerId].autopilot.timeCounter);
	}
	
	
	P_UpdateSSBoundaries(playerId);
}


void P_UpdateSSBoundaries(uchar pId)
{
	players[pId].ss_boudaries[UP] = (players[pId].ss_position[Y] + 0.03)*SS_H;
	players[pId].ss_boudaries[DOWN] = (players[pId].ss_position[Y] - 0.03)*SS_H;
	players[pId].ss_boudaries[LEFT] = (players[pId].ss_position[X] - 0.04)*SS_W; 
	players[pId].ss_boudaries[RIGHT] = (players[pId].ss_position[X] + 0.04)*SS_W;
	
}


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
 *  player.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-02-02.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_PLAYER
#define DE_PLAYER

#include "entities.h"
#include "md5.h"
#include "matrix.h"
#include "texture.h"
#include "fx.h"


extern texture_t pointersTexture;

#define MAX_PLAYER_BULLETS 16
#define BULLET_DEFAULT_ENERGY 1
#define MAX_NUM_PLAYERS 2
#define PLAYER_NUM_LIVES 3
#define PLAYER_SPEED_MS (3/1000.0f)
#define PLAYER_INVUL_TIME_MS 3700
#define PLAYER_RESPAWN_REPLACMENT 1700.0f
#define PLAYER_ENDLEVEL_REPLACMENT 1000.0f
typedef struct player_bullet_config_t
{
	//From config file
		texture_t	bulletTexture;
		float		ttl;			//ms 	
		float		heightRatio;	// ratio vs screen
		float		widthRatio;		// ratio vs height

		float screenSpaceXDeltaRatio;
		float screenSpaceYDeltaRatio;

		//FLASH
		float		flashHeightRatio;	// ratio vs screen
		float		flashWidthRatio;		// ratio vs height
		float		flashScreenSpaceXDeltaRatio;
		float		flashScreenSpaceYDeltaRatio;

	//Calculated
		int			halfHeight;			//pixels
		int			halfWidth ;			//pixels
		float		distPerLifepsan;			////screenSpace units per ms
		int			msBetweenBullets;
		int			ss_deltaX;
		int			ss_deltaY;
	
		// FLASH
		short		flashScreenSpaceXDelta;
		short		flashScreenSpaceYDelta;
		short		flashHalfWidth;
		short		flashHeight;
	
	
} player_bullet_config_t;

extern player_bullet_config_t bulletConfig;

/*
typedef struct playerBulletSprite_t
{
	short int pos[2];
	short int text[2];
	
} playerBulletSprite_t;
*/

extern int numPBulletsIndices;
extern unsigned short bulletIndices[(MAX_PLAYER_BULLETS*2 * 6 + 2*6)*MAX_NUM_PLAYERS];
extern xf_colorless_sprite_t pBulletVertices[(MAX_PLAYER_BULLETS*2*4+2*4)*MAX_NUM_PLAYERS];


typedef struct bullet_t
{	
	short ss_boudaries[4];
	
	short spawnedY;
	
	int expirationTime;
	int spawnedTime;
	
	short energy;
	
	uchar type;
	
} bullet_t ;

//#define GHOST_DELTA_T_MS 3
//#define GHOST_DELTA_T_MS 8
#define GHOST_DELTA_T_MS 8
#define GHOST_TAIL_VERTICES (400/GHOST_DELTA_T_MS*2)
#define GHOST_TTL_MS 1500
#define MS_BETWEEN_GHOST 5000
//#define GHOST_FREE_TIME_MS 140
#define GHOST_FREE_TIME_MS ((int)(GHOST_TTL_MS/(float)100*13))
//#define GHOST_FREE_TIME_MS ((int)(GHOST_TTL_MS - GHOST_TTL_MS/(float)100*25))
#define GHOST_AUTO_AIM_TIME_LIMIT_MS ((int)(GHOST_TTL_MS - GHOST_TTL_MS/(float)100*45))
#define GHOST_SPEED_SS_MS (2.2/350.0) 
#define GHOST_HALFWIDTH 0.03
#define GHOST_MAX_NUM_VERTICES (2 * GHOST_TTL_MS / GHOST_DELTA_T_MS +2)
#define GHOSTS_NUM 8
#define GHOST_ROT_SPEED_RAD_MS (2*M_PI/400)
#define GHOST_ROT_RAD_PER_DELTA (GHOST_ROT_SPEED_RAD_MS*GHOST_DELTA_T_MS)

typedef struct ghost_t
{
	
	short short_ss_position[2];
	
	vec2_t ss_position;
	vec2_t ss_direction;

	short energy;
	int timeCounter;
	int lastTimeSimulated;
	
	ushort head;
	
	xf_colorless_sprite_t wayPoints[GHOST_MAX_NUM_VERTICES];
	
	ushort startVertexArray;
	ushort lengthVertexArray;
	
	enemy_t* target;
	unsigned int targetUniqueId ;
	
} ghost_t;

extern texture_t ghostTexture;

typedef struct bezierCtrl_t
{
	int start;
	int duration;
	
	vec3_t controlPoints[4];
	
} bezierCtrl_t;

typedef struct autopilot_t
{
	uchar enabled;
	int timeCounter;
	float originalTime;
	vec2_t diff_ss_position;
	vec2_t end_ss_position;
} autopilot_t ;

typedef struct player_t
{
	uchar cameraAttached ;
	
	vec2_t ss_position;
	
	entity_t entity;
	
	char modelPath[256];
	
	int nextBulletFireTime;
	uchar numBullets;
	bullet_t bullets[MAX_PLAYER_BULLETS];
	
	uchar lastBulletType;
	
	int firingUpTo;
	uchar lastFiringFlashType;
	
	 
	
	short ss_boudaries[4];
	
	ghost_t ghosts[GHOSTS_NUM];
	int nextGhostFireTime;
	
	int invulnerableFor;
	ushort invulFlickering;
	char respawnCounter;
	uchar shouldDraw;
	
	autopilot_t autopilot;
	int showPointer;
	
	uchar playerId;
	
	//vec3_t spawnWorldPosition;
	
	uint score;
	
} player_t ;






extern uchar numPlayers;
extern uchar controlledPlayer;
extern player_t players[2];
extern uchar entitiesAttachedToCamera;

void P_InitPlayers(void);
void P_ResetPlayers(void);
void PL_ResetPlayersScore(void);

void P_InitPlayersBullets(void);
void P_AttachToCamera(matrix_t globalMatrix);
void P_DetachToCamera(void);

void P_Update(void);
void P_PrepareBulletSprites(void);


void P_FireOneBullet(player_t* player);
void P_FireTwoBullet(player_t* player);



void P_UpdateGhosts(player_t* player);
void P_PrepareGhostSprites(void);
void P_FireGhosts(player_t* player);

void P_Die(uchar playerId);
void P_UpdateSSBoundaries(uchar pId);

void P_CreatePointerCoordinates(void);
void P_PreparePointerSprites(void);

void PL_RenderPlayerPointers(void);


// Structure array populated to draw player cursor

//extern xf_colorless_sprite_t pointerSprVertices[NUM_VERTICE_POINTER_PER_PLAYER*MAX_NUM_PLAYERS];
//extern ushort pointerSprIndices[36];
//extern ushort  numSprIndices;

//extern xf_sprite_t lifeCountSpriteVertice[PLAYER_NUM_LIVES*4];

extern unsigned char numPlayerRespawn[];

#endif

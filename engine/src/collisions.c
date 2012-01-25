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
 *  collisions.c
 *  dEngine
 *
 *  Created by fabien sanglard on 09-11-15.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */


/*
 Let A,B,C,D be 2-space position vectors. Then the directed line segments AB and CD are given by:
 
 AB=A+r(B-A), r ∈ [0,1]
 CD=C+s(D-C), s ∈ [0,1]
 
 Ax+r(Bx-Ax)=Cx+s(Dx-Cx)
 Ay+r(By-Ay)=Cy+s(Dy-Cy) for some r,s ∈ [0,1]
 
 (Ay-Cy)(Dx-Cx)-(Ax-Cx)(Dy-Cy)
 r =    -----------------------------  (eqn 1)
 (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
 
 
 (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay)
 s =     -----------------------------  (eqn 2)
 (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
 
 Testing DOWN, UP, LEFT, RIGHT
 */


/*
 // First: early rejection phase
 if (ss_enemy_boudaries[DOWN] > ghost->short_ss_position[Y]      &&
 ss_enemy_boudaries[DOWN] > ghost->prev_short_ss_position[Y] ||
 ss_enemy_boudaries[UP] < ghost->short_ss_position[Y]      &&
 ss_enemy_boudaries[UP] < ghost->prev_short_ss_position[Y] ||
 ss_enemy_boudaries[LEFT] > ghost->short_ss_position[X]      &&
 ss_enemy_boudaries[LEFT] > ghost->prev_short_ss_position[X] ||
 ss_enemy_boudaries[RIGHT] < ghost->short_ss_position[X]      &&
 ss_enemy_boudaries[RIGHT] < ghost->prev_short_ss_position[X])
 continue;
 
 //Crap with have an (unlikely) intersection with one of our four plane, detailled analysis here
 //This is a segment/segment collision test that we run on all four segment of the bounding box
 // This is an early accept algorythm
 
 
 
 #define Cx (ghost->short_ss_position[X])
 #define Cy (ghost->short_ss_position[Y])
 #define Dx (ghost->prev_short_ss_position[X])			
 #define Dy (ghost->prev_short_ss_position[Y])								
 
 
 
 // Down Left - Down Right
 #define Ay (ss_enemy_boudaries[DOWN])
 #define Ax (ss_enemy_boudaries[LEFT])
 #define Bx (ss_enemy_boudaries[LEFT])				
 #define By (ss_enemy_boudaries[LEFT])	
 
 r = (Ay-Cy)*(Dx-Cx)-(Ax-Cx)*(Dy-Cy) / ((Bx-Ax)*(Dy-Cy)-(By-Ay)*(Dx-Cx)) ;
 if (r >= 0 && r <= 1) goto collision;
 #undef Ay
 #undef Ax	
 #undef By
 #undef Bx				
 // Down Left - Down Right
 #define Ay ss_enemy_boudaries[DOWN]
 #define Ax ss_enemy_boudaries[LEFT]
 #define Bx ss_enemy_boudaries[LEFT]				
 #define By ss_enemy_boudaries[LEFT]								
 r = (Ay-Cy)*(Dx-Cx)-(Ax-Cx)*(Dy-Cy) / ((Bx-Ax)*(Dy-Cy)-(By-Ay)*(Dx-Cx)) ;
 if (r >= 0 && r <= 1) goto collision;
 #undef Ay
 #undef Ax	
 #undef By
 #undef Bx
 // Down Left - Down Right
 #define Ay ss_enemy_boudaries[DOWN]
 #define Ax ss_enemy_boudaries[LEFT]
 #define Bx ss_enemy_boudaries[LEFT]				
 #define By ss_enemy_boudaries[LEFT]								
 r = (Ay-Cy)*(Dx-Cx)-(Ax-Cx)*(Dy-Cy) / ((Bx-Ax)*(Dy-Cy)-(By-Ay)*(Dx-Cx)) ;
 if (r >= 0 && r <= 1) goto collision;
 #undef Ay
 #undef Ax	
 #undef By
 #undef Bx
 
 // Down Left - Down Right
 #define Ay ss_enemy_boudaries[DOWN]
 #define Ax ss_enemy_boudaries[LEFT]
 #define Bx ss_enemy_boudaries[LEFT]				
 #define By ss_enemy_boudaries[LEFT]								
 r = (Ay-Cy)*(Dx-Cx)-(Ax-Cx)*(Dy-Cy) / ((Bx-Ax)*(Dy-Cy)-(By-Ay)*(Dx-Cx)) ;
 if (r >= 0 && r <= 1) goto collision;
 
 #undef Ay
 #undef Ax	
 #undef By
 #undef Bx
 #undef Cy
 #undef Cx
 #undef Dy
 #undef Dx
 
 continue;
 
 collision:
 */




#include "collisions.h"
#include "player.h"
#include "enemy.h"
#include "timer.h"
#include "fx.h"
#include "sounds.h"
#include "dEngine.h"
#include "enemy_particules.h"

plan_t unitCubePlans[6];

void COLL_InitUnitCube(void)
{
	plan_t* near;
	plan_t* far;
	plan_t* right;
	plan_t* left;
	plan_t* top;
	plan_t* bottom;
	
	near = &unitCubePlans[PLAN_NEAR];
	far  = &unitCubePlans[PLAN_FAR];
	left = &unitCubePlans[PLAN_LEFT];
	right= &unitCubePlans[PLAN_RIGHT];
	top   = &unitCubePlans[PLAN_TOP];
	bottom = &unitCubePlans[PLAN_BOTTOM];
	
	near->normal[0] = 0 ;
	near->normal[1] = 0 ;
	near->normal[2] = 1 ;
	near->d = 1 ;
	
	far->normal[0] = 0 ;
	far->normal[1] = 0 ;
	far->normal[2] = -1 ;
	far->d = 1 ;
	
	left->normal[0] = 1 ;
	left->normal[1] = 0 ;
	left->normal[2] = 0 ;
	left->d = 1 ;
	
	right->normal[0] = -1 ;
	right->normal[1] = 0 ;
	right->normal[2] = 0 ;
	right->d = 1 ;
	
	bottom->normal[0] = 0 ;
	bottom->normal[1] = 1 ;
	bottom->normal[2] = 0 ;
	bottom->d = 1 ;
	
	top->normal[0] = 0 ;
	top->normal[1] = -1 ;
	top->normal[2] = 0 ;
	top->d = 1 ;	
}





static float COLL_distPointPlan(vec3_t point, plan_t* plan)
{
	return DotProduct(point,plan->normal) + plan->d;
}

/*
Intersection_test COLL_CheckFaceAgainstPlan(prec_face_t* face,plan_t* plan)
{
	char numIn  = 0;
	char numOut = 0;
	
	if (COLL_distPointPlan(face->screenPoints[0],plan) < 0 )
		numOut++;
	else
		numIn++;

	if (COLL_distPointPlan(face->screenPoints[1],plan) < 0 )
		numOut++;
	else
		numIn++;
	
	if (COLL_distPointPlan(face->screenPoints[2],plan) < 0 )
		numOut++;
	else
		numIn++;
	
	if (numIn == 0)
		return INT_OUT;
	
	if (numOut == 0)
		return INT_IN;
	
	return INT_INTERSEC;
}






Intersection_test COLL_CheckAgainstUnitCube(prec_face_t* face)
{
	
	int i;
	Intersection_test test;
	char interesction = 0;
	
	for(i=0; i < 6 ; i++)
	{
		test = COLL_CheckFaceAgainstPlan(face,&unitCubePlans[i]);
		
		if (test == INT_INTERSEC  ) interesction = 1;
		if (test == INT_OUT  ) return INT_OUT;
	}
	
	if (interesction)
		return INT_INTERSEC;
	
	return INT_IN ;
}
 */

void COLL_GenerateFrustrum(matrix_t pv,frustrum_t frustrum)
{
	//Fast frustrum extraction
	frustrum[PLAN_LEFT].normal[0] = pv[ 3] + pv[ 0];
	frustrum[PLAN_LEFT].normal[1] = pv[ 7] + pv[ 4];
	frustrum[PLAN_LEFT].normal[2] = pv[11] + pv[ 8];
	frustrum[PLAN_LEFT].d         = pv[15] + pv[12];
	
	frustrum[PLAN_RIGHT].normal[0] = pv[ 3] - pv[ 0];
	frustrum[PLAN_RIGHT].normal[1] = pv[ 7] - pv[ 4];
	frustrum[PLAN_RIGHT].normal[2] = pv[11] - pv[ 8];
	frustrum[PLAN_RIGHT].d         = pv[15] - pv[12];
	
	frustrum[PLAN_BOTTOM].normal[0] = pv[ 3] + pv[ 1];
	frustrum[PLAN_BOTTOM].normal[1] = pv[ 7] + pv[ 5];
	frustrum[PLAN_BOTTOM].normal[2] = pv[11] + pv[ 9];
	frustrum[PLAN_BOTTOM].d         = pv[15] + pv[13];
	
	frustrum[PLAN_TOP].normal[0] = pv[ 3] - pv[ 1];
	frustrum[PLAN_TOP].normal[1] = pv[ 7] - pv[ 5];
	frustrum[PLAN_TOP].normal[2] = pv[11] - pv[ 9];
	frustrum[PLAN_TOP].d         = pv[15] - pv[13];
	
	frustrum[PLAN_NEAR].normal[0] = pv[ 3] + pv[ 2];
	frustrum[PLAN_NEAR].normal[1] = pv[ 7] + pv[ 6];
	frustrum[PLAN_NEAR].normal[2] = pv[11] + pv[10];
	frustrum[PLAN_NEAR].d         = pv[15] + pv[14];
	
	frustrum[PLAN_FAR].normal[0] = pv[ 3] - pv[ 2];
	frustrum[PLAN_FAR].normal[1] = pv[ 7] - pv[ 6];
	frustrum[PLAN_FAR].normal[2] = pv[11] - pv[10];
	frustrum[PLAN_FAR].d         = pv[15] - pv[14];

	

}

Intersection_test COLL_CheckBoxAgainstPlan(bbox_t box,plan_t* plan)
{
	int i;
	char numIn  = 0;
	char numOut = 0;
	
	for (i=0 ; i < 8 ; i++) 
	{
		if ( COLL_distPointPlan(box[i],plan) < 0 )
			numOut++;
		else
			numIn++;
	}
	
	if (numIn == 0)
		return INT_OUT;
	
	if (numOut == 0)
		return INT_IN;
	
	return INT_INTERSEC;
}


Intersection_test COLL_CheckBoxAgainstFrustrum(bbox_t box, frustrum_t frustrum)
{
	int i;
	Intersection_test test;
	char interesction = 0;
	
	for(i=0; i < 6 ; i++)
	{
		test = COLL_CheckBoxAgainstPlan(box,&frustrum[i]);
		
		if (test == INT_INTERSEC  ) interesction = 1;
		if (test == INT_OUT  ) return INT_OUT;
	}
	
	if (interesction)
		return INT_INTERSEC;
	
	return INT_IN ;
}

#define ENTITY_EX_PARTICULE_RATIO 1.1
void Spawn_BulletParticules(bullet_t* bullet, int type)
{
	vec2_t ss_position;
	/*
	vec2_t random;
	
	
	//Particule here
	random[X] = (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1);
	random[X] /= 4;
	random[Y] =  1;
	normalize2(random);
	FX_GetParticule(enemy->ss_position,random 	,0.01*PARTICULE_SIZE_GLOBAL*ENTITY_EX_PARTICULE_RATIO	,PARTICULE_TRAVEL_DIST/1.4f,PARTICULE_TYPE_EXPLOSION, PARTICULE_COLOR_BLUE,PARTICULE_DEFAULT_STRECH-1);
	
	random[X] = (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1);
	random[X] /= 2;
	random[Y] = 1;	
	normalize2(random);
	FX_GetParticule(enemy->ss_position,random 	,0.011*PARTICULE_SIZE_GLOBAL*ENTITY_EX_PARTICULE_RATIO	,PARTICULE_TRAVEL_DIST/1.4f,PARTICULE_TYPE_EXPLOSION, PARTICULE_COLOR_BLUE,PARTICULE_DEFAULT_STRECH);
	 */
	ss_position[X] = ( bullet->ss_boudaries[LEFT]+bulletConfig.halfWidth ) / (float)SS_H;
	//ss_position[Y] = (bullet->ss_boudaries[DOWN]+((bullet->ss_boudaries[UP]    - bullet->ss_boudaries[DOWN]) >>1)) / (float)SS_H;
	ss_position[Y] = bullet->ss_boudaries[UP] / (float)SS_H;
	
	FX_GetExplosion(ss_position, type, 0.35f,0.0f);
}

void Spawn_GhostParticules(ghost_t* ghost)
{
	
	FX_GetExplosion(ghost->ss_position, IMPACT_TYPE_BLUE, 0.65f,0.0f);
}


void Spawn_EntityParticules(vec2_t ss_position)
{
	vec2_t random ;
	
	random[X] = (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1);
	random[Y] = (rand() - RAND_MAX ) / (float)RAND_MAX;
	normalize2(random);
	FX_GetParticule(ss_position,random		,0.012*PARTICULE_SIZE_GLOBAL*ENTITY_EX_PARTICULE_RATIO	,PARTICULE_TRAVEL_DIST,PARTICULE_TYPE_EXPLOSION, PARTICULE_COLOR_YELLOW,PARTICULE_DEFAULT_STRECH-1);
	random[X]+=0.1f;
	random[Y]-=0.1f;
	normalize2(random);
	FX_GetParticule(ss_position,random 	,0.011*PARTICULE_SIZE_GLOBAL*ENTITY_EX_PARTICULE_RATIO	,PARTICULE_TRAVEL_DIST+PARTICULE_TRAVEL_DIST*15/100,PARTICULE_TYPE_EXPLOSION, PARTICULE_COLOR_YELLOW,PARTICULE_DEFAULT_STRECH);
	
	
	
	random[X] = (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1);
	random[Y] = (rand() - RAND_MAX ) / (float)RAND_MAX;
	normalize2(random);
	FX_GetParticule(ss_position,random 	,0.014*PARTICULE_SIZE_GLOBAL*ENTITY_EX_PARTICULE_RATIO	,PARTICULE_TRAVEL_DIST,PARTICULE_TYPE_EXPLOSION, PARTICULE_COLOR_YELLOW,PARTICULE_DEFAULT_STRECH-1);
	
	random[X] = (rand() - (RAND_MAX >> 1)) / (float)(RAND_MAX >> 1);
	random[Y] = -(rand() - RAND_MAX ) / (float)RAND_MAX;
	normalize2(random);
	FX_GetParticule(ss_position, random	,0.012*PARTICULE_SIZE_GLOBAL*ENTITY_EX_PARTICULE_RATIO	,PARTICULE_TRAVEL_DIST,PARTICULE_TYPE_EXPLOSION, PARTICULE_COLOR_YELLOW,PARTICULE_DEFAULT_STRECH);
	random[X]+=0.1f;
	random[Y]-=0.1f;
	normalize2(random);
	FX_GetParticule(ss_position, random	,0.015*PARTICULE_SIZE_GLOBAL*ENTITY_EX_PARTICULE_RATIO	,PARTICULE_TRAVEL_DIST+PARTICULE_TRAVEL_DIST*15/100,PARTICULE_TYPE_EXPLOSION, PARTICULE_COLOR_YELLOW,PARTICULE_DEFAULT_STRECH-1);
}

void COLL_CheckPlayers(void)
{
	int j;
	
	// User in invulnerable
	if (players[controlledPlayer].invulnerableFor > 0)
		return;
		
	for (j=0; j < partLib.numParticules; j++) 
	{
		if (players[controlledPlayer].ss_boudaries[DOWN]  >  partLib.particules[j].ss_boudaries[UP]    ||
			players[controlledPlayer].ss_boudaries[UP]    <  partLib.particules[j].ss_boudaries[DOWN]  ||
			players[controlledPlayer].ss_boudaries[LEFT]  >  partLib.particules[j].ss_boudaries[RIGHT] ||
			players[controlledPlayer].ss_boudaries[RIGHT] <  partLib.particules[j].ss_boudaries[LEFT]
		)
			continue;
		
		//We have a collision here
		
		P_Die(controlledPlayer);
		partLib.particules[j].ttl = 0;
	}
	
}

void COLL_CheckEnemies(void)
{
	enemy_t* enemy;
	player_t* player;
	bullet_t* bullets;
	ghost_t* ghost;
	const short* ss_bullet_boudaries;
	const short* ss_enemy_boudaries;
	int i,j;
	ushort tmpEnergy;
	
	enemy = ENE_GetFirstEnemy();
	
	while (enemy != NULL) 
	{
		ss_enemy_boudaries = enemy->ss_boudaries;
		
		
		// Enenemy VS Player's bullet
		for(i=0 ; i < numPlayers ; i++)
		{
			player = &players[i];
			bullets = player->bullets;
			
			for (j=0; j< MAX_PLAYER_BULLETS; j++) 
			{
				if (bullets[j].expirationTime < simulationTime)
					continue;
				
				ss_bullet_boudaries= bullets[j].ss_boudaries;
				
				if (ss_enemy_boudaries[DOWN]  >  ss_bullet_boudaries[UP]    ||
					ss_enemy_boudaries[UP]    <  ss_bullet_boudaries[DOWN]  ||
					ss_enemy_boudaries[LEFT]  >  ss_bullet_boudaries[RIGHT] ||
					ss_enemy_boudaries[RIGHT] <  ss_bullet_boudaries[LEFT]
					)
					continue;
					
				engine.playerStats.bulletsHit[i]++;
				
				//We have a collision here
				enemy->shouldFlicker = 1;
				
				tmpEnergy = bullets[j].energy ;
				bullets[j].energy -= MAX(enemy->energy,0);
				enemy->energy -= MAX(tmpEnergy,0);		
				
				if (bullets[j].energy <= 0)
				{

					bullets[j].expirationTime = simulationTime;
					Spawn_BulletParticules(&bullets[j],i);
				}
				
				if (enemy->energy <= 0)
				{
					engine.playerStats.enemyDestroyed[i]++;
					players[i].score += enemy->score ;
				}
								
			}
			
		}
		
	
		
		
		//Enemy Vs Player's GHOST
		for(i=0 ; i < numPlayers ; i++)
		{
			player = &players[i];
			
			for (j=0; j < GHOSTS_NUM; j++) 
			{
				ghost = &player->ghosts[j];
				
				if (ghost->timeCounter >= GHOST_TTL_MS || ghost->energy <= 0)
					continue;
				
				
				
				if (ss_enemy_boudaries[DOWN]  >  ghost->short_ss_position[Y] ||
					ss_enemy_boudaries[UP]    <  ghost->short_ss_position[Y] ||
					ss_enemy_boudaries[LEFT]  >  ghost->short_ss_position[X] ||
					ss_enemy_boudaries[RIGHT] <  ghost->short_ss_position[X]
					)
					continue;
				
				//We have a collision here
				
				engine.playerStats.bulletsHit[i]++;
				
				
				tmpEnergy = ghost->energy ;
				ghost->energy -= MAX(enemy->energy,0);
				enemy->energy -= MAX(tmpEnergy,0);		
				
				enemy->shouldFlicker = 1;
				
				if (ghost->energy <= 0)
				{
					//ghost->timeCounter = GHOST_TTL_MS;
					Spawn_GhostParticules(ghost);
				}

				if (enemy->energy <= 0)
				{
					engine.playerStats.enemyDestroyed[i]++;
					players[i].score += enemy->score ;
				}
			}
		}
		
		//if (enemy->shouldFlicker = 1)
		//	printf("Enemy enery = %d.\n",enemy->energy );

		if (enemy->energy <= 0)
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
		}
		
		enemy = enemy->next;
	}
	
	
	
	//Now Check local player's collisions
	
	
	// User in invulnerable
	if (players[controlledPlayer].invulnerableFor > 0)
		return;
	
	
	enemy = ENE_GetFirstEnemy();
	while (enemy != NULL) 
	{
		
		if (enemy->ss_boudaries[DOWN]  >  players[controlledPlayer].ss_boudaries[UP] ||
			enemy->ss_boudaries[UP]    <  players[controlledPlayer].ss_boudaries[DOWN] ||
			enemy->ss_boudaries[LEFT]  >  players[controlledPlayer].ss_boudaries[RIGHT] ||
			enemy->ss_boudaries[RIGHT] <  players[controlledPlayer].ss_boudaries[LEFT]
			)
		{
			enemy = enemy->next;
			continue;
		}
		else 
		{
			P_Die(controlledPlayer);
			break;
		}
	}
}		



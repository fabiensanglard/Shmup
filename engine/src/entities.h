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
    along with SHMUP.  If not, see <http://www.gnu.org/licenses/>.
*/    
/*
 *  entities.h
 *  dEngine
 *
 *  Created by fabien sanglard on 11/09/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */


#ifndef ED_ENTITIES
#define ED_ENTITIES


#include "globals.h"
#include "math.h"
#include "matrix.h"
#include "material.h"
#include "md5.h"
#include "collisions.h"


typedef enum { UP, DOWN, LEFT, RIGHT} ScreenSpaceBoundaries_e;

typedef struct entity_t {

	md5_mesh_t* model;
	
	material_t* material;
	
	//object space to world space
	matrix_t matrix;
	//world space to object space
	matrix_t cachePVMShadow;
	
	ushort uid;
	
	// Euler angles
	float xAxisRot;
	float yAxisRot;
	float zAxisRot;
	
	ushort numIndices;
	ushort* indices;
	
	float color[4];
	
	bbox_t worldSpacebbox;
	
	uchar usage;
	
	uchar mouvementPatternType;
	
} entity_t;

void ENT_GenerateWorldSpaceBBox(entity_t* entity);


#define ENT_FULL_DRAW 0
#define ENT_PARTIAL_DRAW 1

char ENT_LoadEntity(entity_t* entity, const char* filename, uchar usage);
void ENT_InitCacheSystem(void);
void ENT_DumpEntityCache(void);
void ENT_ClearModelsLibrary(void);

//Variable used to make entities stick to screen
extern float distanceZFromCamera; 
extern float widthAtDistance;
extern float heightAtDistance;
extern matrix_t cameraInvRot;

#endif

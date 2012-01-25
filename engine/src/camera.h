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
 *  camera.h
 *  dEngine
 *
 *  Created by fabien sanglard on 15/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_CAMERA
#define DE_CAMERA

#include "globals.h"
#include "math.h"
#include "quaternion.h"

typedef struct entity_visset_t
{
	ushort entityId;
	
	//Used for keyframes, full indices update
	ushort numIndices;
	ushort* indices;
	
	//Used for delta update
	ushort numFacesToRemove;
	ushort* facesToRemove;
	ushort numFacesToAdd;
	ushort* facesToAdd;
	
} entity_visset_t;

typedef struct world_vis_set_update_t
{
	char isKey;
	
	ushort numVisSets;
	entity_visset_t* visSets;
	
} world_vis_set_update_t;

typedef struct camera_frame_t
{
	uint time;
	vec3_t position;
	quat4_t orientation;
	
	world_vis_set_update_t visUpdate;
	
	struct camera_frame_t* next;
} camera_frame_t;

typedef struct 
{
	vec3_t position ;
	
	vec3_t forward ;
	vec3_t right ;
	vec3_t up;
		
	float aspect;
	float fov;
	float zNear;
	float zFar;
	
	uchar playing;
	char pathFilename[256];
	
	camera_frame_t* path;
	camera_frame_t* currentFrame;
	
} camera_t;





extern camera_t camera;


void CAM_Update(void);
void CAM_InitUnitCube(void);
void CAM_LoadPath(void);
void CAM_StartPlaying(void);
void CAM_ClearAllRemainingCameraVS(void);
#endif
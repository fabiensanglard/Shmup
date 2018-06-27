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
 *  preproc.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-02-16.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_PREPROC
#define DE_PREPROC


#include "math.h"
#include "quaternion.h"

//#define PREPROC_INTRO
//#define TIME_STEP 33
//#define TIME_STEP 16

#define KEY_FRAME_INTERVAL_MS 1000

#ifdef PREPROC_INTRO
	#define MAX_POLY_VIS_PER_FRAME 4000
#else
	#define MAX_POLY_VIS_PER_FRAME 2500
#endif


#define PREC_FACE_MAX_VERTICES 14
typedef struct prec_face_t
{
	ushort objectId;
	ushort faceId;
	
	//Model space vertices
	vec4_t ms_vertices[3];
	
	//Homogenous space vertices
	uchar	hs_numVertices;
	vec4_t	hs_vertices[PREC_FACE_MAX_VERTICES];
	
	//Screen space vertices
	uchar	ss_numVertices;
	vec2_t	ss_vertices[PREC_FACE_MAX_VERTICES];
	
	float area;
	
} prec_face_t;


typedef struct faceId_t
{
	ushort objectId;
	ushort faceId;
} faceId_t;


typedef struct entityAddRemoveStats_t
{
	ushort numFacestoRemove;
	ushort numFacesToAdd;
	
	ushort indexInVisUpdate;
} entityAddRemoveStats_t ;

typedef struct rawVisFacesSet_t
{
	prec_face_t* visFaces;//[MAX_POLY_VIS_PER_FRAME+1];
	ushort numVisFaces;
	
	char isKey;
	
} rawVisFacesSet_t;


typedef struct prec_camera_frame_t
{
	uint time;
	vec3_t position;
	quat4_t orientation;
	
	rawVisFacesSet_t visSet;
	
	struct prec_camera_frame_t* next;
	
} prec_camera_frame_t;



typedef struct entity_sort_t
{
	float dist ;
	ushort indexId;
	
} entity_sort_t ;


void PREPROC_ConvertCp1Tocp2b(char* cpFilename, char* cp2bFilename, char* logfilename);

#endif
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
 *  md5.h
 *  dEngine
 *
 *  Created by fabien sanglard on 09-11-02.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_MD5
#define DE_MD5

#include "globals.h"
#include "math.h"
#include "quaternion.h"
#include "material.h"
#include "math.h"

typedef struct md5_joint_t
{
	char name[64];
	int parent;
	vec3_t position;
	quat4_t orientation;
	
} md5_bone_t;

typedef struct md5_vertex_t
{
	vec2short_t st;
	
	int start; /* start weight */
	int count; /* weight count */
	
} md5_vertex_t;

typedef struct md5_triangle_t
{
	ushort index[3];
} md5_triangle_t; 

typedef struct md5_weight_t
{
	int boneId;
	float bias;
	
	//Cache to make animation faster
	vec3_t modelSpacePos;
	vec3_t modelSpaceNormal;
	vec3_t modelSpaceTangent;
	
	vec3_t		boneSpacePos;
	vec3short_t boneSpaceNormal;
	vec3short_t boneSpaceTangent;


} md5_weight_t;

typedef struct md5_bbox_t
{
	vec3_t min;
	vec3_t max;
	
} md5_bbox_t;

#define MD5_MEMLOC_DISK 0
#define MD5_MEMLOC_RAM 1
#define MD5_MEMLOC_VRAM 2

typedef struct md5_mesh_t
{
	//Store the default pose.
	ushort numBones;
	md5_bone_t* bones;
	
	int numVertices;
	md5_vertex_t *vertices;
	
	int numTriangles;
	md5_triangle_t *triangles;
	
	int numWeights;
	md5_weight_t *weights;
		
	
	vertex_t* vertexArray ;
	ushort* indices ;
	ushort numIndices;	
	
	char* materialName;
	
	md5_bbox_t modelSpacebbox;
	
	uchar memLocation;
	uint vboId;
	
	uchar indicesMemoryLoc;
	uint indicesVboId;
	
	uchar memStatic;
	
} md5_mesh_t;



char MD5_LoadMesh(md5_mesh_t* mesh, const char* filename);
void MD5_FreeMesh(md5_mesh_t* mesh);
#endif




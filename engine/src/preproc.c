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
 *  preproc.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-02-16.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "preproc.h"
#include "filesystem.h"
#include "lexer.h"
#include "collisions.h"
#include "entities.h"
#include "md5.h"
#include "renderer.h"
#include "camera.h"
#include "world.h"
#include "timer.h"

FILE* logFile=0;

#ifndef min
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif


void PREPROC_ClipPolygon(prec_face_t* face)
{
	#define vector4Subtract( a, b, c )	( (c)[ 0 ] = (a)[ 0 ] - (b)[ 0 ], (c)[ 1 ] = (a)[ 1 ] - (b)[ 1 ], (c)[ 2 ] = (a)[ 2 ] - (b)[ 2 ], (c)[ 3 ] = (a)[ 3 ] - (b)[ 3 ] )
	#define vector4Add( a, b, c )		( (c)[ 0 ] = (a)[ 0 ] + (b)[ 0 ], (c)[ 1 ] = (a)[ 1 ] + (b)[ 1 ], (c)[ 2 ] = (a)[ 2 ] + (b)[ 2 ], (c)[ 3 ] = (a)[ 3 ] + (b)[ 3 ] )
	#define vector4Copy( a, b )			( (b)[ 0 ] = (a)[ 0 ], (b)[ 1 ] = (a)[ 1 ], (b)[ 2 ] = (a)[ 2 ], (b)[ 3 ] = (a)[ 3 ] )
	#define	vector4Scale( v, s, o )		( (o)[ 0 ] = (v)[ 0 ] * (s),(o)[ 1 ] = (v)[ 1 ] * (s), (o)[ 2 ] = (v)[ 2 ] * (s), (o)[ 3 ] = (v)[ 3 ] * (s) )

	int i;
	
	vec4_t* currentVertice;
	vec4_t* previousVertice;
	
	
	//Just to stop the while loop
	vec4_t* lastVertice;
	
	uchar in_numVertices=0;
	vec4_t in_vertices[PREC_FACE_MAX_VERTICES];
	
	char previousDot;
	char currentDot;
	
	float intersectionFactor;
	vec4_t intersectionPoint;
	
	// PREC_FACE_MAX_VERTICES
	
	// We have all 3 vertices in w!=1.
	// Clipping order:
	
	// If a vertice is in w<0, clip associated edge with w=0
	// top		clipping
	// bottom	clipping
	// left		clipping
	// right	clipping
	// near		clipping
	// far		clipping
	
	//Clip against w=0
	
	previousVertice = &face->hs_vertices[face->hs_numVertices-1];
	previousDot = ((*previousVertice)[W] < 0) ? -1 : 1;
	lastVertice = &face->hs_vertices[face->hs_numVertices-1];
	currentVertice = &face->hs_vertices[0];
	while (currentVertice != (lastVertice+1) ) 
	{
		currentDot = ((*currentVertice)[W] < 0) ? -1 : 1;
		
		if (previousDot * currentDot < 0)
		{
			//Need to clip against plan w=0
			
			intersectionFactor = (*previousVertice)[W] / ((*previousVertice)[W]- (*currentVertice)[W]);
			
			// I = Qp + f(Qc-Qp))
			vector4Copy(*currentVertice,intersectionPoint);								//          Qc
			vector4Subtract(intersectionPoint, *previousVertice  ,intersectionPoint);	//         (Qc-Qp)
			vector4Scale(intersectionPoint,intersectionFactor,intersectionPoint);		//        f(Qc-Qp))
			vector4Add(intersectionPoint,*previousVertice,intersectionPoint);			//Qp	+ f(Qc-Qp))
			
			// Insert
			vector4Copy(intersectionPoint,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		if (currentDot > 0)
		{
			//Insert
			vector4Copy(*currentVertice,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		previousDot = currentDot;
		
		//Move forward
		previousVertice = currentVertice;
		currentVertice++;
	}
	//Copy the output(in_vertices) into the source (face)
	face->hs_numVertices = in_numVertices;
	for(i=0 ; i < in_numVertices ; i++)
	{
		vector4Copy(in_vertices[i],face->hs_vertices[i]);
	}
	in_numVertices = 0;
	
	
	// Now it is rince repeat for the 6 clipping plans
	
	
	
	
	
	
	if (face->hs_numVertices < 3)
	{
		//printf("W clipping discared entire face.\n");
		return;
	}
	
	//Clip against TOP
	
	previousVertice = &face->hs_vertices[face->hs_numVertices-1];
	previousDot = ((*previousVertice)[Y] <= (*previousVertice)[W]) ? 1 : -1;
	lastVertice = &face->hs_vertices[face->hs_numVertices-1];
	currentVertice = &face->hs_vertices[0];
	while (currentVertice != (lastVertice+1) ) 
	{
		currentDot = ((*currentVertice)[Y] <= (*currentVertice)[W]) ? 1 : -1;
		
		if (previousDot * currentDot < 0)
		{
			//Need to clip against plan w=0
			
			intersectionFactor = 
			((*previousVertice)[W] - (*previousVertice)[Y]) / 
			(((*previousVertice)[W] - (*previousVertice)[Y]) - ((*currentVertice)[W] - (*currentVertice)[Y]) );
			
			vector4Copy(*currentVertice,intersectionPoint);
			vector4Subtract(intersectionPoint, *previousVertice  ,intersectionPoint);
			vector4Scale(intersectionPoint,intersectionFactor,intersectionPoint);
			vector4Add(intersectionPoint,*previousVertice,intersectionPoint);
			
			// Insert
			vector4Copy(intersectionPoint,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		if (currentDot > 0)
		{
			//Insert
			vector4Copy(*currentVertice,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		previousDot = currentDot;
		
		//Move forward
		previousVertice = currentVertice;
		currentVertice++;
	}
	//Copy the output(in_vertices) into the source (face)
	face->hs_numVertices = in_numVertices;
	for(i=0 ; i < in_numVertices ; i++)
	{
		vector4Copy(in_vertices[i],face->hs_vertices[i]);
	}
	in_numVertices = 0;
	
	
	if (face->hs_numVertices < 3)
	{
		//printf("TOP clipping discared entire face.\n");
		return;
	}
	
	//Clip against DOWN
	
	
	previousVertice = &face->hs_vertices[face->hs_numVertices-1];
	previousDot = (-(*previousVertice)[Y] <= (*previousVertice)[W]) ? 1 : -1;
	lastVertice = &face->hs_vertices[face->hs_numVertices-1];
	currentVertice = &face->hs_vertices[0];
	while (currentVertice != (lastVertice+1) ) 
	{
		currentDot = (-(*currentVertice)[Y] <= (*currentVertice)[W]) ? 1 : -1;
		
		if (previousDot * currentDot < 0)
		{
			//Need to clip against plan w=0
			
			intersectionFactor = 
			( (*previousVertice)[W] + (*previousVertice)[Y]) / 
			( ( (*previousVertice)[W] + (*previousVertice)[Y]) - ((*currentVertice)[W] + (*currentVertice)[Y]) );
			
			vector4Copy(*currentVertice,intersectionPoint);
			vector4Subtract(intersectionPoint, *previousVertice  ,intersectionPoint);
			vector4Scale(intersectionPoint,intersectionFactor,intersectionPoint);
			vector4Add(intersectionPoint,*previousVertice,intersectionPoint);
			
			// Insert
			vector4Copy(intersectionPoint,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		if (currentDot > 0)
		{
			//Insert
			vector4Copy(*currentVertice,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		previousDot = currentDot;
		
		//Move forward
		previousVertice = currentVertice;
		currentVertice++;
	}
	//Copy the output(in_vertices) into the source (face)
	face->hs_numVertices = in_numVertices;
	for(i=0 ; i < in_numVertices ; i++)
	{
		vector4Copy(in_vertices[i],face->hs_vertices[i]);
	}
	in_numVertices = 0;
	
	
	if (face->hs_numVertices < 3)
	{
		//printf("DOWN clipping discared entire face.\n");
		return;
	}
	
	// LEFT		clipping
	
	
	previousVertice = &face->hs_vertices[face->hs_numVertices-1];
	previousDot = (- (*previousVertice)[X] <= (*previousVertice)[W]) ? 1 : -1;
	lastVertice = &face->hs_vertices[face->hs_numVertices-1];
	currentVertice = &face->hs_vertices[0];
	while (currentVertice != (lastVertice+1) ) 
	{
		currentDot = (- (*currentVertice)[X] <= (*currentVertice)[W]) ? 1 : -1;
		
		if (previousDot * currentDot < 0)
		{
			//Need to clip against plan w=0
			
			intersectionFactor = 
			( (*previousVertice)[W] + (*previousVertice)[X]) / 
			(( (*previousVertice)[W] + (*previousVertice)[X]) - ( (*currentVertice)[W] + (*currentVertice)[X]) );
			
			vector4Copy(*currentVertice,intersectionPoint);
			vector4Subtract(intersectionPoint, *previousVertice  , intersectionPoint);
			vector4Scale(intersectionPoint,intersectionFactor,intersectionPoint);
			vector4Add(intersectionPoint,*previousVertice,intersectionPoint);
			
			// Insert
			vector4Copy(intersectionPoint,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		if (currentDot > 0)
		{
			//Insert
			vector4Copy(*currentVertice,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		previousDot = currentDot;
		
		//Move forward
		previousVertice = currentVertice;
		currentVertice++;
	}
	//Copy the output(in_vertices) into the source (face)
	face->hs_numVertices = in_numVertices;
	for(i=0 ; i < in_numVertices ; i++)
	{
		vector4Copy(in_vertices[i],face->hs_vertices[i]);
	}
	in_numVertices = 0;
	
	
	if (face->hs_numVertices < 3)
	{
		//printf("LEFT clipping discared entire face.\n");
		return;
	}
	
	// RIGHT	clipping
	
	
	previousVertice = &face->hs_vertices[face->hs_numVertices-1];
	previousDot = ( (*previousVertice)[X] <= (*previousVertice)[W]) ? 1 : -1;
	lastVertice = &face->hs_vertices[face->hs_numVertices-1];
	currentVertice = &face->hs_vertices[0];
	while (currentVertice != (lastVertice+1) ) 
	{
		currentDot = ( (*currentVertice)[X] <= (*currentVertice)[W]) ? 1 : -1;
		
		if (previousDot * currentDot < 0)
		{
			//Need to clip against plan w=0
			
			intersectionFactor = 
			(  (*previousVertice)[W] - (*previousVertice)[X]) / 
			(( (*previousVertice)[W] - (*previousVertice)[X]) - ( (*currentVertice)[W] - (*currentVertice)[X]) );
			
			vector4Copy(*currentVertice,intersectionPoint);
			vector4Subtract(intersectionPoint, *previousVertice  ,intersectionPoint);
			vector4Scale(intersectionPoint,intersectionFactor,intersectionPoint);
			vector4Add(intersectionPoint,*previousVertice,intersectionPoint);
			
			// Insert
			vector4Copy(intersectionPoint,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		if (currentDot > 0)
		{
			//Insert
			vector4Copy(*currentVertice,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		previousDot = currentDot;
		
		//Move forward
		previousVertice = currentVertice;
		currentVertice++;
	}
	//Copy the output(in_vertices) into the source (face)
	face->hs_numVertices = in_numVertices;
	for(i=0 ; i < in_numVertices ; i++)
	{
		vector4Copy(in_vertices[i],face->hs_vertices[i]);
	}
	in_numVertices = 0;
	
	
	if (face->hs_numVertices < 3)
	{
		//printf("RIGHT clipping discared entire face.\n");
		return;
	}
	
	// NEAR		clipping
	
	
	previousVertice = &face->hs_vertices[face->hs_numVertices-1];
	previousDot = (- (*previousVertice)[Z] <= (*previousVertice)[W]) ? 1 : -1;
	lastVertice = &face->hs_vertices[face->hs_numVertices-1];
	currentVertice = &face->hs_vertices[0];
	while (currentVertice != (lastVertice+1) ) 
	{
		currentDot = (- (*currentVertice)[Z] <= (*currentVertice)[W]) ? 1 : -1;
		
		if (previousDot * currentDot < 0)
		{
			//Need to clip against plan w=0
			
			intersectionFactor = 
			( (*previousVertice)[W] + *(previousVertice)[Z]) / 
			(((*previousVertice)[W] + *(previousVertice)[Z]) - ((*currentVertice)[W] + (*currentVertice)[Z]) );
			
			vector4Copy(*currentVertice,intersectionPoint);
			vector4Subtract(intersectionPoint, *previousVertice  ,intersectionPoint);
			vector4Scale(intersectionPoint,intersectionFactor,intersectionPoint);
			vector4Add(intersectionPoint,*previousVertice,intersectionPoint);
			
			// Insert
			vector4Copy(intersectionPoint,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		if (currentDot > 0)
		{
			//Insert
			vector4Copy(*currentVertice,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		previousDot = currentDot;
		
		//Move forward
		previousVertice = currentVertice;
		currentVertice++;
	}
	//Copy the output(in_vertices) into the source (face)
	face->hs_numVertices = in_numVertices;
	for(i=0 ; i < in_numVertices ; i++)
	{
		vector4Copy(in_vertices[i],face->hs_vertices[i]);
	}
	in_numVertices = 0;
	
	if (face->hs_numVertices < 3)
	{
		//printf("NEAR clipping discared entire face.\n");
		return;
	}
	
	// FAR		clipping
	
	
	previousVertice = &face->hs_vertices[face->hs_numVertices-1];
	previousDot = ( (*previousVertice)[Z] <= (*previousVertice)[W]) ? 1 : -1;
	lastVertice = &face->hs_vertices[face->hs_numVertices-1];
	currentVertice = &face->hs_vertices[0];
	while (currentVertice != (lastVertice+1) ) 
	{
		currentDot = ( (*currentVertice)[Z] <= (*currentVertice)[W]) ? 1 : -1;
		
		if (previousDot * currentDot < 0)
		{
			//Need to clip against plan w=0
			
			intersectionFactor = 
			( (*previousVertice)[W] - (*previousVertice)[Z]) / 
			(((*previousVertice)[W] - (*previousVertice)[Z]) - ( (*currentVertice)[W] - (*currentVertice)[Z]) );
			
			vector4Copy(*currentVertice,intersectionPoint);
			vector4Subtract(intersectionPoint, *previousVertice  ,intersectionPoint);
			vector4Scale(intersectionPoint,intersectionFactor,intersectionPoint);
			vector4Add(intersectionPoint,*previousVertice,intersectionPoint);
			
			// Insert
			vector4Copy(intersectionPoint,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		if (currentDot > 0)
		{
			//Insert
			vector4Copy(*currentVertice,in_vertices[in_numVertices]);
			in_numVertices++;
		}
		
		previousDot = currentDot;
		
		//Move forward
		previousVertice = currentVertice;
		currentVertice++;
	}
	//Copy the output(in_vertices) into the source (face)
	face->hs_numVertices = in_numVertices;
	for(i=0 ; i < in_numVertices ; i++)
	{
		vector4Copy(in_vertices[i],face->hs_vertices[i]);
	}
	in_numVertices = 0;
	
	if (face->hs_numVertices < 3)
	{
		//printf("FAR clipping discared entire face.\n");
		return;
	}
	
	//Done.
}


prec_camera_frame_t* PREPROC_ReadFrameFromFile(void)
{
	prec_camera_frame_t* frame ;
	matrix3x3_t rotMatrix;
	vec3_t lookAt;
	vec3_t Zaxis;
	vec3_t Yaxis;
	vec3_t Xaxis;
	int i;
	
	frame = calloc(1,sizeof(prec_camera_frame_t));
	
	
	//printf("PREPROC_ReadFrameFromFile");
	
	LE_readToken(); //time
	frame->time = LE_readReal();
	
	LE_readToken(); //position
	frame->position[0] = LE_readReal();
	frame->position[1] = LE_readReal();
	frame->position[2] = LE_readReal();
	
	LE_readToken(); //q or m
	if (!strcmp("q", LE_getCurrentToken()))
	{
		for( i=0; i < 4 ; i++)
			frame->orientation[i]= LE_readReal();
	}
	else if (!strcmp("lookat", LE_getCurrentToken()))
	{
		lookAt[X] = LE_readReal();
		lookAt[Y] = LE_readReal();
		lookAt[Z] = LE_readReal();
		
		LE_readToken(); // upVector
		
		Yaxis[X] = LE_readReal();
		Yaxis[Y] = LE_readReal();
		Yaxis[Z] = LE_readReal();
		
		//c = a -b 
		vectorSubtract(frame->position,lookAt,Zaxis);
		normalize(Zaxis);
		normalize(Yaxis);
		
		vectorCrossProduct(Yaxis, Zaxis, Xaxis);
		//vectorCrossProduct(Zaxis, Xaxis, Yaxis);
		
		rotMatrix[0] = Xaxis[0]; rotMatrix[3] = Yaxis[0];	rotMatrix[6] = Zaxis[0] ; 
		rotMatrix[1] = Xaxis[1]; rotMatrix[4] = Yaxis[1];	rotMatrix[7] = Zaxis[1] ;  
		rotMatrix[2] = Xaxis[2]; rotMatrix[5] = Yaxis[2];	rotMatrix[8] = Zaxis[2] ; 
		
		Quat_CreateFromMat3x3(rotMatrix,frame->orientation);
	}	
	
	return frame;
}

void PREPROC_InterpolateFrames(prec_camera_frame_t* currentFrame, prec_camera_frame_t* nextFrame, float interpolationFactor, vec3_t position, quat4_t orientation)
{

	//Interpolate position
	//
#ifdef PREPROC_INTRO
	//TOREMOVE Camera mouvement is interpolated cos style for intro only
	vectorCosinuInterpolate(currentFrame->position,nextFrame->position,interpolationFactor,position);
#else
	vectorLinearInterpolate(currentFrame->position,nextFrame->position,interpolationFactor,position);
#endif
	//
	
	
	//Interpolate quaternion
	Quat_slerp(currentFrame->orientation, nextFrame->orientation, interpolationFactor, orientation);

	
}

void PREPROC_ExpandCameraWayPoints(prec_camera_frame_t* startFrame,prec_camera_frame_t* endFrame)
{
	prec_camera_frame_t*	newFrame;
	prec_camera_frame_t*	currentFrame;
	int						timeDifference;
	float					interpolationFactor;
	int						timeStep=0;
	float					extraAccuracyTimeStep =0;
	
	timeDifference = endFrame->time - startFrame->time;
	
	currentFrame = startFrame;
	
	while (currentFrame->time +17 < endFrame->time) 
	{
		newFrame = (prec_camera_frame_t*)calloc(1, sizeof(prec_camera_frame_t));
		newFrame->time = currentFrame->time + timeStep ;
		newFrame->next = endFrame;
		currentFrame->next = newFrame;
		
		
		
		//Interpolate position and oritentation for the given time.
		
		interpolationFactor = (newFrame->time - startFrame->time) / (float)timeDifference;
		PREPROC_InterpolateFrames(startFrame,endFrame,interpolationFactor,newFrame->position,newFrame->orientation);
		
		printf("Expanded frame t=%d\n",newFrame->time);
		
		currentFrame = newFrame;
		extraAccuracyTimeStep += 0.6666667f;
		timeStep = 16 + (int)extraAccuracyTimeStep;
		extraAccuracyTimeStep -= (int)extraAccuracyTimeStep;
	}
}


void PREPROC_SwitchFaces(prec_face_t* element1, prec_face_t* element2)
{
	prec_face_t tmp;
	
	tmp = *element1;
	*element1 = *element2;
	*element2 = tmp;
}

void PREPROC_InsertFaceIntoVisSet(prec_face_t* face,prec_face_t* visFaces,ushort*  numVisFaces )
{
	ushort i;
	
	visFaces[*numVisFaces] = *face;
	
	i = *numVisFaces;
	while (i > 0 && visFaces[i].area> visFaces[i-1].area) 
	{
		PREPROC_SwitchFaces(&visFaces[i],&visFaces[i-1]);
		i--;
	}
	
	(*numVisFaces)++;
	
	*numVisFaces = min(MAX_POLY_VIS_PER_FRAME,*numVisFaces);
	
	
}

int compareEntity_sort_t (const void * a, const void * b)
{
	return ( (*(entity_sort_t*)a).dist - (*(entity_sort_t*)b).dist );
}


void PREPROC_ComputeSignedArea(prec_face_t* face)
{
	int i;
	//Need to iterate on the ss_vertices with ss_numVertices
	
	face->area = 0;
	
	//In order to avoid modulo, copy first vertex after the last one.
	if (face->ss_numVertices >= PREC_FACE_MAX_VERTICES)
	{
		printf("CAM_ComputeSignedArea too many vertices.\n");
		exit(0);
	}
	vector2Copy(face->ss_vertices[0],face->ss_vertices[face->ss_numVertices]);
	
	
	
	for(i=0; i < face->ss_numVertices ; i++)
	{
		face->area += 
		(face->ss_vertices[i]  [X]  * face->ss_vertices[i+1][Y]) - 
		(face->ss_vertices[i+1][X]  * face->ss_vertices[i]  [Y]) ;
		
	}
	
	face->area *= 0.5;
}

void PREPROC_PopulateRawFaceSet(prec_camera_frame_t* frame)
{
	prec_face_t		face;
	
	matrix3x3_t		quatMatrix;
	vec3_t			up;
	vec3_t			forward;
	vec3_t			vLookat;
	
	frustrum_t		frustrum;
	matrix_t		prec_ViewMatrix;
	matrix_t		prec_ProjectionMatrix;
	matrix_t		pv;
	matrix_t		pvm;
	
	entity_t*		entity;
	md5_mesh_t*		mesh;
	int				i,j,k;
	
	entity_sort_t* sortedIndex;
	
	frame->visSet.numVisFaces = 0;
	memset(frame->visSet.visFaces, 0, (MAX_POLY_VIS_PER_FRAME+1)*sizeof(prec_face_t));
	
	Quat_ConvertToMat3x3(quatMatrix, frame->orientation);
	
	if (logFile)
		fprintf(logFile, "PREPROC_PopulateRawFaceSet: processing frame t=%d.\n",frame->time);
	
	printf( "PREPROC_PopulateRawFaceSet: processing frame t=%d.\n",frame->time);
	
	up[0] = quatMatrix[3];
	up[1] = quatMatrix[4];
	up[2] = quatMatrix[5];
	forward[0] = -quatMatrix[6];
	forward[1] = -quatMatrix[7];
	forward[2] = -quatMatrix[8];	
	
	// Get mv matrix
	vectorClear(vLookat);
	vectorAdd(frame->position,forward,vLookat);
	gluLookAt(frame->position, vLookat, up, prec_ViewMatrix);
	
	// Get p matrix	using a wider fov this way polygons are marked visibles juuuust before they get on screen
	gluPerspective(camera.fov+4,camera.aspect,camera.zNear,camera.zFar,prec_ProjectionMatrix);
	
	matrix_multiply(prec_ProjectionMatrix,prec_ViewMatrix,pv);
	
	//Generate world space frustrum volume
	COLL_GenerateFrustrum(pv,frustrum);
	
	
	
	//printf("Frame t=%d:\n",frame->time);
	
//	printf("Matrix orientation:\n");
//	matrix_print3x3(quatMatrix);
	
	//printf("Matrix pv.\n");	matrix_print(pv);
	
	// Sorting entities from near to far to increase early face area rejection rate
	sortedIndex = malloc(num_map_entities * sizeof(entity_sort_t)) ;
	for(i=0 ; i < num_map_entities ; i++)
	{
		entity = &map[i];
		sortedIndex[i].indexId = i;
		
		//No sqrt because we only want the smaller value, it the value is squared it doesn't matter.
		sortedIndex[i].dist =  powf(frame->position[X] - entity->matrix[12] ,2) + powf(frame->position[Y] - entity->matrix[13],2) + powf(frame->position[Z] - entity->matrix[14],2)  ;
	}
	
	qsort(sortedIndex, num_map_entities, sizeof(entity_sort_t), compareEntity_sort_t);
	
	
	
	if (logFile)
	{
		fprintf(logFile,"-----------------\n");
		for( i = 0 ; i < 4 ; i++)		//Column dest
		{
			for( j = 0 ; j < 4 ; j++)	//Row dest
			{
				// m1 j row * m2 i column
				fprintf(logFile," %.4f ", pv[i+j*4]);
			}
			fprintf(logFile,"\n");
		}
		fprintf(logFile,"-----------------\n");
	}
	
	for(i=0 ; i < num_map_entities ; i++)
	{
		entity = &map[sortedIndex[i].indexId];
				
		mesh = entity->model;
		
		if (logFile)fprintf(logFile, "	Processing entity : %d.\n",i);
		
		face.objectId = sortedIndex[i].indexId ;
		
		//if (i != 3)
		//	continue;
		
		
		//Test entity bbox against camera frustrum
		if (INT_OUT == COLL_CheckBoxAgainstFrustrum(entity->worldSpacebbox,frustrum) )
		{
			//printf("	Entity %hu  is not in the view frustrum : discard.\n",i);
			continue;
		}
		
		
		for ( j=0; j < mesh->numIndices/3 ; j++)
		{
			
			face.faceId =  j ;
			
			//Check camera frustrum against entity aabb
			//if (!CheckAABBAgainstFrustrum(&frustrum,entity->aabb))
			//	continue;
			
			//Concatenate all matrices
			matrix_multiply(pv,entity->matrix,pvm);
			
			
			
			// Copy face coordinates into model space vertices
			vectorCopy(	mesh->vertexArray[mesh->indices[3*j]].pos	,face.ms_vertices[0]) ;
			vectorCopy(	mesh->vertexArray[mesh->indices[3*j+1]].pos ,face.ms_vertices[1]) ;
			vectorCopy(	mesh->vertexArray[mesh->indices[3*j+2]].pos ,face.ms_vertices[2]) ;
			face.ms_vertices[0][3] = 1;
			face.ms_vertices[1][3] = 1;
			face.ms_vertices[2][3] = 1;			
			
			if (logFile)
			{
				fprintf(logFile, "	face %hd ms_vertices[0] = %f, %f. %f %f.\n",face.faceId,face.ms_vertices[0][X],face.ms_vertices[0][Y],face.ms_vertices[0][Z],face.ms_vertices[0][W]);
				fprintf(logFile, "	face %hd ms_vertices[1] = %f, %f. %f %f.\n",face.faceId,face.ms_vertices[1][X],face.ms_vertices[1][Y],face.ms_vertices[1][Z],face.ms_vertices[1][W]);
				fprintf(logFile, "	face %hd ms_vertices[2] = %f, %f. %f %f.\n",face.faceId,face.ms_vertices[2][X],face.ms_vertices[2][Y],face.ms_vertices[2][Z],face.ms_vertices[3][W]);	
			}
			
			//Project points into face
			matrix_transform_vec4t(pvm,face.ms_vertices[0],face.hs_vertices[0]);
			matrix_transform_vec4t(pvm,face.ms_vertices[1],face.hs_vertices[1]);
			matrix_transform_vec4t(pvm,face.ms_vertices[2],face.hs_vertices[2]);
			
		
			
			//We now are in homogenous space with w != 1. Clipping time.
			face.hs_numVertices = 3; // Enter as a triangles, who knows how it's going to get out ?
			PREPROC_ClipPolygon(&face);
			
			
			if (logFile) fprintf(logFile, "	face %hd after polygon clipping: has %uhd vertices.\n",face.faceId, face.hs_numVertices);
			
			//Perspective divide
			face.ss_numVertices = 0;
			for (k=0 ; k < face.hs_numVertices ; k++)
			{
				//vectorScale( v, s, o ) v= 0*s
				vectorScale(face.hs_vertices[k],1.0f/face.hs_vertices[k][3],face.ss_vertices[k]); 
				face.ss_numVertices++;
			}
			
			//We now have a polygon in 2D
			
			
			// Check is the polygon is facing the camera
			PREPROC_ComputeSignedArea(&face);
			if (face.area <= 0) //Not facing
			{
//TOREMOVE backfacing polygones are not discarded in intro
#ifdef PREPROC_INTRO
				;
#else
				if (logFile) fprintf(logFile, "	face %hd is invisible (not facing or out of screen %i) area=%f.\n",face.faceId,face.ss_numVertices,face.area);
				continue;
#endif
			}
			
			
			//Check against unit cube
			// USELESS !! Clipping already too care of this.
			/*
			 if (COLL_CheckAgainstUnitCube(&face) == INT_OUT)
			 {
			 //printf("	Entity %d, Face %hu is not in the view frustrum : discard.\n",i,face.indexId);
			 //printf("		Face %hu Size area: %f\n",j,face.area);
			 continue;
			 }
			 */
			
			if (logFile) fprintf(logFile, "	face %hd is visible.\n",face.faceId);
			
			PREPROC_InsertFaceIntoVisSet(&face,frame->visSet.visFaces, &frame->visSet.numVisFaces );
			
			}
	}
	
	
	//printf("Frame t=%d has %d faces visibles.\n",frame->time,frame->visSet.numVisFaces);
	
	free(sortedIndex);
	
	
}


void PREPROC_SaveFramesToCP2Binary(char* filename, camera_frame_t* firstFrame)
{
	FILE* fileHandle;
	char newFileName[256];
	char* magicNumber = "CP2B" ;
	int num_frames= 0 ;
	int i,j;
	camera_frame_t* frame;
	world_vis_set_update_t* worldVisSet;
	entity_visset_t* entityVisSet;
	float fbuffer[4];
	uint keyFrameCounter=0;
	uint nonKeyFrameCounter=0;
	
		
	memset(newFileName, 0, 256*sizeof(char));
	
	strcat(newFileName, "/Users/fabiensanglard/tmp/");
	strcat(newFileName, FS_GetFilenameOnly(filename));

	
	fileHandle = fopen(newFileName, "wb");
	
	if (!fileHandle)
	{
		printf("[CAM_SaveFramesToCP2Binary] Failed saving binary cp2.\n");
		return;
	}
	
	frame = firstFrame;
	
	while (frame != NULL) {
		frame = frame->next;
		num_frames++;
	}
	
	//Write magic number (CP2B)
	fwrite(magicNumber, 1, 4, fileHandle);
	
	//Number of frames
	fwrite(&num_frames, sizeof(num_frames), 1, fileHandle);
	
	frame = firstFrame;
	for(i=0 ; i < num_frames ; i++)
	{
		fwrite(&frame->time, sizeof(frame->time), 1, fileHandle);
		
		fbuffer[0] = frame->position[X];
		fbuffer[1] = frame->position[Y];
		fbuffer[2] = frame->position[Z];
		fwrite(fbuffer, sizeof(float), 3, fileHandle);
		
		fbuffer[0] = frame->orientation[X];
		fbuffer[1] = frame->orientation[Y];
		fbuffer[2] = frame->orientation[Z];
		fbuffer[3] = frame->orientation[W];		
		fwrite(fbuffer, sizeof(float), 4, fileHandle);
		
		worldVisSet = &frame->visUpdate;
		
		fwrite(&worldVisSet->isKey, sizeof(uchar), 1, fileHandle);
		
		fwrite(&worldVisSet->numVisSets, sizeof(ushort), 1, fileHandle);
	
		if (frame->visUpdate.isKey)
		{
			keyFrameCounter++;
			for(j=0 ; j < worldVisSet->numVisSets ; j++)
			{
				entityVisSet = &worldVisSet->visSets[j];
				
				fwrite(&entityVisSet->entityId, sizeof(ushort), 1, fileHandle);
				
				fwrite(&entityVisSet->numIndices, sizeof(ushort), 1, fileHandle);
				
				//for(k=0;  k < entityVisSet->numIndices ; k++)
				//	fwrite(entityVisSet->indices[k], sizeof(ushort), 1, fileHandle);
				
				fwrite(entityVisSet->indices, sizeof(ushort), entityVisSet->numIndices, fileHandle);
			}
		}
		else 
		{
			nonKeyFrameCounter++;
			for(j=0 ; j < worldVisSet->numVisSets ; j++)
			{
				entityVisSet = &worldVisSet->visSets[j];
				
				fwrite(&entityVisSet->entityId, sizeof(ushort), 1, fileHandle);
				
				fwrite(&entityVisSet->numFacesToAdd, sizeof(ushort), 1, fileHandle);
				fwrite(entityVisSet->facesToAdd, sizeof(ushort), entityVisSet->numFacesToAdd, fileHandle);
				
				fwrite(&entityVisSet->numFacesToRemove, sizeof(ushort), 1, fileHandle);
				fwrite(entityVisSet->facesToRemove, sizeof(ushort), entityVisSet->numFacesToRemove, fileHandle);
				
			}
		}

		
		
		
		
		frame = frame->next;
	}
	
	fclose(fileHandle);
	Timer_resetTime();
	printf("[PREPROC_SaveFramesToCP2Binary] Wrote %u keyFrames and %u deltaFrames.\n",keyFrameCounter,nonKeyFrameCounter);
}



ushort**					entityIndiceToModelIndice;
ushort**					modelIndiceToEntityIndice;

ushort*						indicesPerObjectId;



 int comparefaceId_t (const void * a, const void * b)
 {
	 return ( modelIndiceToEntityIndice[(*(prec_face_t*)b).objectId][(*(prec_face_t*)b).faceId*3] - modelIndiceToEntityIndice[(*(prec_face_t*)a).objectId][(*(prec_face_t*)a).faceId*3] );
 }
 

void printIndexes(FILE*	debug)
{
	int i,j;
	
	for (i=0 ; i <  num_map_entities ; i++) 
	{
		fprintf(debug,"indicesPerObjectId[%hu]=%hu.\n",i,indicesPerObjectId[i]);
	}
	
	
	
	for (i=0 ; i <  num_map_entities ; i++) 
	{
		if (indicesPerObjectId[i] == 0)
			continue;
		
		for (j=0 ; j < map[i].model->numIndices ; j+=3)
			fprintf(debug,"entityIndiceToModelIndice[%hu][%hu]=%hu\n",i,j,entityIndiceToModelIndice[i][j]);
	}

	for (i=0 ; i <  num_map_entities ; i++) 
	{
		if (indicesPerObjectId[i] == 0)
			continue;
		
		for (j=0 ; j < map[i].model->numIndices ; j+=3)
			fprintf(debug,"modelIndiceToEntityIndice[%hu][%hu]=%hu\n",i,j,modelIndiceToEntityIndice[i][j]);
	}
	
}

#define TRACE_CONVERT_FRAME 0


void PREPROC_ConvertPrecToRuntime(prec_camera_frame_t* prevFrame,prec_camera_frame_t* currentFrame,camera_frame_t* runTimeFrame)
{
	world_vis_set_update_t*		worldVisSet;
	entity_visset_t*			visSet;
	int							i,j;
	prec_face_t*				face;
	
	faceId_t					toBeRemoved[MAX_POLY_VIS_PER_FRAME];
	ushort						numToBeRemoved=0;
	
	ushort						numToBeAdded=0;
	faceId_t					toBeAdded[MAX_POLY_VIS_PER_FRAME];
	
	uchar						faceFound;

	entityAddRemoveStats_t*		entityStats;
	
	int							toAddCursor, toRemoveCursor;
	
	ushort						entId;
	
	
	FILE*						debug;
	char						debugFilename[256];
	
	if (TRACE_CONVERT_FRAME)
	{
		
		sprintf(debugFilename, "/Users/fabiensanglard/tmp/debug/frame-%6d.txt",currentFrame->time);
		debug = fopen(debugFilename, "wt");
	}
	
	printf("Converting frame t=%d to camera_frame_t.\n",currentFrame->time);
	
	runTimeFrame->time = currentFrame->time;
	runTimeFrame->next = 0;
	
	vectorCopy(currentFrame->position,runTimeFrame->position);
	vector4Copy( currentFrame->orientation, runTimeFrame->orientation);
	
	
	
	
	worldVisSet = &runTimeFrame->visUpdate;
	
	worldVisSet->isKey = currentFrame->visSet.isKey;
	
	if (currentFrame->visSet.isKey)
	{
		if (TRACE_CONVERT_FRAME)
			fprintf(debug, "frame %d is key.\n",runTimeFrame->time);
		
		
		worldVisSet->numVisSets = num_map_entities ;
		worldVisSet->visSets = (entity_visset_t*)calloc(num_map_entities, sizeof(entity_visset_t));
		
		
		
		
		// Count numIndices for each entity in the set
		for (i=0 ; i < currentFrame->visSet.numVisFaces; i++) 
		{
			face = &currentFrame->visSet.visFaces[i];
			
			visSet = &worldVisSet->visSets[face->objectId];
			visSet->numIndices += 3;
		}
		
		
		//Allocate memory for the indices array
		for(i=0 ; i < worldVisSet->numVisSets ; i++)
		{
			visSet = &worldVisSet->visSets[i];
			visSet->indices = (ushort*)calloc(visSet->numIndices, sizeof(ushort));
			visSet->numIndices = 0;
			visSet->entityId = i ;
		}
		
		// Need to zero indicesPerObjectId
		for (i=0 ; i < num_map_entities; i++) {
			indicesPerObjectId[i] = 0;
		}
		
		
		// write data to camera_frame_t, update indicesIndexes used on non-key camera_frame_t
		for (i=0 ; i < currentFrame->visSet.numVisFaces; i++) 
		{
			face = &currentFrame->visSet.visFaces[i];
			
			visSet = &worldVisSet->visSets[face->objectId];
					
			vectorCopy( &(map[face->objectId].indices[face->faceId*3]) , &(visSet->indices[visSet->numIndices]) ) ;
			
			//Keeping track of where faces are written.
			modelIndiceToEntityIndice[face->objectId][face->faceId*3] = visSet->numIndices ;
			entityIndiceToModelIndice[face->objectId][visSet->numIndices] = face->faceId*3 ;

			if (TRACE_CONVERT_FRAME)
				fprintf(debug,"	Entity %hu model face %hu is at %hu.\n",face->objectId,face->faceId*3,visSet->numIndices);
			
			visSet->numIndices += 3;		
			
			//Used later during delta frame encoding
			indicesPerObjectId[face->objectId] += 3;
			
		}
		
		
		
		if (TRACE_CONVERT_FRAME)
		{
			printIndexes(debug);
			fclose(debug);
		}
	}
	
	
	
	
	
	
	else 
				
		
	{
		if (TRACE_CONVERT_FRAME)
			fprintf(debug, "frame %d is NOT key.\n",runTimeFrame->time);
		
		
		// Check what needs to be added from the currentFrame, compared to previous
		for (i=0 ; i < currentFrame->visSet.numVisFaces; i++) 
		{
			faceFound = 0;
			for (j=0 ; j < prevFrame->visSet.numVisFaces; j++) 
			{
				if (currentFrame->visSet.visFaces[i].objectId == prevFrame->visSet.visFaces[j].objectId && currentFrame->visSet.visFaces[i].faceId == prevFrame->visSet.visFaces[j].faceId)
				{
					faceFound = 1;
					break;
				}
			}
			if (!faceFound)
			{
				toBeAdded[numToBeAdded].objectId = currentFrame->visSet.visFaces[i].objectId;
				toBeAdded[numToBeAdded].faceId = currentFrame->visSet.visFaces[i].faceId;
				
				if (TRACE_CONVERT_FRAME)
					fprintf(debug,"	[Add   ] Entity %hu, ModelFace %hu.\n",toBeAdded[numToBeAdded].objectId,toBeAdded[numToBeAdded].faceId*3);
				
				numToBeAdded++;
			}
		}
		
		
		
		// Check what needs to be removed   from the currentFrame, compared to previous
		for (i=0 ; i < prevFrame->visSet.numVisFaces; i++) 
		{
			faceFound = 0;
			for (j=0 ; j < currentFrame->visSet.numVisFaces; j++) 
			{
				if (currentFrame->visSet.visFaces[j].objectId == prevFrame->visSet.visFaces[i].objectId && currentFrame->visSet.visFaces[j].faceId == prevFrame->visSet.visFaces[i].faceId)
				{
					faceFound=1;
					break;
				}
			}
			
			if (!faceFound)
			{
				toBeRemoved[numToBeRemoved].objectId = prevFrame->visSet.visFaces[i].objectId;
				toBeRemoved[numToBeRemoved].faceId = prevFrame->visSet.visFaces[i].faceId;
				
				if (TRACE_CONVERT_FRAME)
				fprintf(debug,"	[Remove ] Entity %hu, ModelFace %hu.\n",toBeRemoved[numToBeRemoved].objectId,toBeRemoved[numToBeRemoved].faceId*3);
				
				numToBeRemoved++;
			}
		}
		
		// Need to sort toBeRemoved indices based on where they are currently in the entity list (because an indice to be removed is flipped with the last element, 
		// indices have to be sorted from last to first to avoid big mess.
		qsort(toBeRemoved, numToBeRemoved, sizeof(faceId_t), comparefaceId_t);
		
		if (TRACE_CONVERT_FRAME)
		{
			fprintf(debug,"numToBeAdded=%hu.\n",numToBeAdded);
			
			fprintf(debug,"numToBeRemoved=%hu.\n",numToBeRemoved);
		}
		
		for (i=0; i < numToBeRemoved ; i++) 
		{
			//printf("ToBeRemoved: @%hu @%hu @%hu.\n",toBeRemoved[i].objectId,toBeRemoved[i].faceId,modelIndiceToEntityIndice[ toBeRemoved[i].objectId   ][ toBeRemoved[i].faceId*3]);
		}
		
		// Determine how many entities will be in this update and how much add and remove each will have
		entityStats = calloc(num_map_entities, sizeof(entityAddRemoveStats_t));
		for (i=0 ; i < numToBeRemoved; i++) 
		{
			entityStats[toBeRemoved[i].objectId].numFacestoRemove++;
		}
		for (i=0 ; i < numToBeAdded; i++) 
		{
			entityStats[toBeAdded[i].objectId].numFacesToAdd++;
		}
		
		
		
		
		
		//Attributing an id in world_vis_set_update_t for each entity that require an update.
		worldVisSet->numVisSets = 0;
		for(i=0 ; i < num_map_entities ; i++)
		{
			if (entityStats[i].numFacesToAdd != 0 || entityStats[i].numFacestoRemove != 0)
				entityStats[i].indexInVisUpdate = worldVisSet->numVisSets++;
		}
		
		if (TRACE_CONVERT_FRAME)
			fprintf(debug,"num entities to update=%hu.\n",worldVisSet->numVisSets);
		
		worldVisSet->visSets = calloc(worldVisSet->numVisSets, sizeof(entity_visset_t));
		
		
		
		// Allocating memory for delta encoding portion of entity_visset_t
		for(i=0 ; i < num_map_entities ; i++)
		{
			if (entityStats[i].numFacesToAdd != 0 || entityStats[i].numFacestoRemove != 0)
			{
				
				visSet = &worldVisSet->visSets [ entityStats[i].indexInVisUpdate ] ;
				
				visSet->entityId = i ;
				
				visSet->numFacesToAdd = 0;//entityStats[i].numFacesToAdd;
				visSet->facesToAdd = calloc(entityStats[i].numFacesToAdd  , sizeof(ushort));

				
				visSet->numFacesToRemove = 0;//entityStats[i].numFacestoRemove;
				visSet->facesToRemove = calloc(entityStats[i].numFacestoRemove, sizeof(ushort));
			}
		}
		
		
		
		
		//Inserting values now, remember to keep track of where things are with faceIdToModelIndice[objectId][faceId] =
		
		//To remove only indicate where the data has to be removed, on a 3 ushort span (indice1,indice2,indice3)
		for (i=0 ; i < numToBeRemoved; i++) 
		{
			visSet = &worldVisSet->visSets [ entityStats[ toBeRemoved[i].objectId  ].indexInVisUpdate ] ;
			
			if (TRACE_CONVERT_FRAME)
				fprintf(debug,"	[toBeRemoved] Entity %hu modelId %hu = entity %hu.\n",toBeRemoved[i].objectId ,toBeRemoved[i].faceId*3,modelIndiceToEntityIndice[ toBeRemoved[i].objectId   ][ toBeRemoved[i].faceId*3]);
			
			visSet->facesToRemove[ visSet->numFacesToRemove++ ] = modelIndiceToEntityIndice[ toBeRemoved[i].objectId   ][ toBeRemoved[i].faceId*3];
		}
		
		
		for (i=0 ; i < numToBeAdded; i++) 
		{
			visSet = &worldVisSet->visSets [ entityStats[ toBeAdded[i].objectId  ].indexInVisUpdate ] ;
			
			if (TRACE_CONVERT_FRAME)
				fprintf(debug,"	[toBeAdded  ] Entity %hu modelId %hu.\n",toBeAdded[i].objectId ,toBeAdded[i].faceId*3);
			
			visSet->facesToAdd[ visSet->numFacesToAdd++ ] = toBeAdded[i].faceId*3;
			
			
		}
		
		
		// now updating indexes to keep track of where is everything.
		for (i=0 ; i < worldVisSet->numVisSets ; i++)
		{
			visSet = &worldVisSet->visSets [ i ];
			
			entId = visSet->entityId ;
			
			toRemoveCursor = 0;//visSet->numFacesToRemove;
			toAddCursor = 0;//visSet->numFacesToAdd;
			
			while (toAddCursor < visSet->numFacesToAdd  && toRemoveCursor < visSet->numFacesToRemove) 
			{
				//To add will go in the empty slot.
				
				modelIndiceToEntityIndice[entId][visSet->facesToAdd[toAddCursor] ] = 	visSet->facesToRemove[ toRemoveCursor] ;
			
				entityIndiceToModelIndice[entId][visSet->facesToRemove[ toRemoveCursor] ] = visSet->facesToAdd[ toAddCursor] ;
				
				
				toAddCursor++;
				toRemoveCursor++;
			}
			
			//Now either facesToAdd or facesToRemove are exhausted.
			
			//Keep removing indices by flipping with last element of the entity indices list
			while (toRemoveCursor < visSet->numFacesToRemove)
			{
				
				indicesPerObjectId[ entId ] -= 3;
				
				modelIndiceToEntityIndice[entId][ entityIndiceToModelIndice[entId][ indicesPerObjectId[ entId ] ] ] = visSet->facesToRemove[toRemoveCursor]  ;
				
				
				
				entityIndiceToModelIndice[entId][ visSet->facesToRemove[toRemoveCursor] ]  = entityIndiceToModelIndice[entId][ indicesPerObjectId[ entId] ] ;
				
				
				
				toRemoveCursor++;
			}
			
			//Add remaining indices to entity indices list
			while (toAddCursor < visSet->numFacesToAdd)
			{
				
				entityIndiceToModelIndice[entId] [ indicesPerObjectId[ entId] ]  = visSet->facesToAdd[ toAddCursor] ;
				modelIndiceToEntityIndice[entId] [ visSet->facesToAdd[ toAddCursor] ] = indicesPerObjectId[ entId ] ;
				
				
				indicesPerObjectId[ visSet->entityId ] += 3;
				toAddCursor++;
			}
			
			
		}
		
		if (TRACE_CONVERT_FRAME)
		{
			printIndexes(debug);
			fclose(debug);
		}
		
		
		
		
		free(entityStats);
	}
	
	
}




void PREPROC_ConvertCp1Tocp2b(char* cpFilename, char* cp2bFilename, char* logFilename)
{
	filehandle_t*			file ;
	uint					num_frames;
	prec_camera_frame_t*	rootFrame;
	prec_camera_frame_t*	currentFrame;
	prec_camera_frame_t*	prevFrame;
	prec_camera_frame_t*	nextAfterExpansion;
	prec_camera_frame_t*	toFree;
	
	camera_frame_t*			currentRunTimeFrame;
	camera_frame_t*			previosRunTimeFrame;
	camera_frame_t			rootRunTimeFrame;
	
	int						i;
	int						frameCounter;
	int						numFrameBetweenKey;
	
	entity_t*				entity;
	
	printf("[PREPROC_ConvertCp1Tocp2b]");
	
	file = FS_OpenFile(camera.pathFilename, "rt");
	
	if (logFilename)
		logFile = fopen(logFilename, "wt");
	
	if (!file)
	{
		printf("CP file cannot be opened. Aborting.\n");
		return;
	}
	
	LE_init(file);
	
	LE_readToken(); //CP1
	
	
	if (strcmp("cp1", LE_getCurrentToken()))
	{
		printf("CP file found but magic number check failed. Aborting.\n");
		return;
	}
		
		
	//Alloc faceIdToModelIndice for each object in the map
	entityIndiceToModelIndice = calloc(num_map_entities, sizeof(ushort*));
	for(i=0; i < num_map_entities; i++)
	{
		entity = &map[i];
		entityIndiceToModelIndice[i] = calloc(entity->numIndices, sizeof(ushort));
	}
	
	//Alloc revert index: faceIdToModelIndice
	modelIndiceToEntityIndice = calloc(num_map_entities, sizeof(ushort*));
	for(i=0; i < num_map_entities; i++)
	{
		entity = &map[i];
		modelIndiceToEntityIndice[i] = calloc(entity->numIndices, sizeof(ushort));
	}
	
	indicesPerObjectId = calloc(num_map_entities, sizeof(ushort));
	
	
	LE_readToken(); //num_frames
	num_frames = LE_readReal();	
	
		
	//Reading every key frames
	rootFrame = PREPROC_ReadFrameFromFile();
	rootFrame->next = NULL;
	currentFrame = rootFrame;
	for(i=1 ; i < num_frames ; i++)
	{
		currentFrame->next = PREPROC_ReadFrameFromFile();
		currentFrame = currentFrame->next;
		currentFrame->next = NULL;
	}
		
	
	//Interpolating camera key Frames to have a TIME_STEP granularity
	currentFrame = rootFrame;
	while (currentFrame->next != NULL) 
	{	
		nextAfterExpansion = currentFrame->next ;
		PREPROC_ExpandCameraWayPoints(currentFrame,currentFrame->next);
		currentFrame= nextAfterExpansion;
	}
		
		
	
	
	
	// Mark pre_camera_frame_t key 
	numFrameBetweenKey  = KEY_FRAME_INTERVAL_MS / 16.6666667f ;
	frameCounter=numFrameBetweenKey;
	
	currentFrame = rootFrame;
	while (currentFrame != NULL) 
	{
		if (frameCounter == numFrameBetweenKey)
		{
			currentFrame->visSet.isKey = 1;
			frameCounter=0;
		}
		else 
		{
			currentFrame->visSet.isKey = 0; //0
			frameCounter++;
		}

		currentFrame = currentFrame->next;
	}
	
	
	
	
	
		
	currentFrame = rootFrame;
	prevFrame = NULL;
	
	previosRunTimeFrame = &rootRunTimeFrame;	
	
	while(currentFrame != NULL)
	{
		
		currentFrame->visSet.visFaces = calloc(sizeof(prec_face_t), MAX_POLY_VIS_PER_FRAME+1);
		
		// Generate raw list of visFaces
		PREPROC_PopulateRawFaceSet(currentFrame);
		
		// Convert to normal camera_frame_t
		currentRunTimeFrame = calloc(1, sizeof(camera_frame_t));
		PREPROC_ConvertPrecToRuntime(prevFrame,currentFrame,currentRunTimeFrame);
		
		toFree = prevFrame;
		prevFrame = currentFrame;
		currentFrame = currentFrame->next;
		
		if (toFree)
		{
			free(toFree->visSet.visFaces);
			free(toFree);
		}
		
		previosRunTimeFrame->next = currentRunTimeFrame;
		previosRunTimeFrame = currentRunTimeFrame;
		
	}
				
	
	
	PREPROC_SaveFramesToCP2Binary(cp2bFilename,rootRunTimeFrame.next);
		
		
	//free face <-> indice indexes
	for(i=0; i < num_map_entities; i++)
		free(entityIndiceToModelIndice[i]);
	free(entityIndiceToModelIndice);
	
	for(i=0; i < num_map_entities; i++)
		free(modelIndiceToEntityIndice[i]);
	free(modelIndiceToEntityIndice);
	
	free(indicesPerObjectId);
}


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
 *  md5.c
 *  dEngine
 *
 *  Created by fabien sanglard on 09-11-02.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "md5.h"
#include "filesystem.h"
#include "lexer.h"
#include "math.h"
#include "quaternion.h"
#include "entities.h"
#include "material.h"
#include <limits.h>
#include <float.h>
#include "renderer.h"
#include "lexer.h"




void MD5_GenerateLightingInfo (md5_mesh_t* mesh)
{
	int verticesCounter;
	int weightCounter;
	//User for tangent space generation
	vec3_t v1,v2,normal;
	
	vec3_t* normalAccumulator;
	vec3_t* normalWeightAccumulator;

	vec3_t tangent;
	float coef;
	vec3_t jointSpaceTangent;
	vec2_t st1,st2;
	vec3_t* tangentAccumulator;
	vec3_t* tangentWeightAccumulator;

	
	vertex_t* currentVertex = NULL;
	md5_vertex_t* md5Vertex;
	int facesCounter;
	
	md5_weight_t* weight;
	md5_bone_t* bone;
	md5_triangle_t* currentFace;
	
	vec3_t jointSpaceNormal;
	
	normalAccumulator		=	calloc(mesh->numVertices, sizeof(vec3_t));
	normalWeightAccumulator =	calloc(mesh->numWeights, sizeof(vec3_t));

	tangentAccumulator		=	calloc(mesh->numVertices, sizeof(vec3_t));
	tangentWeightAccumulator=	calloc(mesh->numWeights, sizeof(vec3_t));

	
	//Log_Printf("\nGenerating normal and tangents.\n");

	
	//Generate the normal and tangent per face
	currentFace = mesh->triangles;
	for(facesCounter = 0; facesCounter < mesh->numTriangles ; facesCounter++,currentFace++)
	{
		
		// Normal part
		vectorSubtract(mesh->vertexArray[currentFace->index[2]].pos , mesh->vertexArray[currentFace->index[0]].pos, v1);
		vectorSubtract(mesh->vertexArray[currentFace->index[1]].pos , mesh->vertexArray[currentFace->index[0]].pos, v2);
		vectorCrossProduct(v2,v1,normal);
		
		normalize(normal);
		
		vectorAdd(normalAccumulator[currentFace->index[0]],normal,normalAccumulator[currentFace->index[0]]);
		vectorAdd(normalAccumulator[currentFace->index[1]],normal,normalAccumulator[currentFace->index[1]]);
		vectorAdd(normalAccumulator[currentFace->index[2]],normal,normalAccumulator[currentFace->index[2]]);
		
		
		// The following part is from "Mathematic for 3D programming" by Eric Lengyel
		// Tangent part
		


		vector2Subtract(mesh->vertexArray[currentFace->index[2]].text,mesh->vertexArray[currentFace->index[0]].text,st1);
		vector2Subtract(mesh->vertexArray[currentFace->index[1]].text,mesh->vertexArray[currentFace->index[0]].text,st2);
		
		vector2Scale(st1,1/(float)32767,st1);
		vector2Scale(st2,1/(float)32767,st2);
		
		if (st1[0] == 0.0f && st2[0] == 0.0f)
		{
			st1[0] = 0.1f ; 
			st2[0] = 0.1f;
		}
		
		if (st1[1] == 0.0f && st2[1] == 0.0f)
		{
			st1[1] = 0.1f ;
			st2[1] = 0.1f;
		}
		
		
		coef = 1/ (st1[0] * st2[1] - st2[0] * st1[1]);
		
		
		
		tangent[0] = coef * (v1[0] * st2[1]  + v2[0] * -st1[1]);
		tangent[1] = coef * (v1[1] * st2[1]  + v2[1] * -st1[1]);
		tangent[2] = coef * (v1[2] * st2[1]  + v2[2] * -st1[1]);
		
		normalize(tangent);		
		
		
		vectorAdd(tangentAccumulator[currentFace->index[0]],tangent,tangentAccumulator[currentFace->index[0]]);
		vectorAdd(tangentAccumulator[currentFace->index[1]],tangent,tangentAccumulator[currentFace->index[1]]);
		vectorAdd(tangentAccumulator[currentFace->index[2]],tangent,tangentAccumulator[currentFace->index[2]]);
		
		
		
	}
	
	//Normalize accumulated normal and tangent
	for(verticesCounter=0 ; verticesCounter < mesh->numVertices ; verticesCounter++,currentVertex++)
	{
		
		
		normalize(normalAccumulator[verticesCounter]);
//		Log_Printf("normalized accumulated normal [%d][%.2f,%.2f,%.2f]\n",verticesCounter,normalAccumulator[verticesCounter][0],normalAccumulator[verticesCounter][1],normalAccumulator[verticesCounter][2]);
		normalize(tangentAccumulator[verticesCounter]);
//		Log_Printf("normalized accumulated tangent [%d][%.2f,%.2f,%.2f]\n",verticesCounter,tangentAccumulator[verticesCounter][0],tangentAccumulator[verticesCounter][1],tangentAccumulator[verticesCounter][2]);
	}
	
	//Now we have all the normal for this model, but need to transform them in bone space for re-usage
	// Translating the normal orientation from object to joint space and Store normals inside weights, 
	md5Vertex = mesh->vertices;
	currentVertex = mesh->vertexArray;
	for(verticesCounter=0 ; verticesCounter < mesh->numVertices ; verticesCounter++,md5Vertex++)
	{
		for (weightCounter = 0; weightCounter < md5Vertex->count; weightCounter++)
		{
			weight = &mesh->weights[md5Vertex->start + weightCounter];
			bone  = &mesh->bones[weight->boneId];
			
			multiplyByInvertQuaternion(normalAccumulator[verticesCounter],bone->orientation,jointSpaceNormal);
			vectorAdd(normalWeightAccumulator[md5Vertex->start + weightCounter],jointSpaceNormal,normalWeightAccumulator[md5Vertex->start + weightCounter]);
					
			multiplyByInvertQuaternion(tangentAccumulator[verticesCounter],bone->orientation,jointSpaceTangent);
			vectorAdd(tangentWeightAccumulator[md5Vertex->start + weightCounter],jointSpaceTangent,tangentWeightAccumulator[md5Vertex->start + weightCounter]);			
		}
		
	}
	
	weight = mesh->weights;
	for (weightCounter = 0; weightCounter < mesh->numWeights; weightCounter++,weight++)
	{
		normalize(normalWeightAccumulator[weightCounter]);
		vectorScale(normalWeightAccumulator[weightCounter],32767,weight->boneSpaceNormal);
		
		normalize(tangentWeightAccumulator[weightCounter]);
		vectorScale(tangentWeightAccumulator[weightCounter],32767,weight->boneSpaceTangent);
			
	}
	
	free(normalAccumulator);
	free(normalWeightAccumulator);

	free(tangentAccumulator);
	free(tangentWeightAccumulator);

}

void MD5_GenerateSkin (md5_mesh_t* mesh, md5_bone_t* bones)
{
	int i, j;
	md5_weight_t* weight;
	md5_bone_t*  bone ;
	
	vec3_t normalAccumulator;
	vec3_t tangentAccumulator;

	vertex_t* currentVertex;
	
	//Log_Printf("\nGenerating weight positions.\n");
	
	// Generate weight position in modelSpace
	weight = mesh->weights;
	for (i = 0; i < mesh->numWeights; i++,weight++)
	{
		bone = &bones[weight->boneId];
		Quat_rotatePoint (bone->orientation, weight->boneSpacePos, weight->modelSpacePos);
		vectorAdd(weight->modelSpacePos,bone->position,weight->modelSpacePos);
		
		
//		Log_Printf("weight[%d].pos=[%.2f,%.2f,%.2f]\n",i,bone->position[0],bone->position[1],bone->position[2]);
//		Log_Printf("weight[%d].pos=[%.2f,%.2f,%.2f]\n",i,weight->modelSpacePos[0],weight->modelSpacePos[1],weight->modelSpacePos[2]);
		
		Quat_rotateShortPoint (bone->orientation, weight->boneSpaceNormal, weight->modelSpaceNormal);
		Quat_rotateShortPoint (bone->orientation, weight->boneSpaceTangent,weight->modelSpaceTangent);
	}
	
	
	/* Setup vertices */
	currentVertex = mesh->vertexArray ;
	for (i = 0; i < mesh->numVertices; ++i)
    {
		vectorClear(currentVertex->pos);
		vectorClear(normalAccumulator);
		vectorClear(tangentAccumulator);

		
		// Calculate final vertex to draw with weights 
		for (j = 0; j < mesh->vertices[i].count; j++)
		{
			weight= &mesh->weights[mesh->vertices[i].start + j];
			bone = &bones[weight->boneId];
			
			// Calculate transformed vertex for this weight 
			currentVertex->pos[0] += weight->modelSpacePos[0] * weight->bias;
			currentVertex->pos[1] += weight->modelSpacePos[1] * weight->bias;
			currentVertex->pos[2] += weight->modelSpacePos[2] * weight->bias;
			
			// Same thing for normal
			
			vectorAdd(normalAccumulator,weight->modelSpaceNormal,normalAccumulator);
			vectorAdd(tangentAccumulator,weight->modelSpaceTangent,tangentAccumulator);

		}
		
//		Log_Printf("currentVertex[%d].pos=[%.2f,%.2f,%.2f]\n",i,currentVertex->pos[0],currentVertex->pos[1],currentVertex->pos[2]);
		
		//Need to normalize normal
		normalize(normalAccumulator);
		vectorScale(normalAccumulator,32767,currentVertex->normal);
//		Log_Printf("currentVertex[%d].normal=[%hu,%hu,%hu]\n",i,currentVertex->normal[0],currentVertex->normal[1],currentVertex->normal[2]);

		normalize(tangentAccumulator);
		vectorScale(tangentAccumulator,32767,currentVertex->tangent);
//		Log_Printf("currentVertex[%d].tangent=[%hu,%hu,%hu]\n",i,currentVertex->tangent[0],currentVertex->tangent[1],currentVertex->tangent[2]);	
		
		currentVertex++;
    }
}

void MD5_ReadMesh(md5_mesh_t* mesh)
{
	int j ;
	md5_triangle_t* triangle;
	md5_vertex_t* vertex;
	md5_weight_t* weight;

	
	
	LE_readToken(); // {
	
	while (LE_hasMoreData() && strcmp("}", LE_getCurrentToken())) 
	{
		LE_readToken();
		
		if (!strcmp("shader", LE_getCurrentToken()))
		{
			LE_readToken();
			LE_cleanUpDoubleQuotes(LE_getCurrentToken());
			mesh->materialName = calloc(strlen(LE_getCurrentToken())+1, sizeof(char));
			strcpy(mesh->materialName, LE_getCurrentToken());
		}
		else
		if (!strcmp("numverts", LE_getCurrentToken()))
		{
		
			mesh->numVertices = LE_readReal();
			//Log_Printf("[MD5_ReadMesh] Found numverts: %d.\n",mesh->numVertices);
			mesh->vertices = (md5_vertex_t*)calloc(mesh->numVertices, sizeof(md5_vertex_t));
			vertex = mesh->vertices;
			for(j=0; j< mesh->numVertices ; j++,vertex++)
			{
				LE_readToken() ; //vert
				LE_readReal();	// id
				vertex->st[0] = LE_readReal() * 32767;
				vertex->st[1] = LE_readReal() * 32767;
				vertex->start = LE_readReal();
				vertex->count = LE_readReal();
				
				//Log_Printf("MD5 Read vertex: uv[%hu,%hu] st,count[%d,%d]\n",vertex->st[0],vertex->st[2],vertex->start,vertex->count);
				
			}
			
		}
		else
		if (!strcmp("numtris", LE_getCurrentToken()))
		{
			
			mesh->numTriangles = LE_readReal();
			//Log_Printf("[MD5_ReadMesh] Found numtris: %d.\n",mesh->numTriangles);
			
			mesh->triangles = (md5_triangle_t*)calloc(mesh->numTriangles, sizeof(md5_triangle_t));
			triangle = mesh->triangles;
			for(j=0; j< mesh->numTriangles ; j++,triangle++)
			{
				LE_readToken() ; //tri
				LE_readReal();	// id
				triangle->index[0] = LE_readReal();
				triangle->index[1] = LE_readReal();
				triangle->index[2] = LE_readReal();
				
				//Log_Printf("MD5 Read tri: [%hu,%hu,%hu]\n",triangle->index[0],triangle->index[1],triangle->index[2]);
			}
		}
		else
		if (!strcmp("numweights", LE_getCurrentToken()))
		{
			
			mesh->numWeights = LE_readReal();
			//Log_Printf("[MD5_ReadMesh] Found numweights: %d.\n",mesh->numWeights);
			
			mesh->weights = (md5_weight_t*)calloc(mesh->numWeights, sizeof(md5_weight_t));
			weight = mesh->weights;
			for(j=0;j<mesh->numWeights ; j++,weight++)
			{
				LE_readToken() ; //tri
				LE_readReal();	// id
				weight->boneId = LE_readReal();
				weight->bias = LE_readReal();
				weight->boneSpacePos[0] = LE_readReal();
				weight->boneSpacePos[1] = LE_readReal();
				weight->boneSpacePos[2] = LE_readReal();
				
				//Log_Printf("MD5 Read weight: Boneid[%d] f[%.2f]  boneSpace[%.2f,%.2f,%.2f]\n",weight->boneId,weight->bias,weight->boneSpacePos[0],weight->boneSpacePos[1],weight->boneSpacePos[2]);
			}
		}
		
	
	}
	
}

void MD5_ReadJoints(md5_bone_t* bones, ushort numBones)
{
	md5_bone_t* bone = 0;
	int i=0;
	
	LE_readToken(); // {
	
	bone = bones;
	for(i=0; i < numBones; i++,bone++)
	{
		
		
		LE_readToken();

		strcpy(bone->name, LE_getCurrentToken());
		
				
		bone->parent = LE_readReal();
		bone->position[0] = LE_readReal();
		bone->position[1] = LE_readReal();
		bone->position[2] = LE_readReal();
		bone->orientation[0] = LE_readReal();
		bone->orientation[1] = LE_readReal();
		bone->orientation[2] = LE_readReal();
		Quat_computeW (bone->orientation);
	}
	
	
	LE_readToken(); // }
}


void MD5_GenerateModelSpaceBBox(md5_mesh_t* mesh )
{
	int i;
	vertex_t* vertex;
	
	mesh->modelSpacebbox.min[0] = FLT_MAX;
	mesh->modelSpacebbox.min[1] = FLT_MAX;
	mesh->modelSpacebbox.min[2] = FLT_MAX;
	
	mesh->modelSpacebbox.max[0] = FLT_MIN;
	mesh->modelSpacebbox.max[1] = FLT_MIN;
	mesh->modelSpacebbox.max[2] = FLT_MIN;
	
	for(i= 0, vertex = mesh->vertexArray ; i < mesh->numVertices  ; i++,vertex++)
	{
		if (vertex->pos[0] > mesh->modelSpacebbox.max[0]) mesh->modelSpacebbox.max[0] = vertex->pos[0];
		if (vertex->pos[1] > mesh->modelSpacebbox.max[1]) mesh->modelSpacebbox.max[1] = vertex->pos[1];
		if (vertex->pos[2] > mesh->modelSpacebbox.max[2]) mesh->modelSpacebbox.max[2] = vertex->pos[2];
		
		if (vertex->pos[0] < mesh->modelSpacebbox.min[0]) mesh->modelSpacebbox.min[0] = vertex->pos[0];
		if (vertex->pos[1] < mesh->modelSpacebbox.min[1]) mesh->modelSpacebbox.min[1] = vertex->pos[1];
		if (vertex->pos[2] < mesh->modelSpacebbox.min[2]) mesh->modelSpacebbox.min[2] = vertex->pos[2];
		
	}
	
	
	//Log_Printf("Bounding box: min=[%f,%f,%f],max=[%f,%f,%f].\n",entity->modelSpacebbox.min[0],entity->modelSpacebbox.min[1],entity->modelSpacebbox.min[2],entity->modelSpacebbox.max[0],entity->modelSpacebbox.max[1],entity->modelSpacebbox.max[2]);
}

#define TRACE_BLOCK 0

char MD5_LoadMesh(md5_mesh_t* mesh, const char* filename)
{
	filehandle_t* fhandle = 0;
	vertex_t* currentVertex = 0;
	ushort verticesCounter=0;
	int versionNumber = 0;
	int i;
	
	 
	
	fhandle = FS_OpenFile(filename, "rt");
	FS_UploadToRAM(fhandle);

	if (!fhandle)
	{
		return 0;
	}
	
	LE_pushLexer();
	LE_init(fhandle);
	
	
	mesh->memLocation = MD5_MEMLOC_RAM;
	
	//Log_Printf("[MD5_LoadEntity] Loading MD5 '%s' .\n",filename); 

	
	
	while (LE_hasMoreData()) 
	{
		LE_readToken();
		
		if(!strcmp("MD5Version", LE_getCurrentToken()))
		{
			versionNumber = LE_readReal();
			if (versionNumber != 10)
			{
				Log_Printf ("[MD5_Loader ERROR] : %s has a bad model version (%d)\n",filename,versionNumber);
				return 0;
			}
		}
		else
		if (!strcmp("numJoints", LE_getCurrentToken()))
		{
			mesh->numBones = LE_readReal();
			mesh->bones = (md5_bone_t*)calloc(mesh->numBones,sizeof(md5_bone_t));
			//Log_Printf("[MD5_LoadEntity] Found numJoints: %d.\n",mesh->numBones);
		}
		else
		if (!strcmp("mesh", LE_getCurrentToken()))
		{
			//Log_Printf("[MD5_LoadEntity] Found mesh.\n");
			MD5_ReadMesh(mesh);  
		}
		else
		if (!strcmp("numMeshes", LE_getCurrentToken()))
		{
			//Log_Printf("[MD5_LoadEntity] Found numMeshes.\n");
			if(LE_readReal() > 1)
			{
				Log_Printf("[MD5_Loader ERROR] %s has more than one mesh: Not supported.\n",filename);
				return 0;
			}
		}		   
		else
		if (!strcmp("joints", LE_getCurrentToken()))
		{
			//Log_Printf("[MD5_LoadEntity] Found joints.\n");
			MD5_ReadJoints(mesh->bones,mesh->numBones);
		}		   
		   
	}
	

	
	mesh->vertexArray = (vertex_t*)calloc(mesh->numVertices, sizeof(vertex_t));
	mesh->indices = (ushort*)calloc(mesh->numTriangles * 3,sizeof(ushort));
	
	//Write indices
	mesh->numIndices=0 ;
	for (i = 0  ; i < mesh->numTriangles; i++)
	{
		mesh->indices[mesh->numIndices++] = mesh->triangles[i].index[0];
		mesh->indices[mesh->numIndices++] = mesh->triangles[i].index[1];
		mesh->indices[mesh->numIndices++] = mesh->triangles[i].index[2];
	}
	
	//Write textureCoo
	//Set all textures coordinate once for all, this will never change
	currentVertex = mesh->vertexArray;
	for(verticesCounter=0 ; verticesCounter < mesh->numVertices ; verticesCounter++,currentVertex++)
	{
		currentVertex->text[0] = mesh->vertices[verticesCounter].st[0];
		currentVertex->text[1] = mesh->vertices[verticesCounter].st[1];
	}
	
	//Log_Printf("[MD5 Loader] %d vertices.\n",mesh->numVertices);
	//Log_Printf("[MD5 Loader] %d triangles.\n",mesh->numTriangles);
	//Log_Printf("[MD5 Loader] %d indices.\n",mesh->numIndices);
	
	
	
	MD5_GenerateSkin(mesh,mesh->bones);
	MD5_GenerateLightingInfo(mesh);
	MD5_GenerateSkin(mesh,mesh->bones);
	
	//Bounding box
	MD5_GenerateModelSpaceBBox(mesh);
	
	if (TRACE_BLOCK)
	{
	if (!strcmp(filename,"data/models/misc/FRA200L.obj.md5mesh"))
	{
		currentVertex = mesh->vertexArray;
		Log_Printf("Listing Vertices.\n");
		for (i=0; i < mesh->numVertices ; i++,currentVertex++) 
		{
			Log_Printf("vertex: %d/%d  (norm: %hd , %hd , %hd ) (text: %hd , %hd ) (pos: %f , %f , %f )\n",
				   i,
				   mesh->numVertices-1,   
				   currentVertex->normal[0],
				   currentVertex->normal[1],
				   currentVertex->normal[2],
				   currentVertex->text[0],
				   currentVertex->text[1],
				   currentVertex->pos[0],
				   currentVertex->pos[1],
				   currentVertex->pos[2]	   
				   );
		}


		Log_Printf("Listing indices.\n");
		for(i=0;  i < mesh->numIndices ; i++)
		{
			Log_Printf("indice: %d/%d, vertex: %hu   (norm: %hd , %hd , %hd ) (text: %hd , %hd ) (pos: %f , %f , %f )\n",
				   i,
				   mesh->numIndices-1,   
					mesh->indices[i],
				   mesh->vertexArray[mesh->indices[i]].normal[0],
				   mesh->vertexArray[mesh->indices[i]].normal[1],
				   mesh->vertexArray[mesh->indices[i]].normal[2],
				   mesh->vertexArray[mesh->indices[i]].text[0],
				   mesh->vertexArray[mesh->indices[i]].text[1],
				   mesh->vertexArray[mesh->indices[i]].pos[0],
				   mesh->vertexArray[mesh->indices[i]].pos[1],
				   mesh->vertexArray[mesh->indices[i]].pos[2]	 
				   );
		}
	}
	}

	FS_CloseFile(fhandle);
	LE_popLexer();
	
	
	//Cache mesh to speed up future loading.
	
	return 1;
}



void MD5_FreeMesh(md5_mesh_t* mesh)
{
		//If GPU resident, free it as well
	if (mesh->memLocation == MD5_MEMLOC_VRAM)
	{
		renderer.FreeGPUBuffer(mesh->vboId);
	}
	else {
		free(mesh->vertexArray);
	}

	free(mesh->bones);
	free(mesh->vertices);
	free(mesh->triangles);
	free(mesh->weights);
	free(mesh->indices);
	free(mesh);
	

	
}
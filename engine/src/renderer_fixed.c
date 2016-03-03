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
 *  r_fixed_renderer.c
 *  dEngine
 *
 *  Created by fabien sanglard on 09/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "config.h"

//#define RENDER_COLL_BOXEX


#include "renderer_fixed.h"
#include "dEngine.h"
#include "camera.h"


#include "target.h"
#if defined(SHMUP_TARGET_WINDOWS)
	#include "GLES/gl.h"
#elif defined(SHMUP_TARGET_MACOSX)
    #include "OpenGL/gl.h"
    #define glOrthof glOrtho
    #define glFogx glFogf
#elif defined(SHMUP_TARGET_IOS)
	#include <OpenGLES/ES1/gl.h>
#elif defined(ANDROID)
    #include <GLES/gl.h>
#elif defined(LINUX)
#if !defined(GLES)
    #include <GL/gl.h>
    #define glOrthof glOrtho
    #define glFogx glFogf
#else
      #include <GLES/gl.h>
#endif
#endif


#include "stats.h"
#include "world.h"
#include "player.h"
#include "enemy.h"
#include "timer.h"
#include <limits.h>
#include "fx.h"
#include "commands.h"
#include "enemy_particules.h"

matrix_t projectionMatrix;
matrix_t modelViewMatrix;
matrix_t textureMatrix = { 1.0f/32767,0,0,0,0,1.0f/32767,0,0,0,0,1,0,0,0,0,1};	//Unpacking matrix since texture coordinates are normalized in a short instead of a float.
unsigned int lastTextureId;



int supportedCompressionFormatF;


void SCR_CheckErrors(char* step, char* details)
{
	GLenum err = glGetError();
	switch (err) {
		case GL_INVALID_ENUM:Log_Printf("Error GL_INVALID_ENUM %s, %s\n", step,details); break;
		case GL_INVALID_VALUE:Log_Printf("Error GL_INVALID_VALUE  %s, %s\n", step,details); break;
		case GL_INVALID_OPERATION:Log_Printf("Error GL_INVALID_OPERATION  %s, %s\n", step,details); break;				
		case GL_OUT_OF_MEMORY:Log_Printf("Error GL_OUT_OF_MEMORY  %s, %s\n", step,details); break;			
		case GL_NO_ERROR: break;
		default :Log_Printf("Error UNKNOWN  %s, %s\n", step,details);break;
	}
}

void SetupCameraF(void)
{
	vec3_t vLookat;
	
	vectorAdd(camera.position,camera.forward,vLookat);
	
	gluLookAt(camera.position, vLookat, camera.up, modelViewMatrix);
	
	//Log_Printf("t=%d, up=[%.2f,%.2f,%.2f]\n",simulationTime,camera.up[X],camera.up[Y],camera.up[Z]);
	
	glLoadMatrixf(modelViewMatrix);
}


void SetupLightingF(void)
{
	glLightfv(GL_LIGHT0, GL_POSITION, light.position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light.ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light.diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light.specula);
	
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, light.constantAttenuation);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, light.linearAttenuation);
}


void Set2DF(void)
{
	//printf("[Set2DF] glClear to be removed.\n");
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glEnable(GL_BLEND);
	//glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrthof(-SS_W, SS_W, -SS_H, SS_H, -1, 1);
	
	
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	
	glDisable(GL_CULL_FACE);
	glDisable(GL_FOG);
	

	glDisable(GL_LIGHTING);

	
	glDisable(GL_DEPTH_TEST);
	
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glBindBuffer(GL_ARRAY_BUFFER,0);
	
}



void Set3DF(void)
{
	//SCrew the iPod 2nd generation !! It seems that color gets cleaned anyway.....with PINK !!?!?!
	glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); 
	//glClear (GL_DEPTH_BUFFER_BIT); 
	
	glEnable(GL_DEPTH_TEST);
	
	
	glDisable(GL_BLEND);
	renderer.isBlending = 0;
	
	
	
	glEnableClientState (GL_NORMAL_ARRAY);
	
	if (light.enabled)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
	}
	
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	
	
	
	
	
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	
	

	//glDisable ( GL_COLOR_MATERIAL ) ;

	glBindBuffer(GL_ARRAY_BUFFER,0);
	
	glColor4f(1, 1, 1, 1);
}

void StopRenditionF(void)
{
	lastTextureId = -1;
}


void UpLoadTextureToGPUF(texture_t* texture)
{
	int i,mipMapDiv;
	
	if (!texture || !texture->data || texture->textureId != 0)
		return;
	
	glGenTextures(1, &texture->textureId);
	glBindTexture(GL_TEXTURE_2D, texture->textureId);
	
		
	if (texture->format == TEXTURE_GL_RGB ||texture->format == TEXTURE_GL_RGBA)
	{
		glTexParameterf(GL_TEXTURE_2D,GL_GENERATE_MIPMAP, GL_TRUE);
		
        if(texture->format == TEXTURE_GL_RGBA)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data[0]);
        }
        else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->data[0]);

		free(texture->data[0]);
		texture->data[0] = 0;
	}
	else
	{
		glTexParameterf(GL_TEXTURE_2D,GL_GENERATE_MIPMAP, GL_FALSE);
		
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, texture->format, texture->width,texture-> height, 0, texture->dataLength[0], texture->data[0]);
		//Log_Printf("Uploading mipmapp %d w=%d, h=%d, size=%d\n",0,texture->width,texture-> height,texture->dataLength[0]);
		
		mipMapDiv = 2;
		for (i=1; i < texture->numMipmaps; i++,mipMapDiv*=2) 
		{
			glCompressedTexImage2D(GL_TEXTURE_2D, i, texture->format, texture->width/mipMapDiv,texture-> height/mipMapDiv, 0, texture->dataLength[i], texture->data[i]);
		//	Log_Printf("Uploading mipmapp %d w=%d, h=%d, size=%d\n",i,texture->width/mipMapDiv,texture-> height/mipMapDiv,texture->dataLength[i]);
			free(texture->data[i]);
			texture->data[i] = 0;
		}
	}
	
	
	//Using mipMapping to reduce bandwidth consumption
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	
	SCR_CheckErrors("Loading texture",texture->path);
	
	free(texture->dataLength); 
	texture->dataLength = 0;
	free(texture->data);	
	texture->data = 0;
	
	texture->memLocation = TEXT_MEM_LOC_VRAM;
	
	if (texture->file != NULL)
		FS_CloseFile(texture->file);
}

//This is just a debug fiunction
short collisionBoxes[8];
ushort collisionBoxesIndices[6] = {0,1,2,0,2,3};
void RenderCollisionBoxes(void)
{
	int i,j;
	enemy_t* enemy;
	xf_colorless_sprite_t* enemyBullet;
	float alpha = 0.3;
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisable(GL_CULL_FACE);
	glBlendFunc(GL_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glColor4f(1, 1, 1, alpha);
	
	//PLAYER
	for (i=0; i < numPlayers; i++) {
		collisionBoxes[1] =  players[i].ss_boudaries[UP];
		collisionBoxes[0] =  players[i].ss_boudaries[LEFT];
		
		collisionBoxes[3] = players[i].ss_boudaries[DOWN];
		collisionBoxes[2] = players[i].ss_boudaries[LEFT];
		
		collisionBoxes[5] = players[i].ss_boudaries[DOWN];
		collisionBoxes[4] = players[i].ss_boudaries[RIGHT];

		collisionBoxes[7] = players[i].ss_boudaries[UP];
		collisionBoxes[6] = players[i].ss_boudaries[RIGHT];

		
		glVertexPointer (2, GL_SHORT,0,collisionBoxes);
		glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, collisionBoxesIndices);	
	}
	
	//ENEMIES
	glColor4f(1, 0, 0, alpha);
	enemy = ENE_GetFirstEnemy();
	while (enemy != NULL) {
		
		collisionBoxes[1] = enemy->ss_boudaries[UP];
		collisionBoxes[0] = enemy->ss_boudaries[LEFT];
		
		collisionBoxes[3] = enemy->ss_boudaries[DOWN];
		collisionBoxes[2] = enemy->ss_boudaries[LEFT];
		
		collisionBoxes[5] = enemy->ss_boudaries[DOWN];
		collisionBoxes[4] = enemy->ss_boudaries[RIGHT];
		
		collisionBoxes[7] = enemy->ss_boudaries[UP];
		collisionBoxes[6] = enemy->ss_boudaries[RIGHT];
		
		
		glVertexPointer (2, GL_SHORT,0,collisionBoxes);
		glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, collisionBoxesIndices);	
		
		enemy = enemy->next;
	}
	
	//PLAYER BULLETS
	glColor4f(0, 0, 1, alpha);
	for (i=0; i < numPlayers; i++) 
	{
		for (j=0; j < MAX_PLAYER_BULLETS; j++) 
		{
			if (players[i].bullets[j].expirationTime < simulationTime)
				continue;
			
			collisionBoxes[1] = players[i].bullets[j].ss_boudaries[UP];
			collisionBoxes[0] = players[i].bullets[j].ss_boudaries[LEFT];
			
			collisionBoxes[3] = players[i].bullets[j].ss_boudaries[DOWN];
			collisionBoxes[2] = players[i].bullets[j].ss_boudaries[LEFT];
			
			collisionBoxes[5] = players[i].bullets[j].ss_boudaries[DOWN];
			collisionBoxes[4] = players[i].bullets[j].ss_boudaries[RIGHT];
			
			collisionBoxes[7] = players[i].bullets[j].ss_boudaries[UP];
			collisionBoxes[6] = players[i].bullets[j].ss_boudaries[RIGHT];
			
			glVertexPointer (2, GL_SHORT,0,collisionBoxes);
			glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, collisionBoxesIndices);	
		}
	}
	
	//ENEMY BULLETS
	glColor4f(1, 0, 1, alpha);
	enemyBullet = partLib.ss_vertices;
	
	i = 0;
	while( i < partLib.numParticules) 
	{
		collisionBoxes[0] =  enemyBullet->pos[X];
		collisionBoxes[1] =  enemyBullet->pos[Y];
		
		collisionBoxes[2] = enemyBullet->pos[X];
		collisionBoxes[3] = enemyBullet[1].pos[Y];
		
		collisionBoxes[4] = enemyBullet[2].pos[X];
		collisionBoxes[5] = enemyBullet[1].pos[Y];
		
		collisionBoxes[6] = enemyBullet[2].pos[X];
		collisionBoxes[7] = enemyBullet->pos[Y];
		
		
		
		glVertexPointer (2, GL_SHORT,0,collisionBoxes);
		glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, collisionBoxesIndices);	
		
		enemyBullet+=4;
		i++;
	}
	
	
	//glDisable(GL_BLEND);
	glBlendFunc(GL_ALPHA, GL_ONE);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
	glColor4f(1, 1, 1, 1);
	
	if (light.enabled)
		glEnable(GL_LIGHTING);
}


void SetTextureF(unsigned int textureId)
{
	if (lastTextureId == textureId)
		return;
	
	glBindTexture(GL_TEXTURE_2D, textureId);
	STATS_AddTexSwitch();
	
	lastTextureId = textureId;
}





void RenderNormalsF(md5_mesh_t* currentMesh)
{
	
	float normalsVertex[20000];
	float* normalVertex;
	int j;
	
	float vScale = 3;
	
	glDisable(GL_TEXTURE_2D);

	glDisable(GL_LIGHTING);

	glDisableClientState (GL_NORMAL_ARRAY);
	glDisableClientState (GL_TEXTURE_COORD_ARRAY);
	
	
	normalVertex = normalsVertex;
	for(j=0; j < currentMesh->numVertices ; j++)
	{
		*normalVertex++ = currentMesh->vertexArray[j].pos[0];
		*normalVertex++ = currentMesh->vertexArray[j].pos[1];
		*normalVertex++ = currentMesh->vertexArray[j].pos[2];
		
		*normalVertex++ = currentMesh->vertexArray[j].pos[0] + currentMesh->vertexArray[j].normal[0]/DE_SHRT_MAX*vScale;
		*normalVertex++ = currentMesh->vertexArray[j].pos[1] + currentMesh->vertexArray[j].normal[1]/DE_SHRT_MAX*vScale;
		*normalVertex++ = currentMesh->vertexArray[j].pos[2] + currentMesh->vertexArray[j].normal[2]/DE_SHRT_MAX*vScale;
	}
	glColor4f(1, 0, 0, 1);
	glVertexPointer (3, GL_FLOAT,0,normalsVertex);
	glDrawArrays(GL_LINES, 0, currentMesh->numVertices * 2);
	
	
	#ifdef TANGENT_ENABLED
	normalVertex = normalsVertex;
	for(j=0; j < currentMesh->numVertices ; j++)
	{
		*normalVertex++ = currentMesh->vertexArray[j].pos[0];
		*normalVertex++ = currentMesh->vertexArray[j].pos[1];
		*normalVertex++ = currentMesh->vertexArray[j].pos[2];
		
		*normalVertex++ = currentMesh->vertexArray[j].pos[0] + currentMesh->vertexArray[j].tangent[0]/DE_SHRT_MAX*vScale;
		*normalVertex++ = currentMesh->vertexArray[j].pos[1] + currentMesh->vertexArray[j].tangent[1]/DE_SHRT_MAX*vScale;
		*normalVertex++ = currentMesh->vertexArray[j].pos[2] + currentMesh->vertexArray[j].tangent[2]/DE_SHRT_MAX*vScale;
	}
	glColor4f(0, 0, 1, 1);
	glVertexPointer (3, GL_FLOAT,0,normalsVertex);
	glDrawArrays(GL_LINES, 0, currentMesh->numVertices * 2);
	#endif
	
	glColor4f(1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	if (light.enabled)		
		glEnable(GL_LIGHTING);

	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

}



#define TRACE_RENDITION 0
//int traceRenderEntity = 0;
static void RenderEntityF(entity_t* entity)
{

//	Log_Printf("RenderEntityF Player1=%p\n",players[0].entity.material);
//	Log_Printf("RenderEntityF Player2=%p\n",players[1].entity.material);		
	
	glPushMatrix();
	
	//if (traceRenderEntity)
	{
		//entity->matrix[13] = 110;
	//	Log_Printf("[RenderEntityF] entity id=%d\n",entity->uid);
	//	Log_Printf("[RenderEntityF] entity pos=[%.2f,%.2f,%.2f,%.2f]\n",entity->matrix[12],entity->matrix[13],entity->matrix[14],entity->matrix[15]);
		//matrix_print(entity->matrix);
		
	}
	
	glMultMatrixf(entity->matrix);
	
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, entity->material->shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, entity->material->specularColor);
	SetTextureF(entity->material->textures[TEXTURE_DIFFUSE].textureId);
	
	//Disabling blending for now
	/*
	if (entity->material->hasAlpha )
	{
		if (!renderer.isBlending)
		{
			renderer.isBlending = 1;
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			STATS_AddBlendingSwitch();
		}
	}
	else
	{
		if (renderer.isBlending)
		{
			renderer.isBlending = 0;
			glDisable(GL_BLEND);
			STATS_AddBlendingSwitch();
		}
	}
	*/
	
		
	if (entity->model->memLocation == MD5_MEMLOC_VRAM)
	{

		glBindBuffer(GL_ARRAY_BUFFER, entity->model->vboId);
		
        //This is very likely not 64bits friendly if the GPU copies stuff as it is presented.
        
		//glVertexPointer (3, GL_FLOAT, sizeof(vertex_t), (char *)NULL + VERTEX_T_DELTA_TO_POS);	
		//glNormalPointer(GL_SHORT, sizeof(vertex_t), (char *)NULL + VERTEX_T_DELTA_TO_NORMAL);
		//glTexCoordPointer (2, GL_SHORT, sizeof(vertex_t), (char *)NULL + VERTEX_T_DELTA_TO_TEXT);
        
        glVertexPointer  (3, GL_FLOAT, sizeof(vertex_t), (char *)( (char *)(&entity->model->vertexArray->pos)    - ((char*)&entity->model->vertexArray->pos)) );	
		glNormalPointer  (   GL_SHORT, sizeof(vertex_t), (char *)((char *)(&entity->model->vertexArray->normal) - ((char*)&entity->model->vertexArray->pos))  );
		glTexCoordPointer(2, GL_SHORT, sizeof(vertex_t), (char *)((char *)(&entity->model->vertexArray->text)   - ((char*)&entity->model->vertexArray->pos))  );
        
        
    }
	else 
	{
		glTexCoordPointer (2, GL_SHORT, sizeof(vertex_t), entity->model->vertexArray->text);	
		glVertexPointer (3, GL_FLOAT, sizeof(vertex_t), entity->model->vertexArray->pos);
		glNormalPointer(GL_SHORT, sizeof(vertex_t), entity->model->vertexArray->normal);
	}

    
    
	if (entity->usage == ENT_PARTIAL_DRAW)
	{
		
		glDrawElements (GL_TRIANGLES, entity->numIndices, GL_UNSIGNED_SHORT, entity->indices);	
		//glDrawArrays(GL_TRIANGLES,0,3);
		//glDrawElements (GL_TRIANGLES, 0, GL_UNSIGNED_SHORT, entity->indices);	

		
		STATS_AddTriangles(entity->numIndices/3);
	}
	else
	{
		glDrawElements (GL_TRIANGLES, entity->model->numIndices, GL_UNSIGNED_SHORT, entity->model->indices);	
		//glDrawArrays(GL_TRIANGLES,0,3);
		//glDrawElements (GL_TRIANGLES, 0, GL_UNSIGNED_SHORT, entity->model->indices);	

		STATS_AddTriangles(entity->model->numIndices/3);
	}

	
	
	
	
	
	
	
	
	//RenderNormalsF(entity->model);
	
	glPopMatrix();
}



void SetTransparencyF(float alpha)
{
	glColor4f(1, 1, 1, alpha);
	
}


void RenderEntitiesF(void)
{
	
	
	
	
	int i;
	entity_t* entity;
	enemy_t* enemy;
	
	//Log_Printf("Starting rendering frame, t=%d.\n",simulationTime);


	
	
	glMatrixMode(GL_PROJECTION);
	gluPerspective(camera.fov, camera.aspect,camera.zNear, camera.zFar, projectionMatrix);
	glLoadMatrixf(projectionMatrix);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	
	SetupCameraF();
		
	if (light.enabled)
		SetupLightingF();

	
	glDisable(GL_CULL_FACE);
	glDisable(GL_FOG);
	
	//traceRenderEntity = 0;
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	for(i=0; i < numBackgroundEntities; i++)
	{
		entity = &map[i];
		
		if (entity->numIndices == 0)
			continue;
		
		RenderEntityF(entity);
	}
	
	if (engine.fogEnabled && (renderer.props & PROP_FOG) == PROP_FOG )
	{
		glEnable(GL_FOG);
		glFogx(GL_FOG_MODE, GL_LINEAR);						// Fog Mode
		glFogfv(GL_FOG_COLOR,renderer.fogColor);			// Set Fog Color
		glFogf(GL_FOG_DENSITY, renderer.fogDensity);		// How Dense Will The Fog Be (not used if GL_FOG_MODE == GL_LINEAR )
		glHint(GL_FOG_HINT, GL_FASTEST);					// Fog Hint Value
		glFogf(GL_FOG_START, renderer.fogStartAt);			// Fog Start Depth
		glFogf(GL_FOG_END, renderer.fogStopAt);				// Fog End Depth
	}
	
	
	
	for(i=numBackgroundEntities; i < num_map_entities; i++)
	{
		entity = &map[i];
		
		if (entity->numIndices == 0)
			continue;
		
		RenderEntityF(entity);
	}
	glEnable(GL_CULL_FACE);
	
	
	
	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	for (i=0 ; i < numPlayers; i++) 
	{
		//Log_Printf("player[%d].shouldDraw=%d\n",i,players[i].shouldDraw);
		if (players[i].shouldDraw)
			RenderEntityF(&players[i].entity);
	}
	
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	//traceRenderEntity=1;
	enemy = ENE_GetFirstEnemy();
	while (enemy != NULL) 
	{
		entity = &enemy->entity;
		
		
		
		if (enemy->shouldFlicker)
		{
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
			RenderEntityF(entity);
			enemy->shouldFlicker = 0;
			glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		}
		else 
		{
			glColor4f(entity->color[R], entity->color[G], entity->color[B], entity->color[A]);
			RenderEntityF(entity);
		}

		
		enemy = enemy->next;
	} 
	glColor4f(1, 1, 1, 1);
}

void RenderStringF(xf_colorless_sprite_t* vertices,ushort* indices, uint numIndices)
{
	glVertexPointer (2, GL_SHORT, sizeof(xf_colorless_sprite_t), vertices->pos);	
	glTexCoordPointer (2, GL_SHORT,sizeof(xf_colorless_sprite_t), vertices->text);	
	glDrawElements (GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT,indices);
	STATS_AddTriangles(numIndices/3);
}

void GetColorBufferF(uchar* data)
{
	glReadPixels(0,0,renderer.glBuffersDimensions[WIDTH],renderer.glBuffersDimensions[HEIGHT],GL_RGBA, GL_UNSIGNED_BYTE,data);
}

void UpLoadEntityToGPUF(entity_t* entity)
{
	md5_mesh_t* mesh;

	if (entity == NULL || entity->model == NULL)
	{
		Log_Printf("Entity was NULL: No vertices to upload.\n");
		return;
	}
	
	if (entity->model->memLocation == MD5_MEMLOC_VRAM)
		return;		
	
	
	mesh = entity->model;
		
	glGenBuffers(1, &mesh->vboId);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->vboId);
	glBufferData(GL_ARRAY_BUFFER, mesh->numVertices * sizeof(vertex_t), mesh->vertexArray, GL_STATIC_DRAW);
	
    
//#define GENERATE_VIDEO	
#ifndef GENERATE_VIDEO	
	free(mesh->vertexArray);
	mesh->vertexArray = 0;
#else
	Log_Printf("Warning, not freeing mesh after GPU upload.\n");
#endif
	
	mesh->memLocation = MD5_MEMLOC_VRAM;
	
	SCR_CheckErrors("UpLoadVerticesToGPUF","");
	
}

void RenderPlayersBulletsF(void)
{
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);	
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	SetTextureF(bulletConfig.bulletTexture.textureId);
	
	//Player bullets
	glVertexPointer(  2, GL_SHORT,  sizeof(xf_colorless_sprite_t), pBulletVertices->pos);
	glTexCoordPointer(2, GL_SHORT,  sizeof(xf_colorless_sprite_t), pBulletVertices->text);			
	glDrawElements (GL_TRIANGLES, numPBulletsIndices, GL_UNSIGNED_SHORT,bulletIndices);
	STATS_AddTriangles(numPBulletsIndices/3);
	
	//Also render enemy bullets
//	glColor4f(1, 1, 1, 1);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	
	glVertexPointer(  2, GL_SHORT,  sizeof(xf_colorless_sprite_t), partLib.ss_vertices[0].pos);
	glTexCoordPointer(2, GL_SHORT,  sizeof(xf_colorless_sprite_t), partLib.ss_vertices[0].text);			
	glDrawElements (GL_TRIANGLES, partLib.num_indices, GL_UNSIGNED_SHORT,partLib.indices);
	STATS_AddTriangles(partLib.num_indices/3);
}

void RenderFXSpritesF(void)
{
	int i,j;
	
	/*
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
	SetTextureF(smokeTexture.textureId);
	if (numSmokeIndices != 0)
	{
		glVertexPointer(  2, GL_SHORT,  sizeof(xf_colorless_sprite_t), smokeVertices->pos);
		glTexCoordPointer(2, GL_SHORT,  sizeof(xf_colorless_sprite_t), smokeVertices->text);
		//glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(xf_sprite_t), smokeVertices->color);
		glDrawElements (GL_TRIANGLES, numSmokeIndices, GL_UNSIGNED_SHORT,smokeIndices);
		STATS_AddTriangles(numSmokeIndices/3);
	}
	
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	 */
	
	
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	SetTextureF(smokeTexture.textureId);
	if (numSmokeIndices != 0)
	{
		glVertexPointer(  2, GL_SHORT,  sizeof(xf_colorless_sprite_t), smokeVertices->pos);
		glTexCoordPointer(2, GL_SHORT,  sizeof(xf_colorless_sprite_t), smokeVertices->text);
		//glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(xf_sprite_t), smokeVertices->color);
		glDrawElements (GL_TRIANGLES, numSmokeIndices, GL_UNSIGNED_SHORT,smokeIndices);
		STATS_AddTriangles(numSmokeIndices/3);
	}
	
	
	
	SetTextureF(ghostTexture.textureId);
	for(i=0 ; i <numPlayers ; i++)
	{
		/*
		if (i==0)
			glColor4f(0.8f, 0.8f, 1.0f, 0.9f);
		else {
			glColor4f(1.0f, 0.4f, 0.4f, 0.9f);
		}
*/
		
		
		for (j=0; j< GHOSTS_NUM; j++) 
		{
			if (players[i].ghosts[j].timeCounter >= GHOST_TTL_MS)
				continue;
			
			
			//vertices = &players[i].ghosts[j].wayPoints[players[i].ghosts[j].startVertexArray];
			glVertexPointer(  2, GL_SHORT,  sizeof(xf_colorless_sprite_t), players[i].ghosts[j].wayPoints[players[i].ghosts[j].startVertexArray].pos);
			glTexCoordPointer(2, GL_SHORT,  sizeof(xf_colorless_sprite_t), players[i].ghosts[j].wayPoints[players[i].ghosts[j].startVertexArray].text);
			//glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(xf_sprite_t), vertices->color);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, players[i].ghosts[j].lengthVertexArray);
			STATS_AddTriangles((players[i].ghosts[j].lengthVertexArray/2));
		}
	}

	
	

	glEnableClientState(GL_COLOR_ARRAY);
	
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	
	//Render all particules
	if (numParticulesIndices != 0)
	{
		
		SetTextureF(bulletConfig.bulletTexture.textureId);
		glVertexPointer(  2, GL_SHORT,  sizeof(xf_sprite_t), particuleVertices->pos);
		glTexCoordPointer(2, GL_SHORT,  sizeof(xf_sprite_t), particuleVertices->text);
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(xf_sprite_t), particuleVertices->color);
		glDrawElements (GL_TRIANGLES, numParticulesIndices, GL_UNSIGNED_SHORT,particuleIndices);
		STATS_AddTriangles(numParticulesIndices/3);
	}
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	
	
	
	//Render all explosions 
	if (numExplosionIndices != 0)
	{
		SetTextureF(explosionTexture.textureId);
		glVertexPointer(  2, GL_SHORT,  sizeof(xf_sprite_t), explosionVertices->pos);
		glTexCoordPointer(2, GL_SHORT,  sizeof(xf_sprite_t), explosionVertices->text);
		//Log_Printf("REMOVE COLOR INDICES EXPLOSIONS RenderFXSpritesF !!!! \n");
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(xf_sprite_t), explosionVertices->color);
		glDrawElements (GL_TRIANGLES, numExplosionIndices, GL_UNSIGNED_SHORT,explosionIndices);
		STATS_AddTriangles(numExplosionIndices/3);
	}
	
	
	//Render enemy FXs
	SetTextureF(bulletConfig.bulletTexture.textureId);
	glVertexPointer(  2, GL_SHORT,  sizeof(xf_sprite_t), enFxLib.ss_vertices[0].pos);
	glTexCoordPointer(2, GL_SHORT,  sizeof(xf_sprite_t), enFxLib.ss_vertices[0].text);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(xf_sprite_t), enFxLib.ss_vertices[0].color);
	glDrawElements (GL_TRIANGLES, enFxLib.num_indices, GL_UNSIGNED_SHORT,enFxLib.indices);
	STATS_AddTriangles(enFxLib.num_indices/3);
	//Log_Printf("enFxLib.num_indices=%d\n",enFxLib.num_indices);
	
	
	
#ifdef RENDER_COLL_BOXEX	
	RenderCollisionBoxes();
#endif
	
	//glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	
	
	glDisableClientState(GL_COLOR_ARRAY);

	
	
}

void DrawControlsF(void)
{
	if (engine.controlMode == CONTROL_MODE_SWIP)
		return;
	
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_TEXTURE_2D);
	
	
	//glBindBuffer(GL_ARRAY_BUFFER, controlVBOId);

	
	glVertexPointer (2, GL_SHORT, sizeof(xf_textureless_sprite_t),controlVertices[0].pos);// (char *)NULL + 0);	
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(xf_textureless_sprite_t),controlVertices[0].color);// (char *)NULL + 4);
	glDrawElements (GL_TRIANGLE_STRIP, controlNumIndices, GL_UNSIGNED_SHORT,controlIndices);
	STATS_AddTriangles(controlNumIndices/2);
		
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	
	
}


void FreeGPUTextureF(texture_t* texture)
{
	glDeleteTextures(1, &texture->textureId);
	texture->textureId = 0;
}


void FreeGPUBufferF(uint bufferId)
{
	glDeleteBuffers(1,&bufferId);
	
}

uint UploadVerticesToGPUF(void* vertices, uint mem_size)
{
	uint vboId;
	
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, mem_size, vertices, GL_STATIC_DRAW);
	
	return vboId;
}

void StartCleanFrameF(void)
{
	//glClear(GL_COLOR_BUFFER_BIT );
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixf(textureMatrix);
	
	//This is disabled in set3D
	//glEnable ( GL_COLOR_MATERIAL ) ;
}

void RenderColorlessSpritesF(xf_colorless_sprite_t* vertices, ushort numIndices, ushort* indices)
{
	glVertexPointer(  2, GL_SHORT,  sizeof(xf_colorless_sprite_t), vertices->pos);
	glTexCoordPointer(2, GL_SHORT,  sizeof(xf_colorless_sprite_t), vertices->text);
	glDrawElements (GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT,indices);
	STATS_AddTriangles(numIndices/2);
}




void FadeScreenF(float alpha)
{

	fadeVertices[0].color[A] = alpha * 255;
	fadeVertices[1].color[A] = alpha * 255;
	fadeVertices[2].color[A] = alpha * 255;
	fadeVertices[3].color[A] = alpha * 255;
	
	
	glDisable(GL_TEXTURE_2D);
	glEnableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glVertexPointer(  2, GL_SHORT,  sizeof(xf_textureless_sprite_t), fadeVertices[0].pos);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(xf_textureless_sprite_t), fadeVertices[0].color);
	glDrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT,fadeIndices);
	STATS_AddTriangles(6/2);
	
	
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_TEXTURE_2D);
	 
	
	
}

void SetMaterialTextureBlendingF(char modulate)
{
	if (modulate)
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	else {
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	}

}

int IsTextureCompressionSupportedF(int type){
    return supportedCompressionFormatF & type;
}

void RefreshViewPortF()
{
    glViewport(renderer.viewPortDimensions[VP_X],
			   renderer.viewPortDimensions[VP_Y], 
			   renderer.viewPortDimensions[VP_WIDTH], 
			   renderer.viewPortDimensions[VP_HEIGHT]);
}

void initFixedRenderer(renderer_t* renderer)
{
	GLenum err;
	char *extensionsList ;
    
	//Log_Printf("[initFixedRenderer] has a nnnasty hack");
	
	renderer->type = GL_11_RENDERER ;
	
	//renderer->supportBumpMapping = 0;
	renderer->props = 0;
	
	
	
	renderer->Set3D = Set3DF;
	renderer->StopRendition = StopRenditionF;
	renderer->SetTexture = SetTextureF;
	renderer->RenderEntities = RenderEntitiesF;
	renderer->UpLoadTextureToGpu = UpLoadTextureToGPUF;
	renderer->UpLoadEntityToGPU = UpLoadEntityToGPUF;
	renderer->Set2D = Set2DF;
	renderer->RenderPlayersBullets = RenderPlayersBulletsF ;
	renderer->RenderString = RenderStringF;
	renderer->GetColorBuffer = GetColorBufferF;
	
	renderer->RenderFXSprites = RenderFXSpritesF;
	renderer->DrawControls = DrawControlsF;
	
	renderer->FreeGPUTexture = FreeGPUTextureF;
	renderer->FreeGPUBuffer = FreeGPUBufferF;
	
	renderer->UploadVerticesToGPU = UploadVerticesToGPUF;
	renderer->StartCleanFrame = StartCleanFrameF;
	renderer->RenderColorlessSprites = RenderColorlessSpritesF;
	renderer->FadeScreen = FadeScreenF;
	renderer->SetMaterialTextureBlending = SetMaterialTextureBlendingF;
	renderer->SetTransparency = SetTransparencyF;
	renderer->IsTextureCompressionSupported = IsTextureCompressionSupportedF;
    renderer->RefreshViewPort = RefreshViewPortF;
	
	glViewport(renderer->viewPortDimensions[VP_X],
			   renderer->viewPortDimensions[VP_Y], 
			   renderer->viewPortDimensions[VP_WIDTH], 
			   renderer->viewPortDimensions[VP_HEIGHT]);
	
	
	
	glEnable(GL_TEXTURE_2D);

		
	
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	
	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	
	
	
	glClearColor(0, 0, 0,1.0f);
	glColor4f(1.0f, 1.0f, 1.0f,1.0f);
	
	glMatrixMode(GL_TEXTURE);
	glLoadMatrixf(textureMatrix);
		
    
    
    //We need to check what texture compression method is supported.
    extensionsList = (char *) glGetString(GL_EXTENSIONS);
    if (strstr(extensionsList,"GL_IMG_texture_compression_pvrtc"))
        supportedCompressionFormatF |= TEXTURE_FORMAT_PVRTC ;
        
        
    
    
	err = glGetError();
	if (err != GL_NO_ERROR)
		Log_Printf("Error initing 1.1: glError: 0x%04X", err);
}

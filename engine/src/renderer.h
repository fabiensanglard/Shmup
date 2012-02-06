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
 *  renderer.h
 *  dEngine
 *
 *  Created by fabien sanglard on 09/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */



#ifndef DE_RENDERER
#define DE_RENDERER


 #ifdef __cplusplus
 extern "C" {
 #endif

#define GL_11_RENDERER 0
#define GL_20_RENDERER 1

// The following defines are used in order to test a bitvector for supported texture compression formats
#define TEXTURE_FORMAT_PNG    0
#define TEXTURE_FORMAT_PVRTC  1     
#define TEXTURE_FORMAT_ETC1   2
#define TEXTURE_FORMAT_ATITC  3
#define TEXTURE_FORMAT_S3TC   4     

     
#define SHADOW_TYPE_NORMAL 0
#define SHADOW_TYPE_DISABLED 1
#define SHADOW_TYPE_VSM 2

#include "config.h"
#include "texture.h"
#include "matrix.h"
#include "math.h"
#include "entities.h"
#include "fx.h"
#include "commands.h"
#include "math.h"
#include "player.h"
	 
//extern int renderWidth;
//extern int renderHeight;
//extern vec2short_t screenSize;

//Calculate the ratio, the original ratio being 320/480
//extern vec2_t viewPortCentering;
#define VP_X 0
#define VP_Y 1	 
#define VP_WIDTH 2	 	 
#define VP_HEIGHT 3	 	 	 
	 
	 
#define TEXT_NOT_CENTERED 0
#define TEXT_CENTERED 1

typedef struct diverSpriteLib_t
{
	xf_colorless_sprite_t vertices[PLAYER_NUM_LIVES*4+NUM_BUTTONS*4];
	ushort numVertices;
	ushort numIndices;
		 
} diverSpriteLib_t;
extern diverSpriteLib_t diverSpriteLib;

typedef struct renderer_t 
{
	int viewPortDimensions[4];
	
	int glBuffersDimensions[2];
	
	
	uint type;
	
	// Define property 
	uchar props;
	//uint supportBumpMapping;
	//uint shadowType;
	uchar isRenderingShadow;
	uchar isBlending;
	
	uint mainFramebufferId;
	
	uint statsEnabled;
	uint materialQuality;
	
    //This is useless
	//float resolution;

	//FOG enabled/diabled via props
	vec4_t fogColor;
	float fogDensity; //Only used for non linear fogs
	uint fogStartAt;
	uint fogStopAt;
	
	uchar enabled;
	
	void (*Set3D)(void); 
	void (*StopRendition  )(void); 
//	void (*PushMatrix)(void);
//	void (*PopMatrix)(void);
	void (*SetTexture)(unsigned int);
	void (*RenderEntities)(void);
	void (*UpLoadTextureToGpu)(texture_t* texture);
	void (*FreeGPUTexture)(texture_t* texture);
	void (*Set2D)(void);
	void (*RenderPlayersBullets)(void);
	void (*RenderFXSprites)(void);
	
	void (*RenderString)(xf_colorless_sprite_t* vertices,ushort* indices, uint numIndices);
	void (*GetColorBuffer)(uchar* data);
	
	void (*UpLoadEntityToGPU)(entity_t* entity);
	uint (*UploadVerticesToGPU)(void* vertices, uint mem_size);
	
	void (*FreeGPUBuffer)(uint texture);
	
	void (*DrawControls)(void);
	void (*StartCleanFrame)(void);
	
	void (*RenderColorlessSprites)(xf_colorless_sprite_t* vertices, ushort numIndices, ushort* indices);

	
	void (*FadeScreen)(float alpha);
	void (*SetMaterialTextureBlending)(char modulate);
	void (*SetTransparency)(float alpha);
	
	int (*IsTextureCompressionSupported)(int type);
	void (*RefreshViewPort)();
          
} renderer_t;

extern renderer_t renderer;

void SCR_Init(void);
void SCR_BindMethods(int rendererType);
void SCR_RenderFrame(void);
void SCR_GetColorBuffer(uchar* data);
void gluPerspective(float fovy,float aspect, float zNear, float zFar,matrix_t projectionMatrix);
void gluLookAt(  vec3_t vEye,  vec3_t vLookat, vec3_t vUp,matrix_t fModelView );

//Fading
void SCR_SetFadeScreenCoo(vec2short_t upperLeft, vec2short_t upperRight, vec2short_t lowerRight, vec2short_t lowerLeft);
void SCR_SetFadeFullScreen(void);	 
	 
void SCR_StartConvertText(void);	 
void SCR_ConvertTextToVertices(const char* string, float size, short ss_cooX, short ss_cooY, uchar centered);
void SCR_RenderText(void);	 
	 
     void SRC_OnResizeScreen(int width, int height);     
     
extern ushort fadeIndices[6] ;
extern xf_textureless_sprite_t fadeVertices[4];
	 
#define MAX_NUM_TEXT_VERTICES 4096	 
extern xf_colorless_sprite_t scr_TextVertices[MAX_NUM_TEXT_VERTICES];
extern ushort scr_TextIndices[MAX_NUM_TEXT_VERTICES/4*6];
extern ushort scr_TextNumIndices;	 
	 

extern texture_t scrFont;	 
	 
#ifdef __cplusplus
 }
 #endif

#endif

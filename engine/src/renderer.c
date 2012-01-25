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
 *  renderer.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "renderer.h"
#include <math.h>
#include "renderer_fixed.h"
#include "renderer_progr.h"
#include "stats.h"
#include "timer.h"
#include "fx.h"
#include "dEngine.h"
#include "menu.h"
#include "titles.h"
#include "text.h"
#include "enemy_particules.h"

//int renderWidth;
//int renderHeight;
float renderAspect;
//vec2short_t screenSize;
//vec2_t viewPortCentering;

renderer_t renderer;

//2D text and stats
texture_t scrFont;	
#define STATS_FONT_PATH "/data/menu/font.png"

ushort indicesString[256]  ;

ushort fadeIndices[6] = {0,1,2,1,2,3};
xf_textureless_sprite_t fadeVertices[4];


void SCR_SetFadeScreenCoo(vec2short_t upperLeft, vec2short_t upperRight, vec2short_t lowerRight, vec2short_t lowerLeft)
{
	
	fadeVertices[0].pos[X] = upperLeft[X];
	fadeVertices[0].pos[Y] = upperLeft[Y];
	
	fadeVertices[1].pos[X] = lowerLeft[X];
	fadeVertices[1].pos[Y] = lowerLeft[Y];
	
	
	fadeVertices[2].pos[X] = upperRight[X];
	fadeVertices[2].pos[Y] = upperRight[Y];
	
	fadeVertices[3].pos[X] = lowerRight[X];
	fadeVertices[3].pos[Y] = lowerRight[Y];
	 
}

void SCR_SetFadeFullScreen(void)
{
	vec2short_t upperLeft  = {- SS_W,   SS_H};
	vec2short_t upperRight = {  SS_W,   SS_H};
	vec2short_t lowerRight = {  SS_W, - SS_H};
	vec2short_t lowerLeft  = {- SS_W, - SS_H};
	
	SCR_SetFadeScreenCoo(upperLeft,upperRight,lowerRight,lowerLeft);
}


void SCR_Init(void)
{
	int i,j;
	float ratio;
	
	//renderWidth =  320 * renderer.resolution;
	//renderHeight = 480 * renderer.resolution;
	
	//Centering rendition area with viewPort
	//vector2Clear(viewPortCentering);
	
	renderer.viewPortDimensions[VP_WIDTH] = 320.0f * renderer.resolution ;
	renderer.viewPortDimensions[VP_HEIGHT] = 480.0f * renderer.resolution ;
	
	if (renderer.viewPortDimensions[VP_WIDTH] > renderer.glBuffersDimensions[WIDTH])
	{
		ratio = renderer.glBuffersDimensions[WIDTH] / renderer.viewPortDimensions[VP_WIDTH];
		renderer.viewPortDimensions[VP_WIDTH]  = renderer.glBuffersDimensions[WIDTH] * ratio;
		renderer.viewPortDimensions[VP_HEIGHT] = renderer.glBuffersDimensions[HEIGHT]* ratio;		
	}
	
	if (renderer.viewPortDimensions[VP_HEIGHT] > renderer.glBuffersDimensions[HEIGHT])
	{
		ratio = renderer.glBuffersDimensions[HEIGHT] / renderer.viewPortDimensions[VP_HEIGHT];
		renderer.viewPortDimensions[VP_WIDTH]  = renderer.glBuffersDimensions[WIDTH] * ratio;
		renderer.viewPortDimensions[VP_HEIGHT] = renderer.glBuffersDimensions[HEIGHT]* ratio;		
		
	}
	
	
	renderer.viewPortDimensions[VP_X] = (renderer.glBuffersDimensions[WIDTH] - renderer.viewPortDimensions[VP_WIDTH]) / 2;
	renderer.viewPortDimensions[VP_Y] = (renderer.glBuffersDimensions[HEIGHT] - renderer.viewPortDimensions[VP_HEIGHT]) / 2;
	
	
	
	
	for (i=0; i < 4; i++) 
	for (j=0; j < 4; j++) 
		fadeVertices[i].color[j] = 0;
	
	/*
	fadeVertices[0].color[R] = 0;
	fadeVertices[0].color[G] = 0;
	fadeVertices[0].color[B] = 0;
	
	fadeVertices[1].color[R] = 0;
	fadeVertices[1].color[G] = 0;
	fadeVertices[1].color[B] = 0;
	fadeVertices[2].color[R] = 0;
	fadeVertices[2].color[G] = 0;
	fadeVertices[2].color[B] = 0;
	fadeVertices[3].color[R] = 0;
	fadeVertices[3].color[G] = 0;
	fadeVertices[3].color[B] = 0;
	*/
	SCR_SetFadeFullScreen();
	
	renderer.enabled = 1;
	
}




void SCR_BindMethods(int rendererType)
{
	if (rendererType == GL_11_RENDERER)
	{
		printf("[Renderer] Running in mode OpenGL ES 1.1\n");
		initFixedRenderer(&renderer);
	}
	
	if (rendererType == GL_20_RENDERER)
	{
		printf("[Renderer] Running in mode OpenGL ES 2.0\n"); 
		initProgrRenderer(&renderer);
	}
	
	scrFont.path = malloc(sizeof(char)*(strlen(STATS_FONT_PATH)+1));
	strcpy(scrFont.path,STATS_FONT_PATH);
	TEX_MakeStaticAvailable(&scrFont);
}



void SCR_RenderFrame(void)
{
	
	if (!renderer.enabled)
		return;
	
	STATS_Begin();
		
	renderer.Set3D();
	
	renderer.RenderEntities();
	
	
	renderer.Set2D();
	
	renderer.RenderPlayersBullets();

//	if (enFxLib.num_indices != 0)
//		printf("fsdf\n");
	// enemy particules + FX + players lives counter
	
	
	
	
	renderer.RenderFXSprites();

	
	
	TITLE_Render();
	
	//Here we render the player pointer because the last call maybe text, we save a texture switch for DYN_TEXT_Render STATS_Render
	PL_RenderPlayerPointers();
	
	DYN_TEXT_Render();
	
	STATS_Render();
	
	
	
	if (engine.controlVisible)
		renderer.DrawControls();
	
	
	
	renderer.StopRendition();
	
	
	
	if (engine.menuVisible)
		MENU_Render();
	
	
	
}


void gluPerspective(float fovy, float aspect, float zNear, float zFar,matrix_t projectionMatrix)
{
	float f  = (float)(1 / tan(fovy*DEG_TO_RAD/2));	
	
	
	projectionMatrix[0]= f/aspect;	projectionMatrix[4]= 0;	projectionMatrix[ 8]= 0;								projectionMatrix[12]= 0;
	projectionMatrix[1]= 0; 		projectionMatrix[5]= f;	projectionMatrix[ 9]= 0;								projectionMatrix[13]= 0;
	projectionMatrix[2]= 0;			projectionMatrix[6]= 0;	projectionMatrix[10]=(zFar+zNear)/(zNear-zFar) ;		projectionMatrix[14]= 2*(zFar*zNear)/(zNear-zFar);
	projectionMatrix[3]= 0;			projectionMatrix[7]=0;	projectionMatrix[11]=-1;								projectionMatrix[15]= 0;
}





void gluLookAt(  vec3_t vEye,  vec3_t vLookat, vec3_t vUp ,matrix_t fModelView)
{
	vec3_t vN,vU,vV;
	
    // determine the new n
    vectorSubtract(vEye,vLookat,vN);
	
    // determine the new u by crossing with the up vector
    vectorCrossProduct(vUp, vN, vU) ;
	
    // normalize both the u and n vectors
    normalize(vU) ; 
	normalize(vN);
	
    // determine v by crossing n and u
    vectorCrossProduct(vN,vU,vV);
	
    // create a model view matrix
	fModelView[0] = vU[0];					fModelView[4] = vU[1];					fModelView[8] = vU[2];					fModelView[12] = - DotProduct(vEye,vU); 
	fModelView[1] = vV[0];					fModelView[5] = vV[1];					fModelView[9] = vV[2];					fModelView[13] = - DotProduct(vEye,vV);
	fModelView[2] = vN[0];					fModelView[6] = vN[1];					fModelView[10]= vN[2];					fModelView[14]=  - DotProduct(vEye,vN);
	fModelView[3]=	0.0f;					fModelView[7]= 0.0f;					fModelView[11]= 0.0f;					fModelView[15]= 1.0f;

}


void SCR_GetColorBuffer(uchar* data)
{
	renderer.GetColorBuffer(data);
}




xf_colorless_sprite_t scr_TextVertices[MAX_NUM_TEXT_VERTICES];
ushort scr_TextIndices[MAX_NUM_TEXT_VERTICES/4*6];

xf_colorless_sprite_t* scr_p_TextVertices;
ushort* scr_p_TextIndices;

ushort scr_TextNumIndices;
ushort scr_TextNumVertices;

void SCR_StartConvertText(void)
{
	scr_p_TextVertices = scr_TextVertices;
	scr_p_TextIndices = scr_TextIndices;
	
	scr_TextNumVertices = 0;
	scr_TextNumIndices = 0;
}

void SCR_ConvertTextToVertices(const char* string, float size, short ss_cooX, short ss_cooY, uchar centered)
{
	const char* currentChar;
	int i;
	
	short textCoo[2];
	ushort charWidth = size* SS_W  / 40 ;	//40 characters per line at size 1
	ushort charHeight = charWidth;
	int stringLength ;
	ushort charSpace = charWidth;
	
	
	stringLength = strlen(string);
	
	//Check that we won't overflow the vertices rendition buffer
	if ( scr_TextNumVertices + stringLength * 4 > MAX_NUM_TEXT_VERTICES)
	{
		printf("[SCR_ConvertTextToVertices] Unable to render string, preventing buffer overflow.\n");
		return;
	}
	
	currentChar = string;
	
	if (centered)
	{
		//Adjust ss_cooX and ss_cooY so text is centered.
		ss_cooX -= stringLength*charSpace/2;
	}
	
	
	// 0 2
	// 1 3
	for (i=0; i < stringLength; i++) 
	{
		//	printf("character %c=%d\n",*currentChar,*currentChar);
		textCoo[X] = *currentChar & 15;
		textCoo[Y] = *currentChar >> 4;
		
		scr_p_TextVertices[0].pos[X] = ss_cooX-charWidth;
		scr_p_TextVertices[0].pos[Y] = ss_cooY+charHeight;
		scr_p_TextVertices[0].text[X] = textCoo[X] * SHRT_MAX/16;
		scr_p_TextVertices[0].text[Y] = textCoo[Y] * SHRT_MAX/16;
		
		scr_p_TextVertices[1].pos[X] = ss_cooX-charWidth;
		scr_p_TextVertices[1].pos[Y] = ss_cooY-charHeight;
		scr_p_TextVertices[1].text[X] = textCoo[X] * SHRT_MAX/16;
		scr_p_TextVertices[1].text[Y] = textCoo[Y] * SHRT_MAX/16 + SHRT_MAX/16;
		
		scr_p_TextVertices[2].pos[X] = ss_cooX+charWidth;
		scr_p_TextVertices[2].pos[Y] = ss_cooY+charHeight;
		scr_p_TextVertices[2].text[X] = textCoo[X] * SHRT_MAX/16 +SHRT_MAX/16;
		scr_p_TextVertices[2].text[Y] = textCoo[Y] * SHRT_MAX/16;
		
		scr_p_TextVertices[3].pos[X] = ss_cooX + charWidth;
		scr_p_TextVertices[3].pos[Y] = ss_cooY - charHeight;
		scr_p_TextVertices[3].text[X] = textCoo[X] * SHRT_MAX/16 + SHRT_MAX/16;
		scr_p_TextVertices[3].text[Y] = textCoo[Y] * SHRT_MAX/16 + SHRT_MAX/16;
		
		scr_p_TextIndices[0] = scr_TextNumVertices+0;
		scr_p_TextIndices[1] = scr_TextNumVertices+1;
		scr_p_TextIndices[2] = scr_TextNumVertices+2;
		scr_p_TextIndices[3] = scr_TextNumVertices+1;
		scr_p_TextIndices[4] = scr_TextNumVertices+2;
		scr_p_TextIndices[5] = scr_TextNumVertices+3;
		
		currentChar++;
		
		scr_p_TextVertices += 4;
		scr_TextNumVertices += 4;
		
		scr_TextNumIndices += 6;
		scr_p_TextIndices += 6;
		
		ss_cooX += charSpace;
		
		
	}	
}

void SCR_RenderText(void)
{
	renderer.SetTexture(scrFont.textureId);
	renderer.RenderColorlessSprites(scr_TextVertices,scr_TextNumIndices,scr_TextIndices);

}
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
 *  titles.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-08-13.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "titles.h"
#include "globals.h"
#include "renderer.h"
#include "timer.h"
#include <limits.h>
#include "text.h"
#include "dEngine.h"


uchar title_mode = MODE_UNKNOWN;

#define NUM_DIM_TITLE_VERTICES 4
#define NUM_DIM_TITLE_INDICES 6
xf_colorless_sprite_t* v_TitleBackground;
ushort* i_TitleBackground;

xf_colorless_sprite_t v_CompletedSprite[4];
ushort i_CompletedSprite[6] = {0,1,2,0,2,3};


float initTimeRemaining;
int timeRemaining;
texture_t titleTexture;

vec2short_t upperLeft ;
vec2short_t upperRight;
vec2short_t lowerRight;
vec2short_t lowerLeft ;

void TITLE_AllocRessources(void)
{
	upperLeft[X] = -SS_COO_SYST_WIDTH  ;
	upperLeft[Y] =  SS_COO_SYST_HEIGHT ;
	
	upperRight[X] = SS_COO_SYST_WIDTH;
	upperRight[Y] = SS_COO_SYST_HEIGHT;
	
	lowerRight[X] = SS_COO_SYST_WIDTH;
	lowerRight[Y] = (480-450);
	
	lowerLeft[X] = -SS_COO_SYST_WIDTH;
	lowerLeft[Y] = (480-300);
	
	
	v_TitleBackground = malloc(NUM_DIM_TITLE_VERTICES * sizeof(xf_colorless_sprite_t));
	i_TitleBackground = malloc(NUM_DIM_TITLE_INDICES * sizeof(ushort));
	
	//  0  3
	//  1  2
	//Define the title vertives and indices
	i_TitleBackground[0] = 0;
	i_TitleBackground[1] = 1;
	i_TitleBackground[2] = 2;
	i_TitleBackground[3] = 0;
	i_TitleBackground[4] = 2;
	i_TitleBackground[5] = 3;
	
	v_TitleBackground[0].pos[X] = -SS_COO_SYST_WIDTH;
	v_TitleBackground[0].pos[Y] = SS_COO_SYST_HEIGHT;
	v_TitleBackground[0].text[U] = 0;
	v_TitleBackground[0].text[V] = 0;
	
	v_TitleBackground[1].pos[X] = -SS_COO_SYST_WIDTH;
	v_TitleBackground[1].pos[Y] = (480-280);
	v_TitleBackground[1].text[U] = 0;
	v_TitleBackground[1].text[V] = SHRT_MAX;
	
	v_TitleBackground[2].pos[X] = SS_COO_SYST_WIDTH;
	v_TitleBackground[2].pos[Y] = (480-280);
	v_TitleBackground[2].text[U] = SHRT_MAX;
	v_TitleBackground[2].text[V] = SHRT_MAX;	
	
	v_TitleBackground[3].pos[X] = SS_COO_SYST_WIDTH;
	v_TitleBackground[3].pos[Y] = SS_COO_SYST_HEIGHT;
	v_TitleBackground[3].text[U] = SHRT_MAX;
	v_TitleBackground[3].text[V] = 0;
	
	/*
	for(int i=0 ; i < 6 ; i++)
	{
		;//printf("CREATE title i=%hd v[x]=%hd v[y]%hd\n",i,v_TitleBackground[i].pos[X],v_TitleBackground[i].pos[Y]);
	}
	*/
	memset(&titleTexture,0,sizeof(texture_t));
	
	//Initilize completed sprite
	//  0  3
	//  1  2
	

	
#define COMPLETED_POSX 0.55
#define COMPLETED_POSY 0.47
#define COMPLETED_WIDTH 0.3
#define COMPLETED_HEIGHT 0.2
//Completed text
/*	
#define COMPLETED_TEXT_POSX 0
#define COMPLETED_TEXT_POSY (88/(float)128*SHRT_MAX)
#define COMPLETED_TEXT_WIDTH (128/(float)256*SHRT_MAX)
#define COMPLETED_TEXT_HEIGHT (40/(float)128*SHRT_MAX)	
*/
// check box
#define COMPLETED_TEXT_POSX (64/(float)256*SHRT_MAX)
#define COMPLETED_TEXT_POSY (0/(float)128*SHRT_MAX)
#define COMPLETED_TEXT_WIDTH (64/(float)256*SHRT_MAX)
#define COMPLETED_TEXT_HEIGHT (64/(float)128*SHRT_MAX)	

	
	v_CompletedSprite[0].pos[X] = (COMPLETED_POSX - COMPLETED_WIDTH/2) *SS_COO_SYST_WIDTH;
	v_CompletedSprite[0].pos[Y] = (COMPLETED_POSY + COMPLETED_HEIGHT/2)*SS_COO_SYST_HEIGHT;
	v_CompletedSprite[0].text[U] = COMPLETED_TEXT_POSX;
	v_CompletedSprite[0].text[V] = COMPLETED_TEXT_POSY;
	
	v_CompletedSprite[1].pos[X] = (COMPLETED_POSX - COMPLETED_WIDTH/2) *SS_COO_SYST_WIDTH;
	v_CompletedSprite[1].pos[Y] = (COMPLETED_POSY - COMPLETED_HEIGHT/2)*SS_COO_SYST_HEIGHT;
	v_CompletedSprite[1].text[U] = COMPLETED_TEXT_POSX;
	v_CompletedSprite[1].text[V] = COMPLETED_TEXT_POSY+COMPLETED_TEXT_HEIGHT;	
	
	v_CompletedSprite[2].pos[X] = (COMPLETED_POSX + COMPLETED_WIDTH/2) *SS_COO_SYST_WIDTH;
	v_CompletedSprite[2].pos[Y] = (COMPLETED_POSY - COMPLETED_HEIGHT/2)*SS_COO_SYST_HEIGHT;
	v_CompletedSprite[2].text[U] = COMPLETED_TEXT_POSX + COMPLETED_TEXT_WIDTH;
	v_CompletedSprite[2].text[V] = COMPLETED_TEXT_POSY + COMPLETED_TEXT_HEIGHT;	
	
	v_CompletedSprite[3].pos[X] = (COMPLETED_POSX + COMPLETED_WIDTH/2) *SS_COO_SYST_WIDTH;
	v_CompletedSprite[3].pos[Y] = (COMPLETED_POSY + COMPLETED_HEIGHT/2)*SS_COO_SYST_HEIGHT;
	v_CompletedSprite[3].text[U] = COMPLETED_TEXT_POSX + COMPLETED_TEXT_WIDTH;
	v_CompletedSprite[3].text[V] = COMPLETED_TEXT_POSY;	
}

void TITLE_Update(void)
{
	if (timeRemaining <= 0)
	{
		return;
	}
	
	timeRemaining -= timediff;
	
	if (timeRemaining <=0)
	{
		renderer.SetTransparency(1);
		
		if (title_mode == MODE_EPILOG)
		{
			title_mode = MODE_UNKNOWN;
			dEngine_GoToNextScene();
		}
	
	}
	
}

void TITLE_Clear(void){
    timeRemaining = 0;
    title_mode = MODE_UNKNOWN;
}

void TITLE_Show_prolog(int tr)
{
	timeRemaining =  tr;
	initTimeRemaining = tr;
	title_mode = MODE_PROLOG;
	SCR_SetFadeScreenCoo(upperLeft,upperRight,lowerRight,lowerLeft);
	entitiesAttachedToCamera = 0;
}

void TITLE_Show_epilog(int tr)
{
	timeRemaining =  tr;
	initTimeRemaining = tr;
	title_mode = MODE_EPILOG;
	SCR_SetFadeScreenCoo(upperLeft,upperRight,lowerRight,lowerLeft);
}

void TITLE_RenderCompletedTitle(void)
{
	renderer.SetTexture(pointersTexture.textureId);
	
	
	renderer.RenderColorlessSprites(v_CompletedSprite,6,i_CompletedSprite);
	
}



void TITLE_Render(void)
{
	char statsString[256];
	//Dim background rendering

	if (timeRemaining <= 0)
		return;
	
	renderer.StartCleanFrame();
	
	
	
	renderer.Set2D();
	
	
	
	renderer.SetMaterialTextureBlending(1);
	
	
	
	renderer.SetTransparency((1-timeRemaining/initTimeRemaining)*4);
	
	renderer.FadeScreen(0.4);
	
	
	//Render the title
	renderer.SetTexture(titleTexture.textureId);
	renderer.RenderColorlessSprites(v_TitleBackground, NUM_DIM_TITLE_INDICES, i_TitleBackground);
	
	//Render the completed tile
	if (title_mode == MODE_EPILOG)
	{
		TITLE_RenderCompletedTitle();
	
		SCR_StartConvertText();
		//Now render all the texts
		
		
		//SCR_ConvertTextToVertices("- Act completed ! - ",3,0,0,TEXT_CENTERED);
		
		sprintf(statsString,"Bullets fired:  %4d",engine.playerStats.bulletsFired[controlledPlayer]);
		SCR_ConvertTextToVertices(statsString,2.6f,-200,-100,TEXT_NOT_CENTERED);
		
		sprintf(statsString,"Bullet # hits:  %4d",engine.playerStats.bulletsHit[controlledPlayer]);
		SCR_ConvertTextToVertices(statsString,2.6f,-200,-150,TEXT_NOT_CENTERED);
		
		
		//sprintf(statsString,"Bullet %% hits:  %.f%%",engine.playerStats.bulletsHit[controlledPlayer]/(float)engine.playerStats.bulletsFired[controlledPlayer]*100);
		//SCR_ConvertTextToVertices(statsString,2,-100*renderer.resolution,-200*renderer.resolution,TEXT_NOT_CENTERED);
		
		sprintf(statsString,"Total Score:  %6d",players[controlledPlayer].score);
		SCR_ConvertTextToVertices(statsString,2.6f,-200,-200,TEXT_NOT_CENTERED);
		
		
		sprintf(statsString,"Enemy cleared:   %2.0f%%",engine.playerStats.enemyDestroyed[controlledPlayer]/ engine.playerStats.numEnemies*100 );
		SCR_ConvertTextToVertices(statsString,2.6f,-200,-250,TEXT_NOT_CENTERED);
		SCR_RenderText();
	}

	
}

void TITLE_FreeRessources()
{
	TEX_UnloadTexture(&titleTexture);
	//free(titleTexture.path);
}


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
 *  text.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-10-06.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "text.h"
#include "timer.h"
#include "renderer.h"


typedef struct text_t
{
	char text[128];
	vec2short_t ss_pos;
	vec2short_t ss_start_pos;
	vec2_t diff;
	float size;
	int ttl;
	float originalTTL;
} text_t;

#define MAX_NUM_TEXTS 16
typedef struct text_lib_t
{
	text_t texts[MAX_NUM_TEXTS];
	char numTexts;
} text_lib_t;

text_lib_t textLib;


void DYN_TEXT_Init(void)
{
	textLib.numTexts = 0;
}

void DYN_TEXT_AddText( vec2short_t ss_start_pos,vec2short_t ss_end_pos, int duration,float size,char* text)
{
	
	if (textLib.numTexts >= MAX_NUM_TEXTS)
		return;
	
	strcpy(textLib.texts[textLib.numTexts].text,text);
	vector2Copy(ss_start_pos,textLib.texts[textLib.numTexts].ss_start_pos);
	
	
	
	textLib.texts[textLib.numTexts].diff[X] = ss_end_pos[X] - ss_start_pos[X];
	textLib.texts[textLib.numTexts].diff[Y] = ss_end_pos[Y] - ss_start_pos[Y];
	
	textLib.texts[textLib.numTexts].size = size;
	textLib.texts[textLib.numTexts].ttl = duration;
	textLib.texts[textLib.numTexts].originalTTL = duration;
	
	
	textLib.numTexts++;
}


void DYN_TEXT_Update(void)
{
	int i;
	float interpolation;
	
	for (i=0; i < textLib.numTexts; i++) 
	{
	
		textLib.texts[i].ttl -= timediff;
		
		if (textLib.texts[i].ttl <= 0)
		{
			if (textLib.numTexts > 0 )
				textLib.texts[i] = textLib.texts[textLib.numTexts-1];
			
			textLib.numTexts--;
		}
		
		interpolation = 1 - textLib.texts[i].ttl / textLib.texts[i].originalTTL;
		
		textLib.texts[i].ss_pos[X] = textLib.texts[i].ss_start_pos[X] + textLib.texts[i].diff[X] * interpolation ;
		textLib.texts[i].ss_pos[Y] = textLib.texts[i].ss_start_pos[Y] + textLib.texts[i].diff[Y] * interpolation ;
		
	//	printf("id=%d, posX=%d, posY=%d\n",i,textLib.texts[i].ss_pos[X],textLib.texts[i].ss_pos[Y]);
	}
}

void DYN_TEXT_Render(void)
{
	int i;
	
	if (textLib.numTexts == 0 )
		return;
	
	renderer.SetTransparency(1.0f);
	SCR_StartConvertText();
	
	for (i=0; i < textLib.numTexts; i++) 
		SCR_ConvertTextToVertices(textLib.texts[i].text,textLib.texts[i].size,textLib.texts[i].ss_pos[X],textLib.texts[i].ss_pos[Y],TEXT_CENTERED);
	
	SCR_RenderText();
}
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
 *  stats.c
 *  dEngine
 *
 *  Created by fabien sanglard on 16/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "stats.h"
#include "fx.h"
#include "renderer.h"
#include <limits.h>
#include "timer.h"
#include "texture.h"
#include "netchannel.h"

unsigned int triCount = 0;
unsigned int textSwitchCount = 0;
unsigned int shaderSwitchCount = 0;
unsigned int blendingSwitchCount = 0;

char fpsText[40]; 
char teSwText[40]; 
char drPkText[40]; 
char netSentText[40];
char netReceivedText[40];
char polCnText[40]; 
char msText[40]; 

void STATS_Begin()
{
	triCount = 0;
	textSwitchCount = 0;
	shaderSwitchCount = 0;
	blendingSwitchCount=0;
}

void STATS_AddTriangles(int count)
{
	triCount+= count;
}

void STATS_AddTexSwitch(){textSwitchCount++;}
void STATS_AddShaderSwitch(){shaderSwitchCount++;}
void STATS_AddBlendingSwitch(){blendingSwitchCount++;}

#define STATS_FONT_SIZE 2
void STATS_Render(void)
{
		if (!renderer.statsEnabled)
	{
		return;
	}
	
	
	sprintf( fpsText,"Fps: %d" ,fps );
	sprintf( teSwText, "Texture Switches: %d",textSwitchCount );
	sprintf( polCnText, "Poly Count: %d",triCount );
	sprintf(msText, "Time: %d",simulationTime);
	sprintf(drPkText, "Dropped Packets: %u", NET_GetDropedPackets());


	sprintf(netSentText,     "Net_Sent: %d", net.lastSentSequenceNumber);
	sprintf(netReceivedText, "Net_Rcvd: %d", net.lastReceivedSequenceNumber);
	
	
	
	
	renderer.SetTransparency(1.0f);
	SCR_StartConvertText();
	
	SCR_ConvertTextToVertices(fpsText  ,STATS_FONT_SIZE,-300,400,TEXT_NOT_CENTERED);
	SCR_ConvertTextToVertices(teSwText ,STATS_FONT_SIZE,-300,370,TEXT_NOT_CENTERED);
	SCR_ConvertTextToVertices(polCnText,STATS_FONT_SIZE,-300,340,TEXT_NOT_CENTERED);
	SCR_ConvertTextToVertices(msText   ,STATS_FONT_SIZE,-300,310,TEXT_NOT_CENTERED);
	SCR_ConvertTextToVertices(drPkText ,STATS_FONT_SIZE,-300,280,TEXT_NOT_CENTERED);
	SCR_ConvertTextToVertices(netSentText ,STATS_FONT_SIZE,-300,250,TEXT_NOT_CENTERED);
	SCR_ConvertTextToVertices(netReceivedText ,STATS_FONT_SIZE,-300,220,TEXT_NOT_CENTERED);
	
	SCR_RenderText();
}


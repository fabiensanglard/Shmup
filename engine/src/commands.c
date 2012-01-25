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
 *  commands.c
 *  dEngine
 *
 *  Created by fabien sanglard on 09-10-09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "commands.h"
#include "player.h"
#include "limits.h"
#include "timer.h"
#include "filesystem.h"
#include "dEngine.h"
#include "titles.h"

command_buffer_t commandsBuffers[MAX_NUM_PLAYERS];

command_t* currentRecordCommand;
command_t* lastRecordCommand;

#define NUM_RECORD_FRAMES (240*60*2)
command_t recordCommands[NUM_RECORD_FRAMES];

command_t toSend;

touch_t touches[NUM_BUTTONS];


//    0 2            4  6            8  10
//    1 3            5  7            9  11
ushort controlIndices[16] = {0,1,2,3,3,4,4,5,6,7,7,8,8,9,10,11};
//ushort controlIndices[NUM_CIRCLES*NUM_VERTICES_PER_CIRCLES*6] ;
ushort controlNumIndices;
xf_textureless_sprite_t controlVertices[NUM_CONTROL_VERTICES];








void COM_Init(void)
{
	xf_textureless_sprite_t* controlVertice;
	int i;
	
	
	float mouvementControlCenter[2];
	float fireControlCenter[2];
	float ghostControlCenter[2];
	
	
	
	controlNumIndices = 16;

	memchr(touches, 0, sizeof(touches));
	
	
	// Init controller data
	controlVertice = controlVertices;
	
	
	for (i=0; i<  NUM_CONTROL_VERTICES ; i++) 
	{
		controlVertice->color[R] = 255;
		controlVertice->color[G] = 255;
		controlVertice->color[B] = 255;
		controlVertice->color[A] = 80;
		controlVertice++;
	}
	
	
	// Move button
	//   0 
	// 1   2
	//   3
	
#define border (1/15.0f*SS_W)
	
#define mouvementButtonSize (1/2.0f*SS_W)
	
	mouvementControlCenter[X] = -SS_W + mouvementButtonSize/2 + border ;
	mouvementControlCenter[Y] = -SS_H + mouvementButtonSize/2 + border;
	
	controlVertices[0].pos[X] = mouvementControlCenter[X] ;
	controlVertices[0].pos[Y] = mouvementControlCenter[Y] + mouvementButtonSize/2;
	
	controlVertices[1].pos[X] = mouvementControlCenter[X] - mouvementButtonSize/2;
	controlVertices[1].pos[Y] = mouvementControlCenter[Y] ;
	
	controlVertices[2].pos[X] = mouvementControlCenter[X] + mouvementButtonSize/2;
	controlVertices[2].pos[Y] = mouvementControlCenter[Y] ;
	
	controlVertices[3].pos[X] = mouvementControlCenter[X] ;
	controlVertices[3].pos[Y] = mouvementControlCenter[Y] - mouvementButtonSize/2;
	
		
	touches[BUTTON_MOVE].iphone_coo_SysPos[X] = (mouvementControlCenter[X] + SS_W) / 2;
	touches[BUTTON_MOVE].iphone_coo_SysPos[Y] = SS_H - ((mouvementControlCenter[Y] + SS_H) /2);
	touches[BUTTON_MOVE].iphone_size = mouvementButtonSize/3 ;
	
	// Fire bullet button
	//    4
    //  5    6
	//    7
#define fireButtonSize (1/3.0f*SS_W)
	fireControlCenter[X] = SS_W - fireButtonSize/2 - border;
	fireControlCenter[Y] = -SS_H + fireButtonSize/2 +border;
	
	controlVertices[4].pos[X] = fireControlCenter[X];
	controlVertices[4].pos[Y] = fireControlCenter[Y] + fireButtonSize/2 ;
	
	controlVertices[5].pos[X] = fireControlCenter[X] - fireButtonSize/2;
	controlVertices[5].pos[Y] = fireControlCenter[Y];
	
	controlVertices[6].pos[X] = fireControlCenter[X] + fireButtonSize/2;
	controlVertices[6].pos[Y] = fireControlCenter[Y] ;
	
	controlVertices[7].pos[X] = fireControlCenter[X] ;
	controlVertices[7].pos[Y] = fireControlCenter[Y] - fireButtonSize/2;
	
	touches[BUTTON_FIRE].iphone_coo_SysPos[X] = (fireControlCenter[X] + SS_W) / 2;
	touches[BUTTON_FIRE].iphone_coo_SysPos[Y] = SS_H - ((fireControlCenter[Y] + SS_H) /2);
	touches[BUTTON_FIRE].iphone_size = fireButtonSize/3 ;
	
	// Ghost button
	//      8
	//   9     10
	//     11
#define ghostButtonSize (1/3.0f*SS_W)	
	ghostControlCenter[X] = SS_W - fireButtonSize - ghostButtonSize/2 - 2 * border;
	ghostControlCenter[Y] = - SS_H  + ghostButtonSize/2 + border;
	
	controlVertices[8].pos[X] = ghostControlCenter[X] ;
	controlVertices[8].pos[Y] = ghostControlCenter[Y] +ghostButtonSize/2;
	
	controlVertices[9].pos[X] = ghostControlCenter[X] - ghostButtonSize/2;
	controlVertices[9].pos[Y] = ghostControlCenter[Y] ;
	
	controlVertices[10].pos[X] = ghostControlCenter[X] + ghostButtonSize/2;
	controlVertices[10].pos[Y] = ghostControlCenter[Y] ;
	
	controlVertices[11].pos[X] = ghostControlCenter[X];
	controlVertices[11].pos[Y] = ghostControlCenter[Y] - ghostButtonSize/2;
	
	touches[BUTTON_GHOST].iphone_coo_SysPos[X] = (ghostControlCenter[X] + SS_W) / 2;
	touches[BUTTON_GHOST].iphone_coo_SysPos[Y] = SS_H - ((ghostControlCenter[Y] + SS_H) /2);
	touches[BUTTON_GHOST].iphone_size = ghostButtonSize/3 ;
	
	
	//controlVBOId = renderer.UploadVerticesToGPU(controlVertices,12*sizeof(xf_textureless_sprite_t));
	
	
	
	//Need to invalidate bufferCommand numCmd=0 ?
	for(i = 0 ; i < numPlayers ; i++)
		commandsBuffers[i].numCommands = 0;
	
	bzero(&toSend, sizeof(command_t));
	
	
	touches[BUTTON_FIRE].down = 0;
}



void COM_UpdateGhostButton(void)
{
	float f;
	float ghostControlCenter[2];
	//float r;
	
	if (engine.controlMode == CONTROL_MODE_SWIP)
		return;
	
	ghostControlCenter[X] = SS_W - fireButtonSize - ghostButtonSize/2 - 2 * border;
	ghostControlCenter[Y] = - SS_H  + ghostButtonSize/2 + border;
	
	/*
	 
	if (players[controlledPlayer].nextGhostFireTime  > simulationTime)
		f = 1 - (players[controlledPlayer].nextGhostFireTime - simulationTime ) / (float)MS_BETWEEN_GHOST ;
	else 
		f = 1;
	*/
	//Nasty branch avoidance
	
	//r = (float)simulationTime;
	//r = fabsf(r);
	
	f =  1 - (players[controlledPlayer].nextGhostFireTime  > simulationTime) * (players[controlledPlayer].nextGhostFireTime - simulationTime ) / (float)MS_BETWEEN_GHOST ;
	
	controlVertices[8].pos[Y] = ghostControlCenter[Y] + f * ghostButtonSize/2;	
	controlVertices[9].pos[X] = ghostControlCenter[X] - f * ghostButtonSize/2;
	controlVertices[10].pos[X] = ghostControlCenter[X] + f * ghostButtonSize/2;
	controlVertices[11].pos[Y] = ghostControlCenter[Y] - f * ghostButtonSize/2;
}


void COM_ConvertLocalTouchsToCommands(void)
{
	//Convert current touch state into a command that goes in commandsBuffers[controllerPlayer]
	command_t* command ;
	
	if (title_mode == MODE_EPILOG)
	{
		return;
	}
	
	command = &commandsBuffers[controlledPlayer].cmds[commandsBuffers[controlledPlayer].numCommands++];
	memset(command, 0, sizeof(command_t));
	
	command->type = NET_RTM_COMMAND;
	command->playerId = controlledPlayer;
	command->time = simulationTime ;
	
	if (touches[BUTTON_MOVE].down && !players[controlledPlayer].autopilot.enabled)
	{
		command->delta[X] = touches[BUTTON_MOVE].dist[X] * PLAYER_SPEED_MS * timediff;
		command->delta[Y] = touches[BUTTON_MOVE].dist[Y] * PLAYER_SPEED_MS * timediff;
	
	}
	
	
	if (touches[BUTTON_FIRE].down)
	{
		command->buttons |= BUTTON_FIRE_PRESSED;
	}
	
	if (touches[BUTTON_GHOST].down)
	{
		command->buttons |= BUTTON_GHOST_PRESSED; 
	}
	
	//if(command->time != simulationTime)
	//	commandsBuffers[controlledPlayer].numCommands = 0;
	//Need to copy cmd to send to netchannel.c
	memcpy(&toSend, command, sizeof(command_t));
	
	touches[BUTTON_MOVE].down = 0;
	touches[BUTTON_MOVE].dist[X] = 0;
	touches[BUTTON_MOVE].dist[Y] = 0;

	touches[BUTTON_GHOST].down = 0;
}


#define FINGER_SPRITE_U_ORIGIN ((64/256.0f)*SHRT_MAX)
#define FINGER_SPRITE_V_ORIGIN ((64/128.0f)*SHRT_MAX)
#define FINGER_SPRITE_WIDTH ((64/256.0f)*SHRT_MAX)
#define FINGER_SPRITE_HEIGHT ((64/128.0f)*SHRT_MAX)


vec2_t showFingerStartPos;
#define FINGER_SPRITE_SIZE 64
void COM_PrepareFingerSprites(command_t* command)
{

	xf_colorless_sprite_t* spriteVertice;
	short ss_sprite_boundaries[4];
	
	spriteVertice = &diverSpriteLib.vertices[diverSpriteLib.numVertices];
	
	if (engine.controlMode == CONTROL_MODE_VIRT_PAD)
	{	
		
		
	
		ss_sprite_boundaries[UP]    = ((SS_H - touches[BUTTON_MOVE].iphone_coo_SysPos[Y] + 8*command->delta[Y]*FINGER_SPRITE_SIZE) - SS_H/2)*2+ FINGER_SPRITE_SIZE ;
		ss_sprite_boundaries[DOWN]  = ((SS_H - touches[BUTTON_MOVE].iphone_coo_SysPos[Y] + 8*command->delta[Y]*FINGER_SPRITE_SIZE) - SS_H/2)*2- FINGER_SPRITE_SIZE ;
		ss_sprite_boundaries[LEFT]  = (touches[BUTTON_MOVE].iphone_coo_SysPos[X]  + 8*command->delta[X]*FINGER_SPRITE_SIZE- SS_W/2)*2- FINGER_SPRITE_SIZE ;
		ss_sprite_boundaries[RIGHT] = (touches[BUTTON_MOVE].iphone_coo_SysPos[X]  + 8*command->delta[X]*FINGER_SPRITE_SIZE- SS_W/2)*2+ FINGER_SPRITE_SIZE;
		
		
		spriteVertice->pos[X] = ss_sprite_boundaries[LEFT];
		spriteVertice->pos[Y] = ss_sprite_boundaries[UP];
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN;
		spriteVertice++;
		
		spriteVertice->pos[X] = ss_sprite_boundaries[LEFT];
		spriteVertice->pos[Y] = ss_sprite_boundaries[DOWN];
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN + FINGER_SPRITE_HEIGHT;
		spriteVertice++;
		
		
		spriteVertice->pos[X] = ss_sprite_boundaries[RIGHT];
		spriteVertice->pos[Y] = ss_sprite_boundaries[DOWN];
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN +FINGER_SPRITE_WIDTH ;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN +FINGER_SPRITE_HEIGHT ;
		spriteVertice++;
		
		
		spriteVertice->pos[X] = ss_sprite_boundaries[RIGHT];
		spriteVertice->pos[Y] = ss_sprite_boundaries[UP];
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN+FINGER_SPRITE_WIDTH;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN;
		
		diverSpriteLib.numVertices+=4;
		diverSpriteLib.numIndices+=6;
	
	
		if ((command->buttons & BUTTON_FIRE_PRESSED) == BUTTON_FIRE_PRESSED )
		{
		spriteVertice = &diverSpriteLib.vertices[diverSpriteLib.numVertices];
		
		ss_sprite_boundaries[UP]    = (SS_H - touches[BUTTON_FIRE].iphone_coo_SysPos[Y] - SS_H/2)*2 + FINGER_SPRITE_SIZE ;
		ss_sprite_boundaries[DOWN]  = (SS_H - touches[BUTTON_FIRE].iphone_coo_SysPos[Y] - SS_H/2)*2 - FINGER_SPRITE_SIZE ;
		ss_sprite_boundaries[LEFT]  = (touches[BUTTON_FIRE].iphone_coo_SysPos[X]  - SS_W/2)*2- FINGER_SPRITE_SIZE ;
		ss_sprite_boundaries[RIGHT] = (touches[BUTTON_FIRE].iphone_coo_SysPos[X]  - SS_W/2)*2+ FINGER_SPRITE_SIZE;
		
		printf("BUTTON_FIRE_PRESSED\n");
		/*
		    0 3
		    1 2
		 */
		spriteVertice->pos[X] = ss_sprite_boundaries[LEFT];
		spriteVertice->pos[Y] = ss_sprite_boundaries[UP];
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN;
		spriteVertice++;
		
		spriteVertice->pos[X] = ss_sprite_boundaries[LEFT];
		spriteVertice->pos[Y] = ss_sprite_boundaries[DOWN];
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN + FINGER_SPRITE_HEIGHT;
		spriteVertice++;

		
		spriteVertice->pos[X] = ss_sprite_boundaries[RIGHT];
		spriteVertice->pos[Y] = ss_sprite_boundaries[DOWN];
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN +FINGER_SPRITE_WIDTH ;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN +FINGER_SPRITE_HEIGHT ;
		spriteVertice++;

		
		spriteVertice->pos[X] = ss_sprite_boundaries[RIGHT];
		spriteVertice->pos[Y] = ss_sprite_boundaries[UP];
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN+FINGER_SPRITE_WIDTH;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN;
		
		
		
		diverSpriteLib.numVertices+=4;
		diverSpriteLib.numIndices+=6;
		}
	
		if ((command->buttons & BUTTON_GHOST_PRESSED) == BUTTON_GHOST_PRESSED ) 
		{
		spriteVertice = &diverSpriteLib.vertices[diverSpriteLib.numVertices];
		
		ss_sprite_boundaries[UP]    = (SS_H - touches[BUTTON_GHOST].iphone_coo_SysPos[Y] - SS_H/2)*2+ FINGER_SPRITE_SIZE ;
		ss_sprite_boundaries[DOWN]  = (SS_H - touches[BUTTON_GHOST].iphone_coo_SysPos[Y] - SS_H/2)*2- FINGER_SPRITE_SIZE ;
		ss_sprite_boundaries[LEFT]  = (touches[BUTTON_GHOST].iphone_coo_SysPos[X]  - SS_W/2)*2- FINGER_SPRITE_SIZE ;
		ss_sprite_boundaries[RIGHT] = (touches[BUTTON_GHOST].iphone_coo_SysPos[X]  - SS_W/2)*2+ FINGER_SPRITE_SIZE;
																	  
		
		/*
		 0 3
		 1 2
		 */
		spriteVertice->pos[X] = ss_sprite_boundaries[LEFT];
		spriteVertice->pos[Y] = ss_sprite_boundaries[UP];
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN;
		spriteVertice++;
		
		spriteVertice->pos[X] = ss_sprite_boundaries[LEFT];
		spriteVertice->pos[Y] = ss_sprite_boundaries[DOWN];
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN + FINGER_SPRITE_HEIGHT;
		spriteVertice++;
		
		
		spriteVertice->pos[X] = ss_sprite_boundaries[RIGHT];
		spriteVertice->pos[Y] = ss_sprite_boundaries[DOWN];
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN +FINGER_SPRITE_WIDTH ;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN +FINGER_SPRITE_HEIGHT ;
		spriteVertice++;
		
		
		spriteVertice->pos[X] = ss_sprite_boundaries[RIGHT];
		spriteVertice->pos[Y] = ss_sprite_boundaries[UP];
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN+FINGER_SPRITE_WIDTH;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN;
		
		

		
		diverSpriteLib.numVertices+=4;
		diverSpriteLib.numIndices+=6;
		}
	}
	else 
	{
		if ((command->buttons & BUTTON_FIRE_PRESSED) == 0 &&
			(command->buttons & BUTTON_GHOST_PRESSED) == 0 &&
			 command->delta[X] == 0 &&
			 command->delta[Y] == 0)
		{
			showFingerStartPos[X] =  0.5;
			showFingerStartPos[Y] = -0.5;
			return;
		}
		showFingerStartPos[X] += command->delta[X]/2;///10.0f*320;
		showFingerStartPos[Y] += command->delta[Y]/2;///10.0f*480;
		/*
		 0 3
		 1 2
		 */
		spriteVertice->pos[X] = showFingerStartPos[X] * SS_W -FINGER_SPRITE_SIZE ;
		spriteVertice->pos[Y] = showFingerStartPos[Y] * SS_H +FINGER_SPRITE_SIZE ;
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN;
		spriteVertice++;
		
		spriteVertice->pos[X] = showFingerStartPos[X] * SS_W -FINGER_SPRITE_SIZE ;
		spriteVertice->pos[Y] = showFingerStartPos[Y] * SS_H -FINGER_SPRITE_SIZE ;
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN + FINGER_SPRITE_HEIGHT;
		spriteVertice++;
		
		
		spriteVertice->pos[X] = showFingerStartPos[X] * SS_W +FINGER_SPRITE_SIZE ;
		spriteVertice->pos[Y] = showFingerStartPos[Y] * SS_H -FINGER_SPRITE_SIZE ;
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN +FINGER_SPRITE_WIDTH ;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN +FINGER_SPRITE_HEIGHT ;
		spriteVertice++;
		
		
		spriteVertice->pos[X] = showFingerStartPos[X] * SS_W +FINGER_SPRITE_SIZE ;
		spriteVertice->pos[Y] = showFingerStartPos[Y] * SS_H +FINGER_SPRITE_SIZE ;
		spriteVertice->text[U] = FINGER_SPRITE_U_ORIGIN+FINGER_SPRITE_WIDTH;
		spriteVertice->text[V] = FINGER_SPRITE_V_ORIGIN;
		
		diverSpriteLib.numVertices+=4;
		diverSpriteLib.numIndices+=6;
	}

	
}

void COM_ExecCommand(command_t* command)
{
	player_t* player;
	uchar	pId = command->playerId;
	player = &players[pId];
	
	
	
	switch (command->type) {
		case NET_RTM_COMMAND :
			
			if (players[pId].respawnCounter < 0 )
				return;
			
			//printf("engine.showFingers=%d\n",engine.showFingers);
			
			if (engine.showFingers)
				COM_PrepareFingerSprites(command);
			
			//Check the commands
			if ((command->buttons & BUTTON_FIRE_PRESSED) == BUTTON_FIRE_PRESSED)
			{
				
				P_FireTwoBullet(player);
			}
			
			//Check the commands
			if ((command->buttons & BUTTON_GHOST_PRESSED) == BUTTON_GHOST_PRESSED) 
			{
				
				P_FireGhosts(player);
			}
			
			if (player->autopilot.enabled)
				return;
			
			
			
			player->ss_position[X] += command->delta[X];
			player->ss_position[Y] += command->delta[Y];
			
			if (player->ss_position[X] < -1 ) player->ss_position[X] = -1;
			if (player->ss_position[X] > 1 ) player->ss_position[X] = 1;
			
			if (player->ss_position[Y] < -1 ) player->ss_position[Y] = -1;
			if (player->ss_position[Y] > 1 ) player->ss_position[Y] = 1;
			P_UpdateSSBoundaries(pId);
			
		
			
			break;
			
		case NET_RTM_ABS_UPDATE :
			
			
			if (player->autopilot.enabled)
				return;
			
			player->ss_position[X] += (command->delta[X] - player->ss_position[X])/10.0f; 
			player->ss_position[Y] += (command->delta[Y] - player->ss_position[Y])/10.0f;  
			P_UpdateSSBoundaries(pId);
			
			break;
		
		case NET_RTM_DIED :
			P_Die(command->playerId);
			break;

		default:
			break;
	}
	
	
}

void COM_UpdateRecord(void)
{
	int i;
	int numCmds;
	command_t* command;
	
	if (!engine.playback.record)
		return;
		
	if (lastRecordCommand < currentRecordCommand)
	{
		printf("Command buffer overflow: stop recording !!\n");
		engine.playback.record = 0;
		return;
	}
	
			
	for(i = 0 ; i < numPlayers ; i++)
	for (numCmds=0; numCmds < commandsBuffers[i].numCommands; numCmds++) 
	{
		command = &commandsBuffers[i].cmds[numCmds];
		memcpy(currentRecordCommand, command, sizeof(command_t));
		currentRecordCommand++;
	}
			
			
	
}

void COM_Update(void)
{
	int i;
	int numCmds;
	command_t* command;
	
	if (!entitiesAttachedToCamera)
		return;
	
	COM_ConvertLocalTouchsToCommands();
	
	
	COM_UpdateGhostButton();
	
	//If playing we need to play the commands in the buffer
	if (engine.playback.play)
	{
		//printf("Next command t=%u.\n",currentRecordCommand->time);
		while (lastRecordCommand > currentRecordCommand && currentRecordCommand->time <= simulationTime) 
		{
			//TO REMOVE This is only here to record a multiplayer game
			memcpy(&toSend, currentRecordCommand, sizeof(command_t));
			//TO REMOVE
			COM_ExecCommand(currentRecordCommand);
			currentRecordCommand++;
		}
	}
	else 
	{
		for (i=0; i < numPlayers; i++) 
		{
			for (numCmds=0; numCmds < commandsBuffers[i].numCommands; numCmds++) 
			{
				command = &commandsBuffers[i].cmds[numCmds];
				if (command->time <= simulationTime )
					COM_ExecCommand(command);
			}
			
			
		}
	}

	//If recording, write commands to buffer
	COM_UpdateRecord();
	
	//Need to invalidate bufferCommand numCmd=0
	//Empty command buffer
	for(i = 0 ; i < numPlayers ; i++)
	{
		
		commandsBuffers[i].numCommands = 0;
	}
}


void COM_ClearBuffers(void)
{
	int i;
	
	//Need to invalidate bufferCommand numCmd=0
	//Empty command buffer
	for(i = 0 ; i < numPlayers ; i++)
	{
		
		commandsBuffers[i].numCommands = 0;
	}
	
	touches[BUTTON_MOVE].down=0;
	touches[BUTTON_FIRE].down=0;
	touches[BUTTON_GHOST].down=0;
}

void COM_StopRecording(void)
{
	filehandle_t* ioFileHandle;
	int int_numPlayers;
	
	if (!engine.playback.record)
		return;
		
	engine.playback.record = 0;
		
	printf("[COM_StopRecording] Writing command record to disk.\n");
	//printf("[COM_StartScene] Allocating %lu kb for inputs recording.\n",NUM_RECORD_FRAMES*sizeof(commandHistoryElem_t)/1024);
	
	ioFileHandle = FS_OpenFile(engine.playback.filename,"wb");
	
	if (!ioFileHandle)
	{
		printf("[COM_StopRecording] Unable to create commandHistory file: %s.\n",engine.playback.filename);
		return;
	}
	
	int_numPlayers = numPlayers;
	printf("Writing numPlayer=%d to file record\n",int_numPlayers);
	fwrite(&int_numPlayers,sizeof(int),1, ioFileHandle->hFile);
	fwrite(recordCommands, sizeof(command_t), NUM_RECORD_FRAMES, ioFileHandle->hFile);
	FS_CloseFile(ioFileHandle);

	
	
}



void COM_StartScene(void)
{
	static char filename[1024];
	//Check if we need to play or record
	filehandle_t* ioFileHandle;

	if (engine.playback.play && engine.playback.record)
	{
		printf("[COM_StartScene] Unable to play and record at the same time: Giving priority to playback.\n");
		engine.playback.record=0;
	}
	
	if (engine.playback.play)
	{
		
		printf("[COM_StartScene] PLAYING BACK VIDEO: ");
		
		filename[0] = '\0';
		strcat(filename,"/data/commandRecord/");
		strcat(filename,engine.playback.filename);
		ioFileHandle = FS_OpenFile(filename,"rb"); 
		
		
		if (!ioFileHandle)
		{
			engine.playback.play = 0;
			//FS_CloseFile(ioFileHandle);
			printf("[COM_StartScene] Cannot start playback: io file missing.\n");
			return;
		}
		
		numPlayers = *((int*)ioFileHandle->ptrStart);
		
		printf("Found %d players in this playback.\n",numPlayers);
		
		//recordCommands = calloc(NUM_RECORD_FRAMES, sizeof(command_t));
		currentRecordCommand = recordCommands;
		memcpy(recordCommands, ioFileHandle->ptrStart+4, sizeof(command_t)*NUM_RECORD_FRAMES);
		//recordCommands = (command_t*)(ioFileHandle->ptrStart+1);
		lastRecordCommand = currentRecordCommand + NUM_RECORD_FRAMES-1 ;
		
		
		
		FS_CloseFile(ioFileHandle);
	}
	
	if (engine.playback.record)
	{
		//recordCommands = calloc(NUM_RECORD_FRAMES, sizeof(command_t));
		currentRecordCommand = recordCommands;
		
		lastRecordCommand = currentRecordCommand + NUM_RECORD_FRAMES-1 ;
	}	
}

void COM_EndtScene(void)
{
	
	//free(recordCommands);
	//recordCommands = 0;
}


void COM_ResetTouchesBuffer(void)
{
	int i;
	
	for (i=0; i < NUM_BUTTONS; i++)
	{
		touches[i].down = 0;
		touches[i].dist[X] = 0;
		touches[i].dist[Y] = 0;
	}
}



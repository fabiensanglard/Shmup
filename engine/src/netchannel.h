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
 *  netchannel.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-06-11.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */



#ifndef DF_NETCHANNEL
#define DF_NETCHANNEL


#include "commands.h"
#include "globals.h"



#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "menu.h"
#include "music.h"
#include "timer.h"
#include "dEngine.h"
#include "player.h"



#define NET_OK		(-1)
#define NET_NO_NETWORK 0
#define NET_NOT_SERVER 1
#define NET_BIND_ERROR 2
#define NET_UDP_SOCKET_CANNOT_BE_CREATED 3

int NET_Init(void);




#define NET_REGISTER 1
#define NET_COMMANDS 2
#define NET_PING_REQUEST 3
#define NET_PING_RESPONSE 4

#define BUFFER_SIZE 1024

#ifndef WIN32
#include <dns_sd.h>
#include <netdb.h>		
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>

typedef struct net_channel_t
{
	int						udpSocket;
	struct sockaddr_in		peerAddr; 
	char					serverAddResolved ;
	char					setupRequested;
	uchar					buffer[BUFFER_SIZE];

#define NET_UNKNOWN 0
#define NET_SERVER  1
#define NET_CLIENT  2
int				type;
	
#define NET_UNDETERMINED	0
#define NET_STARTED			1
#define NET_PRELOADED		2
#define NET_RUNNING			3
int				state;
	
	
	unsigned int lastReceivedSequenceNumber;
	unsigned int lastSentSequenceNumber;
	
	uint numDropedPackets;
	
} net_channel_t;
#else
	
typedef struct net_channel_t
{
	int						udpSocket;
	//struct sockaddr_in		peerAddr; 
	char					serverAddResolved ;
	char					setupRequested;
	uchar					buffer[BUFFER_SIZE];

#define NET_UNKNOWN 0
#define NET_SERVER  1
#define NET_CLIENT  2
int				type;
	
#define NET_UNDETERMINED	0
#define NET_STARTED			1
#define NET_PRELOADED		2
#define NET_RUNNING			3
int				state;
	
	
	unsigned int lastReceivedSequenceNumber;
	unsigned int lastSentSequenceNumber;
	
	uint numDropedPackets;
	
} net_channel_t;
	
#endif



extern net_channel_t net;



void NET_Setup(void);
void NET_Receive(void);
void NET_Send(void);

void NET_Free(void);

char NET_IsInitialized();

void Net_SendDie(command_t* command);

void NET_OnActLoaded(void);

void NET_OnNextLevelLoad(void);
char NET_IsRunning(void);

uint NET_GetDropedPackets(void);
#endif
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
 *  netchannel.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-06-11.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "netchannel.h"

// The network version was designed on iOS with Unix socket. This part still needs to be ported using winsock32.
#if defined(WIN32) || defined(ANDROID) || defined(LINUX)
	int NET_Init(void){return 1;}
	void NET_Setup(void){}
	void NET_Receive(void){}
	void NET_Send(void){}
	void NET_Free(void){}
	char NET_IsInitialized(){return 1;}
	void Net_SendDie(command_t* command){}
	void NET_OnActLoaded(void){}
	void NET_OnNextLevelLoad(void){}
	char NET_IsRunning(void){return 0;}
	uint NET_GetDropedPackets(void){return 0;}

	net_channel_t net;
#else

#define DNSServiceRefDeallocate(x)Log_Printf("DNSServiceRefDeallocate(" #x ")\n"); DNSServiceRefDeallocate(x) 


#define MESSAGE_NETMYIP 1
#define MESSAGE_NETPEERPIP 2
#define MESSAGE_NETYPE 0
#define MESSAGE_NETSTATE 3
#define MESSAGE_NETLASTSENT 4
#define MESSAGE_NETLASTRECEIVED 5

#ifdef __APPLE__
	#include "TargetConditionals.h"
	#if TARGET_IPHONE_SIMULATOR
		#define INTERFACE_NAME "en1"
	#else
		#define INTERFACE_NAME "en0"
	#endif
#else
	#define INTERFACE_NAME "en0"
#endif

DNSServiceRef		browseRef=0;
DNSServiceRef		registerRef=0;
DNSServiceRef		resolveRef=0;

static const char	*serviceName = "_DodgeServer._udp.";




typedef struct service_t{
	int				interfaceIndex;
	char			browseName[1024];
	char			browseRegtype[1024];
	char			browseDomain[1024];
} service_t ;

#define MAX_SERVICE_INTEFACES 10
service_t		serviceInterfaces[MAX_SERVICE_INTEFACES];


#define PORT_NUMBER 31978




// This is a designated initializers, a C99 feature which allows you to name members to be initialized
net_channel_t net = { .type = NET_UNKNOWN };

typedef struct net_packet_t
{
#define SETUP_PACKET 1
#define RUNTIME_PACKET 2
	char type;
	
	int sequenceNumber;
	int ackSequenceNumber;
	
	int time;
	int ackTime;

	
//#define NET_CMD_NOOP 0
#define NET_CMD_LOAD_NEXT_LEVEL 0
#define NET_CMD_NOTIFY_LOADED 1
#define NET_CMD_START_LEVEL 2
	
	command_t command;
	
} net_packet_t;


// PREDICTION
#define MAX_CMD_HISTORY 4
typedef struct cmdHistory
{
	command_t array [MAX_CMD_HISTORY];
	uchar ptr;
} cmdHistory_t;
cmdHistory_t cmdHistory;

#define MAX_FAKE_CMD_HISTORY 16
typedef struct fakeCmdHistory_t
{
	command_t stack[MAX_FAKE_CMD_HISTORY];
	uchar num;
} fakeCmdHistory_t ;

fakeCmdHistory_t fakeCmdHistory;

// END PREDICTION


void NET_Free(void)
{
	Log_Printf("NET_FREE\n");
	
	// unregister
	DNSServiceRefDeallocate(browseRef); browseRef=0;
	DNSServiceRefDeallocate(registerRef);registerRef=0;
	DNSServiceRefDeallocate(resolveRef);resolveRef=0;
	
	net.type=NET_UNKNOWN;
	net.serverAddResolved = 0;
	net.setupRequested = 0;
	net.state = NET_UNDETERMINED;
	
	net.lastReceivedSequenceNumber = 0;
	net.lastSentSequenceNumber = 1;
	
	net.numDropedPackets = 0 ;
	
	//free(buffer);
	
	// unbind
	close(net.udpSocket);
	net.udpSocket=0;
	
	//Also reset all messages
	MENU_GetMultiplayerTextLine(0)[0]='\0';
	MENU_GetMultiplayerTextLine(1)[0]='\0';
	MENU_GetMultiplayerTextLine(2)[0]='\0';
	MENU_GetMultiplayerTextLine(3)[0]='\0';
	MENU_GetMultiplayerTextLine(4)[0]='\0';
	MENU_GetMultiplayerTextLine(5)[0]='\0';
}

char NET_IsNetworkAvailable() {
	struct ifaddrs *ifap;
	if ( getifaddrs( &ifap ) == -1 ) {
		return 0;
	}
	
	Log_Printf("NET_IsNetworkAvailable\n");
	
	//Log_Printf("NET_IsNetworkAvailable() searching for interface %s with type %d\n",INTERFACE_NAME,AF_INET);
	
	// We can't tell if bluetooth is available from here, because
	// the interface doesn't appear until after the service is found,
	// but I decided not to support bluetooth for now due to the poor performance.
	char	goodInterface = 0;
	
	for ( struct ifaddrs *ifa = ifap ; ifa ; ifa = ifa->ifa_next ) {
		struct sockaddr_in *ina = (struct sockaddr_in *)ifa->ifa_addr;
	//	Log_Printf("[NET_IsNetworkAvailable] Searching interface: %s, family=%d.\n",ifa->ifa_name,ina->sin_family);
	//	Log_Printf("current if: %s, family=%d @=%s IFF_UP=%d IFF_RUNNING=%d .\n",ifa->ifa_name,ina->sin_family,inet_ntoa(ina->sin_addr),ifa->ifa_flags & IFF_UP != 0, ifa->ifa_flags & IFF_RUNNING != 0);
		if ( ina->sin_family == AF_INET ) {
			if ( !strcmp( ifa->ifa_name, INTERFACE_NAME ) ) {
		//		Log_Printf("[NET_IsNetworkAvailable] Found interface: %s, family=%d.\n",ifa->ifa_name,ina->sin_family);
				goodInterface = 1;
				break;
			}
		}
	}
	freeifaddrs( ifap );
	
	return goodInterface;
}

struct sockaddr_in NET_GetAddressForInterfaceName( const char *ifname ) 
{	
	
	struct sockaddr_in s;
	
	
	Log_Printf("NET_GetAddressForInterfaceName\n");
	
	memset( &s, 0, sizeof( s ) );
	
	struct ifaddrs *ifap;
	if ( getifaddrs( &ifap ) == -1 ) {
		perror( "getifaddrs()" );
		return s;
	}
	
	struct ifaddrs *ifa;
	for ( ifa = ifap ; ifa ; ifa = ifa->ifa_next ) {
		struct sockaddr_in *ina = (struct sockaddr_in *)ifa->ifa_addr;
		if ( ina->sin_family == AF_INET && !strcmp( ifa->ifa_name, ifname ) ) {
			uchar *ip = (uchar *)&ina->sin_addr;
			Log_Printf("if: %s, family=%d @=%s IFF_UP=%d IFF_RUNNING=%d .\n",
                   ifa->ifa_name,ina->sin_family,
                   inet_ntoa(ina->sin_addr),
                   (ifa->ifa_flags & IFF_UP) != 0, 
                   (ifa->ifa_flags & IFF_RUNNING) != 0);
//			Log_Printf( "AddressForInterfaceName( %s ) = ifa_name: %s ifa_flags: %i sa_family: %i=AF_INET ip: %i.%i.%i.%i\n", ifname, ifa->ifa_name, ifa->ifa_flags,ina->sin_family, ip[0], ip[1], ip[2], ip[3]  );
			sprintf(MENU_GetMultiplayerTextLine(1),"My IP: %i.%i.%i.%i",ip[0], ip[1], ip[2], ip[3]);
			freeifaddrs( ifap );
			return *ina;
		}
	}
	freeifaddrs( ifap );
	Log_Printf( "AddressForInterfaceName( %s ): Couldn't find IP address\n", ifname );
	return s;
}

int NET_InterfaceIndexForInterfaceName( const char *ifname ) {
	struct if_nameindex *ifnames = if_nameindex();
	if ( !ifnames ) {
		perror( "if_nameindex()" );
		return 0;
	}
	for ( int i = 0 ; ifnames[i].if_index != 0 ; i++ ) {
		if ( !strcmp( ifname, ifnames[i].if_name ) ) {
			int	index = ifnames[i].if_index;
			if_freenameindex( ifnames );
			return index;
		}
	}	
	Log_Printf( "InterfaceIndexForName( %s ): Couldn't find interface\n", ifname );
	if_freenameindex( ifnames );
	return 0;
}


void DNSServiceRegisterReplyCallback ( 
									  DNSServiceRef sdRef, 
									  DNSServiceFlags flags, 
									  DNSServiceErrorType errorCode, 
									  const char *name, 
									  const char *regtype, 
									  const char *domain, 
									  void *context ) {
	
	Log_Printf("DNSServiceRegisterReplyCallback\n");
	
	if ( errorCode == kDNSServiceErr_NoError ) 
	{
		net.type = NET_SERVER;
		net.state = NET_STARTED;
		Log_Printf("Able to register: I am the one and only SERVER.\n");
	} 
	else 
	{
		net.type = NET_CLIENT;
		net.state = NET_STARTED;
		Log_Printf("Registering error: I have to be a client.\n");
	}
}
int NET_CheckServerAvailability(void)
{
	int	socket;
	fd_set	set;
	
	net.type = NET_UNKNOWN;
	
	Log_Printf("NET_CheckServerAvailability\n");
	Log_Printf("DNSServiceRegister\n");
	DNSServiceErrorType	err = DNSServiceRegister( 
												 &registerRef, 
												 kDNSServiceFlagsNoAutoRename,		// we want a conflict error
												 NET_InterfaceIndexForInterfaceName( INTERFACE_NAME ),		// pass 0 for all interfaces
												 "Dodge shmup server",
												 serviceName,
												 NULL,	// domain
												 NULL,	// host
												 htons( PORT_NUMBER ),
												 0,		// txtLen
												 NULL,	// txtRecord
												 DNSServiceRegisterReplyCallback,
												 NULL		// context
												 );
	
	
	
	if ( err != kDNSServiceErr_NoError ) 
	{
		Log_Printf( "DNSServiceRegister error\n" );
		net.type = NET_UNKNOWN;
		return 0;
	} 
	
	
	
	
	
	
	// poll the socket for updates
	socket = DNSServiceRefSockFD( registerRef );
	if ( socket <= 0 ) {
		return 0;
	}
	
	FD_ZERO( &set );
	FD_SET( socket, &set );
	
	struct timeval tv;
	//memset( &tv, 0, sizeof( tv ) );
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	
	if ( select( socket+1, &set, NULL, NULL, &tv ) > 0 ) {
		Log_Printf("Received response from dnsDeamon\n");
		DNSServiceProcessResult( registerRef );
	}	
	else {
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETPEERPIP), "DNSServiceRegister timeout (%ld sec)",tv.tv_sec);	
	}
	/*
	// block until we get a response, process it, and run the callback
	// Do this instead of using a select 
	Log_Printf("DNSServiceProcessResult\n");
	err = DNSServiceProcessResult( registerRef );
	if ( err != kDNSServiceErr_NoError ) 
	{
		Log_Printf( "DNSServiceProcessResult error\n" );
		net.type = UNKNOWN;
		return 0;
	}
	*/
	
	
	
	
	if(net.type == NET_SERVER)
	{
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETYPE), "Waiting for client to connect...");
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETYPE+1), " ");
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETYPE+2), "You are Player ONE.");
	}
	
	if (net.type == NET_CLIENT)
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETYPE), "Contacting server...");
	
	
	return 1;
}

void DNSServiceQueryRecordReplyCallback ( 
										 DNSServiceRef DNSServiceRef, 
										 DNSServiceFlags flags, 
										 uint32_t interfaceIndex, 
										 DNSServiceErrorType errorCode, 
										 const char *fullname, 
										 uint16_t rrtype, 
										 uint16_t rrclass, 
										 uint16_t rdlen, 
										 const void *rdata, 
										 uint32_t ttl, 
										 void *context ) {
	
	
	char	interfaceName[IF_NAMESIZE];
	Log_Printf("DNSServiceQueryRecordReplyCallback\n");
	
	if_indextoname( interfaceIndex, interfaceName );
	
	//Log_Printf( "DNSServiceQueryRecordReplyCallback: Found service %s on interface %s.\n",fullname,interfaceName);
	//Log_Printf( "DNSServiceQueryRecordReplyCallback: %s, interface[%i] = %s, [%i] = %i.%i.%i.%i\n", fullname, interfaceIndex, interfaceName, rdlen, ip[0], ip[1], ip[2], ip[3] );
	
	
	//ReportNetworkInterfaces();
	
	memset( &net.peerAddr, 0, sizeof( net.peerAddr ) );
	//struct sockaddr_in *sin = (struct sockaddr_in *)&net.peerAddr;
	net.peerAddr.sin_len = sizeof( net.peerAddr );
	net.peerAddr.sin_family = AF_INET;
	net.peerAddr.sin_port = htons( PORT_NUMBER );
	memcpy( &net.peerAddr.sin_addr, rdata, 4 );
	
	net.serverAddResolved = 1;
	
	sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETPEERPIP),"PEER IP %s",inet_ntoa(net.peerAddr.sin_addr));
}

void DNSServiceResolveReplyCallback ( 
									 DNSServiceRef sdRef, 
									 DNSServiceFlags flags, 
									 uint32_t interfaceIndex, 
									 DNSServiceErrorType errorCode, 
									 const char *fullname, 
									 const char *hosttarget, 
									 uint16_t port, 
									 uint16_t txtLen, 
									 const unsigned char *txtRecord, 
									 void *context ) {
	
	DNSServiceRef	queryRef;
	char	interfaceName[IF_NAMESIZE];
	
	Log_Printf("DNSServiceResolveReplyCallback\n");
	
	if_indextoname( interfaceIndex, interfaceName );
	//Log_Printf( "Resolve: interfaceIndex [%i]=%s : %s @ %s\n", interfaceIndex, interfaceName, fullname, hosttarget );
	

	
	// look up the name for this host
	DNSServiceErrorType err = DNSServiceQueryRecord ( 
													 &queryRef, 
													 kDNSServiceFlagsForceMulticast, 
													 interfaceIndex, 
													 hosttarget, 
													 kDNSServiceType_A,		// we want the host address
													 kDNSServiceClass_IN, 
													 DNSServiceQueryRecordReplyCallback, 
													 NULL /* may be NULL */
													 );  	
	if ( err != kDNSServiceErr_NoError ) 
	{
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETPEERPIP), "DNSServiceQueryRecord error");
	} 
	else 
	{
		// block until we get a response, process it, and run the callback
		err = DNSServiceProcessResult( queryRef );
		
		if ( err != kDNSServiceErr_NoError ) 
			sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETPEERPIP), "DNSServiceProcessResult error");
		
		DNSServiceRefDeallocate( queryRef );
	}
}
	

void DNSServiceBrowseReplyCallback(
								   DNSServiceRef sdRef, 
								   DNSServiceFlags flags, 
								   uint32_t interfaceIndex, 
								   DNSServiceErrorType errorCode, 
								   const char *serviceName, 
								   const char *regtype, 
								   const char *replyDomain, 
								   void *context ) {
	
	Log_Printf("DNSServiceBrowseReplyCallback\n");
	
	//Log_Printf( "DNSServiceBrowseReplyCallback %s: interface:%i name:%s regtype:%s domain:%s\n", (flags & kDNSServiceFlagsAdd) ? "ADD" : "REMOVE",interfaceIndex, serviceName, regtype, replyDomain );
	
	service_t* service ;
	
	if ( flags & kDNSServiceFlagsAdd ) 
	{
		// add it to the list
		if ( interfaceIndex == 1 ) 
		{
			Log_Printf( "Not adding service on loopback interface.\n" );
			return;
		} 
		
		service = &serviceInterfaces[interfaceIndex];
		//Log_Printf("DNSServiceBrowseReplyCallback processing service interface= %d.\n",service->interfaceIndex);
			
		strncpy( service->browseName, serviceName, sizeof( service->browseName ) -1 );
		strncpy( service->browseRegtype, regtype, sizeof( service->browseRegtype ) -1 );
		strncpy( service->browseDomain, replyDomain, sizeof( service->browseDomain ) -1 );
		service->interfaceIndex = interfaceIndex;
			
		char	interfaceName[IF_NAMESIZE];	
		if_indextoname(interfaceIndex,interfaceName);
		
		//If interface namei s INTERFACE_NAME let's try to resolve this guy.
		if (!strcmp(INTERFACE_NAME,interfaceName ))
		{
			
			DNSServiceRef	resolveRef;
			DNSServiceErrorType err = DNSServiceResolve ( 
														 &resolveRef, 
														 kDNSServiceFlagsForceMulticast,	// always on local link
														 service->interfaceIndex ,		// don't use -1 for bluetooth
														 service->browseName, 
														 service->browseRegtype, 
														 service->browseDomain, 
														 DNSServiceResolveReplyCallback, 
														 NULL			/* context */
														 );  
			
			if ( err != kDNSServiceErr_NoError ) {
				sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETPEERPIP), "DNSServiceResolve error");	
				
			} else {
				// We can get two callbacks when both wifi and bluetooth are enabled
				//callbackFlags = 0;
				//do {
					err = DNSServiceProcessResult( resolveRef );
					if ( err != kDNSServiceErr_NoError ) {
						sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETPEERPIP), "DNSServiceProcessResult error");	
					}
				//} while ( callbackFlags & kDNSServiceFlagsMoreComing );
				DNSServiceRefDeallocate( resolveRef );
			}
			
			
			
		}
		
	} 
	else 
	{
		// remove it from the list
		for ( int i = 0 ; i < MAX_SERVICE_INTEFACES ; i++ ) 
		{
			if ( serviceInterfaces[i].interfaceIndex == interfaceIndex ) 
			{
				serviceInterfaces[i].interfaceIndex = -1;
			}
		}
	}
	
	
	// Need to resolved
}

int NET_ResolveNetworkServer( ) 
{
	fd_set	set;
	int	socket;
	
	Log_Printf("NET_ResolveNetworkServer\n");
	
	Log_Printf("DNSServiceBrowse\n");
	//Browse and then Resolve
	DNSServiceErrorType err = DNSServiceBrowse ( 
												&browseRef, 
												0,					/* flags */
												0,					/* interface */
												serviceName, 
												NULL,				/* domain */
												DNSServiceBrowseReplyCallback, 
												NULL				/* context */
												);  
	if ( err != kDNSServiceErr_NoError ) {
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETPEERPIP), "DNSServiceBrowse error");
		return 0;
	}		
	
	
	
	//Now we will wait for mDNSResponder to send us an update, then call DNSServiceProcessResult, this should call DNSServiceBrowseReplyCallback
	// poll the socket for updates
	socket = DNSServiceRefSockFD( browseRef );
	if ( socket <= 0 ) {
		return 0;
	}
	
	FD_ZERO( &set );
	FD_SET( socket, &set );
	
	struct timeval tv;
	//memset( &tv, 0, sizeof( tv ) );
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	
	if ( select( socket+1, &set, NULL, NULL, &tv ) > 0 ) {
		Log_Printf("Received response from dnsDeamon\n");
		DNSServiceProcessResult( browseRef );
	}	
	else {
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETPEERPIP), "DNSServiceBrowse timeout (%ld sec)",tv.tv_sec);	
	}
	
	return 1;
	
}


//char isInitialized(void)
//{
//	return (netType != UNKNOWN && (netType == SERVER || (netType == CLIENT && serverAddResolved)));
//}


void NET_CreateSocket(void)
{
	struct sockaddr_in bindingIp_address;
	
	Log_Printf("NET_CreateSocket\n");
	
	bzero(&bindingIp_address, sizeof(bindingIp_address));
	//ip_address = NET_GetAddressForInterfaceName(INTERFACE_NAME);
	bindingIp_address.sin_family = AF_INET;
	bindingIp_address.sin_port = htons( PORT_NUMBER );
	//	bindingIp_address.sin_addr.s_addr = htonl(INADDR_ANY);
	//inet_pton(AF_INET,"10.0.1.3",&bindingIp_address.sin_addr.s_addr);
	
	
	// Create socket and bind it to IP+Port
	net.udpSocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if ( net.udpSocket == -1 ) 
	{
		Log_Printf( "UDP socket failed: %s\n", strerror( errno ) );
		return ;
	}
	
	
	
	// enable non-blocking IO
	//int x;
	//x = fcntl(udpSocket,F_GETFL,0);
	if (fcntl(net.udpSocket,F_SETFL, O_NONBLOCK)== -1 ) {
		Log_Printf( "UDP fcntl failed: %s\n", strerror( errno ) );
		close( net.udpSocket );
		
		return ;
	}
	
	
	//if (netType == SERVER)
	int errorCheck;
	errorCheck = bind( net.udpSocket, (struct sockaddr *)&bindingIp_address, sizeof( struct sockaddr_in ) );
	if (errorCheck == -1)
	{
		Log_Printf("UDP bind failed: %s\n", strerror( errno ) );
		return ;
	}
	
	Log_Printf("[NETCHANNEL ] Bind on %s:%hud\n",inet_ntoa(bindingIp_address.sin_addr),bindingIp_address.sin_port);
	
	
}



void Net_ProcessSetupPacket(void)
{
	// Read all incoming UDP datagrams
	socklen_t len ;
	//struct sockaddr incomingAdd;
	struct sockaddr_in incomingAdd;
	int byteReceived;
	net_packet_t* packet;
	uchar packetConsumed = 0;
	net_packet_t outPacket;
	
//	Log_Printf("Net_ProcessSetupPacket\n");
	
	bzero(&incomingAdd, sizeof(incomingAdd));
	len = sizeof(incomingAdd);
	//Log_Printf("Net_ProcessSetupPacket()\n");
	
	byteReceived = recvfrom(net.udpSocket,net.buffer,sizeof(net.buffer),0, (struct sockaddr*)&incomingAdd, &len );
	if (byteReceived == -1)
	{
		if (errno != EAGAIN )
			sprintf(MENU_GetMultiplayerTextLine(4),"Error recvfrom:%d %s.\n",errno,strerror( errno ));
	
		//Log_Printf("No packets.\n");
		return;
	}

	Log_Printf("Net_ProcessSetupPacket() read %d bytes\n",byteReceived);

	packet = (net_packet_t*)net.buffer;
		
	Log_Printf("packet->type=%d\n",packet->type);
	
	if (packet->type != SETUP_PACKET)
	{
		Log_Printf("Not a setup packet.\n");
		return;
	}
		
	if (packet->sequenceNumber <= net.lastReceivedSequenceNumber)
	{
		Log_Printf("Old packet.\n");
		return;
	}
		
		

//	net.numDropedPackets += 1 - packet->sequenceNumber - net.lastReceivedSequenceNumber;
		
	net.lastReceivedSequenceNumber = packet->sequenceNumber;
		
	sprintf(MENU_GetMultiplayerTextLine(4),"Received setup packet %i.\n",packet->type);
		
	//outPacket.cmd = NET_CMD_NOOP;
	
	if (net.type == NET_SERVER && net.state == NET_STARTED &&  packet->command.type == NET_CMD_LOAD_NEXT_LEVEL)
	{
		packetConsumed=1;
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETLASTRECEIVED), "LAST RECV=NET_CMD_LOAD_NEXT_LEVEL");
		
		//Save peer informations to send replies as this is the first time the server will hear of the client
		memcpy(&net.peerAddr,&incomingAdd,sizeof(incomingAdd));
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETPEERPIP),"PEER IP %s",inet_ntoa(net.peerAddr.sin_addr));
		
		//Perform preload, pause music, pause timer
		dEngine_RequireSceneId(engine.sceneId + 1  % engine.numScenes);
		
		numPlayers=2;
		controlledPlayer=0;
		
//		Log_Printf("PPRE Player1=%p\n",players[0].entity.material);
//		Log_Printf("PPRE Player2=%p\n",players[1].entity.material);
		
		dEngine_CheckState();
		
//		Log_Printf("POST Player1=%p\n",players[0].entity.material);
//		Log_Printf("POST Player2=%p\n",players[1].entity.material);		
		
		SND_PauseSoundTrack();
		Timer_Pause();
		net.state = NET_PRELOADED;
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETSTATE), "state=NET_PRELOADED.\n");
		
		Log_Printf("Client loaded level, but Timer still paused sending NET_CMD_LOAD_NEXT_LEVEL.\n");
		
		// Trigger preload on the other end as well by sending a NET_CMD_LOAD_NEXT_LEVEL to peer
		outPacket.command.type = NET_CMD_LOAD_NEXT_LEVEL;
		outPacket.sequenceNumber = net.lastSentSequenceNumber++;
		outPacket.ackSequenceNumber = net.lastReceivedSequenceNumber;
		outPacket.type = SETUP_PACKET;
		sendto(net.udpSocket, &outPacket, sizeof(outPacket), 0, (struct sockaddr*)&net.peerAddr, sizeof(net.peerAddr));
		
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETLASTSENT), "LAST SENT=NET_CMD_LOAD_NEXT_LEVEL");
	}
	
	if (net.type == NET_CLIENT && net.state == NET_STARTED &&  packet->command.type == NET_CMD_LOAD_NEXT_LEVEL)
	{
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETLASTRECEIVED), "LAST RECV=NET_CMD_LOAD_NEXT_LEVEL");
		packetConsumed=1;
		
		//Perform preload, pause music, pause timer
		dEngine_RequireSceneId(engine.sceneId + 1  % engine.numScenes);
		
		numPlayers=2;
		controlledPlayer=1;
		
		dEngine_CheckState();
		
		SND_PauseSoundTrack();
		Timer_Pause();
		net.state = NET_PRELOADED;
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETSTATE), "state=NET_PRELOADED.\n");

		Log_Printf("Client loaded level, but Timer still paused sending NET_CMD_NOTIFY_LOADED.\n");
		
		// Tell server we are ready to start by sending NET_CMD_NOTIFY_LOADED
		outPacket.command.type = NET_CMD_NOTIFY_LOADED;
		outPacket.sequenceNumber = net.lastSentSequenceNumber++;
		outPacket.ackSequenceNumber = net.lastReceivedSequenceNumber;
		outPacket.type = SETUP_PACKET;
		sendto(net.udpSocket, &outPacket, sizeof(outPacket), 0, (struct sockaddr*)&net.peerAddr, sizeof(net.peerAddr));
		
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETLASTSENT), "LAST SENT=NET_CMD_NOTIFY_LOADED");
		
	}
	
	if (net.type == NET_SERVER && net.state == NET_PRELOADED &&  packet->command.type == NET_CMD_NOTIFY_LOADED)
	{
		packetConsumed=1;
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETLASTRECEIVED), "LAST RECV=NET_CMD_NOTIFY_LOADED");
		
		net.state = NET_RUNNING;
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETSTATE), "state=NET_RUNNING.\n");
		//Log_Printf(MENU_GetMultiplayerTextLine(MESSAGE_NETSTATE), "state=NET_RUNNING.\n");
		//Send NET_CMD_START_LEVEL
		
		//Start level (unpause time, unpause music)
		SND_ResumeSoundTrack();
		Timer_resetTime();
		Timer_Resume();
		
		Log_Printf("Server Received NET_CMD_NOTIFY_LOADED, starting and asking client to start as well: NET_CMD_START_LEVEL.\n");
		
		MENU_Set(MENU_NONE);
		
		//Trigger client start
		outPacket.command.type = NET_CMD_START_LEVEL;
		outPacket.sequenceNumber = net.lastSentSequenceNumber++;
		outPacket.ackSequenceNumber = net.lastReceivedSequenceNumber;
		outPacket.type = SETUP_PACKET;
		sendto(net.udpSocket, &outPacket, sizeof(outPacket), 0, (struct sockaddr*)&net.peerAddr, sizeof(net.peerAddr));
		usleep(16*1000);
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETLASTSENT), "LAST SENT=NET_CMD_START_LEVEL");
	}
	
	if (net.type == NET_CLIENT && net.state == NET_PRELOADED &&  packet->command.type == NET_CMD_START_LEVEL)
	{
		packetConsumed=1;
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETLASTRECEIVED), "LAST RECV=NET_CMD_START_LEVEL");
		
		net.state = NET_RUNNING;
		//Start level (unpause time, unpause music)
		SND_ResumeSoundTrack();
		Timer_resetTime();
		Timer_Resume();
		
		Log_Printf("Client Received NET_CMD_START_LEVEL, starting.\n");
		
		MENU_Set(MENU_NONE);
		
		
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETSTATE), "state=NET_RUNNING.\n");
		//Log_Printf(MENU_GetMultiplayerTextLine(MESSAGE_NETSTATE), "state=NET_RUNNING.\n");
		
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETLASTSENT), "LAST SENT=NET_CMD_LOAD_NEXT_LEVEL");
	}
	
	//if(outPacket.cmd != NET_CMD_NOOP)
	//{
		
	//}
	
	if (!packetConsumed)
		Log_Printf("Packet type=%d was not consumed.",packet->command.type );
}

#define isInitialized (net.state == NET_RUNNING && (net.type == NET_SERVER || (net.type == NET_CLIENT && net.serverAddResolved)))
void NET_Setup(void)
{
	net_packet_t registerPacket;
	struct sockaddr_in localAddress;
	
	//Log_Printf("NET_Setup\n");
	
	if (!net.setupRequested)
	{
	//	Log_Printf("!setupRequested\n");
		return ;
	
	}
	if (isInitialized)
	{
	//	Log_Printf("isInitialized\n");
		return ;
	}
	
	//Log_Printf("NET_Setup\n");
	
	//NET_Free();
	//buffer = calloc(, sizeof(uchar));
	
	
	if (net.type == NET_UNKNOWN && !NET_IsNetworkAvailable())
	{
		sprintf(MENU_GetMultiplayerTextLine(0), "No WIFI network available !");
		sprintf(MENU_GetMultiplayerTextLine(1), " ");
		sprintf(MENU_GetMultiplayerTextLine(2), "Make sure WIFI is enabled" );
		sprintf(MENU_GetMultiplayerTextLine(3), "and the device is connected." );
	
		return;	
	}
	
	
	if (net.type == NET_UNKNOWN && !NET_CheckServerAvailability())
	{
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETMYIP), "Error while NET_CheckServerAvailability.\n");
		return ;
	}
	
	
	if (net.type == NET_CLIENT && !net.serverAddResolved)
	{
		if (!NET_ResolveNetworkServer())
		{
			sprintf(MENU_GetMultiplayerTextLine(0),   "Unable to find the server !");
			sprintf(MENU_GetMultiplayerTextLine(1),   " ");
			sprintf(MENU_GetMultiplayerTextLine(2), "Restart the server then try");
			sprintf(MENU_GetMultiplayerTextLine(3), "connecting again.");
			return ;
		}
		//sprintf(MENU_GetMultiplayerTextLine(0), "Resolved server :) !\n");
		//sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETPEERPIP), "Server IP: %s !",inet_ntoa(net.peerAddr.sin_addr));
	}
	

	if (MENU_GetMultiplayerTextLine(MESSAGE_NETMYIP)[0] == '\0')
	{
		localAddress =  NET_GetAddressForInterfaceName(INTERFACE_NAME);	
		sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETMYIP),"Local IP @:'%s'",inet_ntoa(localAddress.sin_addr));
	}
	
		
	if (net.udpSocket == 0)
	{
		NET_CreateSocket();
		Log_Printf("File descriptor UDP socket = %d.\n",net.udpSocket);
	}
	
	//Process to setup
	//Log_Printf("net.state =%d\n",net.state );
	
	if (net.state == NET_STARTED)
	{
		//We need to register
		if (net.type == NET_CLIENT)
		{
			registerPacket.sequenceNumber = net.lastSentSequenceNumber++;
			registerPacket.ackSequenceNumber = net.lastReceivedSequenceNumber;
			registerPacket.type = SETUP_PACKET;
			registerPacket.command.type = NET_CMD_LOAD_NEXT_LEVEL;
			sendto(net.udpSocket, &registerPacket, sizeof(registerPacket), 0, (struct sockaddr*)&net.peerAddr, sizeof(net.peerAddr));
			//sprintf(MENU_GetMultiplayerTextLine(MESSAGE_NETLASTSENT), "LAST SENT=NET_CMD_LOAD_NEXT_LEVEL");
			
		}
		
	}
	
	if (net.state != NET_RUNNING)
	{
		Net_ProcessSetupPacket();
	}
	else
	{
		Log_Printf("Stoping setup, as we reached NET_RUNNING\n");
		net.setupRequested = 0;
		
		memset(&fakeCmdHistory,0,sizeof(fakeCmdHistory_t));
		
		memset(&cmdHistory,0,sizeof(cmdHistory_t));

	}
	
	
}

char NET_IsInitialized()
{
	return isInitialized;
}

uint outSequenceNumber;
uint inSequenceNumber;
typedef struct net_message_t
{
	uint sequenceNumber;
	uchar playerId;
	uchar type;
	void* payload;
	
} net_message_t;

#define deltaT 2
command_t* NET_GenerateFakeCMD(void)
{
	static command_t cmd;
	command_t* last;
	command_t* lastMinusOne;
	command_t* lastMinusTwo;
	vec3_t delta1;
	vec3_t delta2;
	float sumDelta;
	
	last = &cmdHistory.array[(cmdHistory.ptr -1) & (MAX_CMD_HISTORY-1) ];
	lastMinusOne = &cmdHistory.array[(cmdHistory.ptr -2) & (MAX_CMD_HISTORY-1) ];
	lastMinusTwo = &cmdHistory.array[(cmdHistory.ptr -3) & (MAX_CMD_HISTORY-1) ];
	
	delta1[X] =      lastMinusOne->delta[X] - lastMinusTwo->delta[X];
	delta1[Y] =      lastMinusOne->delta[Y] - lastMinusTwo->delta[Y];
	delta1[deltaT] = lastMinusOne->time     - lastMinusTwo->time;
	
	delta2[X] =      last->delta[X] - lastMinusOne->delta[X];
	delta2[Y] =      last->delta[Y] - lastMinusOne->delta[Y];
	delta2[deltaT] = last->time     - lastMinusOne->time;
	
	sumDelta = delta1[deltaT]+delta2[deltaT];
	if (sumDelta == 0)
	{
		delta1[X]=0;
		delta1[Y]=0;
		delta2[X]=0;
		delta2[Y]=0;
		sumDelta=0.001f;
	}
	//Need need to use the three previous cmds to generate two deltas.
	
	
	cmd.type = NET_RTM_COMMAND;
	cmd.time = simulationTime;
	cmd.playerId = !controlledPlayer;
	cmd.buttons = 0;
	cmd.delta[X] = delta1[X]*delta1[deltaT]/sumDelta + delta2[X]*delta2[deltaT] /sumDelta;
	cmd.delta[Y] = delta1[Y]*delta1[deltaT]/sumDelta + delta2[Y]*delta2[deltaT] /sumDelta;
	cmd.buttons = last->buttons;
	
	return &cmd;
}

void NET_AddFakeToHistory(command_t* cmd)
{
	if (fakeCmdHistory.num == MAX_FAKE_CMD_HISTORY-1)
		return;
	
	fakeCmdHistory.stack[fakeCmdHistory.num].delta[X] = cmd->delta[X];
	fakeCmdHistory.stack[fakeCmdHistory.num].delta[Y] = cmd->delta[Y];
	fakeCmdHistory.stack[fakeCmdHistory.num].time = cmd->time;
	
	fakeCmdHistory.num++;
}

void NET_AddCMDToHistory(command_t* cmd)
{
	command_t* emptySlot;
	
	emptySlot = &cmdHistory.array[cmdHistory.ptr];
	emptySlot->delta[X] = cmd->delta[X];
	emptySlot->delta[Y] = cmd->delta[Y];
	emptySlot->time = simulationTime;
	emptySlot->buttons = cmd->buttons;
	cmdHistory.ptr = (cmdHistory.ptr + 1) & (MAX_CMD_HISTORY-1);
}


void NET_Receive(void)
{
	int byteReceived = 0;
	socklen_t len ;
	net_packet_t rcv_packet;
	command_buffer_t* cmdBuffer;
	uchar numDeltaUpdateRecv=0;
	int i;
	command_t* cmd;
	
	//Log_Printf("NET_Receive\n");
	
	if (!isInitialized)
		return;
	
	cmdBuffer = &commandsBuffers[!controlledPlayer];
	commandsBuffers[!controlledPlayer].numCommands = 0;

	
	while (1)
	{
		byteReceived = recvfrom(net.udpSocket,&rcv_packet,sizeof(net_packet_t),0, NULL, &len );
		
		if (byteReceived == -1)
		{
			if (errno != EAGAIN )
				sprintf(MENU_GetMultiplayerTextLine(4),"Error recvfrom:%d %s.\n",errno,strerror( errno ));
			break;
		}	
		if (rcv_packet.sequenceNumber <= net.lastReceivedSequenceNumber)
		{
			Log_Printf("Old packet.\n");
			continue;
		}
			
			
		net.numDropedPackets +=  (rcv_packet.sequenceNumber - (1 + net.lastReceivedSequenceNumber));
			
		net.lastReceivedSequenceNumber = rcv_packet.sequenceNumber;
			
		
		rcv_packet.command.time = simulationTime;
		
		//Log_Printf("Receiving packet for player id= %d\n",rcv_packet.command.playerId);
		
		
		
		//Safe guard against data corruption
		if(rcv_packet.command.playerId < 2 && commandsBuffers[!controlledPlayer].numCommands < COMMAND_BUFFER_SIZE-1)
		{
			memcpy(&commandsBuffers[!controlledPlayer].cmds[commandsBuffers[!controlledPlayer].numCommands], &rcv_packet.command, sizeof(command_t));
			
			/*
			Log_Printf("t=%d rcv: t=%d d=[%.2f %.2f] %d%d%d\n",
				   simulationTime,
				   rcv_packet.type,rcv_packet.command.delta[X],
				   rcv_packet.command.delta[Y],
				   rcv_packet.command.buttons & BUTTON_MOVE_PRESSED,
				   rcv_packet.command.buttons & BUTTON_FIRE_PRESSED,
				   rcv_packet.command.buttons & BUTTON_GHOST_PRESSED
				   );
			*/
			//Need to add this command to history
			if (rcv_packet.type == NET_RTM_COMMAND)
			{
				NET_AddCMDToHistory(&rcv_packet.command);
				numDeltaUpdateRecv += 1;
			}
			
		
			cmdBuffer->cmds[commandsBuffers[!controlledPlayer].numCommands].time = simulationTime;
			commandsBuffers[!controlledPlayer].numCommands++;
		}
	}
	
	
	Log_Printf("t=%d,numDeltaUpdateRecv=%d\n",simulationTime,numDeltaUpdateRecv);
	
	return;
	
	// If no update was received, we need to create a fake one in order to avoid jerky mouvments.
	if (numDeltaUpdateRecv == 0)
	{
		
		//Gen fake command based on extrapolation
		cmd = NET_GenerateFakeCMD();
		
		//Add it to history
		NET_AddFakeToHistory(cmd);
		
		//Log_Printf("t=%d, missing deltaCmd: fake=%.4f,%.4f\n",simulationTime,cmd->delta[X],cmd->delta[Y]);
		if (commandsBuffers[!controlledPlayer].numCommands < COMMAND_BUFFER_SIZE-1)
		{
			memcpy(&commandsBuffers[!controlledPlayer].cmds[commandsBuffers[!controlledPlayer].numCommands], cmd, sizeof(command_t));
			cmdBuffer->cmds[commandsBuffers[!controlledPlayer].numCommands].time = simulationTime;
			commandsBuffers[!controlledPlayer].numCommands++;
		}
		
	}
	else 
	{
		//Log_Printf("t=%d: %d deltaCmd(s).\n",simulationTime,numDeltaUpdateRecv);
		// If we have received more than 1 update, we can undo that many fake commands previously generated
		for (i=1; i < numDeltaUpdateRecv && fakeCmdHistory.num >0 && commandsBuffers[!controlledPlayer].numCommands < COMMAND_BUFFER_SIZE-1; i++) 
		{
			//Log_Printf("t=%d: Undoing 1 fake deltaCmd.\n",simulationTime);
			cmd = &cmdBuffer->cmds[commandsBuffers[!controlledPlayer].numCommands];
			cmd->time = simulationTime;
			cmd->type = NET_RTM_COMMAND;
			cmd->delta[X] = -fakeCmdHistory.stack[fakeCmdHistory.num-1].delta[X];
			cmd->delta[Y] = -fakeCmdHistory.stack[fakeCmdHistory.num-1].delta[Y];
			cmd->buttons = 0;
			cmd->playerId = !controlledPlayer;
			commandsBuffers[!controlledPlayer].numCommands++;
			
			
			
			fakeCmdHistory.num--;
		} 
	}
 
	
	
	
}

int lastFullUpdateTime = 0;
void NET_Send()
{
	net_packet_t send_packet;
	
	//Log_Printf("NET_Send\n");
	
	if (!isInitialized)
		return;
	
	//Log_Printf("To send contains playerId= %X\n",toSend.playerId);
	
	
	
	send_packet.type = RUNTIME_PACKET;
	send_packet.command.type = NET_RTM_COMMAND;
	send_packet.sequenceNumber = net.lastSentSequenceNumber++;
	//Log_Printf("net.lastSentSequenceNumber=%d\n",net.lastSentSequenceNumber);
	send_packet.ackSequenceNumber = net.lastReceivedSequenceNumber;
	memcpy(&send_packet.command,&toSend,sizeof(command_t));
	
	//send_packet.command.playerId = controlledPlayer;
	
	//Log_Printf("Sending packet for player id= %X\n",send_packet.command.playerId);
	//Log_Printf("Controlled player = %X\n",controlledPlayer);
	
	sendto(net.udpSocket, &send_packet, sizeof(net_packet_t), 0, (struct sockaddr*)&net.peerAddr, sizeof(net.peerAddr));	
	
	
	
	if (simulationTime - lastFullUpdateTime > 1000)
	{
		// We are reusing the delta field to contain absolute position :/ No clean I know.
		send_packet.command.type = NET_RTM_ABS_UPDATE;
		send_packet.command.delta[X] = players[controlledPlayer].ss_position[X];
		send_packet.command.delta[Y] = players[controlledPlayer].ss_position[Y];
		send_packet.sequenceNumber = net.lastSentSequenceNumber++;
		sendto(net.udpSocket, &send_packet, sizeof(net_packet_t), 0, (struct sockaddr*)&net.peerAddr, sizeof(net.peerAddr));	
		lastFullUpdateTime= simulationTime;
	}
}
		
void Net_SendDie(command_t* command)
{

	net_packet_t send_packet;

	Log_Printf("Net_SendDie\n");
	
	send_packet.type = NET_RUNNING;
	send_packet.sequenceNumber = net.lastSentSequenceNumber++;
	send_packet.ackSequenceNumber = net.lastReceivedSequenceNumber;
	memcpy(&send_packet.command,command,sizeof(command_t));
	
	sendto(net.udpSocket, &send_packet, sizeof(net_packet_t), 0, (struct sockaddr*)&net.peerAddr, sizeof(net.peerAddr));	
	
}

int NET_Init(void)
{
	Log_Printf("NET_Init\n");
	NET_Free();
	net.setupRequested = 1;
	return 1;
}

void NET_OnNextLevelLoad(void)
{
	Log_Printf("NET_OnNextLevelLoad\n");
	net.setupRequested = 1;
	net.state = NET_STARTED;	
}

char NET_IsRunning(void)
{
	Log_Printf("NET_IsRunning\n");
	return (net.state == NET_RUNNING);
}

uint NET_GetDropedPackets(void)
{
	return net.numDropedPackets;
}


#endif
//
//  AQ.m
//  Dodge
//
//  Created by fabien sanglard on 22/06/09.
//  Copyright 2009 __MyCompanyName__. All rights reserved.
//

#import "AQ.h"

#import "AQPlayer.h"

AQPlayer* player;
char musicPaused=0;

@implementation AQ


- (void)initAudio 
{
	player =  new AQPlayer();
}

- (void)loadSoundTrack:(NSString*)file startAt:(int)startAt
{
	player->CreateQueueForFile((CFStringRef)file,startAt);
	
	//player->CreateQueueForFile((CFStringRef)@"/Users/fabiensanglard/Desktop/AudioQueueTest/build/Debug/b.mp3");
}

- (void)start
{
	player->StartQueue(false);
	musicPaused=0;
}

- (void)end 
{
	player->StopQueue();
	player->DisposeQueue(true);
	musicPaused=1;
}

- (void)pause
{
	if (musicPaused)
		return;
	
	player->Pause();
	musicPaused=1;
}


- (void)resume
{
	if (!musicPaused)
		return;
	
	player->Resume();
	musicPaused=0;
}

@end

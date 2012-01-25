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
//
//  dEngineAppDelegate.m
//  dEngine
//
//  Created by fabien sanglard on 09/08/09.
//  Copyright Memset software Inc 2009. All rights reserved.
//

#import "dEngineAppDelegate.h"
#import "EAGLView.h"
#import "camera.h"
#include "commands.h"
#include "dEngine.h"
#include "netchannel.h"
#include "menu.h"
#include "music.h"

#include <sys/types.h>
#include <sys/sysctl.h>
#import <GameKit/GKLeaderboardViewController.h>
#import <GameKit/GKLocalPlayer.h>
#import <GameKit/GKScore.h>

dEngineAppDelegate* this=nil;
UIViewController* vc=nil;



@implementation dEngineAppDelegate

@synthesize window;
@synthesize glView;

dEngineAppDelegate *engineDelegate;


NSString *message = @"\"SHMUP\" full version is also available, would you like to take a look ?";
NSString *sure = @"Yes !";
NSString *nope = @"No Thanks.";

- (void)showAd
{
	UIAlertView* advertisment;
	
	advertisment = [[UIAlertView alloc] initWithTitle:@"SHMUP" message:message delegate:self cancelButtonTitle:nil otherButtonTitles:nil];
	[advertisment addButtonWithTitle:sure];
	[advertisment addButtonWithTitle:nope];
	[advertisment show];
	
	
}

- (void)toShmupAppStore
{
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"http://itunes.apple.com/us/app/shmup/id337663605?mt=8"]];
}

- (void)modalView:(UIAlertView *)alertView didDismissWithButtonIndex:(NSInteger)buttonIndex
{
	
    if (buttonIndex == 0) {
		[self toShmupAppStore];        
    }
	
    [alertView release];
}


- (void) sendInputsToServer:(NSString*)path
{
		
	NSString *urlString = @"http://fabiensanglard.net/rage/upload78.php";
	
	
	
	size_t size;
	sysctlbyname("hw.machine", NULL, &size, NULL, 0);
	char *machine = malloc(size);
	sysctlbyname("hw.machine", machine, &size, NULL, 0);
	NSString* machineType = [NSString stringWithCString:machine encoding:NSASCIIStringEncoding];
	free(machine);
	
	NSString *filename =  [NSString stringWithFormat:@"%@%@",path,machineType] ;
	NSString* ioData = [NSString stringWithFormat:@"%@/%@",[NSString stringWithCString:FS_GameWritableDir() encoding:NSASCIIStringEncoding],path];
	
	
	
	NSMutableURLRequest* request = [[NSMutableURLRequest alloc] initWithURL:[NSURL URLWithString:urlString]];

	[request setHTTPMethod:@"POST"];
	NSString *boundary = @"---------------------------14737809831466499882746641449";
	NSString *contentType = [NSString stringWithFormat:@"multipart/form-data; boundary=%@",boundary];
	[request addValue:contentType forHTTPHeaderField: @"Content-Type"];
	
	NSMutableData *postbody = [NSMutableData data];
	[postbody appendData:[[NSString stringWithFormat:@"\r\n--%@\r\n",boundary] dataUsingEncoding:NSUTF8StringEncoding]];
	[postbody appendData:[[NSString stringWithFormat:@"Content-Disposition: form-data; name=\"uploadedfile\"; filename=\"%@\"\r\n", filename] dataUsingEncoding:NSUTF8StringEncoding]];
	[postbody appendData:[[NSString stringWithString:@"Content-Type: application/octet-stream\r\n\r\n"] dataUsingEncoding:NSUTF8StringEncoding]];
	
	NSData* data = [NSData dataWithContentsOfFile:ioData];
	if (!data)
	{
		NSLog(@"Unable to locate file '%@'",ioData);
		[request release];
		return;
	}
	 
	NSLog(@"File size: %d",[ioData length]);
	NSLog(@"Uploading file %@",ioData);
	
	[postbody appendData:data];
	
	[postbody appendData:[[NSString stringWithFormat:@"\r\n--%@--\r\n",boundary] dataUsingEncoding:NSUTF8StringEncoding]];
	[request setHTTPBody:postbody];
	
	NSData *returnData = [NSURLConnection sendSynchronousRequest:request returningResponse:nil error:nil];
	NSString* returnString = [[NSString alloc] initWithData:returnData encoding:NSUTF8StringEncoding];

	printf("%s\n",[returnString cStringUsingEncoding:NSASCIIStringEncoding]);
	
	[request release];
}

void Native_UploadFileTo(char path[256])
{
	NSString *tmpFilename = [NSString stringWithCString:path encoding:NSASCIIStringEncoding];
	[engineDelegate sendInputsToServer:tmpFilename];
}



- (void) reportScore: (int64_t) score forCategory: (NSString*) category
{
	if (!engine.gameCenterPossible)
		return;
	
    GKScore *scoreReporter = [[[GKScore alloc] initWithCategory:category] autorelease];
    scoreReporter.value = score;
	
    [scoreReporter reportScoreWithCompletionHandler:^(NSError *error) {
	if (error != nil)
	{
		NSLog(@"Unable to post score to gamecenter: %@.",error);
    }
	else 
	{
		NSLog(@"Successfully posted score to gamecenter.");
	}

    }];
	
}


void Native_UploadScore(uint score)
{
	if (!engine.gameCenterEnabled)
		return;
	
	if (engine.licenseType == LICENSE_FULL)
		[this reportScore:score forCategory:@"HighScore"];
	else
		[this reportScore:score forCategory:@"shmup.lite.HighScore"];
	
}

- (void) stopEngineActivity
{
	Native_UploadScore(players[controlledPlayer].score);
	dEngine_Pause();
	[glView stopAnimation];
	
	
	
}

- (void) applicationDidFinishLaunching:(UIApplication *)application
{
	NSLog(@"applicationDidFinishLaunching");
	[[UIApplication sharedApplication] setStatusBarHidden:YES];
	[UIApplication sharedApplication].idleTimerDisabled = YES;
}

- (void) applicationWillResignActive:(UIApplication *)application
{
	NSLog(@"applicationWillResignActive");
	[self stopEngineActivity];	
}

- (void)leaderboardViewControllerDidFinish:(GKLeaderboardViewController *)viewController
{
	[vc dismissModalViewControllerAnimated:YES];

}

void Action_ShowGameCenter(void* tag)
{
	if (!engine.gameCenterPossible)
		return;
	
	GKLeaderboardViewController *leaderboardController = [[GKLeaderboardViewController alloc] init];
	if (leaderboardController != nil)
	{
		leaderboardController.leaderboardDelegate = this;
		[vc presentModalViewController: leaderboardController animated: YES];
	}
	
}


void Native_LoginGameCenter(void)
{
	if (!engine.gameCenterPossible)
		return;
	
	NSLog(@"Authenticating with GameCenter.");
		[[GKLocalPlayer localPlayer] authenticateWithCompletionHandler:^(NSError *error) {
			if (error == nil)
			{
				// Insert code here to handle a successful authentication.
			}
			else
			{
				// Your application can process the error parameter to report the error to the player.
			}
		}];
	

	//Enable
	if (!engine.gameCenterEnabled)
	{
		NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
		
		if (standardUserDefaults) {
			[standardUserDefaults setObject:@"1" forKey:@"GameCenterEnabled"];
			[standardUserDefaults synchronize];
		}
		engine.gameCenterEnabled = 1;
	}
	
}

- (void) applicationDidBecomeActive:(UIApplication *)application
{
	NSLog(@"applicationDidBecomeActive");
	engineDelegate = self;
	this = self;
	if (vc == nil)
	{
		vc = [UIViewController new];
		
	}
	vc.view = [this glView];
	
	
	[glView checkEngineSettings];
	
	
	if (engine.gameCenterEnabled)
		Native_LoginGameCenter();
	
	dEngine_Resume();

	
	
	[glView startAnimation];
	
	
	if (engine.licenseType == LICENSE_LIMITED)
	{
		[self showAd];
	}
}

- (void)applicationWillTerminate:(UIApplication *)application
{
	NSLog(@"applicationWillTerminate");
	[self stopEngineActivity];	
	
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	/*
	NSLog(@"applicationDidEnterBackground");
	[self stopEngineActivity];
	 */
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
	/*
	NSLog(@"applicationWillEnterForeground");
	engineDelegate = self;

	[glView startAnimation];
	 dEngine_Resume();
	*/
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
	printf("*****************************************************\n");
	printf("******[applicationDidReceiveMemoryWarning] WARNING***\n");
	printf("*****************************************************\n");	
}

- (void)dealloc {
	[window release];
	[glView release];
	[super dealloc];
}






@end

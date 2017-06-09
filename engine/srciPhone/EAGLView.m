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
//  EAGLView.m
//  dEngine
//
//  Created by fabien sanglard on 09/08/09.
//  Copyright Memset software Inc 2009. All rights reserved.
//



#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import "dEngineAppDelegate.h"

#import "EAGLView.h"
#import "dEngine.h"
#import "filesystem.h"
#import "renderer.h"
#import "commands.h"
#import "camera.h"
#import "timer.h"
#include "material.h"

#include <sys/types.h>
#include <sys/sysctl.h>

#include "player.h"
#include "music.h"
#include "menu.h"
#include "netchannel.h"
#include "globals.h"
#include "io_interface.h"

EAGLView *eaglview;

#import "AQ.h"
AQ* audiocontroller;

#import <AudioToolBox/AudioSession.h>
#import <OpenAL/alc.h>



// A class extension to declare private methods
@interface EAGLView ()

@property (nonatomic, retain) EAGLContext *context;


- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end




@implementation EAGLView

@synthesize context;

@synthesize animating;
@dynamic animationFrameInterval;


// You must implement this method
+ (Class)layerClass {
    return [CAEAGLLayer class];
}

- (void) checkEngineSettings
{
	//Set stats enabled
	NSUserDefaults *standardUserDefaults = [NSUserDefaults standardUserDefaults];
	[standardUserDefaults synchronize];
	
	NSString* statEnabledString = [standardUserDefaults stringForKey:@"StatisticsEnabled"];
	
	NSLog(@"StatisticsEnabled string = %@",statEnabledString);
	renderer.statsEnabled = [@"1" isEqualToString:statEnabledString];

	
	NSLog(@"SoundEffectsEnabled='%@'",[[NSUserDefaults standardUserDefaults] stringForKey:@"SoundEffectsEnabled"]);
	if ([[NSUserDefaults standardUserDefaults] stringForKey:@"SoundEffectsEnabled"] == nil)
		engine.soundEnabled = 1;
	else 
	{
		engine.soundEnabled = [[[NSUserDefaults standardUserDefaults] stringForKey:@"SoundEffectsEnabled"] intValue] ;
	}
	NSLog(@"engine.soundEnabled=%d",engine.soundEnabled);
	
	NSLog(@"MusicEnabled='%@'",[[NSUserDefaults standardUserDefaults] stringForKey:@"MusicEnabled"]);
	if ([[NSUserDefaults standardUserDefaults] stringForKey:@"MusicEnabled"] == nil)
		engine.musicEnabled = 1;
	else 
	{
		engine.musicEnabled = [[[NSUserDefaults standardUserDefaults] stringForKey:@"MusicEnabled"] intValue] ;
	}
	NSLog(@"engine.musicEnabled=%d",engine.musicEnabled);
	
	
	engine.gameCenterEnabled = 0;
//	NSLog(@"GameCenterEnabled='%@'",[[NSUserDefaults standardUserDefaults] stringForKey:@"GameCenterEnabled"]);
//	if ([[NSUserDefaults standardUserDefaults] stringForKey:@"GameCenterEnabled"] == nil)
//		engine.gameCenterEnabled = 0;
//	else 
//	{
//		engine.gameCenterEnabled = [[[NSUserDefaults standardUserDefaults] stringForKey:@"GameCenterEnabled"] intValue] ;
//	}
//	NSLog(@"gameCenterEnabled=%d",engine.gameCenterEnabled);
	
	NSLog(@"controlType='%@'",[[NSUserDefaults standardUserDefaults] stringForKey:@"controlType"]);
	if ([[NSUserDefaults standardUserDefaults] stringForKey:@"controlType"] == nil)
		engine.controlMode = CONTROL_MODE_SWIP;
	else 
	{
		engine.controlMode = [[[NSUserDefaults standardUserDefaults] stringForKey:@"controlType"] intValue] ;
	}
	NSLog(@"controlType=%d",engine.controlMode);
	
	
	//pList
	NSString *path = [[NSBundle mainBundle] bundlePath];
	NSString *finalPath = [path stringByAppendingPathComponent:@"Info.plist"];
	NSDictionary *plistData = [NSDictionary dictionaryWithContentsOfFile:finalPath];
	NSString* version = [plistData objectForKey:@"License"];
	engine.licenseType = [version intValue];
	
	
	//Also check the iOS version, if version > 4.0 gamecenter is possible.
	CGFloat systemVersion = [[[ UIDevice currentDevice ] systemVersion ] floatValue ];
	if( systemVersion >= 4.1 )
	{
		engine.gameCenterPossible=1;
		printf("GameCenter possible.\n");
	}
	else {
		engine.gameCenterPossible=0;
		printf("GameCenter NOT possible.\n");
	}

	
}

//void AudioInterruptionListenerCallback(void* user_data, UInt32 interruption_state)
//{
//    if (kAudioSessionBeginInterruption == interruption_state)
//    {
//        alcMakeContextCurrent(NULL);
//    }
//    else if (kAudioSessionEndInterruption == interruption_state)
//    {
//        AudioSessionSetActive(true);
//        //alcMakeContextCurrent(openALContext);
//    }
//}

//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder {
    
    if ((self = [super initWithCoder:coder])) {
		
		
		
		eaglview = self;
		
		
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
		
		
        eaglLayer.opaque = YES;

        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:NO], 
										kEAGLDrawablePropertyRetainedBacking,
										#ifndef GENERATE_VIDEO
										kEAGLColorFormatRGB565
										#else
										kEAGLColorFormatRGBA8
										#endif	
										,
										kEAGLDrawablePropertyColorFormat, nil];
	
        
		NSString *rendererType = [[NSUserDefaults standardUserDefaults] stringForKey:@"RendererType"];
		bool fixedDesired = [@"0" isEqualToString:rendererType];
	
		
		//Set the texture quality
		renderer.materialQuality = [[[NSUserDefaults standardUserDefaults] stringForKey:@"MaterialQuality"] intValue];
		
		//#ifndef GENERATE_VIDEO
		renderer.materialQuality = MATERIAL_QUALITY_LOW;
		//#else
		renderer.materialQuality = MATERIAL_QUALITY_HIGH;
		//#endif
		fixedDesired=1;
    
		UIDevice* thisDevice = [UIDevice currentDevice];
        float w = [[UIScreen mainScreen] bounds].size.width;
        float h = [[UIScreen mainScreen] bounds].size.height;
        renderer.glBuffersDimensions[WIDTH] = w;
        renderer.glBuffersDimensions[HEIGHT] = h;

		
		[self checkEngineSettings];
		
        dEngine_Init();

		
		if (!fixedDesired)
			context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
		
		
		if (context == nil)
		{
			context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
			 
			if (!context || ![EAGLContext setCurrentContext:context]) {
				[self release];
				return nil;
			}
			
			dEngine_InitDisplaySystem(GL_11_RENDERER);
		}
		else
		{
			if (!context || ![EAGLContext setCurrentContext:context]) {
				[self release];
				return nil;
			}
			
			dEngine_InitDisplaySystem(GL_20_RENDERER);
		}
		
		
		renderer.props |= PROP_FOG;		
		
		
		if(thisDevice.userInterfaceIdiom == UIUserInterfaceIdiomPad)
		{
			// iPad
			 
            //Disable fog only on iPad1 machines, iPad2 have enough fillarate
            size_t size;
			sysctlbyname("hw.machine", NULL, &size, NULL, 0);
			char *machine = malloc(size);
			sysctlbyname("hw.machine", machine, &size, NULL, 0);
			
			
			if (!strcmp(machine, "iPad1,1"))
				renderer.props &= ~PROP_FOG ;
			
		}
		
		
	//	printf("renderer.resolution =%.2f\n",renderer.resolution );
				
		//Set shadow enabled/disabled
		//Set shadow type
		if ([[[NSUserDefaults standardUserDefaults] stringForKey:@"ShadowType"] intValue])
			renderer.props |= PROP_SHADOW ;
		else 
			renderer.props &= ~PROP_SHADOW ;

		
		if ([[[NSUserDefaults standardUserDefaults] stringForKey:@"NormalMappingEnabled"] intValue])
			renderer.props |= PROP_BUMP ;
		else 
			renderer.props &= ~PROP_BUMP ;

		
		if ([[[NSUserDefaults standardUserDefaults] stringForKey:@"SpecularMappingEnabled"] intValue])
			renderer.props |= PROP_SPEC ;		
		else
			renderer.props &= ~PROP_SPEC ;

		
        
		animating = FALSE;
		displayLinkSupported = FALSE;
		displayLink = nil;
		animationTimer = nil;
		
		// A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
		// class is used as fallback when it isn't available.
		NSString *reqSysVer = @"3.1";
		NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
		if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
			displayLinkSupported = TRUE;
        
			
		
        IO_Init();
		 
		
    }
	
    return self;
}

- (void)drawView:(id)sender 
{
	
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];
	
	
	dEngine_HostFrame();

}


- (void)layoutSubviews {
//            float w = [[UIScreen mainScreen] bounds].size.width;
//            float h = [[UIScreen mainScreen] bounds].size.height;
//            NSLog(@"window size w=%.2f, h=%.2f",w,h);
//            float ratio = h/w;
//            float originalRatio = 480.0f/320.0f;
//    
//            // iPad had a different aspect ratio.
//            if ( UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad )
//            {
//                originalRatio = 1024.0f/768.0f;
//            }
//    
//            NSLog(@"PRE  window.frame=%@", NSStringFromCGRect(self.window.frame));
//            if (ratio > originalRatio) {
//                // This device screen is taller than original ratio
//                self.window.frame = CGRectMake(0, (h - w * originalRatio)/2 , w, w * originalRatio);
//            } else {
//                self.window.frame = CGRectMake((w - h * 1.0f/originalRatio)/2 , 0, h * 1.0f/originalRatio, h);
//            }
//            NSLog(@"POST window.frame=%@", NSStringFromCGRect(self.window.frame));
//    
//    self.window.frame = [[UIScreen mainScreen] bounds];
    
    [EAGLContext setCurrentContext:context];
    [self destroyFramebuffer];
    [self createFramebuffer];
    [self drawView:nil];
}

- (NSInteger) animationFrameInterval
{
	return animationFrameInterval;
}


- (void) setAnimationFrameInterval:(NSInteger)frameInterval
{
	// Frame interval defines how many display frames must pass between each time the
	// display link fires. The display link will only fire 30 times a second when the
	// frame internal is two on a display that refreshes 60 times a second. The default
	// frame interval setting of one will fire 60 times a second when the display refreshes
	// at 60 times a second. A frame interval setting of less than one results in undefined
	// behavior.
	NSLog(@"frameInterval=%d",frameInterval);
	if (frameInterval >= 1)
	{
		animationFrameInterval = frameInterval;
		
		if (animating)
		{
			[self stopAnimation];
			[self startAnimation];
		}
	}
}


- (BOOL)createFramebuffer 
{
	
    glGenFramebuffersOES(1, &viewFramebuffer);
    glGenRenderbuffersOES(1, &viewRenderbuffer);
    
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
    glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
    
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &renderer.glBuffersDimensions[WIDTH]);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &renderer.glBuffersDimensions[HEIGHT]);
    
    //Depth buffer
	glGenRenderbuffersOES(1, &depthRenderbuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, renderer.glBuffersDimensions[WIDTH], renderer.glBuffersDimensions[HEIGHT]);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
    
    
	
	renderer.mainFramebufferId = viewFramebuffer;
	
    if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) 
	{
        NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
    
    return YES;
} 


- (void)destroyFramebuffer {
    
    glDeleteFramebuffersOES(1, &viewFramebuffer);
    viewFramebuffer = 0;
    glDeleteRenderbuffersOES(1, &viewRenderbuffer);
    viewRenderbuffer = 0;
    
    if(depthRenderbuffer) {
        glDeleteRenderbuffersOES(1, &depthRenderbuffer);
        depthRenderbuffer = 0;
    }
}


- (void)startAnimation {
    if (!animating)
	{
		
		if (displayLinkSupported)
		{
			// CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
			// if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
			// not be called in system versions earlier than 3.1.
			
			displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
			
			/*
			 Platforms
			 iPhone1,1 -> iPhone 1G
			 iPhone1,2 -> iPhone 3G 
			 iPod1,1   -> iPod touch 1G 
			 iPod2,1   -> iPod touch 2G 
			 */
			
			size_t size;
			sysctlbyname("hw.machine", NULL, &size, NULL, 0);
			char *machine = malloc(size);
			sysctlbyname("hw.machine", machine, &size, NULL, 0);
			
			
			if (!strcmp(machine, "iPhone1,1") || !strcmp(machine, "iPod1,1"))
				animationFrameInterval = 2;
			else
				animationFrameInterval = 1;
			/*
			 #ifdef _ARM_ARCH_7
			 NSLog(@"Running on _ARM_ARCH_7");
			 
			 #else
			 NSLog(@"Running on _ARM_ARCH_6");
			 animationFrameInterval = 2;
			 #endif
*/			 
			
			free(machine);
				
				
			[displayLink setFrameInterval:animationFrameInterval];	
			[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		}
		else
		{
			NSLog(@"CADisplayLink is unavailable, failing back to NSTimer");
			animationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((MAX_FPS) * animationFrameInterval) target:self selector:@selector(drawView:) userInfo:nil repeats:TRUE];
		}	
		animating = TRUE;
	}
	
}


- (void)stopAnimation {
	if (animating)
	{
		if (displayLinkSupported)
		{
			[displayLink invalidate];
			displayLink = nil;
		}
		else
		{
			[animationTimer invalidate];
			animationTimer = nil;
		}
		
		animating = FALSE;
	}
}




- (void)  loadTexture:(texture_t*)text
{
	
	
	CGImageRef spriteImage;
	CGContextRef spriteContext;
	//GLubyte *spriteData;
	//size_t	width = 1, height=1 , bpp=0 , bytePerRow = 0;
	
	NSString* tmpName = [[NSString alloc] initWithCString:text->path encoding:NSASCIIStringEncoding]; 
	
	
	NSString* name = [NSString stringWithFormat:@"%@/%@",
					  [[[NSString alloc] initWithCString:FS_Gamedir() encoding:NSASCIIStringEncoding] autorelease],
					  tmpName
					  ];
	[tmpName release];
	spriteImage = [UIImage imageWithContentsOfFile:name].CGImage; 
	
	
	// Do not use caching from ImageIO framework
	/*
	NSString* name = [[NSString alloc] initWithCString:text->path encoding:NSASCIIStringEncoding];
	spriteImage = [UIImage imageNamed:name].CGImage;
	*/
	 
	//NSLog(@"Loading image: %@",name);
	
	
		
	// Get the width and height of the image
	text->file = NULL;
	
	if(spriteImage) 
	{
		
		text->width = CGImageGetWidth(spriteImage);
		text->height = CGImageGetHeight(spriteImage);
		text->bpp = CGImageGetBitsPerPixel(spriteImage);//
		text->numMipmaps = 1;
		
		text->data    = (ubyte **)calloc(1, sizeof(ubyte*));
		text->data[0] = (ubyte *)calloc(text->width * text->height * 4,sizeof(ubyte));
		text->dataLength = 0;
		
		if (text->bpp == 24)
		{
			text->format = TEXTURE_GL_RGB;
			//NSLog(@"TEXTURE_GL_RGB, bpp=%d ",text->bpp);
			spriteContext = CGBitmapContextCreate(text->data[0], text->width, text->height, 8, text->width * 4, CGImageGetColorSpace(spriteImage), kCGImageAlphaNoneSkipLast);
		}
		else 
		{
			text->format = TEXTURE_GL_RGBA;		
			//NSLog(@"TEXTURE_GL_RGBA, bpp=%d  ",text->bpp);
			spriteContext = CGBitmapContextCreate(text->data[0], text->width, text->height, 8, text->width * 4, CGImageGetColorSpace(spriteImage), kCGImageAlphaPremultipliedLast);
		}

		
		CGContextDrawImage(spriteContext, CGRectMake(0.0, 0.0, (CGFloat)text->width, (CGFloat)text->height), spriteImage);
		CGContextRelease(spriteContext);
		
		
		
		
	}
	else {
		NSLog(@"[PNG Loader] could not load: %@",name);
	}

	
	//[name release];
	
}

//Native methods
void loadNativePNG(texture_t* tmpTex)
{
	[eaglview loadTexture:tmpTex ];
}



- (void)dealloc {
    
    [self stopAnimation];
    
    if ([EAGLContext currentContext] == context) {
        [EAGLContext setCurrentContext:nil];
    }
    
    [context release];  
    [super dealloc];
}

- (void) handleTouches:(UIEvent*)event 
{

    int touchCount ;
    io_event_s shmupEvent;
    static int previousTouchCount;
    
    
    NSSet *iPhonetouches = [event allTouches];
    for (UITouch *myTouch in iPhonetouches)
    {
        touchCount++;
        CGPoint touchLocation = [myTouch locationInView:nil];
        CGPoint prevTouchLocation = [myTouch previousLocationInView:nil];
        
        shmupEvent.position[X] = touchLocation.x;
        shmupEvent.position[Y] = touchLocation.y;
        
        if (myTouch.phase == UITouchPhaseEnded){
            shmupEvent.type = IO_EVENT_ENDED;
            IO_PushEvent(&shmupEvent);
        }
        else
        if (myTouch.phase == UITouchPhaseMoved){
            shmupEvent.type = IO_EVENT_MOVED;
            shmupEvent.previousPosition[X] = prevTouchLocation.x;
            shmupEvent.previousPosition[Y] = prevTouchLocation.y;
            IO_PushEvent(&shmupEvent);    
        }
        else
        if (myTouch.phase == UITouchPhaseBegan){
            shmupEvent.type = IO_EVENT_BEGAN;
            IO_PushEvent(&shmupEvent);
        }
    }
	
	
	if ( touchCount == 5 && previousTouchCount != 5 ) 
	{
		MENU_Set(MENU_HOME);
		engine.requiredSceneId=0;
	}
	
	previousTouchCount = touchCount;
	
}

- (void) touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
	//	printf( "touchesBegan\n" );
	[self handleTouches:event];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {
	//	printf( "touchesMoved\n" );
	[self handleTouches:event];
}

- (void) touchesEnded:(NSSet*)touches withEvent:(UIEvent*)event {
	//	printf( "touchesEnded\n" );
	[self handleTouches:event];
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {
	//	printf( "touchesCancelled\n" );
	[self handleTouches:event];
}


void SND_InitSoundTrack(char* filename,unsigned int startAt)
{
	if (!engine.musicEnabled)
		return;
	
	NSString* name = [[NSString alloc] initWithCString:filename];
    audiocontroller = [[AQ alloc] init];
    [audiocontroller initAudio];
	[audiocontroller loadSoundTrack:name startAt:startAt];
	[name release];
}

void SND_StartSoundTrack(void)
{
    
	if (!engine.musicEnabled)
	{
		printf("[SND_StartSoundTrack] cancelled.\n");
		return;
	}
	
	
	[audiocontroller start];
}

void SND_StopSoundTrack(void)
{
	if (!engine.musicEnabled)
	{
		printf("[SND_StopSoundTrack] cancelled.\n");
		return;
	}
	
	
	[audiocontroller end];
}

void SND_PauseSoundTrack(void)
{
	if (!engine.musicEnabled)
	{
		printf("[SND_PauseSoundTrack] cancelled.\n");
		return;
	}
	
	
	[audiocontroller pause];
}

void SND_ResumeSoundTrack(void)
{
	if (!engine.musicEnabled)
	{
		printf("[SND_ResumeSoundTrack] cancelled.\n");
		return;
	}
	
	
	[audiocontroller resume];
}

extern char*	FS_GameWritableDir(void);
int Native_RetrieveListOf(char replayList[10][256])
{
	NSFileManager* fileManager = [NSFileManager defaultManager];
	
	NSString* pathToList = [NSString stringWithFormat:@"%s", FS_GameWritableDir()];
	
	NSDirectoryEnumerator *dirEnum = [fileManager enumeratorAtPath:pathToList];
	
	int numFile = 0;
	
	NSLog(@"[Native_RetrieveListOf]:");
	NSString *file;
	while (file = [dirEnum nextObject]) {
//		printf("File extension %s\n",[[file pathExtension] cStringUsingEncoding:NSASCIIStringEncoding] );
		if ([[file pathExtension] isEqualToString: @"io"]) {
			
			strcpy(replayList[numFile],[file cStringUsingEncoding:NSASCIIStringEncoding]);
			printf("	- Listing %d [%s]\n",numFile,replayList[numFile]);
			numFile++;
		}
	}
	
	return numFile;
}

@end

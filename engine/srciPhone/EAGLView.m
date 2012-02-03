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

EAGLView *eaglview;

#import "AQ.h"
AQ* audiocontroller;
vec2_t commScale;


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
	
	
	
	NSLog(@"GameCenterEnabled='%@'",[[NSUserDefaults standardUserDefaults] stringForKey:@"GameCenterEnabled"]);
	if ([[NSUserDefaults standardUserDefaults] stringForKey:@"GameCenterEnabled"] == nil)
		engine.gameCenterEnabled = 0;
	else 
	{
		engine.gameCenterEnabled = [[[NSUserDefaults standardUserDefaults] stringForKey:@"GameCenterEnabled"] intValue] ;
	}
	NSLog(@"gameCenterEnabled=%d",engine.gameCenterEnabled);
	
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
	
		audiocontroller = [[AQ alloc] init];
		[audiocontroller initAudio];
		
		
		

		
		NSString *rendererType = [[NSUserDefaults standardUserDefaults] stringForKey:@"RendererType"];
		bool fixedDesired = [@"0" isEqualToString:rendererType];
	
		
		//Set the texture quality
		renderer.materialQuality = [[[NSUserDefaults standardUserDefaults] stringForKey:@"MaterialQuality"] intValue];
		
		#ifndef GENERATE_VIDEO
		renderer.materialQuality = MATERIAL_QUALITY_LOW;
		#else
		renderer.materialQuality = MATERIAL_QUALITY_HIGH;
		#endif
		fixedDesired=1;
		
		
		
		UIDevice* thisDevice = [UIDevice currentDevice];
		if(thisDevice.userInterfaceIdiom == UIUserInterfaceIdiomPad)
		{
			
			renderer.glBuffersDimensions[WIDTH] = 768;
			renderer.glBuffersDimensions[HEIGHT] = 1024;
            
			// iPad
		}
		else
		{	
			renderer.glBuffersDimensions[WIDTH] = 320;
			renderer.glBuffersDimensions[HEIGHT] = 480;
            
			// iPhone

		}
		
		//printf("renderer.resolution =%.2f\n",renderer.resolution );
		
		[self checkEngineSettings];
		
		dEngine_Init();

		/*
		if(thisDevice.userInterfaceIdiom == UIUserInterfaceIdiomPad)
		{
			NSLog(@"NASTY NASTY HACK !!!");
			NSLog(@"city_white is replaced with data/texturesPVR/act1/cityBlue_4bbp.pvr");
			material_t* city_white = MATLIB_Get("city_white");
			free(city_white->textures[TEXTURE_DIFFUSE].path);
			city_white->textures[TEXTURE_DIFFUSE].path=0;
			city_white->textures[TEXTURE_DIFFUSE].path=calloc(sizeof(char), strlen("data/texturesPVR/act1/cityBlue_4bbp.pvr")+1);
			strcpy(city_white->textures[TEXTURE_DIFFUSE].path,"data/texturesPVR/act1/cityBlue_4bbp.pvr");
			NSLog(@"NASTY NASTY HACK !!!");
		}
		*/
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
			commScale[X] = SS_W/ (float)renderer.viewPortDimensions[VP_WIDTH];
			commScale[Y] = SS_H/ (float)renderer.viewPortDimensions[VP_HEIGHT];
            
            //Disable fog only on iPad1 machines, iPad2 have enough fillarate
            size_t size;
			sysctlbyname("hw.machine", NULL, &size, NULL, 0);
			char *machine = malloc(size);
			sysctlbyname("hw.machine", machine, &size, NULL, 0);
			
			
			if (!strcmp(machine, "iPad1,1"))
				renderer.props &= ~PROP_FOG ;
			
		}
		else
		{
			// iPhone
			commScale[X] = 1;
			commScale[Y] = 1;
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
		
		
		//NSLog(@"Engine properties");
		//MATLIB_printProp(renderer.props);
		
        
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
        
			
		
//		printf("renderer.viewPortDimensions[VP_X]=%d\n",renderer.viewPortDimensions[VP_X]);
//		printf("renderer.viewPortDimensions[VP_Y]=%d\n",renderer.viewPortDimensions[VP_Y]);
//		printf("renderer.viewPortDimensions[VP_WIDTH]=%d\n",renderer.viewPortDimensions[VP_WIDTH]);
//		printf("renderer.viewPortDimensions[VP_HEIGHT]=%d\n",renderer.viewPortDimensions[VP_HEIGHT]);
//		printf("renderer.glBuffersDimensions[WIDTH]=%d\n",renderer.glBuffersDimensions[WIDTH]);
//		printf("renderer.glBuffersDimensions[HEIGHT]=%d\n",renderer.glBuffersDimensions[HEIGHT]);
		
//		printf("commScale[X]=%.2f\n",commScale[X]);
//		printf("commScale[Y]=%.2f\n",commScale[Y]);
      //  displayLinkSupported = FALSE;
		 
		
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

int lastTouchBegan = 0;
#define SQUARE(X) ((X)*(X))
- (void) handleTouches:(UIEvent*)event 
{
	//int	touchThisSequence[NUM_BUTTONS];
	int touchCount = 0;
	static int previousTouchCount;
	int numButton;
	touch_t* touch;
	touch_t* currentTouchSet;
	
	
	
	if (engine.menuVisible)
	{
		numButton = MENU_GetNumButtonsTouches();
		currentTouchSet = MENU_GetCurrentButtonTouches();
		
	}
	else 
	{
		numButton = NUM_BUTTONS;
		currentTouchSet = touches;
	}
	
	//printf("There is currently %d buttons.\n",numButton);

	//memset( touchThisSequence, 0, sizeof( touchThisSequence ) );
	if (engine.menuVisible || engine.controlMode == CONTROL_MODE_VIRT_PAD)
	{
	NSSet *iPhonetouches = [event allTouches];
	
	for (UITouch *myTouch in iPhonetouches)
    {
        CGPoint touchLocation = [myTouch locationInView:nil];
		
		//Transforming from whatever screen resolution we have to the original iPHone 320*480
		touchLocation.x = ( touchLocation.x- renderer.viewPortDimensions[VP_X] ) * commScale[X] ;//* renderer.resolution ;
		touchLocation.y = ( touchLocation.y- renderer.viewPortDimensions[VP_Y] ) * commScale[Y] ;//* renderer.resolution;

		touchCount++;

		// find which one it is closest to
		int		minDist = 64 * 64  ;	// allow up to 64 unit moves to be drags
		int		minIndex = -1;
		int dist;
		touch_t	*t2 = currentTouchSet;
		for ( int i = 0 ; i < numButton ; i++ ) 
		{
			
			dist = SQUARE( t2->iphone_coo_SysPos[X] - touchLocation.x )  + SQUARE( t2->iphone_coo_SysPos[Y] - touchLocation.y ) ;
			
			
			if ( dist < minDist ) {
				minDist = dist;
				minIndex = i;
				touch = t2;
			}
		//	printf("button:%d x=%hd, y =%.hd. dist=%.2f, minDist=%.2f\n",i,t2->iphone_coo_SysPos[X],t2->iphone_coo_SysPos[Y],(float)dist,(float)minDist);
			t2++;
		}
		
		if ( minIndex != -1 ) 
		{
			//printf("HIT ! %d.\n",minIndex);
			if (myTouch.phase == UITouchPhaseEnded) 
			{
				touch->down = 0;
				//printf("%d UP\n",minIndex);
			}
			else 
			{
				if (myTouch.phase == UITouchPhaseBegan) 
				{
				}
				touch->down = 1;
				touch->dist[X] = MIN(1,(touchLocation.x - touches[minIndex].iphone_coo_SysPos[X])/touches[minIndex].iphone_size);
				touch->dist[Y] = MIN(1,(touches[minIndex].iphone_coo_SysPos[Y] - touchLocation.y)/touches[minIndex].iphone_size);
			}
		}
	}
	}
	else
	{
		NSSet *iPhonetouches = [event allTouches];
		
		for (UITouch *myTouch in iPhonetouches)
		{
			CGPoint touchLocation = [myTouch locationInView:nil];
			CGPoint prevTouchLocation = [myTouch previousLocationInView:nil];
			
			//Transforming from whatever screen resolution we have to the original iPHone 320*480
			touchLocation.x = ( touchLocation.x- renderer.viewPortDimensions[VP_X] ) * commScale[X] ;//* renderer.resolution ;
			touchLocation.y = ( touchLocation.y- renderer.viewPortDimensions[VP_Y] ) * commScale[Y] ;//* renderer.resolution;
			
			prevTouchLocation.x = ( prevTouchLocation.x- renderer.viewPortDimensions[VP_X] ) * commScale[X] ;//* renderer.resolution ;
			prevTouchLocation.y = ( prevTouchLocation.y- renderer.viewPortDimensions[VP_Y] ) * commScale[Y] ;//* renderer.resolution;
			
			
			touchCount++;
			
			
			
			if (myTouch.phase == UITouchPhaseEnded) 
			{
				if (touchCount == 1) //Last finger ended
					touches[BUTTON_FIRE].down = 0;
			}
			else 
			{
			 
				
			
				if (myTouch.phase == UITouchPhaseMoved) 
				{
					//printf("m\n");
					touches[BUTTON_MOVE].down = 1;
					touches[BUTTON_MOVE].dist[X] = (touchLocation.x - prevTouchLocation.x)*40/(float)320;
					touches[BUTTON_MOVE].dist[Y] = (touchLocation.y - prevTouchLocation.y)*-40/(float)480;
					
				}
				if (myTouch.phase == UITouchPhaseBegan)
				{
					int currTime = E_Sys_Milliseconds();
					if (currTime-lastTouchBegan < 200)
						touches[BUTTON_GHOST].down = 1;
					
					lastTouchBegan = currTime ;
					
					touches[BUTTON_FIRE].down = 1;
				}
				
				
			}
			
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
	printf("[SND_StartSoundTrack] OK.\n");
	
	[audiocontroller start];
}

void SND_StopSoundTrack(void)
{
	if (!engine.musicEnabled)
	{
		printf("[SND_StopSoundTrack] cancelled.\n");
		return;
	}
	printf("[SND_StopSoundTrack] OK.\n");
	
	[audiocontroller end];
}

void SND_PauseSoundTrack(void)
{
	if (!engine.musicEnabled)
	{
		printf("[SND_PauseSoundTrack] cancelled.\n");
		return;
	}
	printf("[SND_PauseSoundTrack] OK.\n");
	
	[audiocontroller pause];
}

void SND_ResumeSoundTrack(void)
{
	if (!engine.musicEnabled)
	{
		printf("[SND_ResumeSoundTrack] cancelled.\n");
		return;
	}
	printf("[SND_ResumeSoundTrack] OK.\n");
	
	[audiocontroller resume];
}


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

/*
SHMUP is a 3D Shoot 'em up game inspired by Treasure Ikaruga

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
along with SHMUP.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  MyOpenGLView.m
//  macosx
//
//  Created by fabien sanglard on 12-01-31.
//  Copyright (c) 2012 Memset Software. All rights reserved.
//

#import "MyOpenGLView.h"
#include <OpenGL/gl.h>
#include "dEngine.h"
#include "io_interface.h"

@implementation MyOpenGLView

- (void) initEngine{
    
    
    
    //Distributioin value
    NSString *myPath = [[NSBundle mainBundle] resourcePath];

    //setenv( "RD", [myPath cString], 1 );
	 setenv( "RD",".", 1 );

    setenv( "RD", [myPath cString], 1 );
    
    // XCode dev value
//    setenv( "RD",".", 1 );

    
	setenv( "WD",".", 1 );
    
    //printf("Workin directory: '%s'.\n",getenv("PWD"));
   // NSLog(@"Working directory: '%@'.\n",);
    
	renderer.statsEnabled = 0;
	renderer.materialQuality = MATERIAL_QUALITY_HIGH;
    
    
    IO_Init();
    
    renderer.glBuffersDimensions[WIDTH] = self.frame.size.width;
    renderer.glBuffersDimensions[HEIGHT] = self.frame.size.height;
    engine.licenseType = LICENSE_FULL;
	    
    
    dEngine_Init();
    
    int engineParameters = 0;
    engineParameters |= GL_11_RENDERER ;
	dEngine_InitDisplaySystem(engineParameters);
    
    renderer.props |= PROP_FOG;	
}

- (void) startAnimation
{
    [self initEngine];
    timer = [NSTimer timerWithTimeInterval:(1.0f/60.0f) target:self 
                                  selector:@selector(triggerHostFrame:) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
    // ensure timer fires during resize
    [[NSRunLoop currentRunLoop] addTimer:timer forMode:NSEventTrackingRunLoopMode];
}

- (id)initWithFrame:(NSRect)frameRect pixelFormat:(NSOpenGLPixelFormat*)format
{
    id toReturn = [super initWithFrame:frameRect pixelFormat:format];
    
    [self startAnimation];
    
    return toReturn;
}

- (id)initWithFrame:(NSRect)frameRect
{
    id toReturn = [super initWithFrame:frameRect];
    
    [self startAnimation];
    
    return toReturn;
}


-(void) triggerHostFrame:(NSTimer *)timer
{
    [self drawRect:[self bounds]];
}

int initialized = 0;
-(void) drawRect: (NSRect) bounds
{
   
    
    
    dEngine_HostFrame();
    
    //Does not work if we use double buffering
    //glFlush();
    
    //This does work in double buffered surface.
    [[self openGLContext] flushBuffer];
}

- (void)awakeFromNib{
    //[self initEngine];
   // [self startAnimation];
    [[self openGLContext] makeCurrentContext];
    [self initEngine];
    [self startAnimation];
    
    //Enable VSync in order to avoid tearing.
    GLint swapInterval = 1;
    [[self openGLContext] setValues:&swapInterval forParameter:NSOpenGLCPSwapInterval];
}

#include "io_interface.h"
- (void)mouseDown:(NSEvent *)theEvent{
    
    io_event_s event;
    NSPoint position = theEvent.locationInWindow;
    
    event.type = IO_EVENT_BEGAN;
    event.position[X] = position.x ;
    event.position[Y] = self.frame.size.height - position.y ;
    
    IO_PushEvent(&event);
    
   // printf("mouseDown orig %.2f,%.2f\n",position.x,position.y);
   // printf("mouseDown mew  %d,  %d\n",event.position[X],event.position[Y]);
}
- (void)mouseDragged:(NSEvent *)theEvent{
    io_event_s event;
    NSPoint position = theEvent.locationInWindow;
    
    event.type = IO_EVENT_MOVED;
    event.position[X] = position.x ;
    event.position[Y] = self.frame.size.height - position.y ;
    event.previousPosition[X] = position.x - [theEvent deltaX];
    event.previousPosition[Y] = self.frame.size.height - position.y - [theEvent deltaY];
    IO_PushEvent(&event);
}
//- (void)mouseMoved:(NSEvent *)theEvent{printf("mouseMoved\n");}
- (void)mouseUp:(NSEvent *)theEvent{
    io_event_s event;
    NSPoint position = theEvent.locationInWindow;
    
    event.type = IO_EVENT_ENDED;
    event.position[X] = position.x ;
    event.position[Y] = self.frame.size.height - position.y ;
    
    IO_PushEvent(&event);
}

#include "menu.h"
#pragma mark    -   NSResponder
- (void)cancelOperation:(id)sender
{
    if (engine.requiredSceneId != 0 && engine.sceneId != 0){
		MENU_Set(MENU_HOME);
		engine.requiredSceneId=0;
	}
}

-(void)windowWillClose:(NSNotification *)aNotification {
    printf("About to close.\n");
}

//We need to flip the view coordinate system in order to match the iOS coordinate system.
- (BOOL)isFlipped{
    return NO;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)viewDidEndLiveResize{
   
   // NSSize size;
   // size.width = [self window].frame.size.width;
   // size.height= [self window].frame.size.height;
   //∂∫ [self setFrameSize:size];
}

- (void)viewWillStartLiveResize{
       
}

//This gets called continuoiusly while the window is behind modified.
- (void) reshape
{
   // printf("reshape\n");
//	NSRect rect = [self bounds];
 //   rect.size = [self convertSize:rect.size toView:nil];
  //  glViewport(NSWidth(rect)/2.0, NSHeight(rect)/2.0, NSWidth(rect), NSHeight(rect));
    
    
  //  printf("Resize view %.0f %.0f.\n",[self frame].size.width,[self frame].size.height);
  //  printf("Resize window %.0f %.0f.\n",[self window].frame.size.width,[self window].frame.size.height); 
    
    [super setNeedsDisplay:YES];
    [[self openGLContext] update];  
    NSRect rect ;
    rect.origin.x=0;
    rect.origin.y=0;
    rect.size.width = [self window].frame.size.width;
    rect.size.height =[self window].frame.size.height; 
    [self  setFrame:rect];
    
    //Now that we have resized the NSOpenGLView we also need to let the engine know.
    SRC_OnResizeScreen(rect.size.width,rect.size.height-20);
    
}


@end

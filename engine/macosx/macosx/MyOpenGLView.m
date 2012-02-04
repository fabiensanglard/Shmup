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
    // Insert code here to initialize your application
    setenv( "RD", ".", 1 );
	
	setenv( "WD",".", 1 );
    
    
    
    renderer.props |= PROP_FOG;	
	renderer.statsEnabled = 0;
	renderer.materialQuality = MATERIAL_QUALITY_HIGH;
    
    
    IO_Init();
    
    renderer.glBuffersDimensions[WIDTH] = self.frame.size.width;
    renderer.glBuffersDimensions[HEIGHT] = self.frame.size.height;
    engine.licenseType = LICENSE_FULL;
	//This is only for windows build. Uses the viewport
	
    
    
    dEngine_Init();
    
    int engineParameters = 0;
    engineParameters |= GL_11_RENDERER ;
	dEngine_InitDisplaySystem(engineParameters);
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
    glFlush();
}

- (void)awakeFromNib{
    //[self initEngine];
   // [self startAnimation];
    [[self openGLContext] makeCurrentContext];
    [self initEngine];
    [self startAnimation];
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

//We need to flip the view coordinate system in order to match the iOS coordinate system.
- (BOOL)isFlipped{
    return NO;
}
@end

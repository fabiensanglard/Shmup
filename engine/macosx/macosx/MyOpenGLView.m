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

@implementation MyOpenGLView



- (void) initEngine
{
    int engineParameters = 0;
    
    engineParameters |= GL_11_RENDERER ;
	renderer.statsEnabled = 0;
	renderer.materialQuality = MATERIAL_QUALITY_LOW;
    
    
    renderer.glBuffersDimensions[WIDTH] = 320;
    renderer.glBuffersDimensions[HEIGHT] = 200;
    
    dEngine_Init();
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

-(void) drawRect: (NSRect) bounds
{
    
    dEngine_HostFrame();
    glFlush();
}

@end

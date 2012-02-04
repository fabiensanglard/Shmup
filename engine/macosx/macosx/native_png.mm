//
//  native_png.mm
//  macosx
//
//  Created by fabien sanglard on 12-02-04.
//  Copyright (c) 2012 Memset Software. All rights reserved.
//

#include "texture.h"
#include <iostream>

#include "ApplicationServices/ApplicationServices.h"

void  loadNativePNG(texture_t* text)
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
	spriteImage = [[[NSImage alloc] initWithContentsOfFile:name] autorelease].CGImage; 
	
	
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

/*
SHMUP is a 3D Shoot 'em up game inspired by Treasure Ikaruga

This file is part of SCHMUP.

SCHMUP is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SCHMUP is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SCHMUP.  If not, see <https://www.gnu.org/licenses/>.
*/

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

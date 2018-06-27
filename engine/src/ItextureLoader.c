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
 *  ItextureLoader.c
 *  dEngine
 *
 *  Created by fabien sanglard on 16/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "ItextureLoader.h"
#include "filesystem.h"
#include "math.h"

static char gPVRTexIdentifier[4] = "PVR!";


typedef struct _PVRTexHeader
	{
		unsigned int headerLength;
		unsigned int height;
		unsigned int width;
		unsigned int numMipmaps;
		unsigned int flags;
		unsigned int dataLength;
		unsigned int bpp;
		unsigned int bitmaskRed;
		unsigned int bitmaskGreen;
		unsigned int bitmaskBlue;
		unsigned int bitmaskAlpha;
		unsigned int pvrTag;
		unsigned int numSurfs;
	} PVRTexHeader;

#define kPVRTextureFlagTypePVRTC_2 24
#define kPVRTextureFlagTypePVRTC_4 25
#define PVR_TEXTURE_FLAG_TYPE_MASK	0xff

#define GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG			0x8C00
#define GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG			0x8C01
#define GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG			0x8C02
#define GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG			0x8C03





void loadNativePVRT(texture_t* texture)
{
	PVRTexHeader* pvrHeader;
	unsigned int flags, pvrTag;
	unsigned int formatFlags;

	unsigned int  blockSize = 0, widthBlocks = 0, heightBlocks = 0;
	unsigned int  width = 0, height = 0, bpp = 4;
	ubyte* pvrDataStart;
	
	
	//printf("[loadNativePVRT] Loading '%s'\n",texture->path);
	texture->file = FS_OpenFile(texture->path,"rb");
	FS_UploadToRAM(texture->file);

	if (!texture->file)
	{
		Log_Printf("[loadNativePVRT] Could not load: '%s'\n",texture->path);
		return;
	}
	
	pvrHeader = (PVRTexHeader *)texture->file->ptrStart;
	
	
	//texture->data = malloc(texture->file->filesize - sizeof(PVRTexHeader)) ;
	//memcpy(texture->data, texture->file->ptrStart+sizeof(PVRTexHeader), texture->file->filesize - sizeof(PVRTexHeader));
	

	
	pvrTag = pvrHeader->pvrTag;
	
	//Analysing header magic number
	if (gPVRTexIdentifier[0] != ((pvrTag >>  0) & 0xff) ||
		gPVRTexIdentifier[1] != ((pvrTag >>  8) & 0xff) ||
		gPVRTexIdentifier[2] != ((pvrTag >> 16) & 0xff) ||
		gPVRTexIdentifier[3] != ((pvrTag >> 24) & 0xff))
	{
		Log_Printf("PVR file '%s' failed magic number check.\n",texture->path);
		return ;
	}
	
	texture->numMipmaps = pvrHeader->numMipmaps + 1;
	Log_Printf("Texture %s has %d mipmaps.\n",texture->path,texture->numMipmaps);
	
	texture->data =       malloc(texture->numMipmaps * sizeof(ubyte*)) ;
	texture->dataLength = malloc(texture->numMipmaps * sizeof(int));
	texture->numMipmaps = 0;
	
	
	//
	flags = pvrHeader->flags;
	formatFlags = flags & PVR_TEXTURE_FLAG_TYPE_MASK;
	
	if (formatFlags == kPVRTextureFlagTypePVRTC_4 || formatFlags == kPVRTextureFlagTypePVRTC_2)
	{

		
		if (pvrHeader->bitmaskAlpha)
		{
			
			if (formatFlags == kPVRTextureFlagTypePVRTC_4)
			{
				texture->format = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
				//printf("GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG.\n");
			}
			else// if (formatFlags == kPVRTextureFlagTypePVRTC_2)
			{
				texture->format = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
				//printf("GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG.\n");
			}
		}
		else
		{
			
			if (formatFlags == kPVRTextureFlagTypePVRTC_4)
			{
				texture->format = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
			//	printf("GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG.\n");
			}
			else// if (formatFlags == kPVRTextureFlagTypePVRTC_2)
			{
				texture->format = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
			//	printf("GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG.\n");
			}
		}
		
		width = pvrHeader->width;
		height = pvrHeader->height;
		texture->width = width;
		texture->height = height;
		
	
		pvrDataStart = (ubyte*)(pvrHeader + 1) ;
		
	
	//	printf("File total size is %lu bytes.\n",texture->file->filesize);
	//	printf("Payload is %d bytes.\n",texture->file->ptrEnd - pvrDataStart);
		
	//	if (!strcmp(texture->path, "data/models/misc/evil.obj.md5mesh"))
	//		printf("DEVIL !.\n");
		
		
		
		while (pvrDataStart < texture->file->ptrEnd) 
		{
			if (formatFlags == kPVRTextureFlagTypePVRTC_4)
			{
				blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
				widthBlocks = width / 4;
				heightBlocks = height / 4;
				bpp	= 4;
			}
			else
			{
				blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
				widthBlocks = width / 8;
				heightBlocks = height / 4;
				bpp = 2;
			}
		
			// Clamp to minimum number of blocks
			if (widthBlocks < 2)
				widthBlocks = 2;
			if (heightBlocks < 2)
				heightBlocks = 2;
		
			
			
			texture->dataLength[texture->numMipmaps] = widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);
		
			texture->data[texture->numMipmaps] = malloc(texture->dataLength[texture->numMipmaps] * sizeof(ubyte));
			
			memcpy(texture->data[texture->numMipmaps], pvrDataStart, texture->dataLength[texture->numMipmaps] * sizeof(ubyte));

			
			//printf("Mippmap %d is %d bytes.\n",texture->numMipmaps,texture->dataLength[texture->numMipmaps]);
			
			pvrDataStart += texture->dataLength[texture->numMipmaps];
			
			texture->numMipmaps++;
			
			width = MAX(width >> 1, 1);
			height = MAX(height >> 1, 1);
		}
	}
}

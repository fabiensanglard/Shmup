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
/*
 *  texture.c
 *  dEngine
 *
 *  Created by fabien sanglard on 16/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "texture.h"
#include "ItextureLoader.h"
#include "filesystem.h"
#include "renderer.h"



//Cache
#define HASH_MAX_VALUE 256

typedef struct tex_cache_bucket_t {
	texture_t* texture;
	struct tex_cache_bucket_t *next;
} tex_cache_bucket_t;

tex_cache_bucket_t** tex_hashtable;

void TEXT_InitCacheSystem(void)
{
	tex_hashtable = calloc(HASH_MAX_VALUE, sizeof(tex_cache_bucket_t*));
}


void TEXT_PrintCache(void)
{
	int i;
	tex_cache_bucket_t* curr;
	
	Log_Printf("	--Texture cache--\n");
	
	for (i=0; i< HASH_MAX_VALUE; i++) 
	{
		curr = tex_hashtable[i];
		while (curr != NULL) 
		{
			Log_Printf("	Tex (%d) = '%s' id=%d\n",i,curr->texture->path,curr->texture->textureId);
			curr=curr->next;
		}
	}
	
	Log_Printf("	--END Texture cache END --\n");
	
}

void TEXT_ClearTextureLibrary(void)
{
	int i=0;	
	tex_cache_bucket_t* curr;
	//tex_cache_bucket_t* prev ;
	//tex_cache_bucket_t* toDelete ;
	
	// Go through the hastable
	for (i=0; i < HASH_MAX_VALUE; i++) 
	{
		
		//prev=NULL mark beginnnig of the list
		//prev = NULL;
		
		for (curr = tex_hashtable[i];curr != NULL;) 
		{
			if (!curr->texture->memStatic)
				TEX_UnloadTexture(curr->texture);

			curr = curr->next;
			/*
			if (!curr->texture->memStatic)
			{
				toDelete = curr;
				if (prev == NULL)
				{
					tex_hashtable[i] = curr->next;
					curr=tex_hashtable[i];
				}
				else 
				{
					prev->next = curr->next;
					curr=curr->next;
				}
				
				
				//Free MD5 mesh
				TEX_UnloadTexture(curr->texture);
				
				free(toDelete);
				toDelete=0;
			}
			else 
			{
				prev = curr;
				curr=curr->next;
			}
			*/
		}
	}
}


W32 tex_strhash( const char *string )
{
	W32 hash = *string;
	
	if( hash )
	{
		for( string += 1; *string != '\0'; ++string )
		{
			hash = (hash << 5) - hash + *string;
		}
	}
	
	return hash;
}



texture_t* TEX_GetTexture(char* mtlName)
{ 
	unsigned int hashValue ;
	tex_cache_bucket_t* bucket;
	
	hashValue= tex_strhash(mtlName) % HASH_MAX_VALUE;
	
	bucket = tex_hashtable[hashValue];
	
	if (bucket == NULL)
		return NULL;
	else
	{
		while (strcmp(bucket->texture->path,mtlName) && bucket->next != NULL)
			bucket = bucket->next;
	}
	
	if (!strcmp(bucket->texture->path,mtlName))
		return bucket->texture;
	else
		return NULL;
}

void TEX_PutTexture(texture_t* texture)
{
	tex_cache_bucket_t* bucket;
	unsigned int hashValue ; 
	
	hashValue= tex_strhash(texture->path) % HASH_MAX_VALUE;
	
	if ( tex_hashtable[hashValue] == NULL)
	{
		bucket = (tex_cache_bucket_t*)calloc(1,sizeof(tex_cache_bucket_t));
		tex_hashtable[hashValue] = bucket;
	}
	else
	{
		//Log_Printf("[MTL parser] Collision detected, while inserting '%s'.\n",mtlName);
		bucket = tex_hashtable[hashValue];
		
		while (bucket->next)
			bucket = bucket->next;
		
		bucket->next = (tex_cache_bucket_t*)calloc(1,sizeof(tex_cache_bucket_t));
		bucket = bucket->next;
	}
	
	
	bucket->texture = texture;
	bucket->next = NULL;
	
}

/*
void TEXT_RemoveFromCache(texture_t* texture)
{
	tex_cache_bucket_t* bucket;
	tex_cache_bucket_t* prevBucket;
	unsigned int hashValue ; 
	
	hashValue= tex_strhash(texture->path) % HASH_MAX_VALUE;
	
	if ( tex_hashtable[hashValue] == NULL)
		return;
	
	bucket = tex_hashtable[hashValue];
	
	// Is it the first element ?
	if (!strcmp(bucket->texture->path, texture->path))
	{
		tex_hashtable[hashValue] = bucket->next;
		return;
	}
	
	//Search for it 
	do {
		prevBucket = bucket;
		bucket = bucket->next;
		
	} while (bucket!= NULL && strcmp(bucket->texture->path, texture->path));
	
	//End of list reached without finding ?
	if (bucket == NULL)
		return;
	
	//Found it
	if (strcmp(bucket->texture->path, texture->path))
	{
		prevBucket->next = bucket->next;
		free(bucket);
	}
}
*/
void TEX_MakeStaticAvailable(texture_t* texture)
{
	if (!texture)
		return;
	
	texture->memStatic = 1;
	texture->cachable = 0;
	TEX_MakeAvailable(texture);
}

void TEX_LoadFromDiskAndUploadToGPU(texture_t* tmpTex)
{
	char* extension; 
	
	extension = FS_GetExtensionAddress(tmpTex->path);
	
	
	
	
	tmpTex->format = TEXTURE_TYPE_UNKNOWN;
	
	
	if (extension[0] == 'p' &&
		extension[1] == 'v' &&
		extension[2] == 'r')
	{
	
		loadNativePVRT(tmpTex);
	}
	
	if (extension[0] == 'p' &&
		extension[1] == 'n' &&
		extension[2] == 'g')
	{
	
		loadNativePNG(tmpTex);
	}
	
	if (tmpTex->format == TEXTURE_TYPE_UNKNOWN)
	{
		Log_Printf("[Texture loader] Texture type for %s is UNKNOWN !!\n",tmpTex->path);
	}
	
	//Upload to GPU
	renderer.UpLoadTextureToGpu(tmpTex);
	
	tmpTex->memLocation = TEXT_MEM_LOC_VRAM ;
}

void TEX_MakeAvailable(texture_t* tmpTex)
{
	
	texture_t* cacheTest;

	if (tmpTex == NULL)
		return;
	
	//Log_Printf("TEX_MakeAvailable %s\n",tmpTex->path);
	
	
	//Check if this texture is in cache
	cacheTest = TEX_GetTexture(tmpTex->path);
	if (cacheTest != NULL)
	{
		//Log_Printf("TEX_MakeAvailable cache it returning.\n");
		*tmpTex = *cacheTest;
		
		if (tmpTex->memLocation != TEXT_MEM_LOC_VRAM)
			TEX_LoadFromDiskAndUploadToGPU(tmpTex);
		
		return;
	}
	
	if (tmpTex->memLocation == TEXT_MEM_LOC_VRAM)
	{
		//Log_Printf("TEX_MakeAvailable already in vram, returning.\n");
		return ;
	}
	
	TEX_LoadFromDiskAndUploadToGPU(tmpTex);
	
	//Cache for futur usage
	if (tmpTex->cachable)
		TEX_PutTexture(tmpTex);
	
	
}

// Only remove texture from GPU and cache. Do not free texture
void TEX_UnloadTexture(texture_t* texture)
{

	if (!texture)
	{
		Log_Printf("[TEX_UnloadTexture] Nothing to unload.\n");
		return;
	}
	
	renderer.FreeGPUTexture(texture);
	

	texture->memLocation = TEXT_MEM_LOC_DISK;

}

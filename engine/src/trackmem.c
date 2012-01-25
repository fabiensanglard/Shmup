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
 *  trackmem.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-08-01.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#define INTERNAL
#include "trackmem.h"
#include <string.h>


struct memblk
{
	long           magic;
	struct memblk *next;
	struct memblk *prev;
	size_t         size;
	const char    *file;
	const char    *expr;
	int            line;
	int            padding;   // Make it an even 16 byte length (total size=32 bytes in 32-bit mode). 
							  // Not sure this is really necessary on GCC compiler or any compiler in the word
};

#define MAGIC1 0x12345678
#define MAGIC2 0x87654321

struct memblk *memblockList = NULL;

static void trackMBDetails(struct memblk *mb)
{
	printf("%d bytes allocated with \"%s\" at %s:%d\n", (int)mb->size, mb->expr, mb->file, mb->line);
}

void* trackcalloc(size_t count, size_t size_elem,const char *expr, const char *file, int line)
{
	struct memblk *mb = malloc(count*size_elem + sizeof(*mb));
	memset(mb,0,count*size_elem + sizeof(*mb));
	
    if (!mb)
	{
        // May want to output some error message here!
		return NULL;
	}
    mb->magic = MAGIC1;
    mb->file = file;
    mb->line = line;
	mb->expr = expr;
	mb->size = count*size_elem;
    mb->prev = NULL;
	mb->next = memblockList;
	if (memblockList)
	{
		memblockList->prev = mb;
	}
	memblockList = mb;
    return (void *)&mb[1];  
}

void *trackmalloc(size_t size, const char *expr, const char *file, int line)
{
    struct memblk *mb = malloc(size + sizeof(*mb));
    if (!mb)
	{
        // May want to output some error message here!
		return NULL;
	}
    mb->magic = MAGIC1;
    mb->file = file;
    mb->line = line;
	mb->expr = expr;
	mb->size = size;
    mb->prev = NULL;
	mb->next = memblockList;
	if (memblockList)
	{
		memblockList->prev = mb;
	}
	memblockList = mb;
    return (void *)&mb[1];  
}


void  trackfree(void *ptr, const char *expr, const char *file, int line)
{
    if (!ptr)
		return;
    else
    {
        struct memblk *mb = &((struct memblk *)(ptr))[-1];
        if (mb->magic != MAGIC1)
        { 
			if (mb->magic == MAGIC2)
			{
				printf("Attempt to free already freed memory:\n");
				trackMBDetails(mb);
			}
			else
			{
				printf("Invalid free of ptr %p (expr=\"%s\" from %s:%d\n", (void *)ptr, expr, file, line);
			}
			return;
        }
        mb->magic = MAGIC2;
        if (mb ==  memblockList)
        {
            memblockList = mb->next;
        }
        // Unlink it. 
        if (mb->next)
			mb->next->prev = mb->prev;
        if (mb->prev)
			mb->prev->next = mb->next;      
        free(mb);
    }
}


void trackListAllocations(void)
{
	printf("*** Allocation list start ***\n");
	if (!memblockList)
	{
		printf(">>> EMPTY <<<\n");
	}
	else
	{
		struct memblk *mb;
		for(mb = memblockList; mb; mb = mb->next)
		{
			trackMBDetails(mb);
		}
	}
	printf("*** Allocation list end ***\n");
}


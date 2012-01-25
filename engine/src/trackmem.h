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
 *  trackmem.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-08-01.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#ifndef TRACK_MALLOC_H
#define TRACK_MALLOC_H

#include <stdio.h>
#include <stdlib.h>

#define DO_FREE 0
#define DO_NOT_FREE 1

	#ifdef DDEBUG
		#ifndef INTERNAL
			#define malloc(size) trackmalloc(size, #size, __FILE__, __LINE__)
			#define calloc(count, size) trackcalloc(count,size,#size, __FILE__, __LINE__)
			#define free(ptr) trackfree(ptr, #ptr, __FILE__, __LINE__)
		#endif

		void *trackmalloc(size_t size, const char *expr, const char *file, int line);
		void* trackcalloc(size_t count, size_t size_elem,const char *expr, const char *file, int line);
		void  trackfree(void *ptr, const char *expr, const char *file, int line);
		
		
	#else

	#endif

	void trackListAllocations();

#endif
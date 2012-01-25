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
 *  filesystem.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#ifndef FS_FILESYSTEM
#define FS_FILESYSTEM
#include "globals.h"



#define	MAX_GAMEPATH	256	// max length of a game pathname
#define	MAX_OSPATH		1024	// max length of a filesystem pathname

/*
 * File status flags: these are used by open(2), fcntl(2).
 * They are also used (indirectly) in the kernel file structure f_flags,
 * which is a superset of the open/fcntl flags.  Open flags and f_flags
 * are inter-convertible using OFLAGS(fflags) and FFLAGS(oflags).
 * Open/fcntl flags begin with O_; kernel-internal flags begin with F.
 */

/* open-only flags */

#define	FS_RDONLY	"r"				/* open for reading only */
#define	FS_RDONLY_TEXT		"rt"	/* open for reading only */
#define	FS_RDONLY_BINARY	"rb"	/* open for reading only */
#define	FS_WRONLY	"w"				/* open for writing only */
#define	FS_RDWR		"rw"			/* open for reading and writing */

typedef unsigned char   W8,		*PW8;
typedef signed char		SW8,    *PSW8;
typedef unsigned short  W16,    *PW16;
typedef signed short    SW16,   *PSW16;
typedef unsigned long   W32,    *PW32;
typedef signed long		SW32,   *PSW32;


typedef struct
	{
		FILE *hFile;
		
		/* Following is used when the file is loaded into memory */
		int bLoaded;				/* Was file loaded into memory? */
		W32	filesize;				/* Size of file data in bytes */
		
		uchar	*ptrStart;				/* pointer to start of file data block */
		uchar	*ptrCurrent;			/* pointer to current position in file data block */
		uchar	*ptrEnd;				/* pointer to end of file data block */
		
		void *filedata;				/* file data loaded into memory */
		
		uchar isWritable;
		
	} filehandle_t;



void	FS_InitFilesystem(void);

char*	FS_GameWritableDir(void);
char*	FS_Gamedir(void);

filehandle_t* FS_OpenFile( const char *filename, char* mode  );
void FS_CloseFile( filehandle_t *fhandle );
SW32 FS_ReadFile( void *buffer, W32 size, W32 count, filehandle_t *fhandle );
W32 FS_FileSeek( filehandle_t *fhandle, SW32 offset, W32 origin );

SW32 FS_GetFileSize( filehandle_t *fhandle );
SW32 FS_FileTell( filehandle_t *fhandle );
void *FS_GetLoadedFilePointer( filehandle_t *fhandle, W32 origin );

void FS_StripExtension( const char *in, char *out );
char *FS_FileExtension( const char *in );
void FS_DirectoryPath(   char *in, char *out );

char* FS_GetExtensionAddress(char* string);
//char* FS_GetDocumentPath(void);


char* FS_GetFilenameOnly(char* string);



#endif
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
 *  filesystem.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "filesystem.h"



//#include <sys/stat.h>
char fs_gamedir[ MAX_OSPATH ];
char fs_writableDir[ MAX_OSPATH ];


bool FS_InitFilesystem( void )
{
	char *p;
	p = getenv("RD");
	sprintf( fs_gamedir, "%s", p );
	//Log_Printf("[Filesystem] Base    directory initialized (%s).\n",fs_gamedir);
	
	p = getenv("WD");\
	sprintf( fs_writableDir, "%s", p );
	//Log_Printf("[Filesystem] Writable directory initialized (%s).\n",fs_writableDir);


	
		
	Log_Init();

	Log_Printf("SHMUP :\n");
	Log_Printf("===========\n");
	Log_Printf("[Filesystem] Initialized.\n");
}

char*	FS_GameWritableDir(void)
{
	return fs_writableDir;
}


char *FS_Gamedir( void )
{
	return fs_gamedir;
}


filehandle_t* FS_OpenFile( const char *filename, char* mode  )
{
	char			netpath[ MAX_OSPATH ];
	filehandle_t	*hFile;
	const char		*pathBase;
	
	FILE*	fd;
	int		pos;
	int		end;
	
	uchar   isWriting;
	size_t i;
	
	memset(netpath,0,MAX_OSPATH);
	
	isWriting=0;
	for(i= 0  ; mode && i < strlen(mode)  ; i++)
	{
		if (mode[i] == 'w' || mode[i] == 'a')
		{
			isWriting = 1;
			break;
		}
	}
	
	
	if (isWriting)
	{
		pathBase = FS_GameWritableDir();
		
	}
	else 
		pathBase = FS_Gamedir();
		
	sprintf( netpath, "%s/%s", pathBase, filename );
	
	fd = fopen( netpath, mode );
	if ( !fd  ) {
		Log_Printf("[FS_OpenFile] Could not open file '%s'\n",netpath);
		return NULL;
	}
	
	

	hFile = (filehandle_t*) calloc(1, sizeof( filehandle_t ) );
    //Useless since calloc already zero memory allocated.
	//memset( hFile, 0, sizeof( filehandle_t ) );
	
	if (isWriting)
		hFile->isWritable = 1;

	//Get filesize.
	pos = ftell (fd);
	fseek (fd, 0, SEEK_END);
	end = ftell (fd);
	fseek (fd, pos, SEEK_SET);
	hFile->filesize = end;
	

	
    
    hFile->hFile = fd;
	
	
	
		

	return hFile;
}

int FS_UploadToRAM( filehandle_t *hFile)
{

	//This should be done in an external method.
	hFile->filedata = calloc( hFile->filesize,sizeof(char) );
	
	if (!hFile->filedata)
	{
		//Out of memory....
		Log_Printf("[FS_UploadToRAM] Unable to load file '%s' to RAM.\n");
		return 0;
	}
	fread(hFile->filedata, sizeof(char),hFile->filesize, hFile->hFile);
	
	hFile->ptrStart =  hFile->ptrCurrent = (PW8)hFile->filedata;
	hFile->ptrEnd =  (PW8)hFile->filedata + hFile->filesize;
	hFile->bLoaded = 1;
	hFile->isWritable = 0;
	
	return 1;
}


SW32 FS_Write( const void * buffer, W32 size, W32 count, filehandle_t * stream )
{
	if (stream->bLoaded)
	{
		Log_Printf("Writing to a file in RAM is not supported. Use the filehandle_t pointer instead.\n");
		exit(0);
		return 0;
	}

	return fwrite(buffer,size,count,stream->hFile);
}

SW32 FS_Read( void *buffer, W32 size, W32 count, filehandle_t *fhandle )
{		
	W8	*buf = (PW8)buffer;
	size_t i;
	
	if (fhandle->bLoaded){

		//Trying to read more than what is remaining.
		if( (size * count) > (fhandle->ptrEnd - fhandle->ptrCurrent) )
		{
			SW32 read;
		
			read = (fhandle->ptrEnd - fhandle->ptrCurrent);
		
			for( i = 0 ; i < (fhandle->ptrEnd - fhandle->ptrCurrent) ; ++i )
			{
				buf[ i ] = fhandle->ptrCurrent[ i ];
			}
		
			fhandle->ptrCurrent = fhandle->ptrEnd;
		
			return( read );
		}
		else
		{
			for( i = 0 ; i < (size * count) ; ++i, fhandle->ptrCurrent++ )
			{
				buf[ i ] = *fhandle->ptrCurrent;
			}
		
			return( (size * count) / size );
		}
	
		/* should never get here */
		return -1;
	}

	return fread(buffer,size,count,fhandle->hFile);
}


void FS_CloseFile( filehandle_t *fhandle )
{
	//If the file was uploaded to RAM we need to free the buffer.
	if( fhandle->filedata )
	{
		
		free( fhandle->filedata );
		fhandle->filedata = NULL;
	}
	
	
	fclose( fhandle->hFile);
	
	free( fhandle );
}


/*
 -----------------------------------------------------------------------------
 Function: FS_GetLoadedFilePointer() -Get file pointer.
 
 Parameters: 
 filestream -[in] Target file handle.
 origin -[in] Pointer position
 SEEK_SET -Beginning of file.
 SEEK_CUR -Current position of file pointer.
 SEEK_END -End of file.
 
 Returns: File pointer on success, otherwise NULL.
 
 Notes: 
 -----------------------------------------------------------------------------
 */
void *FS_GetLoadedFilePointer( filehandle_t *fhandle, W32 origin )
{
	switch( origin )
	{
		case SEEK_SET:
			return( (void *)fhandle->ptrStart );
			
		case SEEK_END:
			return( (void *)fhandle->ptrEnd );
			
		case SEEK_CUR:
			return( (void *)fhandle->ptrCurrent );
	}
	
	return NULL;
}


char* FS_GetExtensionAddress(char* string)
{
	char* extension;
	
	extension = &string[strlen(string)-1];
	
	while(*extension != '.' && extension != string)
		extension--;
	
	return (extension+1);
}

char* FS_GetFilenameOnly(char* string)
{
	char* filename = &string[strlen(string)-1];;
	
	while (filename != string) {
		if (*filename == '\\' || *filename == '/')
			return filename+1;
		filename--;
	}
	
	return filename;
}
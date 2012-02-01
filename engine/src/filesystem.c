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


void FS_InitFilesystem( void )
{
	char *p;
	p = getenv("RD");
	sprintf( fs_gamedir, "%s", p );
	//printf("[Filesystem] Base    directory initialized (%s).\n",fs_gamedir);
	
	p = getenv("WD");\
	sprintf( fs_writableDir, "%s", p );
	//printf("[Filesystem] Writable directory initialized (%s).\n",fs_writableDir);
	
	//printf("[Filesystem] Base directory = '%s'.\n",fs_gamedir);
	
	printf("[Filesystem] Initialized.\n");
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
		if (mode[i] == 'w')
		{
			isWriting = 1;
			break;
		}
	}
	
	
	if (isWriting)
		pathBase = FS_GameWritableDir();
	else 
		pathBase = FS_Gamedir();
		
	sprintf( netpath, "%s/%s", pathBase, filename );
	
	fd = fopen( netpath, mode );
	if ( !fd  ) {
		printf("[FS_OpenFile] Could not open file '%s'\n",netpath);
		return NULL;
	}
	
	
	hFile = (filehandle_t*) calloc(1, sizeof( filehandle_t ) );
	memset( hFile, 0, sizeof( filehandle_t ) );
	
	
	pos = ftell (fd);
	fseek (fd, 0, SEEK_END);
	end = ftell (fd);
	fseek (fd, pos, SEEK_SET);
	hFile->filesize = end;
	
	//if (!strcmp("data/scenes/techDemo.scene", filename))
	//{
	//	printf("techDemo.scene filesize = %d",hFile->filesize);
	//}
	
	hFile->filedata = calloc( hFile->filesize,sizeof(char) );
	
	fread(hFile->filedata, sizeof(char),hFile->filesize, fd);
	
	hFile->ptrStart =  hFile->ptrCurrent = (PW8)hFile->filedata;
	hFile->ptrEnd =  (PW8)hFile->filedata + hFile->filesize;
	hFile->bLoaded = 1;
	hFile->isWritable = isWriting;
	hFile->hFile = fd;
	//printf("Closing file: '%s'\n",netpath);
	
	if (!hFile->isWritable)
		fclose( fd );
	
	return hFile;
}

SW32 FS_ReadFile( void *buffer, W32 size, W32 count, filehandle_t *fhandle )
{		
	W8	*buf = (PW8)buffer;
	size_t i;
	
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

SW32 FS_GetFileSize( filehandle_t *fhandle )
{
	return fhandle->filesize;
}


void FS_CloseFile( filehandle_t *fhandle )
{
	if( fhandle->filedata )
	{
		
		free( fhandle->filedata );
		fhandle->filedata = NULL;
	}
	
	if(	fhandle->isWritable)
		fclose( fhandle->hFile);
	
	free( fhandle );
}


W32 FS_FileSeek( filehandle_t *fhandle, SW32 offset, W32 origin )
{
	switch( origin )
	{
		case SEEK_SET:
			if( offset < 0 || offset > fhandle->filesize )
			{
				return 1;
			}
			
			fhandle->ptrCurrent = fhandle->ptrStart + offset;
			break;
			
		case SEEK_END:
			if( offset > 0 )
			{
				return 1;
			}
			
			// offset is negative 
			if( (int)(fhandle->filesize + offset) < 0  )
			{
				return 1;
			}
			
			// offset is negative 
			fhandle->ptrCurrent = fhandle->ptrEnd + offset;
			break;
			
		case SEEK_CUR:
			if( offset < 0 )
			{
				// offset is negative
				if( ((fhandle->ptrCurrent - fhandle->ptrStart) + offset) < 0 )
				{
					return 1;
				}
			}
			
			if( offset > 0 )
			{
				if( offset > (fhandle->ptrEnd - fhandle->ptrCurrent) )
				{
					return 1;
				}
			}
			
			fhandle->ptrCurrent += offset;
			break;
			
		default:
			return 1;
	}
	
	return 0;
}

SW32 FS_FileTell( filehandle_t *fhandle )
{
	return( fhandle->ptrCurrent - fhandle->ptrStart );
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




void FS_StripExtension( const char *in, char *out )
{
	while( *in && *in != '.' )
	{
		*out++ = *in++;
	}
	
	*out = '\0'; // NUL-terminate string.
}

char *FS_FileExtension( const char *in )
{
	static char exten[ 8 ];
	char*		j;
	char*       i;
	
	i = (char*)in + strlen(in);
	j = (char*)exten + 7;
	
	exten[7] = '\0';
	
	while(*i != '.')
	{
		j--;
		i--;
		*j = *i;
		//in--;
	}
	j++;
	
	//exten[7] = '\0'; // NUL-terminate string.
	
	return j;
}

void FS_DirectoryPath(  char *in, char *out )
{
	char *s;
	
	s = in + strlen( in ) ;
	out += strlen( in ) ;
	
	while( s != in && *s != '/' && *s != '\\')
	{
		s--;
		out--;
	}
	
	while( s != in-1)
		*out-- = *s--;
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
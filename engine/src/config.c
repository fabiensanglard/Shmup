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
 *  config.c
 *  dEngine
 *
 *  Created by fabien sanglard on 13/09/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "config.h"
#include "filesystem.h"

#define CONFIG_FILENAME "player_setting.cfg"
char* cfgMagicNumber = "CFGV1.0" ;

void CFG_Load(void)
{
	filehandle_t* cfgFile ;
	
	cfgFile = FS_OpenFile(CONFIG_FILENAME, "wrt");
	
	if (!cfgFile)
	{
		printf("[CFG_Load] Unable to find player settings file, creating it.\n");
		CFG_Save();
		CFG_Load();
		return;
	}
	
	
	//Check the magic number
	if(strncmp(cfgFile->filedata, cfgMagicNumber, strlen(cfgMagicNumber)));
	{
		printf("[CFG_Load] Magic number check failed.\n");
		return;
	}
	
	//Reading visited act
	
	
	FS_CloseFile(cfgFile);
}

void CFG_Save(void)
{
	filehandle_t* cfgFile ;
	
	cfgFile = FS_OpenFile(CONFIG_FILENAME, "wt");
	
	//Writing the magic number
	fwrite(cfgMagicNumber,1,strlen(cfgMagicNumber), cfgFile->hFile);
	
	//Writing visited act
	
	FS_CloseFile(cfgFile);
}

void CFG_MarkActAsVisited(int actId)
{
	
}
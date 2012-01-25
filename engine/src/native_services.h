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
 *  native_services.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-10-08.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_NATIVE_SERVICES
#define DE_NATIVE_SERVICES

int  Native_RetrieveListOf(char replayList[10][256]);
void Native_UploadFileTo(char path[256]);
void Action_ShowGameCenter(void* tag);
void Native_UploadScore(uint score);
void Native_LoginGameCenter(void);

#endif


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
 *  titles.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-08-13.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_TITLES
#define DE_TITLES

#include "texture.h"

void TITLE_AllocRessources(void);
void TITLE_FreeRessources();
void TITLE_Show_prolog(int dr);
void TITLE_Show_epilog(int dr);
void TITLE_Render(void);
void TITLE_Update(void);

extern texture_t titleTexture;
#define MODE_PROLOG 0
#define MODE_EPILOG 1
#define MODE_UNKNOWN 2
extern uchar title_mode ;

#endif
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
 *  text.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-10-06.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_TEXT
#define DE_TEXT

#include "fx.h"

void DYN_TEXT_Init(void);
void DYN_TEXT_AddText(vec2short_t ss_start_pos,vec2short_t ss_end_pos, int duration,float size,char* text);
void DYN_TEXT_Update(void);
void DYN_TEXT_Render(void);
#endif
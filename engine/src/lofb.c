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
 *  lofb.c
 *  dEngine
 *
 *  Created by fabien sanglard on 10-10-25.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#include "lofb.h"
#include "globals.h"
#include "timer.h"

void updateLOFBSpawning(enemy_t* enemy)
{
	
}

stateFunction lofb_states[] = {updateLOFBSpawning};

void updateLOFB(enemy_t* enemy)
{
	
	
		//enemy->ss_position[X] =0;//+= 0.002*cosf( (enemy->uniqueId + enemy->timeCounter / (float)2000) * 4 * 2 * M_PI);
		//enemy->ss_position[Y] =0;//+= 0.002*sinf( (enemy->uniqueId + enemy->timeCounter / (float)2000) * 4 * 2 * M_PI);
		
		//printf("lofb !!\n");
		
		//lofb_states[enemy->state](enemy);
		
	
	
}
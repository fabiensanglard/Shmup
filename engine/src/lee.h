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
 *  lee.h
 *  dEngine
 *
 *  Created by fabien sanglard on 10-10-19.
 *  Copyright 2010 Memset software Inc. All rights reserved.
 *
 */

#ifndef DE_LEE
#define DE_LEE

#include "enemy.h"

#define PARAMETER_LEE_FIRING_TYPE 0
#define PARAMETER_LEE_START_ANGLE 1
#define PARAMETER_LEE_FIRE_FREQUENCY 2
#define PARAMETER_LEE_BULLET_SPEED_FACTOR 3

#define LEE_FIRING_TYPE_DOWN 1
#define LEE_FIRING_TYPE_TARGET_PLAYER 0
#define LEE_FIRING_TYPE_NO_FIRE 2



void updateLEE(enemy_t* enemy);

#endif
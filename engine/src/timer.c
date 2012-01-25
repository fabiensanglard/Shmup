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
 *  timer.c
 *  dEngine
 *
 *  Created by fabien sanglard on 15/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */

#include "timer.h"
#include "dEngine.h"


int forcedTimeIncrement=0;
void Timer_ForceTimeIncrement(int ms)
{
	printf("[Timer] !! WARNING !! Time increment is forced !! WARNING !!.\n");
	forcedTimeIncrement = ms;
}

#ifndef WIN32
#include <sys/time.h>
int E_Sys_Milliseconds( void )
{
	struct timeval tp;
	static int	secbase;
	
	
	
	gettimeofday( &tp, 0 );
	
	if( ! secbase )
	{
		secbase = tp.tv_sec;
		return tp.tv_usec / 1000;
	}
	
	return (tp.tv_sec - secbase) * 1000 + tp.tv_usec / 1000;
}
#else
#include "windows.h"
#include "MMSystem.h"
int E_Sys_Milliseconds( void )
{
	return (int)timeGetTime();
}
#endif

unsigned char paused = 1;
int fps=0;
int lastTime=0;
int currentTime=0;
int fpsAcc=0;
int fpsTimeAcc=0;
unsigned int simulationTime=0;
int timediff=0;

void Timer_Pause(void)
{
	paused=1;
	timediff = 0;
}

void Timer_Resume(void)
{
	paused = 0;
	lastTime = E_Sys_Milliseconds();
	currentTime= lastTime;
}


int frameCounter=0;
float extraPrecision=0;
void Timer_tick(void)
{
	//printf("t=%d\n",simulationTime);
	
	if (paused)
		return;
	
	
	
	
	lastTime = currentTime;
	currentTime = E_Sys_Milliseconds();
	
	/*
	if (forcedTimeIncrement)
	{
		timediff = forcedTimeIncrement;
	}
	else
	{
		timediff = currentTime - lastTime; 
	}	
	*/
	
	
	/*
	timediff = 16;
	simulationTime += timediff;
	*/
	if (engine.mode == DE_MODE_SINGLEPLAYER)
	{
		extraPrecision += 0.6666667f;
		timediff =16+(int)extraPrecision;
		extraPrecision -= timediff-16;
	}
	else 
	{
		timediff = currentTime - lastTime;
	}

		
	simulationTime += timediff;
	
	
	
	
	//Do not use the hard-coded 16fps timediff otherwise we measure time in the simulation.
	//We want realTime measure
	fpsTimeAcc += currentTime - lastTime;
	fpsAcc++;
	
	if (fpsTimeAcc >= 500)
	{
		fps = (int)(fpsAcc * 1000.0f/ fpsTimeAcc);
		fpsAcc = 0;
		fpsTimeAcc = 0;
	}
}

void Timer_resetTime(void)
{
	//lastTime = E_Sys_Milliseconds();
	Timer_tick();
	simulationTime = 0;
	extraPrecision=0;
	timediff=0;
	printf("[Timer] simulationTime = %d.\n",simulationTime);
}
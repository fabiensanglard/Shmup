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
 *  main.h
 *  dEngine
 *
 *  Created by fabien sanglard on 09/08/09.
 *  Copyright 2009 Memset software Inc. All rights reserved.
 *
 */



/*
 
 Note about compiling on Windows and running PVSStudio:
 
 Side effect within a sequence point with different behavior.
 strlen(x+1)
 calloc(sizeof(event)) instead of calloc(sizeof(event_t))
 
 */


#ifndef DE_MAIN
#define DE_MAIN 1

#define DIFFICULTY_EASY 0
#define DIFFICULTY_NORMAL 1
#define DIFFICULTY_INSANE 2

#define CONTROL_MODE_SWIP 0
#define CONTROL_MODE_VIRT_PAD 1

#define LICENSE_LIMITED 0
#define LICENSE_FULL 1

#include "globals.h"
#include "renderer.h"

#define SHIPPED_PLAYBACK 0
#define PLAYER_PLAYBACK 1

typedef struct playback_t
{
	
	char filename[256];
	
	char play;
	char record;
} playback_t ;

#define DE_MODE_SINGLEPLAYER 0
#define DE_MODE_MULTIPLAYER 1

typedef struct scene_t
{
	char path[256];
	char name[64];
	short defaultMenuId;
} scene_t;

typedef struct player_stats_t
{
	float numEnemies;
	int bulletsFired[MAX_NUM_PLAYERS];
	int bulletsHit[MAX_NUM_PLAYERS];
	int enemyDestroyed[MAX_NUM_PLAYERS];
	
} player_stats_t;

#define MAX_NUM_SCENES 16

typedef struct engine_info_t
{
	uchar soundEnabled;
	uchar musicEnabled;
	uchar gameCenterEnabled;
	uchar gameCenterPossible;
	
	int numScenes;
	scene_t scenes[MAX_NUM_SCENES];
	
	playback_t playback;
	
	char musicFilename[256];
	uint musicStartAt;
	
	player_stats_t playerStats;
	
	char menuVisible;

	int requiredSceneId;
	int sceneId;			//Valid for both mode.
	
	char controlVisible;
	
	uchar mode;
	
	uchar fogEnabled;
	
	uchar showFingers;
	
	uchar controlMode;
	
	uchar licenseType;
	
	uchar difficultyLevel ;
	
}  engine_info_t;

extern engine_info_t engine;





void dEngine_Init(void);
void dEngine_InitDisplaySystem(uchar rendererType);
void dEngine_RequireSceneId(int sceneId);
void dEngine_HostFrame(void);
void dEngine_CheckState(void);
void dEngine_WriteScreenshot(char* directory);
void dEngine_Pause(void);
void dEngine_Resume(void);
void dEngine_GoToNextScene(void);

#endif

/*
 
 Update December 31, 2010
 
 X Add difficulty level.
 X Make score go higher for higher difficulty levels.
 X Compile game for 4.0, detect gamecenter availability at runtime.
 - Fix multiplayer bug.
 - Release.
 
 */

/* 
 
 Articles:
 
 - Fighting memory fragmentation (preallocation + indexed list)
 - Tracking memory consumption
 - Memory handling with entity is pretty neat (VRAM or RAM with indices in entity or model)
 - Precalculating polygon visibility (Polygon codec)
 - Homogenuous clipping
 - Pure C, no C++, no STL
 - How to generate gorgeous videos
 - A summary of all the tools used to generate this great game: objToMD5, PNG assembler, Blender movie generator, polygon codec preprocessor.
 - SIMD, autovectorization
 - disable thumb on arm6, enable on arm7
 - -fsingle-precision-constant ?
 - Make text.c text dissapear with a fading effect.
 
 Ideas:
 
 Use VBO for elements Indices, not only vertexArray
 
 Change ComputeInvModelMatrix to be fast (no need to multiply)
 X - Run game in airport mode.
 Disable PNG compression to get ride of the hasAlpha in the material descriptor
 Try GCC auto-vectorization, if no dice implement SIMD method for NEON cortex in 3GS

 
 
 
 X - Look at m3G object format
 X - Check shader precision, try lowp (10bits [-2,2] range floating point)
 X - Checkout matrix palette to perform vertex skinning on GPU. OES_matrix_palette
 
 X - Fix ghost, no transparency is performed
 X - Fix ghost: Pick enemy
 Fix ghost: Gets longer and accelerate with time.
 
 X - Fix SND: NOT WORKING !!!
 
 */

/*		TODO
	 
 X - Fix textures matrix (/255) in fixed renderer
 Fix shadowMapping Moire pattern (polyOffset)
 More light with ship
 Enable 2d light when bullet is fired, sync intensity with bullet

 Reduce ghost DeltaT, right now it's 2ms but maybe a higher number will do.
 

 Improve bullet drawing GL_TRIANGLE_STRIP instead of INDEXED GL_TRIANGLES
 
 FREE MEMORY
 FREE MEMORY
 FREE MEMORY
	Memory is not freed at the end of a scene: 
			1/material cache, 
			2/cameraPath, 
			3/mesh cache
			4/events 
 
	Done: Memory is cleared for:
			- Texture manager
			- Material manager
			- Model manager
			
	TODO: Free camera path and event memory on the fly
	DONE: Free map elements at the end of the act/scene
	TODO: Free events
 
 X - FREE MEMORY
 X - FREE MEMORY
 X - FREE MEMORY
 


 // http://www.devmaster.net/forums/showthread.php?t=5784
 // http://stackoverflow.com/questions/1854254/fast-sine-cosine-for-armv7neon-looking-for-testers
 // http://code.google.com/p/math-neon/
 Remove cos and sin usage in devil, use cos and sin table
 Remove cos and sin usage in ghost
 Change enemy default rotation ( fromAbove rotation can be avoided)
 Change enemy creation to something more generic
 
 X - Screen space coordiante system: Choose something godamnit !!!
 
 
 
 Added new enemy ( small )
 X - Collision detection
 
 Explosion: 
	- pregenerated indices for bullets and fx. 
	- Use a VBO to store indices as they do not change. Reuse them accross bullets and fx.
 
 
 //ghost are full of sin and cos :/ !
 
 Center mine
 X - Make hero bullets longer
 X - Make her bullet "rotate"
 X - Flip explosion texture, to simulate diversity
 Remove on explosion stage.
 Make wave rotate
 Make Particules effects 
	- more diverse ( differents textures) 
	- convert to PVR
	- precalculate texture coordinates
	- make particule go cold red -> yellow
 X - Make smoke effects
 
 
 X Laser, precalculated on spawning. 
 X Add laser impact (3d, not a sprite) Note: Using a few particules worked like a charm
 - Convert laser texture to PVRTC
 - Add laser firing wave (3d, not a sprite) 
 X Add Smoke, move wave around
 - Adjust bullets to change type during travel
 X Adjuct bullet blast to grow and shrink 

 
 
 TODO NEXT:
 
 - Check usage of memcpy usage can be optimized if array is declared const & restrict
   For more about this see: http://developers.sun.com/solaris/articles/cc_restrict.html
 
 Finish ghost:
	- Acceleration
	X Tail length variation
	X collisions
	X Enemy target picking.
 
 X Add bullets impact particules.
 - Change bullets particules (white)
 - Change explosion particules (yellow)
 X Add color change when enemy is hit.

 TONIGHT
 X Add smoke effect
 X Generate a sweet video
 X Try to not interrupt sound but mix them with openAL with slightly different sounds in order to
 X  avoid and echo effect
 
 - Asses tools usability: 
	- Sound: TwistedWave
	- Explosions: 
		- ParticleIllusions
	    - ParticleIllusions SE
 
 X Convert explosion sprite to PVRTC
 - Convert ghost sprite to PVRTC
 
 X Drop color component from vertex Array for:
	X Ghost
	X Explosion 
 X Have the alpha set in the PNG/PVTRC
 
 - Change explosion to consum less fillrate
 
 
 
 WE, RELEASE:
 
 X Fix sound explosion
 X Ghost auto-aiming
 - Press screen to start.
 X Network stack:
	- Repeat 4 last updates.
 X Generate movie
 
 
 X Raise TTL lance
 X Fix ghost: straight line after a while
 X Fix commands recording
 X Fix collision detection error
 - Arcade look: 
	- Press start to play. 
	- Menu with Screen with start and 
	- Replay. 
	- Game Over
	- Lives remaining
	- Power gauge
 
 
 
 - TODO TODO TODO TODO TODO TODO TODO TODO
 
 X Fix statistic not appearing
 X Test performances on iPod touch
 - Try a bigger level (TENSO with two skyes), assess performances.
 X Fix ghosts
 X Fix enemy appearance (circles)
 - Check diff GHOST_DELTA_T_MS=3 and GHOST_DELTA_T_MS=6
*/




/*
 
 June 19th:
 Redone control, button less interface sucked. 
 
 June 8th:
 Added particules on bullets impacts
 Added enemy flicketing when hit
 
 June 7th:
 Designed a sound system on top of OpenAL. Music is still being handled by the hardware. Sounds effect don't suck up CPU, only bandwidth as they are WAV.
 
 June 6th:
 Redone ghost. Effect is now gorgeous
 
 
 March 18yj:
 
 Finished ghost. Effect sucks elephant balls, need to redo it with aspect non-independant on framerate.
 
 
 March 2th:
 
 Done with particules effects
 
 February:

 
 24th:
 
 - Primary collision detection bullet/enemies ( all enemies are 0.1*0.1 screenSpace box).
 - Incorporated Raph's mine02.obj.md5mesh
 - Explosions (Wave)
 
 23th:
 
 - Finished PVRTC mipmapping (Something is wrong with the fucking mipmappg settings: Resolved !!!, problem was incomplete mipmap sequence)
 - Changed enemy memory allocation model Enemy mem allocation and managment does deal with premature death of an enemy ( gap )
 - Added Enemy precaching by scanning events linked list on scene loading.
 
 
 22th:
 
 Now using FVO. Performance improved on 3GS. Itouch still maxed out at 60 fps.
 
 Clear only Depth buffer instead of both color and depth (glClear(GL_DEPTH_BUFFER_BIT);)

 
 Switched to GCC 4.2
 Added Thumb2 for ARM7
 
 Remove face culling on map rendition ( all polygons are visibles anyway). Done on fixed and shader renderer.
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

 
 21th:
 
 Finally got delta compression working. Took me a fucking week.
	Compress camera path with:
	- DeltaEncoding using vertices indices
 
 18th:
 
 Working on reducing memory consumption of camera path. It's a B.I.T.C.H .
 
 
 13th:
 
 Reduced size of a vertices by using texture coordinate normalization ( using GL_TEXTURE matrix to perform translation)
 Reduced size of normal and tangent by using vector nomalization
 
 
 12th:
 
 Added two players controls
 Added player inputs
 Added player inputs recording and playback		(Consume 400Kb for 4 minutes of playback)
 Generated a good looking video Dodge2.mov
 
 10th:
 
 Added enemies
 Added events sub-system
 Added players mouvements
 
 
 9th:
 
 MD5 Cache system				( reduce memory footprint, improve loading speed)
 Player movements				Basic, need to be fixed
 Enemy ( Devil )				Garbage, done fast to check poly and fillrate capability: So far so good
 Added music via Queue Audio	No impact on framerate !!! Still 60fps !!!
 
 
 8th:
 
 Moved fog conf to text file.
 Fixed glitches in bullets sprites
 Fixed bullet mouvement, now interpolating
 Added firing flash (effect sucks for now)
 
 */

/*
 January:
 
 Added  precalculated Visibility Set
 Added support for binary camera path.
 Added ships, 2D sticky position
 Added bullets
 Added Fog support
 
 Removed usage of OBJ altogether. MD5 only now.
 
 
 Dropped VSM shadowMapping, no floating point texture and no enough horsepower anyway.
 TRY AGAIN VSM SHADOWS on ending if enoguh time.
 
 
 */

/*
 
 October 22th
 
 Add support for Camera path (md5camera)		Done
 Consolidate shaders into uberShader			Done
 Specular support								Done
 Specular map									Done
 */

/*
 
 October 10th
 
 Multifinger mouvement.							Done
 
 
 */

/*

 October 9th
 
 Add FPS control.
 Fix OBJ loading (reuse vertices when building vertexArray) Done
 
 */

/*
 
 October 5th
 
	Add shadowMapping					Done
 
 */

/*
 
 September 23th
 
  Modelspace bumpmapping				Done
  Doom3 map imported.					Done
 
 */

/*
 September 11th
 Change bumpMapping to use model space	Done (No gain in terms of framerate :( !!! )
 */

/*
 September 8th
 Add bumpMapping						Done
 
*/

/*
	September 1th

	Add shader matrixStack				Done
	Add skeleton interpolated md5 anim	Done
 */


/*
	August 29th
	
	Add md5anim support					Done
	Add lighting support				Done
 
 */

/*
	August 18th
	Add stats and framerate display		Done
*/

/*
	August 16th
 
	Add PVRTC texture support			Done	 
 */

/*
	Project started on August 15th
 
 
	Add 3GS rendering path			Done
	Add md5 support					Done
	Add filesystem					Done
	
 */



/*
SHMUP is a 3D Shoot 'em up game inspired by Treasure Ikaruga
Copyright (C) 2009 Fabien Sanglard

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
along with SHMUP.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <string.h>

#include "SDL/SDL.h"

#include "../src/dEngine.h"
#include "../src/commands.h"
#include "../src/timer.h"
#include "../src/menu.h"
#include "../src/io_interface.h"

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480

int quit = 0;

static void SetupMouse(void)
{
    if (engine.sceneId == 0 || MENU_Get() == MENU_GAMEOVER)
    {
        SDL_WM_GrabInput(SDL_GRAB_OFF);
        SDL_ShowCursor(SDL_ENABLE);
    }
    else
    {
        SDL_WM_GrabInput(SDL_GRAB_ON);
        SDL_ShowCursor(SDL_DISABLE);
    }
}

static void ReadInput(void)
{
    SDL_Event sdlevent;
    io_event_s event;

    event.type = IO_EVENT_ENDED;
    event.position[X] = 0;
    event.position[Y] = 0;

    while (SDL_PollEvent(&sdlevent))
    {
        switch (sdlevent.type)
        {
            case SDL_KEYDOWN:

                switch (sdlevent.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        if (engine.requiredSceneId != 0 && engine.sceneId != 0)
                        {
                            MENU_Set(MENU_HOME);
                            engine.requiredSceneId = 0;
                        }
                        break;

                    default:
                        /* Ignore other events */
                        break;
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                event.type = IO_EVENT_BEGAN;
                event.position[X] = sdlevent.button.x;
                event.position[Y] = sdlevent.button.y;

                IO_PushEvent(&event);
                break;

            case SDL_MOUSEBUTTONUP:
                event.type = IO_EVENT_ENDED;
                event.position[X] = sdlevent.button.x;
                event.position[Y] = sdlevent.button.y;

                IO_PushEvent(&event);
                break;

            case SDL_MOUSEMOTION:
                event.type = IO_EVENT_MOVED;
                event.position[X] = sdlevent.motion.x;
                event.position[Y] = sdlevent.motion.y;
                event.previousPosition[X] = sdlevent.motion.x - sdlevent.motion.xrel;
                event.previousPosition[Y] = sdlevent.motion.y - sdlevent.motion.yrel;

                IO_PushEvent(&event);
                break;

            case SDL_QUIT:
                quit = 1;
                break;
            default:
                /* Ignore other events */
                break;
        }
    }

}

int main(void)
{
    int old_time;
    int new_time;
    int time_for_frame;
    int sleep_time;
    SDL_Surface *screen;
    uchar engineParameters = 0;

#ifndef RELEASE
    setenv("RD", "../..", 1);
    setenv("WD", "../..", 1);
#else
    setenv("RD", ".", 1);
    setenv("WD", ".", 1);
#endif

    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_WM_SetCaption("Shmup", NULL);

    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(SDL_ENABLE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 16, SDL_OPENGL);

    engineParameters |= GL_11_RENDERER ;

    renderer.statsEnabled    = 0;
    renderer.materialQuality = MATERIAL_QUALITY_HIGH;

    renderer.glBuffersDimensions[WIDTH]  = SCREEN_WIDTH;
    renderer.glBuffersDimensions[HEIGHT] = SCREEN_HEIGHT;

    dEngine_Init();
    renderer.statsEnabled = 0;
    renderer.props |= PROP_FOG;
    engine.licenseType = LICENSE_FULL;

    IO_Init();

    dEngine_InitDisplaySystem(engineParameters);

    old_time = SDL_GetTicks();

    while (!quit)
    {
        SetupMouse();

        ReadInput();
        dEngine_HostFrame();
        SDL_GL_SwapBuffers();

        new_time = SDL_GetTicks();
        time_for_frame = new_time - old_time;
        old_time = new_time;

        sleep_time = timediff - time_for_frame;

        if (sleep_time > 0)
            SDL_Delay(sleep_time);
    }

    SDL_Quit();

    return 0;
}


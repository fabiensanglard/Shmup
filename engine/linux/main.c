#include <string.h>

#ifdef GLES
#include "SDL2/SDL.h"
#else
#include "SDL/SDL.h"
#endif

#include "../src/dEngine.h"
#include "../src/commands.h"
#include "../src/timer.h"
#include "../src/menu.h"
#include "../src/io_interface.h"

#ifndef GLES
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480
#endif

#ifdef GLES
// SDL2
#define SDL_WM_GrabInput	SDL_SetRelativeMouseMode
#define SDL_GRAB_ON		SDL_TRUE
#define SDL_GRAB_OFF		SDL_FALSE

#include <SDL_opengles2.h>

typedef GLubyte* (APIENTRY * glGetString_Func)(unsigned int);
glGetString_Func glGetStringAPI = NULL;

#endif

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
#ifndef GLES
    SDL_Surface *screen;
#endif
    uchar engineParameters = 0;

#ifndef RELEASE
    setenv("RD", "../..", 1);
    setenv("WD", "../..", 1);
#else
    setenv("RD", ".", 1);
    setenv("WD", ".", 1);
#endif

#ifndef GLES
// SDL 1.2
    SDL_Init(SDL_INIT_EVERYTHING);

    SDL_WM_SetCaption("Shmup", NULL);

    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(SDL_ENABLE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 16, SDL_OPENGL);

#elif GLES
// SDL 2
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
      fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
      exit(1);
    }

    SDL_DisplayMode current;
    int should_be_zero = SDL_GetCurrentDisplayMode(0, &current);
    if(should_be_zero != 0) {
      SDL_Log("Could not get display mode for video display 0 %s", SDL_GetError());
    }
    else {
      #define SCREEN_WIDTH current.w
      #define SCREEN_HEIGHT current.h
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#ifdef GLES2
    // not working at the moment
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    SDL_Window* window = SDL_CreateWindow("Shmup", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);

    SDL_GLContext ctx = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, ctx);

    glGetStringAPI = (glGetString_Func)SDL_GL_GetProcAddress("glGetString");

    SDL_Log("Available renderers:\n");
    SDL_Log("\n");
    for(int it = 0; it < SDL_GetNumRenderDrivers(); it++) {
        SDL_RendererInfo info;
        SDL_GetRenderDriverInfo(it,&info);

        SDL_Log("%s\n", info.name);

    }
    SDL_Log("\n");
    SDL_Log("Vendor     : %s\n", glGetStringAPI(GL_VENDOR));
    SDL_Log("Renderer   : %s\n", glGetStringAPI(GL_RENDERER));
    SDL_Log("Version    : %s\n", glGetStringAPI(GL_VERSION));
    SDL_Log("Extensions : %s\n", glGetStringAPI(GL_EXTENSIONS));

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

#endif

#ifndef GLES
    engineParameters |= GL_11_RENDERER ;
#elif GLES
// GLES 2 should be possible but fails to compile shader and with different GL issues right now
#ifdef GLES2
    engineParameters |= GL_20_RENDERER ;
#else
    engineParameters |= GL_11_RENDERER ;
#endif
#endif
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
#ifndef GLES
// SDL 1.2
        SDL_GL_SwapBuffers();
#endif

#ifdef GLES
// SDL 2
	SDL_GL_SwapWindow(window);
#endif
        new_time = SDL_GetTicks();
        time_for_frame = new_time - old_time;
        old_time = new_time;

        sleep_time = timediff - time_for_frame;

        if (sleep_time > 0)
            SDL_Delay(sleep_time);
    }
#ifdef GLES
    SDL_DestroyWindow(window);
#endif
    SDL_Quit();

    return 0;
}


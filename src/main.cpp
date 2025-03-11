#define SDL_MAIN_USE_CALLBACKS
#include <SDL3/SDL_main.h>
#include "engine.hpp"

SDL_AppResult SDL_AppInit(void** appstate_pp, int argc, char** argv) {
    // create new engine object and put it into the SDL appstate
    Engine* engine_p = new Engine();
    *appstate_pp = engine_p;
    // init engine and return success
    engine_p->init();
    return SDL_AppResult::SDL_APP_CONTINUE;
}
SDL_AppResult SDL_AppEvent(void* appstate_p, SDL_Event* event_p) {
    Engine* engine_p = (Engine*)(appstate_p);
    return engine_p->execute_event(event_p);
}
SDL_AppResult SDL_AppIterate(void* appstate_p) {
    Engine* engine_p = (Engine*)(appstate_p);
    engine_p->execute_frame();
    return SDL_AppResult::SDL_APP_CONTINUE;
}
void SDL_AppQuit(void* appstate_p, SDL_AppResult) {
    Engine* engine_p = (Engine*)(appstate_p);
    engine_p->destroy();
    delete engine_p;
}
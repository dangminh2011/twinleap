#include "twinleap/core/App.hpp"
#include <SDL3/SDL_main.h>
int main(int argc, char **argv)
{
    twinleap::App app;
    return app.run(argc, argv);
}

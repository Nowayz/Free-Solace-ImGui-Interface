#ifdef __EMSCRIPTEN__
#include "runtime/web_app.h"
#else
#include "runtime/desktop_app.h"
#endif

int main(int, char**)
{
#ifdef __EMSCRIPTEN__
    return solace::runtime::run_web_app();
#else
    return solace::runtime::run_desktop_app();
#endif
}

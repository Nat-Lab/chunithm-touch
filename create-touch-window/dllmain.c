#include <windows.h>
#include "create-touch-window.h"

BOOL WINAPI DllMain(HMODULE mod, DWORD cause, void *ctx) {
    do_hook();
    return TRUE;
}

#include <windows.h>
#include "create-touch-window.h"
#include "hook/table.h"
#include "hook/com-proxy.h"

static HWND(WINAPI* n_CreateWindowExW)(
    DWORD     dwExStyle,
    LPCWSTR   lpClassName,
    LPCWSTR   lpWindowName,
    DWORD     dwStyle,
    int       X,
    int       Y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
);

static HWND(WINAPI* n_CreateWindowExA)(
    DWORD     dwExStyle,
    LPCSTR    lpClassName,
    LPCSTR    lpWindowName,
    DWORD     dwStyle,
    int       X,
    int       Y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
);

HWND WINAPI m_CreateWindowExW(
    DWORD     dwExStyle,
    LPCWSTR   lpClassName,
    LPCWSTR   lpWindowName,
    DWORD     dwStyle,
    int       X,
    int       Y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
) {
    HWND h = n_CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    RegisterTouchWindow(h, 0);
    return h;
}

HWND WINAPI m_CreateWindowExA(
    DWORD     dwExStyle,
    LPCSTR    lpClassName,
    LPCSTR    lpWindowName,
    DWORD     dwStyle,
    int       X,
    int       Y,
    int       nWidth,
    int       nHeight,
    HWND      hWndParent,
    HMENU     hMenu,
    HINSTANCE hInstance,
    LPVOID    lpParam
) {
    HWND h = n_CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
    RegisterTouchWindow(h, TWF_FINETOUCH | TWF_WANTPALM);
    return h;
}

static const struct hook_symbol cw_hooks[] = {
    { "CreateWindowExW", 0, m_CreateWindowExW, (void**)&n_CreateWindowExW },
    { "CreateWindowExA", 0, m_CreateWindowExA, (void**)&n_CreateWindowExA }
};

void do_hook() {
    hook_table_apply(NULL, "user32.dll", cw_hooks, _countof(cw_hooks));
}

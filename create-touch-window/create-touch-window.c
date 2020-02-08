#include "create-touch-window.h"
#include "hook/table.h"
#include "hook/com-proxy.h"
#include <windows.h>
#include <winuser.h>


void make_touchable (HWND h) {
    const BOOL enabled = FALSE;
    SetWindowFeedbackSetting(h, FEEDBACK_TOUCH_CONTACTVISUALIZATION, 0, sizeof(BOOL), &enabled);
    SetWindowFeedbackSetting(h, FEEDBACK_PEN_BARRELVISUALIZATION, 0, sizeof(BOOL), &enabled);
    SetWindowFeedbackSetting(h, FEEDBACK_PEN_TAP, 0, sizeof(BOOL), &enabled);
    SetWindowFeedbackSetting(h, FEEDBACK_PEN_DOUBLETAP, 0, sizeof(BOOL), &enabled);
    SetWindowFeedbackSetting(h, FEEDBACK_PEN_PRESSANDHOLD, 0, sizeof(BOOL), &enabled);
    SetWindowFeedbackSetting(h, FEEDBACK_PEN_RIGHTTAP, 0, sizeof(BOOL), &enabled);
    SetWindowFeedbackSetting(h, FEEDBACK_TOUCH_TAP, 0, sizeof(BOOL), &enabled);
    SetWindowFeedbackSetting(h, FEEDBACK_TOUCH_DOUBLETAP, 0, sizeof(BOOL), &enabled);
    SetWindowFeedbackSetting(h, FEEDBACK_TOUCH_PRESSANDHOLD, 0, sizeof(BOOL), &enabled);
    SetWindowFeedbackSetting(h, FEEDBACK_TOUCH_RIGHTTAP, 0, sizeof(BOOL), &enabled);
    SetWindowFeedbackSetting(h, FEEDBACK_GESTURE_PRESSANDTAP, 0, sizeof(BOOL), &enabled);
    RegisterTouchWindow(h, TWF_FINETOUCH | TWF_WANTPALM);
}

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
    make_touchable(h);
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
    make_touchable(h);
    return h;
}

static const struct hook_symbol cw_hooks[] = {
    { "CreateWindowExW", 0, m_CreateWindowExW, (void**)&n_CreateWindowExW },
    { "CreateWindowExA", 0, m_CreateWindowExA, (void**)&n_CreateWindowExA }
};

void do_hook() {
    hook_table_apply(NULL, "user32.dll", cw_hooks, _countof(cw_hooks));
}

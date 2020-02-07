#include <process.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <io.h>
#include "chuniio.h"
#include "log.h"
#define CHUNI_WINPROC CallWindowProc(chuni_wndproc, hwnd, msg, w_param, l_param)
#define MAXFINGERS 10
#define CONFIG L".\\chunitouch.ini"

static LONG chuni_ir_height = 5000;
static LONG chuni_key_height = 22000;
static LONG chuni_key_start = 31800;
static LONG chuni_key_width = 4000;

static LONG chuni_key_end = 0; //chuni_key_start + 32 * chuni_key_width;
static LONG chuni_key_min_y = 0; //108000 - chuni_key_height;
static LONG chuni_ir_max_y = 0; //chuni_key_min_y;
static LONG chuni_ir_min_y = 0; //chuni_key_min_y - 6 * chuni_ir_height;
static bool raw_input = false;

static unsigned int __stdcall chuni_io_slider_thread_proc(void* ctx);

static bool chuni_coin_pending = true;
static uint16_t chuni_coins = 0;
static uint8_t chuni_ir_sensor_map = 0;
static HANDLE chuni_io_slider_thread;
static bool chuni_io_slider_stop_flag;
static uint8_t chuni_sliders[32];
static WNDPROC chuni_wndproc;

static int get_ir_from_pos(LONG x, LONG y) {
    if (y < chuni_ir_min_y || y > chuni_ir_max_y) return -1;
    return 5 - ((y - chuni_ir_min_y) / chuni_ir_height);
}

static int get_slider_from_pos(LONG x, LONG y) {
    if (x < chuni_key_start || x > chuni_key_end || y < chuni_key_min_y) return -1;
    return 31 - ((x - chuni_key_start) / chuni_key_width);
}

static void chuni_io_ir(uint8_t *bitmap, uint8_t sensor_id, bool set) {
    if (sensor_id % 2 == 0) sensor_id++;
    else sensor_id--;
    if (set) *bitmap |= 1 << sensor_id;
    else *bitmap &= ~(1 << sensor_id);
}


LRESULT CALLBACK chuni_winproc_hook(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
    if (msg != WM_TOUCH) return CHUNI_WINPROC;

    UINT fingers = LOWORD(w_param);
    if (fingers <= 0) return CHUNI_WINPROC;
    POINT local_point;
    TOUCHINPUT inputs[MAXFINGERS];
    static uint8_t clicked_sliders[32];

    for (UINT i = 0; i < 32; i++) clicked_sliders[i] = 0;
    uint8_t chuni_ir_map_local = 0;

    if (GetTouchInputInfo((HTOUCHINPUT)l_param, fingers, inputs, sizeof(TOUCHINPUT))) {
        for (UINT i = 0; i < fingers; i++) {
            TOUCHINPUT p = inputs[i];
            local_point.x = p.x;
            local_point.y = p.y;
            if (!raw_input) ScreenToClient(hwnd, &local_point);
            int slider_id = get_slider_from_pos(local_point.x, local_point.y);
            int ir_id = get_ir_from_pos(local_point.x, local_point.y);
            if (slider_id >= 0 && slider_id < 32) {
                clicked_sliders[slider_id] = 128;
            }
                
            if (ir_id >= 0 && ir_id < 6) {
                chuni_io_ir(&chuni_ir_map_local, ir_id, true);
            }
                
        }
    }

    memcpy(chuni_sliders, clicked_sliders, 32);
    chuni_ir_sensor_map = chuni_ir_map_local;
    return CHUNI_WINPROC;
}

HRESULT chuni_io_jvs_init(void) {
    // hook winproc
    HWND hwnd = GetForegroundWindow();
    chuni_wndproc = (WNDPROC)SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)&chuni_winproc_hook);

    // alloc console for debug output
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    if (hwnd == NULL) log_error("can't get window handle for chuni.\n");

    log_info("allocated debug console.\n");

    chuni_ir_height = GetPrivateProfileIntW(L"ir", L"height", 50, CONFIG) * 100;
    chuni_key_height = GetPrivateProfileIntW(L"slider", L"height", 220, CONFIG) * 100;
    chuni_key_start = GetPrivateProfileIntW(L"slider", L"offset", 318, CONFIG) * 100;
    chuni_key_width = GetPrivateProfileIntW(L"slider", L"width", 40, CONFIG) * 100;
    raw_input = GetPrivateProfileIntW(L"io", L"raw_input", 0, CONFIG);

    chuni_key_end = chuni_key_start + 32 * chuni_key_width;
    chuni_key_min_y = 108000 - chuni_key_height;
    chuni_ir_max_y = chuni_key_min_y;
    chuni_ir_min_y = chuni_key_min_y - 6 * chuni_ir_height;

    log_info("ir: height: %ld (calculated min_y: %ld, max_y: %ld)\n", chuni_ir_height/100, chuni_ir_min_y/100, chuni_ir_max_y/100);
    log_info("key: height: %ld, width: %ld, min_x: %ld (calculated min_y: %ld, max_x: %ld)\n", chuni_key_height/100, chuni_key_width/100, chuni_key_start/100, chuni_key_min_y/100, chuni_key_end/100);
    log_info("raw_input: %s\n", raw_input ? "enabled" : "disabled");

    return S_OK;
}

void chuni_io_jvs_read_coin_counter(uint16_t* out) {
    if (out == NULL) {
        return;
    }

    if (GetAsyncKeyState(VK_F3)) {
        if (chuni_coin_pending) {
            chuni_coins++;
            chuni_coin_pending = false;
        }
    } else chuni_coin_pending = true;

    *out = chuni_coins;
}

void chuni_io_jvs_poll(uint8_t* opbtn, uint8_t* beams) {
    *beams = chuni_ir_sensor_map;
}

void chuni_io_jvs_set_coin_blocker(bool open) {
    if (open) log_info("coin blocker disabled");
    else log_info("coin blocker enabled.");

}

HRESULT chuni_io_slider_init(void) {
    log_info("init slider...\n");
    return S_OK;
}

void chuni_io_slider_start(chuni_io_slider_callback_t callback) {
    log_info("starting slider...\n");
    if (chuni_io_slider_thread != NULL) {
        return;
    }

    chuni_io_slider_thread = (HANDLE)_beginthreadex(
        NULL,
        0,
        chuni_io_slider_thread_proc,
        callback,
        0,
        NULL);
}

void chuni_io_slider_stop(void) {
    log_info("stopping slider...\n");
    if (chuni_io_slider_thread == NULL) {
        return;
    }

    chuni_io_slider_stop_flag = true;

    WaitForSingleObject(chuni_io_slider_thread, INFINITE);
    CloseHandle(chuni_io_slider_thread);
    chuni_io_slider_thread = NULL;
    chuni_io_slider_stop_flag = false;
}

void chuni_io_slider_set_leds(const uint8_t* rgb) {
    // we are touching chuni directly, so... don't care
}

static unsigned int __stdcall chuni_io_slider_thread_proc(void* ctx) {
    chuni_io_slider_callback_t callback;

    for (size_t i = 0; i < _countof(chuni_sliders); i++) chuni_sliders[i] = 0;

    callback = (chuni_io_slider_callback_t)ctx;

    while (!chuni_io_slider_stop_flag) {
        callback(chuni_sliders);
        Sleep(1);
    }

    return 0;
}

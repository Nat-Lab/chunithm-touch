#include <process.h>
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <io.h>
#include "chuniio.h"
#include "leapio/leapio.h"
#include "log.h"

// FIXME: free d2d on exit

#define CHUNI_WINPROC CallWindowProc(chuni_wndproc, hwnd, msg, w_param, l_param)
#define DEF_WINPROC DefWindowProc(hwnd, msg, w_param, l_param)
#define MAXFINGERS 10
#define CONFIG L".\\chunitouch.ini"

extern IMAGE_DOS_HEADER __ImageBase;
#define M_HINST ((HINSTANCE) &__ImageBase)

#define CSRC_TOUCH 0
#define CSRC_LEAP 1

#define LEAP_X 0
#define LEAP_Y 1
#define LEAP_Z 2

static BOOL separate_control = FALSE;
static LONG chuni_ir_trigger_threshold = 7000;
static LONG chuni_ir_height = 5000;
static UINT chuni_ir_leap_trigger = 500;
static UINT chuni_ir_leap_step = 300;
static uint8_t leap_orientation = LEAP_Y;

static LONG chuni_key_start = 31800;
static LONG chuni_key_width = 4000;

static LONG chuni_key_end = 0; //chuni_key_start + 32 * chuni_key_width;

static bool raw_input = false;
static uint8_t ir_control_source = CSRC_TOUCH;
static bool ir_keep_slider = false;

static unsigned int __stdcall chuni_io_slider_thread_proc(void* ctx);

static bool chuni_coin_pending = true;
static uint16_t chuni_coins = 0;
static uint8_t chuni_ir_sensor_map = 0;
static HANDLE chuni_io_slider_thread;
static bool chuni_io_slider_stop_flag;
static uint8_t chuni_sliders[32];
static WNDPROC chuni_wndproc;

static LONG start_locations[MAXFINGERS];
static LONG finger_ids[MAXFINGERS];

static D2D1_SIZE_U canvas_sz;
static ID2D1HwndRenderTarget *target = NULL;
static ID2D1SolidColorBrush* brushes[32];

static int get_slider_from_pos(LONG x, LONG y) {
    if (x < chuni_key_start || x > chuni_key_end) return -1;
    return 31 - ((x - chuni_key_start) / chuni_key_width);
}

static void chuni_io_ir(uint8_t *bitmap, int8_t sensor_id, bool set) {
    if (sensor_id > 5) sensor_id = 5;
    if (sensor_id < 0) sensor_id = 0;
    if (sensor_id % 2 == 0) sensor_id++;
    else sensor_id--;
    if (set) *bitmap |= 1 << sensor_id;
    else *bitmap &= ~(1 << sensor_id);
}

static int get_finger_index(DWORD id) {
    int avail_indx = -1;
    for (int i = 0; i < MAXFINGERS; i++) {
        if (finger_ids[i] > 0 && (DWORD) finger_ids[i] == id) return i;
        if (avail_indx == -1 && finger_ids[i] == -1) avail_indx = i;
    }

    if (avail_indx == -1) return -1;
    finger_ids[avail_indx] = id;
    return avail_indx;
}

LRESULT CALLBACK winproc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
    if (msg != WM_TOUCH) return separate_control ? DEF_WINPROC : CHUNI_WINPROC;

    UINT fingers = LOWORD(w_param);
    if (fingers <= 0) return separate_control ? DEF_WINPROC : CHUNI_WINPROC;
    POINT local_point;
    TOUCHINPUT inputs[MAXFINGERS];
    static uint8_t clicked_sliders[32];

    memset(clicked_sliders, 0, 32);
    uint8_t chuni_ir_map_local = 0;

    if (GetTouchInputInfo((HTOUCHINPUT)l_param, fingers, inputs, sizeof(TOUCHINPUT))) {
        for (UINT i = 0; i < fingers; i++) {
            TOUCHINPUT p = inputs[i];
            local_point.x = TOUCH_COORD_TO_PIXEL(p.x);
            local_point.y = TOUCH_COORD_TO_PIXEL(p.y);
            int fid = get_finger_index(p.dwID);
            if (fid < 0) {
                log_error("too many fingers.\n");
                continue;
            }
            if (p.dwFlags & TOUCHEVENTF_UP) {
                finger_ids[fid] = -1;
                continue;
            }
            if (!raw_input) {
                if (ScreenToClient(hwnd, &local_point) == 0) {
                    log_error("screen-to-client mapping failed");
                }
            }
            if (p.dwFlags & TOUCHEVENTF_DOWN) start_locations[fid] = local_point.y;
            LONG x_diff = start_locations[fid] - local_point.y;
            if (ir_control_source == CSRC_TOUCH && x_diff > chuni_ir_trigger_threshold) {
                int8_t ir_id = (x_diff / chuni_ir_height) - 1;
                chuni_io_ir(&chuni_ir_map_local, ir_id, true);
            }
            if (ir_control_source == CSRC_LEAP || x_diff <= chuni_ir_trigger_threshold || ir_keep_slider) {
                int slider_id = get_slider_from_pos(local_point.x, local_point.y);
                if (slider_id >= 0 && slider_id < 32) clicked_sliders[slider_id] = 128;
            }
        }
    }

    CloseTouchInputHandle((HTOUCHINPUT)l_param);

    memcpy(chuni_sliders, clicked_sliders, 32);
    chuni_ir_sensor_map = chuni_ir_map_local;
    return separate_control ? DEF_WINPROC : CHUNI_WINPROC;
}

static void render() {
    if (!target) return;
    ID2D1HwndRenderTarget_BeginDraw(target);

    float step = canvas_sz.width/32.;

    for (int i = 0; i < 32; i++) {
        D2D1_RECT_F r = { step * i, 0, step * (i+1), canvas_sz.height };
        ID2D1HwndRenderTarget_FillRectangle(target, &r, (ID2D1Brush *) brushes[i]);
    }

    if (ID2D1HwndRenderTarget_EndDraw(target, NULL, NULL) < 0) { // fixme: read tag
        log_fatal("render failed.\n");
    }
}

static void make_control_window() {
    if (target) return;
    ID2D1Factory* d2df = NULL;
    D2D1_FACTORY_OPTIONS opt = { D2D1_DEBUG_LEVEL_INFORMATION };
    if (D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &IID_ID2D1Factory, &opt, (void **) &d2df) != S_OK) {
        log_fatal("can't create d2d factoy.\n");
        // return?
    }
    const char *name = "chuni-controller";

    WNDCLASS c = { CS_NOCLOSE, winproc, 0, 0, M_HINST, NULL, LoadCursor(0, IDC_ARROW), NULL, NULL, name };
    RegisterClass(&c);
    HWND hwnd = CreateWindowEx(
        0, name, name, 
        WS_OVERLAPPED | WS_CAPTION,
        CW_USEDEFAULT, CW_USEDEFAULT, (32 * chuni_key_width), 
        (chuni_ir_height * 12 + chuni_ir_trigger_threshold), NULL, NULL, M_HINST, NULL
    );

    if (!hwnd) {
        log_fatal("can't create control window.\n");
        return;
    }

    RECT rc;
    GetClientRect(hwnd, &rc);

    canvas_sz.height = rc.bottom - rc.top;
    canvas_sz.width = rc.right - rc.left;

    D2D1_RENDER_TARGET_PROPERTIES rtp;
    rtp.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
    rtp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
    rtp.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    rtp.dpiX = rtp.dpiY = 0;
    rtp.usage = D2D1_RENDER_TARGET_USAGE_NONE;
    rtp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;

    D2D1_HWND_RENDER_TARGET_PROPERTIES hrtp;
    hrtp.hwnd = hwnd;
    hrtp.pixelSize = canvas_sz;
    hrtp.presentOptions = D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS;
    
    if (ID2D1Factory_CreateHwndRenderTarget(d2df, &rtp, &hrtp, &target) < 0) {
        log_fatal("can't create d2d render target.\n");
        // return
    }

    for (int i = 0; i < 32; i++) {
        D2D1_COLOR_F color = { i/32., i/32., i/32., 1. };
        if (ID2D1HwndRenderTarget_CreateSolidColorBrush(target, &color, NULL, &brushes[i]) < 0) {
            log_fatal("d2d brush creation failed.\n");
            // return
        }
    }

    ShowWindow(hwnd, SW_SHOWNORMAL);
}

void leap_handler(const LEAP_TRACKING_EVENT *ev) {
    uint8_t chuni_ir_map_local = 0;

    for(uint32_t h = 0; h < ev->nHands; h++) {
        const LEAP_HAND* hand = &(ev->pHands[h]);
        float pos = 0;
        if (leap_orientation == LEAP_X) pos = hand->palm.position.x;
        if (leap_orientation == LEAP_Y) pos = hand->palm.position.y;
        if (leap_orientation == LEAP_Z) pos = hand->palm.position.z;

        if (pos > chuni_ir_leap_trigger) {
            int8_t ir_id = (pos - chuni_ir_leap_trigger) / chuni_ir_leap_step - 1;
            if (ir_id > 5) ir_id = 5;
            if (ir_id < 0) ir_id = 0;
            chuni_io_ir(&chuni_ir_map_local, ir_id, true);
        }
    }

    chuni_ir_sensor_map = chuni_ir_map_local;
}

HRESULT chuni_io_jvs_init(void) {
    // hook winproc
    HWND hwnd = FindWindowA(NULL, "teaGfx DirectX Release");

    // alloc console for debug output
    AllocConsole();
    SetConsoleTitle("chuni-touch");
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    log_info("allocated debug console.\n");

    WCHAR str_control_src[16];
    WCHAR str_leap_orientation[16];

    separate_control = GetPrivateProfileIntW(L"options", L"separate_control", FALSE, CONFIG);
    chuni_ir_height = GetPrivateProfileIntW(L"ir", L"touch_height", 50, CONFIG);
    chuni_ir_trigger_threshold = GetPrivateProfileIntW(L"ir", L"touch_trigger", 70, CONFIG);
    chuni_ir_leap_trigger = GetPrivateProfileIntW(L"ir", L"leap_trigger", 500, CONFIG);
    chuni_ir_leap_step = GetPrivateProfileIntW(L"ir", L"leap_step", 300, CONFIG);
    chuni_key_start = GetPrivateProfileIntW(L"slider", L"offset", 318, CONFIG);
    chuni_key_width = GetPrivateProfileIntW(L"slider", L"width", 40, CONFIG);
    raw_input = GetPrivateProfileIntW(L"io", L"raw_input", 0, CONFIG);
    ir_keep_slider = GetPrivateProfileIntW(L"misc", L"ir_keep_slider", 0, CONFIG);

    if (separate_control) {
        chuni_key_start = 0;
        log_info("ignoring slider.offset in separate_control mode.\n");
    }

    if (hwnd == NULL) log_error("can't get window handle for chuni.\n");
    else if (!separate_control) {
        ULONG flags;
        if (!IsTouchWindow(hwnd, &flags)) log_warn("IsTouchWindow() returned false, touch might not work.\n");
#ifdef _WIN64
        chuni_wndproc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)&winproc);
#else
        chuni_wndproc = (WNDPROC)SetWindowLongPtr(hwnd, GWL_WNDPROC, (LONG_PTR)&winproc);
#endif
    
        log_info("hooked WNDPROC.\n");
    }

    GetPrivateProfileStringW(L"ir", L"control_source", L"touch", str_control_src, 16, CONFIG);
    GetPrivateProfileStringW(L"ir", L"leap_orientation", L"y", str_leap_orientation, 16, CONFIG);

    chuni_key_end = chuni_key_start + 32 * chuni_key_width;
    ir_control_source = (wcscmp(str_control_src, L"leap") == 0) ? CSRC_LEAP : CSRC_TOUCH;
    /**/ if (wcscmp(str_leap_orientation, L"x") == 0) leap_orientation = LEAP_X;
    else if (wcscmp(str_leap_orientation, L"y") == 0) leap_orientation = LEAP_Y;
    else if (wcscmp(str_leap_orientation, L"z") == 0) leap_orientation = LEAP_Z;

    for(int i = 0; i < MAXFINGERS; i++) finger_ids[i] = -1;

    if (ir_control_source == CSRC_LEAP) {
        log_info("connecting to leap service...\n");
        leap_connect(NULL);
        leap_set_tracking_handler(leap_handler);
        while (!leap_is_connected()) {
            Sleep(10);
        }
        log_info("connected to leap service.\n");
    }


    log_info("raw_input: %s\n", raw_input ? "enabled" : "disabled");
    log_info("separate_control: %s\n", separate_control ? "enabled" : "disabled");
    log_info("ir_keep_slider: %s\n", ir_keep_slider ? "enabled" : "disabled");
    log_info("key: start: %ld, width: %ld, end: %ld\n", chuni_key_start, chuni_key_width, chuni_key_end);

    if (ir_control_source == CSRC_TOUCH) {
        log_info("ir: touch mode, trigger_threshold: %ld, height: %ld\n", chuni_ir_trigger_threshold, chuni_ir_height);
    } else {
        log_info("ir: leap mode, axis: %u, trigger_threshold: %u, step: %u\n", leap_orientation, chuni_ir_leap_trigger, chuni_ir_leap_step);
    }

    if (separate_control) {
        log_info("creating separated control window...\n");
        make_control_window();
        render();
    }

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

void chuni_io_slider_set_leds(const uint8_t* brg) {
    if (separate_control) {
        for (int i = 31, ii = 0; i >= 0; i--, ii += 3) {
            D2D1_COLOR_F c = { brg[ii+1]/255., brg[ii+2]/255., brg[ii]/255., 1. };
            ID2D1SolidColorBrush_SetColor(brushes[i], &c);
        }
    }
    D2D1_COLOR_F c = { brg[91]/255., brg[92]/255., brg[90]/255., 1. };
    ID2D1SolidColorBrush_SetColor(brushes[0], &c); // hmm...
    render();
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

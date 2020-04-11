#include <math.h>
#include "leapio/leapio.h"
#include "log.h"

#define CONFIG L".\\chunitouch.ini"

static BOOL _test_mode = FALSE;
static uint32_t _n_hands = 0;
static float _x, _y, _z;
static BOOL use_leap;
static UINT leap_trigger;
static UINT leap_step;
static UINT leap_orientation;
static BOOL leap_inverted;

#define LEAP_X 0
#define LEAP_Y 1
#define LEAP_Z 2

void handle_track(const LEAP_TRACKING_EVENT *ev) {
    // log_debug("saw %u hands.\n", ev->nHands);
    static int8_t last_id = -1;
    _n_hands = ev->nHands;
    for(uint32_t h = 0; h < ev->nHands; h++) {
        const LEAP_HAND* hand = &(ev->pHands[h]);
        _x = hand->palm.position.x;
        _y = hand->palm.position.y;
        _z = hand->palm.position.z;

        if (_test_mode) {
            int8_t id = -1;
            float pos = 0;
            if (leap_orientation == LEAP_X) pos = _x;
            if (leap_orientation == LEAP_Y) pos = _y;
            if (leap_orientation == LEAP_Z) pos = _z;
            if ((!leap_inverted && pos > leap_trigger) || (leap_inverted && leap_trigger > pos)) {
                id = (leap_inverted ? -1 : 1) * (pos - leap_trigger) / leap_step - 1;
                if (id > 5) id = 5;
                if (id < 0) id = 0;
            }

            if (last_id != id) {
                if (id >= 0) log_info("IR %d triggered.\n", id + 1);
                else log_info("No IR triggered.\n");
                last_id = id;
            }
        }

        //log_debug("hand %u is a %s hand. location (%f, %f, %f).\n", 
        //    hand->id, hand->type == eLeapHandType_Left ? "left" : "right",
        //    hand->palm.position.x, hand->palm.position.y, hand->palm.position.z);
    }
}

char prompt(const char *p, const char *s, uint8_t n) {
    if (p != NULL) printf("%s ", p);
    if (n > 0 && s != NULL) {
        printf("[%c", s[0]);
        for (uint8_t i = 1; i < n; i ++) printf("/%c", s[i]);
        printf("] ");
    }
    char c = getchar();
    
    if (c == '\n') {
        if (s != NULL) return s[0];
    } else (void) getchar(); // eats "\n"
    return c;
}

void configure() {
    float low_x, low_y, low_z, high_x, high_y, high_z;

    while (TRUE) {
        prompt("Put your hand at the location you want the bottommost sensor to be activated, press [ENTER] when you are ready.", NULL, 0);
        if (_n_hands == 0) {
            printf("I can't see any hands.\n");
            continue;
        }
        if (_n_hands > 1) {
            printf("I saw more than one hand.\n");
            continue;
        }
        low_x = _x;
        low_y = _y;
        low_z = _z;
        break;
    }

    while (TRUE) {
        prompt("Put your hand at the location you want the topmost sensor to be activated, press [ENTER] when you are ready.", NULL, 0);
        if (_n_hands == 0) {
            printf("I can't see any hands.\n");
            continue;
        }
        if (_n_hands > 1) {
            printf("I saw more than one hand.\n");
            continue;
        }
        high_x = _x;
        high_y = _y;
        high_z = _z;
        break;
    }
    
    log_info("low: (%f, %f, %f), high: (%f, %f, %f).\n", low_x, low_y, low_z, high_x, high_y, high_z);
    float dx = high_x - low_x;
    float dy = high_y - low_y;
    float dz = high_z - low_z;
    float dmax = max(max(fabs(dx), fabs(dy)), fabs(dz));
    WCHAR leap_orientation_char;

    if (dmax == fabs(dx)) {
        leap_orientation_char = 'x';
        leap_orientation = LEAP_X;
        leap_trigger = low_x;
    }
    if (dmax == fabs(dy)) {
        leap_orientation_char = 'y';
        leap_orientation = LEAP_Y;
        leap_trigger = low_y;
    }
    if (dmax == fabs(dz)) {
        leap_orientation_char = 'z';
        leap_orientation = LEAP_Z;
        leap_trigger = low_z;
    }
    if (leap_orientation == LEAP_X && dx < 0) {
        leap_inverted = TRUE;
    }
    if (leap_orientation == LEAP_Y && dy < 0) {
        leap_inverted = TRUE;
    }
    if (leap_orientation == LEAP_Z && dz < 0) {
        leap_inverted = TRUE;
    }
    leap_step = dmax/6;
    use_leap = TRUE;

    WCHAR leap_trigger_str[16];
    WCHAR leap_step_str[16];
    WCHAR leap_orientation_str[16];

    swprintf_s(leap_trigger_str, 16, L"%d", leap_trigger);
    swprintf_s(leap_step_str, 16, L"%d", leap_step);
    swprintf_s(leap_orientation_str, 16, L"%s%c", leap_inverted ? L"-" : L"", leap_orientation_char);

    WritePrivateProfileStringW(L"ir", L"leap_trigger", leap_trigger_str, CONFIG);
    WritePrivateProfileStringW(L"ir", L"leap_step", leap_step_str, CONFIG);
    WritePrivateProfileStringW(L"ir", L"leap_orientation", leap_orientation_str, CONFIG);
}

void test() {
    printf("Move your hand around. Configurator will print out which IR sensor is being activated.\n");
    prompt("Press [ENTER] to begin test, press [ENTER] again to end.", NULL, 0);
    _test_mode = true;
    (void) getchar();
    _test_mode = false;
}

int main () {
    leap_trigger = GetPrivateProfileIntW(L"ir", L"leap_trigger", 50, CONFIG);
    leap_step = GetPrivateProfileIntW(L"ir", L"leap_step", 30, CONFIG);

    WCHAR str_control_src[16];
    WCHAR str_leap_orientation[16];

    GetPrivateProfileStringW(L"ir", L"control_source", L"touch", str_control_src, 16, CONFIG);
    GetPrivateProfileStringW(L"ir", L"leap_orientation", L"y", str_leap_orientation, 16, CONFIG);
    use_leap = wcscmp(str_control_src, L"leap") == 0;

    /**/ if (wcscmp(str_leap_orientation, L"x") == 0) leap_orientation = LEAP_X;
    else if (wcscmp(str_leap_orientation, L"y") == 0) leap_orientation = LEAP_Y;
    else if (wcscmp(str_leap_orientation, L"z") == 0) leap_orientation = LEAP_Z;
    else if (wcscmp(str_leap_orientation, L"-x") == 0) { leap_orientation = LEAP_X; leap_inverted = TRUE; }
    else if (wcscmp(str_leap_orientation, L"-y") == 0) { leap_orientation = LEAP_Y; leap_inverted = TRUE; }
    else if (wcscmp(str_leap_orientation, L"-z") == 0) { leap_orientation = LEAP_Z; leap_inverted = TRUE; }

    log_info("connecting to leap service...\n");
    leap_set_tracking_handler(handle_track); // debug

    leap_connect(NULL);
    while (!leap_is_connected()) {
        Sleep(10);
    }
    log_info("connected to leap service.\n");

    while (TRUE) {
        printf("chuni-touch: leap configurator\n");
        printf("current configured values: enabled: %s, trigger: %d, step: %d, orientation: %s%d.\n", use_leap ? "true" : "false", leap_trigger, leap_step, leap_inverted ? "-" : "", leap_orientation);
        printf("    c) configure\n");
        printf("    t) test\n");
        printf("\n");
        printf("    q) quit\n");
        char a = prompt("action", "Ctq", 3);
        switch (a) {
            case 'C':
            case 'c':
                configure();
                break;
            case 't':
                test();
                break;
            case 'q':
                return 0;
            default:
                printf("bad selection: %c.\n", a);
        }
    }

    leap_join_thread(); 

    // TODO

    log_info("disconnecting from leap service...\n");
    leap_disconnect();
    log_info("disconnected.\n");
    return 0;
}


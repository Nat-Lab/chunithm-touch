#include "leapio/leapio.h"
#include "log.h"

static BOOL _test_mode = false;
static uint32_t _n_hands = 0;
float _x, _y, _z;

void log_tracks(const LEAP_TRACKING_EVENT *ev) {
    log_debug("saw %u hands.\n", ev->nHands);
    _n_hands = ev->nHands;
    for(uint32_t h = 0; h < ev->nHands; h++) {
        const LEAP_HAND* hand = &(ev->pHands[h]);
        log_debug("hand %u is a %s hand. location (%f, %f, %f).\n", 
            hand->id, hand->type == eLeapHandType_Left ? "left" : "right",
            hand->palm.position.x, hand->palm.position.y, hand->palm.position.z);
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
    bool low_ok = false, high_ok = false;
    float low_x, low_y, low_z, high_x;

    while (!low_ok) {
        prompt("Put your hand at the lowest location you want the IR sensor to be triggered, then press [ENTER]", NULL, 0);
        if (_n_hands == 0) {
            printf("I can't see any hands.\n");
            continue;
        }
        if (_n_hands > 1) {
            printf("I saw more than one hand.\n");
            continue;
        }
    }
    
}

void test() {
    printf("Move your hand around. Configurator will print out which IR sensor is being activated.\n");
    prompt("Press [ENTER] to begin test, press [ENTER] again to end.", NULL, 0);
    _test_mode = true;
    (void) getchar();
}

int main () {
    log_info("connecting to leap service...\n");
    leap_set_tracking_handler(log_tracks); // debug

    leap_connect(NULL);
    while (!leap_is_connected()) {
        Sleep(10);
    }
    log_info("connected to leap service.\n");

    while (TRUE) {
        printf("chuni-touch: leap configurator\n");
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


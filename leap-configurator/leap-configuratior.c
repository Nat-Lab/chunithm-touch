#include "leapio/leapio.h"
#include "log.h"

void log_tracks(const LEAP_TRACKING_EVENT *ev) {
    log_debug("saw %u hands.\n", ev->nHands);
    for(uint32_t h = 0; h < ev->nHands; h++) {
        const LEAP_HAND* hand = &(ev->pHands[h]);
        log_debug("hand %u is a %s hand. location (%f, %f, %f).\n", 
            hand->id, hand->type == eLeapHandType_Left ? "left" : "right",
            hand->palm.position.x, hand->palm.position.y, hand->palm.position.z);
    }
}

int main () {
    log_info("connecting to leap service...\n");
    leap_set_tracking_handler(log_tracks); // debug

    leap_connect(NULL);
    while (!leap_is_connected()) {
        Sleep(10);
    }
    log_info("connected to leap service.\n");

    log_info("waiting for any leap device...\n");
    while (!leap_is_device_connected()) {
        Sleep(10);
    }
    log_info("leap device connected.\n");

    join(); 

    // TODO

    log_info("disconnecting from leap service...\n");
    leap_disconnect();
    log_info("disconnected.\n");
    return 0;
}


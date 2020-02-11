#include "leapio/leapio.h"
#include "log.h"

int main () {
    log_info("connecting to leap service...\n");

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

    // TODO

    log_info("disconnecting from leap service...\n");
    leap_disconnect();
    log_info("disconnected.\n");
    return 0;
}


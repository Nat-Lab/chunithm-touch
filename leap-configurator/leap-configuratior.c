#include "leapio/leapio.h"
#include "log.h"

int main () {
    log_info("waiting for leap device...\n");

    leap_connect(NULL);
    while (!leap_is_connected()) {
        Sleep(10);
    }

    // TODO

    return 0;
}


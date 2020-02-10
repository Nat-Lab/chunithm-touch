#pragma once
#include <stdlib.h>
#include "leapc.h"

typedef void (*leap_connect_callback_t)();
typedef void (*leap_tracking_callback_t)(const LEAP_TRACKING_EVENT *ev);

void leap_connect(leap_connect_callback_t cb);
void leap_disconnect();
void leap_set_tracking_handler(leap_tracking_callback_t cb);
void leap_unset_tracking_handler();
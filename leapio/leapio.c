#include "leapio.h"

static LEAP_CONNECTION _connection = NULL;
static leap_connect_callback_t _conn_cb = NULL;
static leap_tracking_callback_t _track_cb = NULL;
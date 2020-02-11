#pragma once
#include <windows.h>
#include <stdlib.h>
#include "LeapC.h"

typedef void (*leap_connect_callback_t)(BOOL connected);
typedef void (*leap_tracking_callback_t)(const LEAP_TRACKING_EVENT *ev);

/**
 * @brief check if 
 * 
 * @return BOOL 
 */
BOOL leap_is_connected();

/**
 * @brief connects to a leap controller.
 * 
 * @param cb callback on device connect/disconnect.
 * leapio will try to re-connect if the disconnect was not caused by a 
 * leap_disconnect call.
 * @return BOOL TRUE on success, FALSE otherwise.
 */
BOOL leap_connect(leap_connect_callback_t cb);

/**
 * @brief disconnect from the connected leap device.
 * 
 * @return BOOL TRUE on success, FALSE otherwise. 
 */
BOOL leap_disconnect();

/**
 * @brief set/update callback from tracking events. 
 * 
 * This call only registers the callback. You may call this before leap_connect.
 * 
 * @param cb tracking event callback.
 */
void leap_set_tracking_handler(leap_tracking_callback_t cb);

/**
 * @brief unset the tracking event callback.
 * 
 */
void leap_unset_tracking_handler();

/**
 * @brief join the event loop thread.
 * 
 */
void join();
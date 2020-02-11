#pragma once
#include <windows.h>
#include <stdlib.h>
#include "LeapC.h"

typedef void (*leap_connect_callback_t)(BOOL connected);
typedef void (*leap_tracking_callback_t)(const LEAP_TRACKING_EVENT *ev);

/**
 * @brief check if connection to leap service is established.
 * 
 * @return BOOL 
 */
BOOL leap_is_connected();

/**
 * @brief check if leap device is connected.
 * 
 * @return BOOL 
 */
BOOL leap_is_device_connected();

/**
 * @brief connects to a leap service.
 * 
 * @param cb callback on device connect/disconnect.
 * 
 * Note that successfully establishing a connection with leap service does not 
 * necessarily mean that we are ready to get hand tracking data from the leap 
 * controller. In fact, there might not even be any leap controller attached to 
 * the device. So, NO tracking data will be sent before device_connect callback.
 * 
 * leapio will try to re-connect if the disconnect was not caused by a 
 * leap_disconnect call.
 * @return BOOL TRUE on success, FALSE otherwise.
 */
BOOL leap_connect(leap_connect_callback_t cb);

/**
 * @brief disconnect from the leap service.
 * 
 * @return BOOL TRUE on success, FALSE otherwise. 
 */
BOOL leap_disconnect();

/**
 * @brief set/update callback for tracking events. 
 * 
 * This call only registers the callback. You may call this before leap_connect.
 * You may set callback to NULL.
 * 
 * @param cb tracking event callback.
 */
void leap_set_tracking_handler(leap_tracking_callback_t cb);

/**
 * @brief set/update callback for device connect/disconnect.
 * 
 * @param cb 
 */
void leap_set_device_ready_handler(leap_connect_callback_t cb);

/**
 * @brief join the event loop thread.
 * 
 */
void join();
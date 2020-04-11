#include "leapio.h"
#include "log.h"
#include <process.h>

static LEAP_CONNECTION _connection = NULL;
static leap_connect_callback_t _conn_cb = NULL;
static leap_connect_callback_t _devconn_cb = NULL;
static leap_tracking_callback_t _track_cb = NULL;
static BOOL _running = FALSE;
static BOOL _connected = FALSE;
static BOOL _device_connected = FALSE;
static HANDLE _polling_thread = NULL;

static const char* leap_result_string(eLeapRS r) {
    switch(r){
        case eLeapRS_Success:                  return "Success";
        case eLeapRS_UnknownError:             return "Unknown Error";
        case eLeapRS_InvalidArgument:          return "Invalid Argument";
        case eLeapRS_InsufficientResources:    return "Insufficient Resources";
        case eLeapRS_InsufficientBuffer:       return "Insufficient Buffer";
        case eLeapRS_Timeout:                  return "Timeout";
        case eLeapRS_NotConnected:             return "Not Connected";
        case eLeapRS_HandshakeIncomplete:      return "Handshake Incomplete";
        case eLeapRS_BufferSizeOverflow:       return "Buffer Size Overflow";
        case eLeapRS_ProtocolError:            return "Protocol Error";
        case eLeapRS_InvalidClientID:          return "Invalid Client ID";
        case eLeapRS_UnexpectedClosed:         return "Unexpected Closed";
        case eLeapRS_UnknownImageFrameRequest: return "Unknown Image Frame Request";
        case eLeapRS_UnknownTrackingFrameID:   return "Unknown Tracking Frame ID";
        case eLeapRS_RoutineIsNotSeer:         return "Routine Is Not Seer";
        case eLeapRS_TimestampTooEarly:        return "Timestamp Too Early";
        case eLeapRS_ConcurrentPoll:           return "Concurrent Poll";
        case eLeapRS_NotAvailable:             return "Not Available";
        case eLeapRS_NotStreaming:             return "Not Streaming";
        case eLeapRS_CannotOpenDevice:         return "Cannot Open Device";
        default:                               return "Internal Error";
    }
}

static void leap_log(const LEAP_LOG_EVENT* e) {
    switch(e->severity) {
        case eLeapLogSeverity_Unknown: log_notice("%s.\n", e->message); break;
        case eLeapLogSeverity_Critical: log_fatal("%s.\n", e->message); break;
        case eLeapLogSeverity_Warning: log_warn("%s.\n", e->message); break;
        // case eLeapLogSeverity_Information: log_info("%s.\n", e->message); break;
    }
}

static void leap_event_loop(void *_) {
    log_debug("spinned up leap event loop.\n");
    eLeapRS rslt;
    LEAP_CONNECTION_MESSAGE msg;

    while (_running) {
        UINT timeout = 1000;
        rslt = LeapPollConnection(_connection, timeout, &msg);

        if (rslt != eLeapRS_Success) {
            log_warn("LeapPollConnection: %s.\n", leap_result_string(rslt));
            continue;
        }

        switch (msg.type){
            case eLeapEventType_Connection:
                _connected = TRUE;
                if (_conn_cb != NULL) _conn_cb(TRUE);
                break;
            case eLeapEventType_ConnectionLost:
                _connected = FALSE;
                if (_conn_cb != NULL) _conn_cb(FALSE);
                break;
            case eLeapEventType_Device: {
                _device_connected = TRUE;
                if (_devconn_cb != NULL) _devconn_cb(TRUE);
                LEAP_DEVICE_INFO device_info;
                LEAP_DEVICE device;

                rslt = LeapOpenDevice(msg.device_event->device, &device);

                if (rslt != eLeapRS_Success) {
                    log_error("LeapOpenDevice: %s\n", leap_result_string(rslt));
                    goto device_handle_end;
                }

                device_info.size = sizeof(LEAP_DEVICE_INFO);
                device_info.serial_length = 1;
                device_info.serial = malloc(1);

                rslt = LeapGetDeviceInfo(device, &device_info);

                if (rslt == eLeapRS_Success) log_info("leap device %s connected.\n", device_info.serial);
                else if (rslt == eLeapRS_InsufficientBuffer) {
                    device_info.serial = realloc(device_info.serial, device_info.serial_length);
                    rslt = LeapGetDeviceInfo(device, &device_info);

                    if (rslt != eLeapRS_Success) {
                        log_error("LeapGetDeviceInfo: %s\n", leap_result_string(rslt));
                        goto device_handle_end;
                    }
                }
device_handle_end:
                free(device_info.serial);
                LeapCloseDevice(device); // this closes the handler for device, not the device itself.
                break;
            }
            case eLeapEventType_DeviceLost:
                _device_connected = FALSE;
                if (_devconn_cb != NULL) _devconn_cb(FALSE);
                log_warn("leap device lost.\n");
                break;
            case eLeapEventType_DeviceFailure:
                _device_connected = FALSE;
                if (_devconn_cb != NULL) _devconn_cb(FALSE);
                log_warn("leap device failure.\n");
                break;
            case eLeapEventType_Tracking:
                if (_track_cb != NULL) _track_cb(msg.tracking_event);
                break;
            case eLeapEventType_ImageComplete:
                break;
            case eLeapEventType_ImageRequestError:
                break;
            case eLeapEventType_LogEvent:
                leap_log(msg.log_event);
                break;
            case eLeapEventType_Policy:
                break;
            case eLeapEventType_ConfigChange:
                break;
            case eLeapEventType_ConfigResponse:
                break;
            case eLeapEventType_Image:
                break;
            case eLeapEventType_PointMappingChange:
                break;
            case eLeapEventType_LogEvents:
                for (uint32_t i = 0; i < msg.log_events->nEvents; i++) {
                    leap_log(&(msg.log_events->events[i]));
                }
                break;
            case eLeapEventType_HeadPose:
                break;
            default:
                break;
        } //switch msg.type

    } // while _running

    log_debug("leap event loop stopped.\n");
}

BOOL leap_is_connected() {
    return _connected;
}

BOOL leap_is_device_connected() {
    return _device_connected;
}

BOOL leap_connect(leap_connect_callback_t cb) {
    if (_running || _connection != NULL) return FALSE;
    _running = TRUE;
    _conn_cb = cb;
    eLeapRS rslt;

    rslt = LeapCreateConnection(NULL, &_connection);
    if (rslt != eLeapRS_Success) {
        log_error("LeapCreateConnection: %s\n", leap_result_string(rslt));
        goto fail;
    }

    rslt = LeapOpenConnection(_connection);
    if (rslt != eLeapRS_Success) {
        log_error("LeapOpenConnection: %s\n", leap_result_string(rslt));
        goto fail;
    }

    _polling_thread = (HANDLE) _beginthread(leap_event_loop, 0, NULL);
    return TRUE;

fail:
    LeapDestroyConnection(_connection);
    _connection = NULL;
    _running = FALSE;
    return FALSE;
}

BOOL leap_disconnect() {
    if (!_running || _connection == NULL) return FALSE;

    _running = FALSE;
    WaitForSingleObject(_polling_thread, INFINITE);
    CloseHandle(_polling_thread);
    LeapCloseConnection(_connection);
    LeapDestroyConnection(_connection);
    _connection = NULL;

    return TRUE;
}

void leap_set_tracking_handler(leap_tracking_callback_t cb) {
    _track_cb = cb;
}

void leap_unset_tracking_handler() {
    _track_cb = NULL;
}

void leap_join_thread() {
    WaitForSingleObject(_polling_thread, INFINITE);
}
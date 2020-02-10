#include "leapio.h"
#include "log.h"
#include <process.h>

static LEAP_CONNECTION _connection = NULL;
static leap_connect_callback_t _conn_cb = NULL;
static leap_tracking_callback_t _track_cb = NULL;
static BOOL _running = FALSE;
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
                break;
            case eLeapEventType_ConnectionLost:
                break;
            case eLeapEventType_Device:
                break;
            case eLeapEventType_DeviceLost:
                break;
            case eLeapEventType_DeviceFailure:
                break;
            case eLeapEventType_Tracking:
                break;
            case eLeapEventType_ImageComplete:
                break;
            case eLeapEventType_ImageRequestError:
                break;
            case eLeapEventType_LogEvent:
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
                break;
            case eLeapEventType_HeadPose:
                break;
            default:
                break;
        } //switch msg.type

    } // while _running

    log_debug("leap event loop stopped.\n");
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

void leap_set_tracking_handler(leap_tracking_callback_t cb) {
    _track_cb = cb;
}

void leap_unset_tracking_handler() {
    _track_cb = NULL;
}
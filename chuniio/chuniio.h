#pragma once
#ifndef WINVER
#define WINVER 0x0602
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0602
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410
#endif
#ifndef _WIN32_IE
#define _WIN32_IE 0x0700
#endif

#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <stdbool.h>
#include <stdint.h>
#include <d2d1.h>
#pragma comment(lib, "d2d1.lib")

typedef void (*chuni_io_slider_callback_t)(const uint8_t *state);
HRESULT chuni_io_jvs_init(void);
void chuni_io_jvs_poll(uint8_t *opbtn, uint8_t *beams);
void chuni_io_jvs_read_coin_counter(uint16_t *total);
void chuni_io_jvs_set_coin_blocker(bool open);
HRESULT chuni_io_slider_init(void);
void chuni_io_slider_start(chuni_io_slider_callback_t callback);
void chuni_io_slider_set_leds(const uint8_t *rgb);
void chuni_io_slider_stop(void);

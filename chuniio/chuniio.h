#pragma once

#include <windows.h>
#include <stdbool.h>
#include <stdint.h>

typedef void (*chuni_io_slider_callback_t)(const uint8_t *state);
HRESULT chuni_io_jvs_init(void);
void chuni_io_jvs_poll(uint8_t *opbtn, uint8_t *beams);
void chuni_io_jvs_read_coin_counter(uint16_t *total);
void chuni_io_jvs_set_coin_blocker(bool open);
HRESULT chuni_io_slider_init(void);
void chuni_io_slider_start(chuni_io_slider_callback_t callback);
void chuni_io_slider_set_leds(const uint8_t *rgb);
void chuni_io_slider_stop(void);

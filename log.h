#pragma once
#include <stdio.h>
#define log_debug(fmt, ...) __log("DEBUG", fmt, ## __VA_ARGS__)
#define log_info(fmt, ...) __log("INFO ", fmt, ## __VA_ARGS__)
#define log_notice(fmt, ...) __log("NOTE ", fmt, ## __VA_ARGS__)
#define log_warn(fmt, ...) __log("WARN ", fmt, ## __VA_ARGS__)
#define log_error(fmt, ...) __log("ERROR", fmt, ## __VA_ARGS__)
#define log_fatal(fmt, ...) __log("FATAL", fmt, ## __VA_ARGS__)
#define __log(log_level, fmt, ...) printf("[" log_level "] %s: " fmt, __FUNCTION__, ## __VA_ARGS__)

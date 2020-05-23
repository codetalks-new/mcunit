#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

const char *colorize(const char *color, const char *fmt, ...) {
  char tmp_buf[256] = {0};
  va_list args;
  va_start(args, fmt);
  vsnprintf(tmp_buf, 256, fmt, args);
  va_end(args);
  char *buf;
  asprintf(&buf, "%s%s%s", color, tmp_buf, "\033[0m");
  return buf;
}

int color_printf(const char *color, const char *fmt, ...) {
  char *buf = (char *)malloc(256);
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, 256, fmt, args);
  va_end(args);
  return printf("%s%s%s", color, buf, "\033[0m");
}

#define RED "\033[31m"
#define RED_BOLD "\033[31;1m"
#define GREEN "\033[32m"
#define GREEN_BOLD "\033[32;1m"
#define YELLOW "\033[33m"
#define YELLOW_BOLD "\033[33;1m"
#define BLUE "\033[34m"
#define BLUE_BOLD "\033[34;1m"
#define MAGENTA "\033[35m"
#define MAGENTA_BOLD "\033[35;1m"
#define CYAN "\033[36m"
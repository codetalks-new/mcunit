#pragma once
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

/// 用于生成适合日志输出显示的时间
char *log_time() {
  const u_int8_t TIME_LEN = 64;
  static char buf[TIME_LEN] = {0};
  bzero(buf, TIME_LEN);
  struct timeval tv;
  gettimeofday(&tv, NULL);
  struct tm *tm = localtime(&tv.tv_sec);
  int millis = tv.tv_usec / 1000;
  size_t pos = strftime(buf, TIME_LEN, "%F %T", tm);
  snprintf(&buf[pos], TIME_LEN - pos, ".%03d", millis);
  return buf;
}

#define CLEAN_ERRNO() (errno == 0 ? "None" : strerror(errno))
#define LOG_ERR(MSG, ...)                                                    \
  fprintf(stderr, "([%s][ERROR] %s:%s:%d: errno: %s) " MSG "\n", log_time(), \
          __FILE__, __func__, __LINE__, CLEAN_ERRNO(), ##__VA_ARGS__)

#define LOG_INFO(MSG, ...) \
  fprintf(stderr, "[%s][INFO] " MSG "\n", log_time(), ##__VA_ARGS__)

#define CHECK_FAIL(ret)       \
  ({                          \
    bool fail = ret == -1;    \
    int __save_errno = errno; \
    if (fail) {               \
      LOG_ERR("");            \
    }                         \
    errno = __save_errno;     \
    fail;                     \
  })

#define GUARD(ret)   \
  ({                 \
    if (ret == -1) { \
      return -1;     \
    }                \
  })
#define GUARD_NULL(ret)      \
  ({                       \
    if (ret == NULL) {     \
      return -1;           \
    }                      \
  }
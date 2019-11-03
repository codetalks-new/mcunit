
#include <assert.h>  /* For assert */
#include <pthread.h> /* For thread */
#include <signal.h>  /* */
#include <stdarg.h>
#include <stdio.h>     /*For printf asprintf*/
#include <stdlib.h>    /* For exit */
#include <string.h>    /* For strsignal */
#include <sys/conf.h>  /* For sysconf */
#include <sys/mman.h>  /* For mmap */
#include <sys/time.h>  /* For getimeofday */
#include <sys/times.h> /* For times */
#include <sys/wait.h>  /* For waitpid */
#include <time.h>      /* For clock,time,clock_gettime*/
#include <unistd.h>    /* For sleep */

/**
 * \033[31m
 *  \033 = ESC
 *  ESC [  : CSI 控制序列
 *  第一个参数: 31
 *  最终字节: m  图形再现
 *   0 : 默认或重置
 *   1 : 加粗或增强
 *
 *  \033[31m
 *  \033[31;1m
 *  \033[0m // 重置
 *  \033[41m
 *  \033[41;1m
 *  \033[38;5;2m
 *  \033[38;2;203:36:49m
 */
void demo_ansi_colors() {
  printf("\033[38;2;203:36:49m Closed \033[0m\n");
  for (int i = 0; i < 8; i++) {
    const int code = 40 + i;
    printf("%d \033[%d;1m color \033[0m\n", code, code);
  }
  for (int i = 0; i < 24; i++) {
    const int code = 232 + i;  // 255
    printf("%d \033[48;5;%dm color \033[0m\n", code, code);
  }
}

char *fmt_time(double ms) {
  // 执行时间: 100ms
  const char *color;
  if (ms < 100) {
    color = "green";
  } else if (ms < 200) {
    color = "yellow";
  } else {
    color = "red";
  }

  const char *reset = "reset";
  static char buf[64];
  snprintf(buf, sizeof(buf), "%s执行时间:%.4lfms%s", color, ms, reset);
  return buf;
}

void log_info(char *msg) {
  fprintf(stdout, "[INFO] %s\n", msg);
}

void log_perror(char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char buf[256];
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);
  perror(buf);
}

void demo_printf() {
  // C语言实战(2)-强大的 printf 家族 [自制单元测试库系列] -by
  // 代码会说话
  printf("Hello World\n");
  fprintf(stderr, "Hello World\n");
  fprintf(stdout, "Hello World\n");
  FILE *logfile = fopen("404.log", "r");
  if (logfile == NULL) {
    log_perror("无法打开 %s", "404.log");
  }

  log_info(fmt_time(300));
}

void demo_docs() {
  // glibc
  // https://sourceware.org/git/?p=glibc.git;a=blob;f=sysdeps/posix/time.c;h=52bc14acafe87d111918f198cceacd47f5ab8325;hb=4e42b5b8f89f0e288e68be7ad70f9525aebc2cff
  // times 每次调用耗时: 8ms 左右
  // clock 每次调用耗时: 5ms 左右
  // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp); 5ms 左右
  // 事实上 clock 在 glib 2.18 之后都是通过调用 clock_gettime 实现的.
  // 值得注意的是在 32位系统上 CLOCKS_PER_SEC 值为  1000000
  // 也就是每72分钟就会打转一次.
}

void demo_time() {
  time_t start = time(NULL);
  long i = 0;
  while (i < 10e6) {
    i++;
  }
  time_t end = time(NULL);
  printf("执行时间:%lds\n", end - start);
}

void demo_gettimeofday() {
  struct timeval start;
  struct timeval end;
  gettimeofday(&start, NULL);
  long i = 0;
  while (i < 10e6) {
    i++;
  }
  // epoch timestamp 1970-01-01 00:00:00 +0000 (UTC)
  gettimeofday(&end, NULL);

  long start_ms = start.tv_sec * 1000 + start.tv_usec / 1000;
  long end_ms = end.tv_sec * 1000 + end.tv_usec / 1000;
  printf("执行时间:%ldms\n", end_ms - start_ms);
}

void demo_clock() {
  clock_t ticks = clock();                       // 72 min
  double secs = (double)ticks / CLOCKS_PER_SEC;  // us
  printf("执行时间:%.4lfms\n", secs * 1000);
}

void demo_clock_gettime() {
  //【C语言实战】(3)如何正确测量程序的运行时间? [自制单元测试库系列] -
  // by 代码会说话
  struct tms t;
  struct timespec start;
  struct timespec end;
  struct timeval tv;
  long i = 0;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);  // 10.12 10.14
  while (i < 10e6) {
    i++;
    // times(&t);
    // gettimeofday(&tv, NULL); // vdto common_page
    // getpid
  }

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

  // double start_ms = start.tv_sec * 1000 + start.tv_nsec / 1000000;
  // double end_ms = end.tv_sec * 1000 + end.tv_nsec / 1000000;
  // printf("执行时间:%.4lfms\n", end_ms - start_ms);

  times(&t);
  long clocks_per_sec = sysconf(_SC_CLK_TCK);
  printf("clocks_per_sec: %ld\n", clocks_per_sec);
  double clocks_per_ms = (double)(clocks_per_sec) / 1000;
  // ms

  printf("用户空间时间:%.4lfms 内核空间时间:%.4lfms\n",
         (double)t.tms_utime / clocks_per_ms,
         (double)t.tms_stime / clocks_per_ms);
}

const char *format(const char *fmt, ...) {
  char *buf = malloc(64);
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, 64, fmt, args);
  va_end(args);
  return buf;
}

const char *colorize(const char *str, const char *color) {
  char *buf;
  asprintf(&buf, "%s%s%s", color, str, "\033[0m");
  return buf;
}
#define RED "\033[31m"
#define GREEN "\033[32m"
#define GREEN_BOLD "\033[32;1m"
#define YELLOW "\033[33m"
#define YELLOW_BOLD "\033[33;1m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define MAGENTA_BOLD "\033[35;1m"
#define CYAN "\033[36m"

typedef struct ColorPalette {
  // 主色调
  char *primary;
  // 辅色
  char *secondary;

} ColorPalette;

typedef struct Step {
  // 步骤名称
  char *name;
  // 用时(单位秒)
  long seconds;
  // 当前状态
  char *status;
} Step;

typedef struct Recipe {
  // 食谱名称
  char *name;
  // 步骤列表
  Step steps[10];
  // 状态
  char *status;
  // 输出信息的主色调
  ColorPalette colors;
} Recipe;

char *tea_egg_status = "无";
char *coffee_status = "无";
const char *const status_finish = "完成";

Recipe recipies[] = {
    {.name = "煮茶叶蛋",
     .steps =
         {
             {.name = "洗鸡蛋", .seconds = 1},
             {.name = "大火煮", .seconds = 2},
             {.name = "小火煮", .seconds = 2},
             {.name = "冷水冲凉", .seconds = 2},
         },
     .colors = {.primary = GREEN_BOLD, .secondary = CYAN}},
    {.name = "煮咖啡",
     .steps =
         {
             {.name = "准备材料", .seconds = 3},
             {.name = "研磨咖啡豆", .seconds = 0},
             {.name = "过滤", .seconds = 1},
             {.name = "加水煮", .seconds = 2},
             {.name = "加糖和奶", .seconds = 1},
         },
     .colors = {.primary = YELLOW_BOLD, .secondary = BLUE}},
};

void cook(Recipe *recipe) {
  const ColorPalette colors = recipe->colors;
  const char *label = colorize(format("[%s]", recipe->name), colors.primary);
  for (int i = 0; recipe->steps[i].name; i++) {
    Step *step = &recipe->steps[i];
    printf("%s [%d] %s 时间:%ld(秒)\n", label, i + 1,
           colorize(step->name, colors.secondary), step->seconds);
    sleep(step->seconds);
    step->status = "完成";
  }
  recipe->status = "完成";
  printf("\n%s\n\n",
         colorize(format("%s煮好了!", recipe->name), colors.primary));
  fflush(stdout);
}
void check_recipe(Recipe *recipe) {
  printf("正在检查 [%s] 的完成情况!\n", recipe->name);
  int finish_cnt = 0;
  int step_cnt = 0;
  for (int i = 0; recipe->steps[i].name; i++) {
    step_cnt += 1;
    Step step = recipe->steps[i];
    if (step.status && strcmp(step.status, status_finish) == 0) {
      finish_cnt += 1;
    }
    printf("[%s] %s\n", step.name, step.status);
  }
  printf("[%s]检查报告:\n", recipe->name);
  printf("  整体完成状况:[%s], 共:[%d步], 完成:[%d步]\n", recipe->status,
         step_cnt, finish_cnt);
}

// 煮茶叶蛋
void *make_tea_egg(void *arg) {
  cook(&recipies[0]);
  return NULL;
}

// 煮咖啡
void *make_coffee(void *arg) {
  cook(&recipies[1]);
  return NULL;
}

const char *format_exit_code(int exit_code) {
  char *buf;
  if (WIFEXITED(exit_code)) {
    // 子进程正常退出
    int status_code = WEXITSTATUS(exit_code);
    switch (status_code) {
      case EXIT_SUCCESS:
        return "完成";
      case EXIT_FAILURE:
        return "失败";
      default:
        asprintf(&buf, "未知退出状态码:[%d]", status_code);
        return buf;
    }
  } else if (WIFSIGNALED(exit_code)) {
    int signal_code = WTERMSIG(exit_code);
    const char *signame = strsignal(signal_code);
    asprintf(&buf, "子进程因为信号中断退出:[%s]", signame);
    return buf;
  } else {
    asprintf(&buf, "子进程异常退出:[%d]", exit_code);
    return buf;
  }
}

void demo_fork() {
  //【C语言实战】(6)Linux/Unix 进程通信-获得子进程退出状态.
  //[自制单元测试库系列]
  // by - 代码会说话
  pid_t tea_worker_id = fork();
  if (tea_worker_id == 0) {
    make_tea_egg(NULL);
  }
  if (tea_worker_id > 0) {
    pid_t coffee_worker_id = fork();
    if (coffee_worker_id == 0) {
      make_coffee(NULL);
    } else {
      int tea_status;
      int coffee_status;
      waitpid(tea_worker_id, &tea_status, 0);
      waitpid(coffee_worker_id, &coffee_status, 0);
      printf("茶叶蛋工作状态: %s\n", format_exit_code(tea_status));
      printf("咖啡完成状态: %s\n", format_exit_code(coffee_status));
    }
  }
  // 父进程退出 ,子进程还在运行, 孤儿进程, 停止, 僵尸进程, init
}

void demo_pthread_create() {
  //【C语言实战】(5) 创建多线程-及与进程比较崩溃 **隔离性**
  //[自制单元测试库系列]
  // POSIX  NPTL  //clone do_fork
  pthread_t tea_worker_id;
  pthread_t coffee_worker_id;
  int res;
  res = pthread_create(&tea_worker_id, NULL, make_tea_egg, NULL);
  res = pthread_create(&coffee_worker_id, NULL, make_coffee, NULL);

  pthread_join(tea_worker_id, NULL);
  pthread_join(coffee_worker_id, NULL);
  // sleep(10);
  printf("\n\n%s\n", colorize("咖啡和茶叶蛋都做好了,请慢用!", MAGENTA_BOLD));

  check_recipe(&recipies[0]);
  check_recipe(&recipies[1]);
}

void demo_fork_stdio_buf() {
  //【C语言实战】(7)Linux 多进程编程  - fork及缓冲区
  // by - 代码会说话
  // stdio stdio buf
  // setvbuf(stdout, NULL, _IONBF, 0);
  printf("以下是连接出错日志信息:\n");
  fflush(stdout);
  const char greetings[] = "\nhost=locahost,port=6379 \n";
  write(STDOUT_FILENO, greetings, sizeof(greetings));
  fork();
  //
}
void demo_fork_2() {
  const int pageSize = 4096;
  const int recipe_size = sizeof(Recipe);
  void *shared_memory_addr = mmap(NULL, pageSize * 1, PROT_READ | PROT_WRITE,
                                  MAP_ANONYMOUS | MAP_SHARED, 0, 0);

  memcpy(shared_memory_addr, recipies, sizeof(recipies));
  Recipe *shared_recipies = shared_memory_addr;
  if (fork() == 0) {
    cook(&shared_recipies[0]);
    return;
  }
  if (fork() == 0) {
    cook(&shared_recipies[1]);
    return;
  }
  while (wait(NULL) > 0)
    ;
  check_recipe(&shared_recipies[0]);
  check_recipe(&shared_recipies[1]);
}

int main(int argc, char const *argv[]) {
  int page_size = sysconf(_SC_PAGE_SIZE);
  printf("page_size=%d\n", page_size);
  printf("sizeof(Recipe) = %d\n", sizeof(Recipe));
  printf("sizeof(recipies) = %d\n", sizeof(recipies));
  demo_fork_2();

  return 0;
}
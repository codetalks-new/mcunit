#define _GNU_SOURCE 1  /*For asprintf */
#include <assert.h>    /* For assert */
#include <fcntl.h>     /* For O_* constants */
#include <pthread.h>   /* For thread */
#include <semaphore.h> /* For sem_* */
#include <stdbool.h>
#include <stdio.h>     /*For printf asprintf*/
#include <stdlib.h>    /* For exit */
#include <string.h>    /* For strsignal */
#include <sys/mman.h>  /* For mmap */
#include <sys/stat.h>  /* For mode constants */
#include <sys/time.h>  /* For getimeofday */
#include <sys/times.h> /* For times */
#include <sys/wait.h>  /* For waitpid */
#include <time.h>      /* For clock,time,clock_gettime*/
#include <unistd.h>    /* For sleep sysconf */
#include "color.h"     /* For colorize and color constants */
#include "mmap_x.h"    /* For mmap utils */

sem_t* sem_A;
sem_t* sem_B;
sem_t* sem_C;
sem_t* sem_D;
sem_t* sem_E1;
sem_t* sem_E2;

void P(sem_t* sem) {
  // 尝试减将信号量的值减1,如果信号量值为0则阻塞
  sem_wait(sem);
}

void V(sem_t* sem) {
  sem_post(sem);
}

void task_A() {
  P(sem_A);
  color_printf(RED, "任务A开始执行\n");
  int seconds = rand() % 5 + 1;
  sleep(seconds);
  color_printf(RED_BOLD, "任务A已经完成\n");
  V(sem_B);
}
void task_B() {
  P(sem_B);
  color_printf(GREEN, "任务B开始执行\n");
  int seconds = rand() % 5 + 1;
  sleep(seconds);
  color_printf(GREEN_BOLD, "任务B已经完成\n");
  V(sem_C);
  V(sem_D);
}

void task_C() {
  P(sem_C);
  color_printf(BLUE, "任务C开始执行\n");
  int seconds = rand() % 5 + 1;
  sleep(seconds);
  color_printf(BLUE_BOLD, "任务C已经完成\n");
  V(sem_E1);
}
void task_D() {
  P(sem_D);
  color_printf(YELLOW, "任务D开始执行\n");
  int seconds = rand() % 5 + 1;
  sleep(seconds);
  color_printf(YELLOW_BOLD, "任务D已经完成\n");
  V(sem_E2);
}
void task_E() {
  P(sem_E1);
  P(sem_E2);
  color_printf(MAGENTA, "任务E开始执行\n");
  int seconds = rand() % 5 + 1;
  sleep(seconds);
  color_printf(MAGENTA_BOLD, "任务E已经完成\n");
}

/**
 *
 *            -->[C] -->➘
 *  [A] -> [B]          [E]
 *            -->[D] -->⬈
 *
 */

int main(int argc, char const* argv[]) {
  // sem_wait 等需要 `-pthread` 连接选项
  //  clang -pthread -std=gnu11 -o bin/demo sem_demo.c
  // [Linux编程实战](10) 如何控制多进程执行顺序与依赖  - 使用 Posix PV信号量
  // Dijkstra(迪杰斯特拉)(1930-2002) 荷兰人  semaphore(信号量)  / ˈseməfɔ:(r) /
  // Proberen(测试,试一下), Verhogen (增加)

  MapOpts opts;
  opts.anonymous = true;
  opts.pages = 1;  // 4096
  void* addr = create_mmap(opts);
  sem_t* sems = (sem_t*)addr;
  sem_A = &sems[0];
  sem_B = &sems[1];
  sem_C = &sems[2];
  sem_D = &sems[3];
  sem_E1 = &sems[4];
  sem_E2 = &sems[5];
  const int pshared = 1;
  const int semInitValue = 0;
  for (int i = 0; i < 6; i++) {
    sem_init(&sems[i], pshared, semInitValue);
  }

  if (fork() == 0) {
    task_A();
    return 0;
  }
  if (fork() == 0) {
    task_B();
    return 0;
  }
  if (fork() == 0) {
    task_C();
    return 0;
  }
  if (fork() == 0) {
    task_D();
    return 0;
  }
  if (fork() == 0) {
    task_E();
    return 0;
  }
  V(sem_A);

  while (wait(NULL) > 0) {
    ;
  }
  return 0;
}

#include <assert.h>    /* For assert */
#include <fcntl.h>     /* For O_* constants */
#include <pthread.h>   /* For thread */
#include <semaphore.h> /* For sem_* */
#include <signal.h>    /* */
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>     /*For printf asprintf*/
#include <stdlib.h>    /* For exit */
#include <string.h>    /* For strsignal */
#include <sys/conf.h>  /* For sysconf */
#include <sys/mman.h>  /* For mmap */
#include <sys/stat.h>  /* For mode constants */
#include <sys/time.h>  /* For getimeofday */
#include <sys/times.h> /* For times */
#include <sys/wait.h>  /* For waitpid */
#include <time.h>      /* For clock,time,clock_gettime*/
#include <unistd.h>    /* For sleep */
#include "color.h"     /* For colorize and color constants */
#include "mmap_x.h"    /* For mmap utils */
static sem_t* sem_A;
static sem_t* sem_B;
static sem_t* sem_C;
static sem_t* sem_D;
static sem_t* sem_E1;
static sem_t* sem_E2;

void task_A() {
  sem_wait(sem_A);
  color_printf(RED, "任务A开始执行\n");
  int seconds = rand() % 5 + 1;
  sleep(seconds);
  color_printf(RED_BOLD, "任务A已经完成\n");
  sem_post(sem_B);
}
void task_B() {
  sem_wait(sem_B);
  color_printf(GREEN, "任务B开始执行\n");
  int seconds = rand() % 5 + 1;
  sleep(seconds);
  color_printf(GREEN_BOLD, "任务B已经完成\n");
  sem_post(sem_C);
  sem_post(sem_D);
}

void task_C() {
  sem_wait(sem_C);
  color_printf(BLUE, "任务C开始执行\n");
  int seconds = rand() % 5 + 1;
  sleep(seconds);
  color_printf(BLUE_BOLD, "任务C已经完成\n");
  sem_post(sem_E1);
}
void task_D() {
  sem_wait(sem_D);
  color_printf(YELLOW, "任务D开始执行\n");
  int seconds = rand() % 5 + 1;
  sleep(seconds);
  color_printf(YELLOW_BOLD, "任务D已经完成\n");
  sem_post(sem_E2);
}
void task_E() {
  sem_wait(sem_E1);
  sem_wait(sem_E2);
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
  //
  MapOpts mapOpts;
  mapOpts.anonymous = true;
  mapOpts.pages = 1;
  void* addr = create_mmap(mapOpts);
  sem_t* sems = (sem_t*)addr;
  sem_A = &sems[0];
  sem_B = &sems[1];
  sem_C = &sems[2];
  sem_D = &sems[3];
  sem_E1 = &sems[4];
  sem_E2 = &sems[5];
  const int pshared = 1;  // 标志在进程间共享
  const int initSemValue = 0;
  for (int i = 0; i < 6; i++) {
    int res = sem_init(&sems[i], pshared, initSemValue);
    if (res == SEM_FAILED) {
      perror("failed to create sem_t");
    }
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
  while (wait(NULL) > 0) {
    ;
  }
  return 0;
}

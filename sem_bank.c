#define _GNU_SOURCE 1  /*For asprintf */
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
#include <sys/mman.h>  /* For mmap */
#include <sys/stat.h>  /* For mode constants */
#include <sys/time.h>  /* For getimeofday */
#include <sys/times.h> /* For times */
#include <sys/wait.h>  /* For waitpid */
#include <time.h>      /* For clock,time,clock_gettime*/
#include <unistd.h>    /* For sleep sysconf */

#include "color.h" /* For colorize and color constants */
#include "log.h"
#include "mmap_x.h" /* For mmap utils */

#define MAX_FUND (100)
#define FUND_TYPE_COUNT (4)

typedef enum FundType {
  FundTypeTrip = 0,  // 模拟旅游基金余额
  FundTypeEdu = 1,   // 模拟教育基金余额
  FundTypeCar = 2,   // 模拟汽车基金余额
  FundTypeFood = 3,  // 模拟美食基金余额
} FundType;

static const FundType ALL_FUND_TYPES[] = {FundTypeTrip, FundTypeEdu,
                                          FundTypeCar, FundTypeFood};
static const char* const fund_type_texts[FUND_TYPE_COUNT] = {
    "旅游基金",
    "教育基金",
    "汽车基金",
    "美食基金",
};
static sem_t* bank_funds[FUND_TYPE_COUNT] = {NULL};

const char* fund_type_to_text(FundType type) {
  return fund_type_texts[type];
}

int bank_init() {
  MapOpts mapOpts;
  mapOpts.anonymous = true;
  mapOpts.pages = 1;
  void* addr = create_mmap(mapOpts);
  sem_t* sems = (sem_t*)addr;
  bank_funds[0] = &sems[0];
  bank_funds[1] = &sems[1];
  bank_funds[2] = &sems[2];
  bank_funds[3] = &sems[3];
  const int pshared = 1;  // 标志在进程间共享
  const unsigned init_fund = MAX_FUND;
  for (int i = 0; i < 4; i++) {
    int res = sem_init(&sems[i], pshared, init_fund);
    if (CHECK_FAIL(res)) {
      return -1;
    }
  }
  return 0;
}

sem_t* fund_of_type(FundType type) {
  return bank_funds[type];
}
int current_balance_of_type(FundType type) {
  sem_t* fund = fund_of_type(type);
  int balance = 0;
  int res = sem_getvalue(fund, &balance);
  if (CHECK_FAIL(res)) {
    return res;
  }
  return balance;
}

int bank_loan(FundType type, unsigned amount) {
  sem_t* fund = fund_of_type(type);
  for (; amount > 0; amount--) {
    int res = sem_wait(fund);
    if (CHECK_FAIL(res)) {
      return -1;
    }
  }
  return 0;
}

int bank_repay(FundType type, unsigned amount) {
  sem_t* fund = fund_of_type(type);
  for (; amount > 0; amount--) {
    int res = sem_post(fund);
    if (CHECK_FAIL(res)) {
      return -1;
    }
  }
  return 0;
}

// static const char* const colors[] = {RED, GREEN, BLUE, YELLOW, MAGENTA,
// CYAN}; static const char* const bold_colors[] = {RED_BOLD,    GREEN_BOLD,
// BLUE_BOLD,
//                                           YELLOW_BOLD, MAGENTA_BOLD, CYAN};
void thinking() {
  int seconds = rand() % 5 + 1;
  sleep(seconds);
}
int make_life(int no) {
  LOG_INFO("[编号%d]开始思考人生", no);
  thinking();
  unsigned my_fund[FUND_TYPE_COUNT] = {0};
  for (int i = 0; i < FUND_TYPE_COUNT; i++) {
    FundType type = ALL_FUND_TYPES[i];
    const char* type_text = fund_type_to_text(type);
    unsigned amount = rand() % current_balance_of_type(type) + 1;
    LOG_INFO("[编号%d]准备借 %d 元 %s", no, amount, type_text);
    int res = bank_loan(type, amount);
    if (CHECK_FAIL(res)) {
      return -1;
    }
    LOG_INFO("[编号%d]借到了 %d 元 %s", no, amount, type_text);
    GUARD(amount);
    my_fund[i] = amount;
  }
  for (int i = 0; i < FUND_TYPE_COUNT; i++) {
    thinking();
    unsigned amount = my_fund[i];
    FundType type = ALL_FUND_TYPES[i];
    const char* type_text = fund_type_to_text(type);
    bank_repay(type, amount);
    LOG_INFO("[编号%d]还%d 元 %s", no, amount, type_text);
  }
  unsigned luck = (rand() % 10) > 5;
  if (luck) {
    return 0;
  } else {
    return make_life(no);
  }
}

int main(int argc, char const* argv[]) {
  int res = bank_init();
  if (CHECK_FAIL(res)) {
    return -1;
  }
  const int max_people = rand() % 8 + 2;
  pid_t pid = -1;
  for (int i = 1; i < (max_people + 1); i++) {
    pid = fork();
    if (pid == 0) {
      make_life(i);
      return 0;
    } else if (CHECK_FAIL(pid)) {
      return -1;
    }
  }
  while (wait(NULL) > 0) {
    ;
  }
  return 0;
}

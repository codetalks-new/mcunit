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

#include "color.h" /* For colorize and color constants */
#include "log.h"
#include "mmap_x.h" /* For mmap utils */

#define MAX_BALANCE (100)
#define BALANCE_TYPE_COUNT (4)

typedef enum BalanceType {
  BalanceTypeTrip = 0,  // 模拟旅游基金余额
  BalanceTypeEdu = 1,   // 模拟教育基金余额
  BalanceTypeCar = 2,   // 模拟汽车基金余额
  BalanceTypeFood = 3,  // 模拟美食基金余额
} BalanceType;

static const BalanceType ALL_BLANCE_TYPES[] = {BalanceTypeTrip, BalanceTypeEdu,
                                               BalanceTypeCar, BalanceTypeFood};
static const char* const balance_type_texts[BALANCE_TYPE_COUNT] = {
    "旅游基金",
    "教育基金",
    "汽车基金",
    "美食基金",
};
static sem_t* bank_balances[BALANCE_TYPE_COUNT] = {NULL};

const char* balance_type_to_text(BalanceType type) {
  return balance_type_texts[type];
}

int bank_init() {
  MapOpts mapOpts;
  mapOpts.anonymous = true;
  mapOpts.pages = 1;
  void* addr = create_mmap(mapOpts);
  sem_t* sems = (sem_t*)addr;
  bank_balances[0] = &sems[0];
  bank_balances[1] = &sems[1];
  bank_balances[2] = &sems[2];
  bank_balances[3] = &sems[3];
  const int pshared = 1;  // 标志在进程间共享
  const unsigned init_balance = MAX_BALANCE;
  for (int i = 0; i < 4; i++) {
    int res = sem_init(&sems[i], pshared, init_balance);
    if (CHECK_FAIL(res)) {
      return -1;
    }
  }
  return 0;
}

sem_t* balance_of_type(BalanceType type) {
  return bank_balances[type];
}

int bank_loan(BalanceType type, unsigned amount) {
  sem_t* balance = balance_of_type(type);
  for (; amount > 0; amount--) {
    int res = sem_wait(balance);
    if (CHECK_FAIL(res)) {
      return -1;
    }
  }
  return 0;
}

int bank_repay(BalanceType type, unsigned amount) {
  sem_t* balance = balance_of_type(type);
  for (; amount > 0; amount--) {
    int res = sem_post(balance);
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
  unsigned my_balance[BALANCE_TYPE_COUNT] = {0};
  for (int i = 0; i < BALANCE_TYPE_COUNT; i++) {
    BalanceType type = ALL_BLANCE_TYPES[i];
    const char* type_text = balance_type_to_text(type);
    unsigned amount = rand() % MAX_BALANCE + 1;
    LOG_INFO("[编号%d]准备借 %d 元 %s", no, amount, type_text);
    int res = bank_loan(type, amount);
    if (CHECK_FAIL(res)) {
      return -1;
    }
    LOG_INFO("[编号%d]借到了 %d 元 %s", no, amount, type_text);
    GUARD(amount);
    my_balance[i] = amount;
  }
  for (int i = 0; i < BALANCE_TYPE_COUNT; i++) {
    thinking();
    unsigned amount = my_balance[i];
    BalanceType type = ALL_BLANCE_TYPES[i];
    const char* type_text = balance_type_to_text(type);
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

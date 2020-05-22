#define _GNU_SOURCE

#include <sched.h>
#include <stdbool.h>

typedef int (*CloneFn)(void *);

typedef struct CloneOpts {
  CloneFn fn;
  void *stack_addr;
  int flags;
  void *fn_args;
  //是否共享虚拟内存 CLONE_VM
  bool share_vm;
  // 是否共享文件描述符 CLONE_FILES
  bool share_files;
  // 是否共享文件系统相关信息 CLONE_FS
  bool share_fs;
  // 是否共享信号处理过程 CLONE_SIGHAND
  bool share_sighand;
  // 是否在同一个线程组
  bool share_thread_group;
  // 是否支持设置 TLS (Thread Local Storage), 通过 CLONE_SETTLS
  bool enable_tls;
  // 如果 启用了 tls 那么此值指向本地存储缓冲区(x86-64是 %fs base register)   或
  // void * newtls
  unsigned long newtls;
  // 是否将新的线程(进程) ID 写到 ptid 指针指向内存地址 CLONE_PARENT_SETTID
  bool set_ptid;
  //
  pid_t *ptid;
  // 新进程(线程)退出时是否需要将 ctid 指针指向的值清零. CLONE_CHILD_CLEARTID
  bool clear_tid;
  pid_t *ctid;
} CloneOpts;

int clone_x(CloneOpts opts) {
  int flags = 0;
  if (opts.share_vm) {
    flags |= CLONE_VM;
  }
  if (opts.share_files) {
    flags |= CLONE_FILES;
  }
  if (opts.share_fs) {
    flags |= CLONE_FS;
  }
  if (opts.share_sighand) {
    flags |= CLONE_SIGHAND;
  }
  if (opts.share_sighand) {
    flags |= CLONE_SIGHAND;
  }
  if (opts.share_thread_group) {
    flags |= CLONE_THREAD;
  }
  if (opts.enable_tls && opts.newtls) {
    flags |= CLONE_SETTLS;
  }
  if (opts.set_ptid && opts.ptid) {
    flags |= CLONE_PARENT_SETTID;
  }
  if (opts.clear_tid && opts.ctid) {
    flags |= CLONE_CHILD_CLEARTID;
  }
  return clone(opts.fn, opts.stack_addr, flags, opts.fn_args, opts.ptid,
               opts.newtls, opts.ctid);
}

// 创建进程
int new_process(CloneFn fn, void *fn_args) {
  CloneOpts opts;
  opts.fn = fn;
  opts.fn_args = fn_args;
  opts.share_sighand = true;
  return clone_x(opts);
}

// 创建进程
int new_thread(CloneFn fn, void *fn_args) {
  CloneOpts opts;
  opts.fn = fn;
  opts.fn_args = fn_args;
  opts.share_vm = true;
  opts.share_files = true;
  opts.share_fs = true;
  opts.share_thread_group = true;
  opts.enable_tls = true;
  opts.set_ptid = true;
  opts.clear_tid = true;
  opts.share_sighand = true;
  return clone_x(opts);
}
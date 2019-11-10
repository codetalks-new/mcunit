#pragma once
#include <stdbool.h>  /* For bool */
#include <sys/mman.h> /* For mmap */
#include <unistd.h>   /* For sysconf  and _SC_* */

typedef struct MapOpts {
  // 是否私有,默认共享
  bool private;
  // 是否是匿名,默认为文件映射
  bool anonymous;
  // 默认可读写
  bool read_only;
  // 映射大小,内存页数
  int pages;
  int fd;
  int offset;
} MapOpts;

void *create_mmap(MapOpts opts) {
  const long pageSize = sysconf(_SC_PAGE_SIZE);
  int prot = PROT_READ;
  if (!opts.read_only) {
    prot |= PROT_WRITE;
  }
  int flags = opts.private ? MAP_PRIVATE : MAP_SHARED;
  int fd = opts.fd;
  if (opts.anonymous) {
    flags |= MAP_ANONYMOUS;
    fd = -1;
  }
  void *hintAddr = NULL;
  const int pages = opts.pages ? opts.pages : 1;
  return mmap(hintAddr, pageSize * opts.pages, prot, flags, fd, opts.offset);
}

void *create_shared_anonymous_mmap(int pages) {
  MapOpts opts;
  opts.anonymous = true;
  opts.pages = pages;
  return create_mmap(opts);
}

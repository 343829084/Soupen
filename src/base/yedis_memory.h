#ifndef YEDIS_MEMORY_H_
#define YEDIS_MEMORY_H_
#include <string.h>
#include "../base/yedis_common.h"
#include "../server/yedis_global_info.h"
//memory allocation
using yedis_server::dbi;
#define yedis_malloc(size) ({ \
  int64_t tmp = size;  \
  void *p = malloc(tmp); \
  __sync_fetch_and_add(&(dbi.yedis_total_memory_used), tmp);\
  p;})

#define yedis_free(p, size) ({ \
  int64_t tmp = size; \
  free(p); \
  __sync_fetch_and_add(&(dbi.yedis_total_memory_used), -tmp);\
  ;})

template<typename T>
void yedis_reclaim(T *&p)
{
  if (YEDIS_LIKELY(nullptr != p)) {
    p->~T();
    yedis_free(p, sizeof(T));
    p = nullptr;
  }
}

//memory op
#define MEMSET(address, content, size) memset(address, content, size)
#define MEMCPY(dest, source, size) memcpy(dest, source, size)
#define MEMCMP(dest, source, size) memcmp(dest, source, size)

#endif /* YEDIS_MEMORY_H_ */

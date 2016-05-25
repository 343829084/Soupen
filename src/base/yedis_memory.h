#ifndef YEDIS_MEMORY_H_
#define YEDIS_MEMORY_H_
#include "../base/yedis_common.h"
#include "../server/yedis_info_manager.h"
//memory allocation

#define yedis_malloc(size) ({ \
  int64_t tmp = size;  \
  void *p = malloc(tmp); \
  yedis_server::YedisServerInfoManager::update_total_memory_used(tmp);\
  p;})

//memory reclaim

#define yedis_free(p, size) ({ \
  int64_t tmp = size; \
  free(p); \
  yedis_server::YedisServerInfoManager::update_total_memory_used(-tmp);\
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

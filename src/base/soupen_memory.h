#ifndef SOUPEN_MEMORY_H_
#define SOUPEN_MEMORY_H_
#include "../base/soupen_common.h"
#include "../server/soupen_info_manager.h"
//memory allocation

#define soupen_malloc(size) ({ \
  int64_t tmp = size;\
  void *p = malloc(tmp); \
  soupen_server::SoupenServerInfoManager::update_total_memory_used(tmp);\
  p;})

//memory reclaim

#define soupen_free(p, size) ({ \
  int64_t tmp = size; \
  free(p); \
  soupen_server::SoupenServerInfoManager::update_total_memory_used(-tmp);\
  ;})

template<typename T>
void soupen_reclaim(T *&p)
{
  if (SOUPEN_LIKELY(nullptr != p)) {
    p->~T();
    soupen_free(p, sizeof(T));
    p = nullptr;
  }
}

//memory op
#define MEMSET(address, content, size) memset(address, content, size)
#define MEMCPY(dest, source, size) memcpy(dest, source, size)
#define MEMCMP(dest, source, size) memcmp(dest, source, size)

#endif /* SOUPEN_MEMORY_H_ */

#include "../server/yedis_info_manager.h"
namespace yedis_server
{
  int64_t YedisServerInfoManager::memory_used_ = 0;
  int64_t YedisServerInfoManager::max_memory_ = 12 * 1024 * 1024 * 1024LL;
  int64_t YedisServerInfoManager::current_db_id_ = 0;
}




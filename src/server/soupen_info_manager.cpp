#include "../server/soupen_info_manager.h"
namespace soupen_server
{
  int64_t SoupenServerInfoManager::memory_used_ = 0;
  int64_t SoupenServerInfoManager::max_memory_ = 12 * 1024 * 1024 * 1024LL;
  int64_t SoupenServerInfoManager::current_db_id_ = 0;
}




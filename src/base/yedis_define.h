#ifndef YEDIS_DEFINE_H_
#define YEDIS_DEFINE_H_
//standard items
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define nullptr 0
#define YEDIS_INT64_MAX 9223372036854775807LL
#define YEDIS_UINT64_MIN 0ULL
#define YEDIS_ASSERT(condition) \
  if (!(condition)) { \
    exit(0);\
  }

//concurrency related items
#define CAS(address,oldValue,newValue) __sync_bool_compare_and_swap(address,oldValue,newValue)
#define ADD_AND_FETCH(address,offset) __sync_add_and_fetch(address,offset)
#define FETCH_AND_ADD(address,offset) __sync_fetch_and_add(address,offset)
#define LOAD_ATOMIC(address) __sync_add_and_fetch(address,0)
#define INC_ATOMIC(address, incremental) __sync_add_and_fetch(address,incremental)
#define ACCESS_ONCE(x) (*((volatile __typeof__(x) *) &x))
#define COMPILER_BARRIER() __asm__ __volatile__("" : : : "memory")
#define CPU_BARRIER() __sync_synchronize()
#define CPU_RELAX() __asm__ __volatile__("pause\n": : :"memory")

//performance related items
#define YEDIS_LIKELY(x) __builtin_expect(!!(x),1)
#define YEDIS_UNLIKELY(x) __builtin_expect(!!(x),0)
#define YEDIS_INLINE inline
#define YEDIS_MUST_INLINE inline __attribute__((always_inline))

//others

#define YEDIS_SUCCED (YEDIS_LIKELY(YEDIS_SUCCESS == ret))
#define YEDIS_FAILED (YEDIS_UNLIKELY(YEDIS_SUCCESS != ret))

//error codes
#define YEDIS_SUCCESS 0
#define YEDIS_ERROR_ENTRY_ALREADY_EXISTS  -1
#define YEDIS_ERROR_NOT_INITED  -2
#define YEDIS_ERROR_NO_MEMORY = -3
#define YEDIS_ERROR_EMPTY -4
#define YEDIS_ERROR_INDEX_OUT_OF_RANGE -5
#define YEDIS_ERROR_INVALID_ARGUMENT -6
#define YEDIS_ERROR_ENTRY_NOT_EXIST -7
#define YEDIS_ERROR_TABLE_FULL -8
#define YEDIS_ERROR_TABLE_NEED_RESIZE -9
#define YEDIS_ERROR_UNEXPECTED -10
#define YEDIS_ERROR_NOT_SUPPORT -11
#define YEDIS_ERROR_MEMORY_LIMITED -12

#endif /* YEDIS_DEFINE_H_ */

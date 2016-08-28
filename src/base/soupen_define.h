#ifndef SOUPEN_DEFINE_H_
#define SOUPEN_DEFINE_H_
//standard items
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define nullptr 0
#define SOUPEN_INT64_MAX 9223372036854775807LL
#define SOUPEN_UINT64_MIN 0ULL
#define SOUPEN_ASSERT(condition) \
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
#define SOUPEN_LIKELY(x) __builtin_expect(!!(x),1)
#define SOUPEN_UNLIKELY(x) __builtin_expect(!!(x),0)
#define SOUPEN_INLINE inline
#define SOUPEN_MUST_INLINE inline __attribute__((always_inline))

//others

#define SOUPEN_SUCCED (SOUPEN_LIKELY(SOUPEN_SUCCESS == ret))
#define SOUPEN_FAILED (SOUPEN_UNLIKELY(SOUPEN_SUCCESS != ret))

#define SOUPEN_FAIL(stmt) (SOUPEN_UNLIKELY(SOUPEN_SUCCESS != (ret = (stmt))))

//global config
#define MAX_DB_NUM 16

//error codes
#define SOUPEN_SUCCESS 0
#define SOUPEN_ERROR_ENTRY_ALREADY_EXISTS  -1
#define SOUPEN_ERROR_NOT_INITED  -2
#define SOUPEN_ERROR_NO_MEMORY -3
#define SOUPEN_ERROR_EMPTY -4
#define SOUPEN_ERROR_INDEX_OUT_OF_RANGE -5
#define SOUPEN_ERROR_INVALID_ARGUMENT -6
#define SOUPEN_ERROR_ENTRY_NOT_EXIST -7
#define SOUPEN_ERROR_TABLE_FULL -8
#define SOUPEN_ERROR_TABLE_NEED_RESIZE -9
#define SOUPEN_ERROR_UNEXPECTED -10
#define SOUPEN_ERROR_NOT_SUPPORT -11
#define SOUPEN_ERROR_MEMORY_LIMITED -12

#endif /* SOUPEN_DEFINE_H_ */

#include "../base/yedis_memory.h"
#include "../server/yedis_order.h"
#include "../ds/yedis_trie.h"
#include "../ds/yedis_bloom_filter.h"
#include <iostream>
#include <algorithm> //reverse
using namespace std;
using namespace yedis_datastructures;
namespace yedis_server
{
  YedisOrderRoutine order_funcs[MAX_TYPE] = {nullptr};
  YedisTrie<YedisOrderTrieNode> yt(false);
  const char *ORDER_NAME[MAX_TYPE] = {
      "hget",
      "hset",
      "mset",
      "mget",
      "get",
      "set",
      "bfadd",
      "bfcontains",
      "bfcreate",
      "bfdel",
      "tset",
      "tcontains",
      "tdel",
      "select",
      "flushdb"
  };
  const int ORDER_PARAM_NUM[MAX_TYPE] = {
      2, //"hget",
      3,//"hset",
      3,//"mset",
      2,//"mget",
      1,//"get",
      2,//"set",
      2,//"bfadd",
      2,//"bfcontains",
      3,//"bfcreate",
      1,//"bfdel",
      3,//"tset",
      2,//"tcontains",
      1,//"tdel"
      1,//"select"
      1//"flushdb"
  };
  YEDIS_MUST_INLINE int64_t char2int(char *start, char *end)
  {
    int64_t ret = 0;
    char *tmp = start;
    int sign = 1;
    if (*start == '-') {
      sign = -1;
      ++tmp;
    }
    while(tmp != end) {
      ret = ret * 10 + (*tmp - '0');
      ++tmp;
    }
    return ret * sign;
  }
  YEDIS_MUST_INLINE int64_t int2char(char *buffer, int64_t value)
  {
    static const char offset[19] = {'9','8','7','6','5','4','3','2','1','0','1','2','3','4','5','6','7','8','9'};
    static const char *digit = offset + 9;
    char *p = buffer;
    int64_t tmp = value;
    do {
      *p = *(digit + tmp % 10);
      tmp /= 10;
      ++p;
    } while(tmp);
    if (value < 0) {
      *p++ = '-';
    }
    std::reverse(buffer, p);
    return p - buffer;
  }
  YEDIS_MUST_INLINE int64_t get_int(char *&p)
  {
    char *tmp = p;
    while(*p != LEFT_SPILT && *(p+1) != RIGHT_SPILT) {
      p++;
    }
    return char2int(tmp, p);
  }
  YEDIS_MUST_INLINE int get_length_of_order(char *&p)
  {
    return static_cast<int>(get_int(p));
  }
  YEDIS_MUST_INLINE int get_param_nums(char *&p)
  {
    return static_cast<int>(get_int(p));
  }
  YEDIS_MUST_INLINE void skip_split(char *&p)
  {
    p += 2;
  }
  void set_params(char *&p, char **params, int *param_lens, int64_t param_num)
  {
    for (int64_t i = 0; i < param_num; i++) {
      char *tmp = p;
      while(*p != LEFT_SPILT && *(p+1) != RIGHT_SPILT) {
        p++;
      }
      param_lens[i] = char2int(tmp, p);
      skip_split(p);
      params[i] = p;
      p += param_lens[i];
      skip_split(p);
    }
  }
  int parser_text(char *text, char *out_buffer)
  {
    int ret = YEDIS_SUCCESS;
    char *p = text;
    char *params[MAX_PARAM_NUMS] = {nullptr};
    int param_lens[MAX_PARAM_NUMS] = {0};
    if (YEDIS_UNLIKELY(dbi.yedis_total_memory_used >= max_memory_limit)) {
      ret = YEDIS_ERROR_MEMORY_LIMITED;
    }
    while(YEDIS_SUCCESS == ret && *p != 'Y' && *(p+1) != 'E' && *(p+2) != 'D' && *(p+3) != 'I') {
      if (*p == '*') {
        p++;//skip '*'
        int length_of_order = get_length_of_order(p);
        skip_split(p);
        YedisOrderRoutine routine = get_order_routine(p, p + length_of_order);
        if (routine == nullptr) {
          strcpy(out_buffer, "-Operation not supported");
          ret = YEDIS_ERROR_NOT_SUPPORT;
          break;
        }
        p += length_of_order;
        skip_split(p);
        int num_of_parameters = get_param_nums(p);
        if (num_of_parameters == 0) {
          ret = YEDIS_ERROR_INVALID_ARGUMENT;
        } else if (num_of_parameters >= 1) {
          skip_split(p);
          set_params(p, params, param_lens, num_of_parameters);
          routine(out_buffer, params, param_lens, num_of_parameters);
        }
      }
    }
    return ret;
  }
  int bfadd(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(ORDER_PARAM_NUM[BFADD] != param_nums)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      YedisBloomFilterDSNode *tmp = nullptr;
      yedis_ds_node_find(params[0], param_lens[0], tmp);
      if (tmp != nullptr) {
        tmp->val->add(params[1], param_lens[1]);
      } else {
        CREATE_BLOOM_FILTER_NODE(YedisBloomFilter::DEFAULT_N, YedisBloomFilter::DEFAULT_M);
        if (YEDIS_LIKELY(YEDIS_SUCCESS == ret)) {
          yedis_ds_node_insert(bfnode, bf, string);
          bf->add(params[1], param_lens[1]);
        }
      }
    }

    if (YEDIS_SUCCESS == ret) {
      strcpy(out_buffer, "+OK");
    } else {
      strcpy(out_buffer, "-Operation failed");
    }
    return ret;
  }
  int bfcreate(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(ORDER_PARAM_NUM[BFCREATE] != param_nums)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      int64_t n = char2int(params[1], params[1] + param_lens[1]);
      int64_t m = char2int(params[2], params[2] + param_lens[2]);
      YedisBloomFilterDSNode *tmp = nullptr;
      yedis_ds_node_find(params[0], param_lens[0], tmp);
      if (tmp != nullptr) {
        YedisBloomFilter *bf = tmp->val;
        yedis_reclaim(bf);
        bf = static_cast<YedisBloomFilter*>(yedis_malloc(sizeof(YedisBloomFilter)));
        if (YEDIS_UNLIKELY(nullptr == bf)) {
          ret = YEDIS_ERROR_NO_MEMORY;
        } else if (YEDIS_UNLIKELY(YEDIS_SUCCESS != (ret = bf->init(n, m)))) {
          yedis_free(bf, sizeof(YedisBloomFilter));
        } else {
          tmp->val = bf;
        }
      } else {
        CREATE_BLOOM_FILTER_NODE(n, m);
        if (YEDIS_SUCCED) {
          yedis_ds_node_insert(bfnode, bf, string);
        }
      }
    }

    if (YEDIS_SUCCED) {
      strcpy(out_buffer, "+OK");
    } else {
      strcpy(out_buffer, "-Operation failed");
    }
    return ret;
  }
  int bfdel(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums)
  {
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(ORDER_PARAM_NUM[BFDEL] != param_nums)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      YedisBloomFilterDSNode *tmp = nullptr;
      yedis_ds_node_find(params[0], param_lens[0], tmp);
      if (tmp != nullptr) {
        yedis_ds_node_del(tmp);
      }
    }

    if (YEDIS_SUCCED) {
      strcpy(out_buffer, "+OK");
    } else {
      strcpy(out_buffer, "-Operation failed");
    }
    return ret;
  }
  int bfcontains(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums)
  {
    int ret = YEDIS_SUCCESS;
    bool is_found = false;
    if (YEDIS_UNLIKELY(ORDER_PARAM_NUM[BFCONTAINS] != param_nums)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      YedisBloomFilterDSNode *tmp = nullptr;
      yedis_ds_node_find(params[0], param_lens[0], tmp);
      if (tmp != nullptr) {
        is_found  = tmp->val->contains(params[1], param_lens[1]);
      }
    }

    if (YEDIS_FAILED) {
      strcpy(out_buffer, "-Operation failed");
    } else if(is_found) {
      strcpy(out_buffer, "+True");
    } else {
      strcpy(out_buffer, "+False");
    }
    return ret;
  }

  int tset(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums)
  {
    //tset db yedis 1
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(ORDER_PARAM_NUM[TSET] != param_nums)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      YedisTrieDSNode *tmp = nullptr;
      bool is_case_sensitive = static_cast<bool>(char2int(params[2], params[2] + param_lens[2]));
      yedis_ds_node_find(params[0], param_lens[0], tmp);
      if (tmp != nullptr) {
        tmp->val->add(params[1], params[1] + param_lens[1]);
      } else {
        CREATE_YEDIS_TRIE_NODE(is_case_sensitive);
        if (YEDIS_SUCCED) {
          yedis_ds_node_insert(trienode, trie, string);
          ret = trie->add(params[1], params[1] + param_lens[1]);
        }
      }
    }

    if (YEDIS_SUCCED) {
      strcpy(out_buffer, "+OK");
    } else {
      strcpy(out_buffer, "-Operation failed");
    }
    return ret;
  }
  int tcontains(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums)
  {
    //tcontains db yedis
    int ret = YEDIS_SUCCESS;
    bool is_found = false;
    if (YEDIS_UNLIKELY(ORDER_PARAM_NUM[TCONTAINS] != param_nums)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      YedisTrieDSNode *tmp = nullptr;
      yedis_ds_node_find(params[0], param_lens[0], tmp);
      if (tmp != nullptr) {
        is_found  = tmp->val->contains(params[1], params[1] + param_lens[1]);
      }
    }

    if (YEDIS_FAILED) {
      strcpy(out_buffer, "-Operation failed");
    } else if(is_found) {
      strcpy(out_buffer, "+True");
    } else {
      strcpy(out_buffer, "+False");
    }
    return ret;
  }
  int tdel(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums)
  {
    //tdel db
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(ORDER_PARAM_NUM[TDEL] != param_nums)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    }  else {
      YedisTrieDSNode *tmp = nullptr;
      yedis_ds_node_find(params[0], param_lens[0], tmp);
      if (tmp != nullptr) {
        yedis_ds_node_del(tmp);
      }
    }

    if (YEDIS_SUCCED) {
      strcpy(out_buffer, "+OK");
    } else {
      strcpy(out_buffer, "-Operation failed");
    }
    return ret;
  }

  int select(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums)
  {
    //change db
    int ret = YEDIS_SUCCESS;
    if (YEDIS_UNLIKELY(ORDER_PARAM_NUM[SELECT] != param_nums)) {
      ret = YEDIS_ERROR_INVALID_ARGUMENT;
    } else {
      int db_id = static_cast<int>(char2int(params[0], params[0] + param_lens[0]));
      if (db_id < 0 || db_id >= MAX_DB) {
        ret = YEDIS_ERROR_INVALID_ARGUMENT;
      } else {
        dbi.yedis_current_db_id = db_id;
      }
    }

    if (YEDIS_SUCCED) {
      strcpy(out_buffer, "+OK");
    } else {
      strcpy(out_buffer, "-Operation failed");
    }
    return ret;
  }
  int flushdb(char *out_buffer,
       char **params,
       int *param_lens,
       int param_nums)
   {
     //change db
     int ret = YEDIS_SUCCESS;
     if (YEDIS_UNLIKELY(ORDER_PARAM_NUM[FLUSHDB] != param_nums)) {
       ret = YEDIS_ERROR_INVALID_ARGUMENT;
     } else {
       int db_id = static_cast<int>(char2int(params[0], params[0] + param_lens[0]));
       if (YEDIS_UNLIKELY(db_id < -1 || db_id >= MAX_DB)) {
         ret = YEDIS_ERROR_INVALID_ARGUMENT;
       } else if (YEDIS_UNLIKELY(db_id == -1)) {
         //flush all db
       } else {
         //flush db_id
       }
     }

     if (YEDIS_SUCCED) {
       strcpy(out_buffer, "+OK");
     } else {
       strcpy(out_buffer, "-Operation failed");
     }
     return ret;
   }
  ////////////////////////////////////////////////////////////////////////
  int set_order_routine()
  {
    int ret = YEDIS_SUCCESS;
    for (int i = 0; YEDIS_SUCCESS == ret && i < MAX_TYPE ; ++i) {
      YedisOrderTrieNode *tn = (YedisOrderTrieNode*)yedis_malloc(sizeof(YedisOrderTrieNode));
      if (YEDIS_UNLIKELY(nullptr == tn)) {
        ret = YEDIS_ERROR_NO_MEMORY;
      } else if (YEDIS_UNLIKELY(YEDIS_SUCCESS != (ret = tn->init(false)))) {
        yedis_reclaim(tn);
      } else {
        tn->order_type = static_cast<YedisOrderType>(i);
        tn->routine_func = order_funcs[i];
        ret = yt.add(ORDER_NAME[i], tn);
      }
    }
    return ret;
  }
  int init_order_funcs()
  {
    order_funcs[BFADD] = bfadd;
    order_funcs[BFCONTAINS] = bfcontains;
    order_funcs[BFCREATE] = bfcreate;
    order_funcs[BFDEL] = bfdel;
    order_funcs[TSET] = tset;
    order_funcs[TCONTAINS] = tcontains;
    order_funcs[TDEL] = tdel;
    order_funcs[SELECT] = select;
    order_funcs[FLUSHDB] = flushdb;
    return YEDIS_SUCCESS;
  }
  YedisOrderRoutine get_order_routine(char *order_name_start, char *order_name_end)
  {
    YedisOrderRoutine ret = nullptr;
    YedisOrderTrieNode *ot = nullptr;
    bool b = yt.contains(order_name_start, order_name_end, ot);
    if (b && nullptr != ot) {
      ret = ot->routine_func;
    }
    return ret;
  }

  ///////////////////////////////////////////////////////////////////////////
  DEFINE_YEDIS_GET_DS_NODE(YedisBloomFilterDSNode, bf)
  DEFINE_YEDIS_DEL_DS_NODE(YedisBloomFilterDSNode, bf)
  DEFINE_YEDIS_INSERT_DS_NODE(YedisBloomFilterDSNode, YedisBloomFilter, bf)
  DEFINE_YEDIS_GET_DS_NODE(YedisTrieDSNode, tn)
  DEFINE_YEDIS_DEL_DS_NODE(YedisTrieDSNode, tn)
  DEFINE_YEDIS_INSERT_DS_NODE(YedisTrieDSNode, YedisTrie<YedisTrieNode>, tn)
}

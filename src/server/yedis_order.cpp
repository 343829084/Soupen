#include "../base/yedis_common.h"
#include "../server/yedis_order.h"
#include "../server/yedis_global_info.h"
#include "../ds/yedis_trie.h"
#include "../ds/yedis_bloom_filter.h"
#include<iostream>
using namespace std;
using namespace yedis_datastructures;
namespace yedis_server
{
  YedisOrderRoutine order_funcs[MAX_TYPE] = {nullptr};
  YedisTrie<YedisOrderTrieNode> yt(false);
  const char *order_name[MAX_TYPE] = {
      "hget",
      "hset",
      "tset",
      "tget",
      "get",
      "set",
      "bfadd",
      "bfcontains",
      "bfcreate",
      "bfdel",
      "tset",
      "tcontains",
      "tdel"
  };
  YEDIS_MUST_INLINE int char2int(char *start, char *end)
  {
    int ret = 0;
    char *tmp = start;
    while(tmp != end) {
      ret = ret * 10 + (*tmp - '0');
      ++tmp;
    }
    return ret;
  }
  void set_order_routine()
  {
    int ret = YEDIS_SUCCESS;
    for (int i = 0; YEDIS_SUCCESS == ret && i < MAX_TYPE ; ++i) {
      YedisOrderTrieNode *tn_buffer = (YedisOrderTrieNode*)yedis_malloc(sizeof(YedisOrderTrieNode));
      if (YEDIS_UNLIKELY(nullptr == tn_buffer)) {
        ret = YEDIS_ERROR_NO_MEMORY;
      } else {
        YedisOrderTrieNode *tn = new(tn_buffer) YedisOrderTrieNode(false);
        tn->order_type = static_cast<YedisOrderType>(i);
        tn->routine_func = order_funcs[i];
        ret = yt.add(order_name[i], tn);
      }
    }
  }
  int get_length_of_order(char *&p)
  {
    char *tmp = p;
    while(*p != LEFT_SPILT && *(p+1) != RIGHT_SPILT) {
      p++;
    }
    return char2int(tmp, p);
  }
  int get_param_nums(char *&p)
  {
    char *tmp = p;
    while(*p != LEFT_SPILT && *(p+1) != RIGHT_SPILT) {
      p++;
    }
    return char2int(tmp, p);
  }
  void skip_split(char *&p)
  {
    p+=2;
  }
  void set_params(char *&p, char **params, int *param_lens, int param_num)
  {
    for (int i = 0; i < param_num; i++) {
      char *tmp = p;
      while(*p != LEFT_SPILT && *(p+1) != RIGHT_SPILT) {
        p++;
      }
      param_lens[i] = char2int(tmp, p);
      skip_split(p);
      params[i] = p;
      p+=param_lens[i];
      skip_split(p);
    }
  }
  int parser_text(char *text, char *out_buffer)
  {
    int ret = YEDIS_SUCCESS;
    char *p = text;
    char *params[MAX_PARAM_NUMS]={nullptr};
    int param_lens[MAX_PARAM_NUMS]={0};
    while(YEDIS_SUCCESS == ret && *p != 'Y' && *(p+1) != 'E' && *(p+2) != 'D' && *(p+3) != 'I') {
      if (*p == '*') {
        p++;//skip '*'
        int length_of_order = get_length_of_order(p);
        skip_split(p);
        YedisOrderRoutine routine = get_order_routine(p, p + length_of_order);
        if (routine == nullptr) {
          strcpy(out_buffer, "op not supported!!!");
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
    YedisBloomFilterDSNode *tmp = nullptr;
    yedis_ds_node_find(params[0], param_lens[0], tmp);
    if (tmp != nullptr) {
      tmp->val->add(params[1], param_lens[1]);
    } else {
      CREATE_BLOOM_FILTER_NODE(YedisBloomFilter::DEFAULT_N, YedisBloomFilter::DEFAULT_M);
      if (YEDIS_LIKELY(YEDIS_SUCCESS == ret)) {
        yedis_ds_node_insert(bfnode, bf, string);
      }
    }
    if (YEDIS_SUCCESS == ret) {
      strcpy(out_buffer, "ok");
    } else {
      strcpy(out_buffer, "op failed");
    }
    return ret;
  }
  int bfcreate(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums)
  {
    int ret = YEDIS_SUCCESS;
    int n = char2int(params[1], params[1] + param_lens[1]);
    int m = char2int(params[2], params[2] + param_lens[2]);
    YedisBloomFilterDSNode *tmp = nullptr;
    yedis_ds_node_find(params[0], param_lens[0], tmp);
    if (tmp != nullptr) {
      YedisBloomFilter *bf = tmp->val;
      bf->~YedisBloomFilter();
      yedis_free(bf, sizeof(YedisBloomFilter));
      YedisBloomFilter *buffer_bf = static_cast<YedisBloomFilter*>(yedis_malloc(sizeof(YedisBloomFilter)));
      if (YEDIS_UNLIKELY(nullptr == buffer_bf)) {
        ret = YEDIS_ERROR_NO_MEMORY;
      } else {
        YedisBloomFilter *bf = new (buffer_bf) YedisBloomFilter(n, m);
        tmp->val = bf;
      }
    } else {
      CREATE_BLOOM_FILTER_NODE(n, m);
      if (YEDIS_LIKELY(YEDIS_SUCCESS == ret)) {
        yedis_ds_node_insert(bfnode, bf, string);
      }
    }
    if (YEDIS_LIKELY(YEDIS_SUCCESS == ret)) {
      strcpy(out_buffer, "ok");
    } else {
      strcpy(out_buffer, "op failed");
    }
    return ret;
  }
  int bfdel(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums)
  {
    int ret = YEDIS_SUCCESS;
    YedisBloomFilterDSNode *tmp = nullptr;
    yedis_ds_node_find(params[0], param_lens[0], tmp);
    if (tmp != nullptr) {
      yedis_ds_node_del(tmp);
    }
    if (YEDIS_SUCCESS == ret) {
      strcpy(out_buffer, "ok");
    } else {
      strcpy(out_buffer, "op failed");
    }
    return ret;
  }
  int bfcontains(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums)
  {
    int ret = YEDIS_SUCCESS;
    YedisBloomFilterDSNode *tmp = nullptr;
    yedis_ds_node_find(params[0], param_lens[0], tmp);
    bool is_found = false;
    if (tmp != nullptr) {
      is_found  = tmp->val->contains(params[1], param_lens[1]);
    }
    if(is_found) {
      strcpy(out_buffer, "exist");
    } else {
      strcpy(out_buffer, "not exist");
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
    YedisTrieDSNode *tmp = nullptr;
    bool is_case_sensitive = static_cast<bool>(char2int(params[2], params[2] + param_lens[2]));
    yedis_ds_node_find(params[0], param_lens[0], tmp);
    if (tmp != nullptr) {
      tmp->val->add(params[1], params[1] + param_lens[1]);
    } else {
      CREATE_YEDIS_TRIE_NODE(is_case_sensitive);
      if (YEDIS_LIKELY(YEDIS_SUCCESS == ret)) {
        yedis_ds_node_insert(trienode, trie, string);
        ret = trie->add(params[1], params[1] + param_lens[1]);
      }
    }
    if (YEDIS_SUCCESS == ret) {
      strcpy(out_buffer, "ok");
    } else {
      strcpy(out_buffer, "op failed");
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
    YedisTrieDSNode *tmp = nullptr;
    yedis_ds_node_find(params[0], param_lens[0], tmp);
    bool is_found = false;
    if (tmp != nullptr) {
      is_found  = tmp->val->contains(params[1], params[1] + param_lens[1]);
    }
    if(is_found) {
      strcpy(out_buffer, "exist");
    } else {
      strcpy(out_buffer, "not exist");
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
    YedisTrieDSNode *tmp = nullptr;
    yedis_ds_node_find(params[0], param_lens[0], tmp);
    if (tmp != nullptr) {
      yedis_ds_node_del(tmp);
    }
    if (YEDIS_SUCCESS == ret) {
      strcpy(out_buffer, "ok");
    } else {
      strcpy(out_buffer, "op failed");
    }
    return ret;
  }
  void init_order_funcs()
  {
    order_funcs[BFADD] = bfadd;
    order_funcs[BFCONTAINS] = bfcontains;
    order_funcs[BFCREATE] = bfcreate;
    order_funcs[BFDEL] = bfdel;
    order_funcs[TSET] = tset;
    order_funcs[TCONTAINS] = tcontains;
    order_funcs[TDEL] = tdel;
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

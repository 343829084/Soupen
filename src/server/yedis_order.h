#ifndef YEDIS_ORDER_H_
#define YEDIS_ORDER_H_
#include "../base/yedis_common.h"
#include "../base/yedis_memory.h"
#include "../server/yedis_db.h"
#define LEFT_SPILT '('
#define RIGHT_SPILT ')'
#define MAX_PARAM_NUMS 16
namespace yedis_server
{

  typedef YedisDSTypeNode<YedisBloomFilter> YedisBloomFilterDSNode;
  typedef YedisDSTypeNode<YedisTrie<YedisTrieNode> > YedisTrieDSNode;


  ////////////////////////////////////////////////////////////////////////////
  enum YedisDSType
  {
    LIST = 0,
    HASHMAP = 1,
    BLOOMFILTER  = 2,
    MAX_DS_TYPE = 8
  };

  enum YedisOrderType
  {
    HGET = 0,//hash map get
    HSET = 1,//hash map set
    MSET = 2,//trie get
    MGET = 3,//trie set
    GET = 4,//common get
    SET = 5,//common set
    BFADD = 6,//bloom filter add
    BFCONTAINS = 7,//bloom filter contains
    BFCREATE = 8,//create bloom filter
    BFDEL = 9,//del bloom filter
    TSET = 10,//create a trie and set
    TCONTAINS = 11,//find a string in a trie
    TDEL = 12,
    MAX_TYPE = 13
  };
  typedef int (*YedisOrderRoutine)(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums);
  struct YedisOrderTrieNode
  {
    YedisOrderTrieNode(bool is_case_sensitive)
    {
      int64_t size = 10 + 26 + 1;
      next_ = (YedisOrderTrieNode **)yedis_malloc(sizeof(YedisOrderTrieNode*) * size);
      for (int64_t i = 0; i < size; i++) {
        next_[i] = nullptr;
      }
      flag_ = false;
      routine_func = nullptr;
      order_type = MAX_TYPE;
    }
    YedisOrderRoutine routine_func;
    YedisOrderType order_type;
    bool flag_;
    YedisOrderTrieNode **next_;
  };

  void set_order_routine();
  void init_order_funcs();
  int parser_text(char *text, char *out_buffer);
  YedisOrderRoutine get_order_routine(char *order_name_start, char *order_name_end);



  ///////////////////////////////////////////////////////////////////////////////
  template<typename T>
  void free_yedis_ds_type_node(YedisDSTypeNode<T> *p)
  {
    yedis_free(p->val, sizeof(T));
    yedis_free(p->key, sizeof(YedisNormalString));
    yedis_free(p, sizeof(YedisDSTypeNode<T>));
  }

#define DECLARE_YEDIS_GET_DS_NODE(TYPE, member_field) \
  void yedis_ds_node_find(char *key, int key_length, TYPE* &out);

#define DEFINE_YEDIS_GET_DS_NODE(TYPE, member_field) \
  void yedis_ds_node_find(char *key, int key_length, TYPE* &out) \
  { \
    out = ydbe[dbi.yedis_current_db_id].member_field; \
    bool is_found = false; \
    while(nullptr != out) { \
      if(out->key->compare(key, key_length) == 0) { \
        is_found = true; \
        break; \
      } \
      out = out->next; \
    } \
  }

#define CREATE_BLOOM_FILTER_NODE(n , m) \
  YedisBloomFilterDSNode *bfnode = nullptr;\
  YedisBloomFilter *bf = nullptr;\
  YedisBloomFilterDSNode *buffer_bfnode = nullptr;\
  YedisNormalString *string = nullptr;\
  YedisBloomFilter *buffer_bf = nullptr;\
  YedisNormalString *buffer_string = static_cast<YedisNormalString*>(yedis_malloc(sizeof(YedisNormalString)));\
  if (YEDIS_UNLIKELY(nullptr == buffer_string)) {\
    ret = YEDIS_ERROR_NO_MEMORY;\
  } else {\
    string = new (buffer_string) YedisNormalString(params[0], param_lens[0]);\
    buffer_bf = static_cast<YedisBloomFilter*>(yedis_malloc(sizeof(YedisBloomFilter)));\
    if (YEDIS_UNLIKELY(nullptr == buffer_bf)) {\
      ret = YEDIS_ERROR_NO_MEMORY;\
      yedis_free(buffer_string, sizeof(YedisNormalString));\
    } else {\
      bf = new (buffer_bf) YedisBloomFilter(n, m);\
      buffer_bfnode = static_cast<YedisBloomFilterDSNode*>(yedis_malloc(sizeof(YedisBloomFilterDSNode)));\
      if (YEDIS_UNLIKELY(nullptr == buffer_bfnode)) {\
        ret = YEDIS_ERROR_NO_MEMORY;\
        yedis_free(buffer_string, sizeof(YedisNormalString));\
        yedis_free(buffer_bf, sizeof(YedisBloomFilter));\
      } else {\
        bfnode = new (buffer_bfnode) YedisBloomFilterDSNode();\
      }\
    }\
  }
#define CREATE_YEDIS_TRIE_NODE(is_case_sensitive) \
  YedisTrieDSNode *trienode = nullptr;\
  YedisTrie<YedisTrieNode> *trie = nullptr;\
  YedisTrieDSNode *buffer_trienode = nullptr;\
  YedisNormalString *string = nullptr;\
  YedisTrie<YedisTrieNode> *buffer_trie = nullptr;\
  YedisNormalString *buffer_string = static_cast<YedisNormalString*>(yedis_malloc(sizeof(YedisNormalString)));\
  if (YEDIS_UNLIKELY(nullptr == buffer_string)) {\
    ret = YEDIS_ERROR_NO_MEMORY;\
  } else {\
    string = new (buffer_string) YedisNormalString(params[0], param_lens[0]);\
    buffer_trie = static_cast<YedisTrie<YedisTrieNode>*>(yedis_malloc(sizeof(YedisTrie<YedisTrieNode>)));\
    if (YEDIS_UNLIKELY(nullptr == buffer_trie)) {\
      ret = YEDIS_ERROR_NO_MEMORY;\
      yedis_free(buffer_string, sizeof(YedisNormalString));\
    } else {\
      trie = new (buffer_trie) YedisTrie<YedisTrieNode>(is_case_sensitive);\
      buffer_trienode = static_cast<YedisTrieDSNode*>(yedis_malloc(sizeof(YedisTrieDSNode)));\
      if (YEDIS_UNLIKELY(nullptr == buffer_trienode)) {\
        ret = YEDIS_ERROR_NO_MEMORY;\
        yedis_free(buffer_string, sizeof(YedisNormalString));\
        yedis_free(buffer_trienode, sizeof(YedisTrie<YedisTrieNode>));\
      } else {\
        trienode = new (buffer_trienode) YedisTrieDSNode();\
      }\
    }\
  }
#define DECLARE_YEDIS_DEL_DS_NODE(TYPE, member_field) \
  void yedis_ds_node_del(TYPE *curr);
#define DEFINE_YEDIS_DEL_DS_NODE(TYPE, member_field) \
  void yedis_ds_node_del(TYPE *curr) \
  { \
    TYPE *prev = ydbe[dbi.yedis_current_db_id].member_field; \
    while(prev != nullptr) { \
      if (prev->next == curr) { \
        break; \
      } else {  \
        prev = prev->next; \
      } \
    } \
    if (prev != nullptr) { \
      prev->next = curr->next; \
    } else { \
      ydbe[dbi.yedis_current_db_id].member_field = curr->next; \
    } \
    curr->~TYPE(); \
    free_yedis_ds_type_node(curr); \
  }

#define DECLARE_YEDIS_INSERT_DS_NODE(TYPE, RAW_TYPE, member_field) \
  void yedis_ds_node_insert(TYPE *p, RAW_TYPE *q, YedisNormalString *string);
#define DEFINE_YEDIS_INSERT_DS_NODE(TYPE, RAW_TYPE, member_field) \
  void yedis_ds_node_insert(TYPE *p, RAW_TYPE *q, YedisNormalString *string) \
  {         \
    p->val = q; \
    p->key = string; \
    p->next = ydbe[dbi.yedis_current_db_id].member_field; \
    ydbe[dbi.yedis_current_db_id].member_field = p; \
  }



  int bfadd(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums);
  int bfcontains(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums);
  int bfcreate(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums);
  int bfdel(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums);

  int tset(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums);
  int tcontains(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums);
  int tdel(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums);


  DECLARE_YEDIS_GET_DS_NODE(YedisBloomFilterDSNode, bf)
  DECLARE_YEDIS_DEL_DS_NODE(YedisBloomFilterDSNode, bf)
  DECLARE_YEDIS_INSERT_DS_NODE(YedisBloomFilterDSNode, YedisBloomFilter, bf)
  DECLARE_YEDIS_GET_DS_NODE(YedisTrieDSNode, tn)
  DECLARE_YEDIS_DEL_DS_NODE(YedisTrieDSNode, tn)
  DECLARE_YEDIS_INSERT_DS_NODE(YedisTrieDSNode, YedisTrie<YedisTrieNode>, tn)

}

#endif /* YEDIS_ORDER_H_ */

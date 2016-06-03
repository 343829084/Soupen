#ifndef SOUPEN_ORDER_H_
#define SOUPEN_ORDER_H_
#include "../server/soupen_db.h"
#define LEFT_SPILT '('
#define RIGHT_SPILT ')'
#define MAX_PARAM_NUMS 16
namespace soupen_server
{

  //define SoupenDataStructureNode
  typedef SoupenDSTypeNode<SoupenBloomFilter> SoupenBloomFilterDSNode;
  typedef SoupenDSTypeNode<SoupenTrie<SoupenTrieNode> > SoupenTrieDSNode;


  ////////////////////////////////////////////////////////////////////////////
  enum SoupenDSType
  {
    LIST = 0,
    HASHMAP = 1,
    BLOOMFILTER  = 2,
    MAX_DS_TYPE = 8
  };

  enum SoupenOrderType
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
    TDEL = 12,//delete a trie
    SELECT = 13,//select another db
    FLUSHDB = 14,//flushdb db
    MAX_TYPE = 15
  };

  typedef int (*SoupenOrderRoutine)(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums);

  struct SoupenOrderTrieNode
  {
    int init(bool is_case_sensitive)
    {
      int ret = SOUPEN_SUCCESS;
      int64_t size = 10 + 26 + 1;
      next_ = (SoupenOrderTrieNode **)soupen_malloc(sizeof(SoupenOrderTrieNode*) * size);
      if (SOUPEN_UNLIKELY(nullptr == next_)) {
        ret = SOUPEN_ERROR_NO_MEMORY;
      } else {
        for (int64_t i = 0; i < size; i++) {
          next_[i] = nullptr;
        }
        flag_ = false;
        routine_func = nullptr;
        order_type = MAX_TYPE;
      }
      return ret;
    }
    SoupenOrderRoutine routine_func;
    SoupenOrderType order_type;
    bool flag_;
    SoupenOrderTrieNode **next_;
  };

  int set_order_routine();
  int init_order_funcs();
  int parser_text(char *text, char *out_buffer);
  SoupenOrderRoutine get_order_routine(char *order_name_start, char *order_name_end);



  ///////////////////////////////////////////////////////////////////////////////
  template<typename T>
  void reclaim_soupen_ds_type_node(SoupenDSTypeNode<T> *p)
  {
    p->~SoupenDSTypeNode<T>();
    soupen_free(p->val, sizeof(T));
    soupen_free(p->key, sizeof(SoupenString));
    soupen_free(p, sizeof(SoupenDSTypeNode<T>));
  }

#define DECLARE_SOUPEN_GET_DS_NODE(TYPE, member_field) \
  void soupen_ds_node_find(char *key, int key_length, TYPE* &out);

#define DEFINE_SOUPEN_GET_DS_NODE(TYPE, member_field) \
  void soupen_ds_node_find(char *key, int key_length, TYPE* &out) \
  { \
    out = ydbe[SoupenServerInfoManager::get_db_id()].member_field; \
    bool is_found = false; \
    while(nullptr != out) { \
      if(out->key->is_equal(key, key_length)) { \
        is_found = true; \
        break; \
      } \
      out = out->next; \
    } \
  }

#define CREATE_BLOOM_FILTER_NODE(n , m) \
  SoupenBloomFilterDSNode *bfnode = nullptr;\
  SoupenBloomFilter *bf = nullptr;\
  SoupenString *string = nullptr;\
  string = static_cast<SoupenString*>(soupen_malloc(sizeof(SoupenString)));\
  if (SOUPEN_UNLIKELY(nullptr == string)) {\
    ret = SOUPEN_ERROR_NO_MEMORY;\
  } else if (SOUPEN_UNLIKELY(SOUPEN_SUCCESS != (ret = string->init(params[0], param_lens[0])))) { \
  } else {\
    bf = static_cast<SoupenBloomFilter*>(soupen_malloc(sizeof(SoupenBloomFilter)));\
    if (SOUPEN_UNLIKELY(nullptr == bf)) {\
      ret = SOUPEN_ERROR_NO_MEMORY;\
    } else if (SOUPEN_UNLIKELY(SOUPEN_SUCCESS != (ret = bf->init(n, m)))) { \
    } else {\
      bfnode = static_cast<SoupenBloomFilterDSNode*>(soupen_malloc(sizeof(SoupenBloomFilterDSNode)));\
      if (SOUPEN_UNLIKELY(nullptr == bfnode)) {\
        ret = SOUPEN_ERROR_NO_MEMORY;\
      } else if (SOUPEN_UNLIKELY(SOUPEN_SUCCESS != (ret = bfnode->init()))) { \
      }\
    }\
  }\
  if (SOUPEN_FAILED) { \
    soupen_reclaim(string);\
    soupen_reclaim(bf);\
    soupen_reclaim(bfnode);\
  }

#define CREATE_SOUPEN_TRIE_NODE(is_case_sensitive) \
  SoupenTrieDSNode *trienode = nullptr;\
  SoupenTrie<SoupenTrieNode> *trie = nullptr;\
  SoupenString *string = nullptr;\
  string = static_cast<SoupenString*>(soupen_malloc(sizeof(SoupenString)));\
  if (SOUPEN_UNLIKELY(nullptr == string)) {\
    ret = SOUPEN_ERROR_NO_MEMORY;\
  } else if (SOUPEN_UNLIKELY(SOUPEN_SUCCESS != (ret = string->init(params[0], param_lens[0])))) { \
  } else {\
    trie = static_cast<SoupenTrie<SoupenTrieNode>*>(soupen_malloc(sizeof(SoupenTrie<SoupenTrieNode>)));\
    if (SOUPEN_UNLIKELY(nullptr == trie)) {\
      ret = SOUPEN_ERROR_NO_MEMORY;\
    } else if (SOUPEN_UNLIKELY(SOUPEN_SUCCESS != (ret = trie->init(is_case_sensitive)))) { \
    } else {\
      trienode = static_cast<SoupenTrieDSNode*>(soupen_malloc(sizeof(SoupenTrieDSNode)));\
      if (SOUPEN_UNLIKELY(nullptr == trienode)) {\
        ret = SOUPEN_ERROR_NO_MEMORY;\
      } else if (SOUPEN_UNLIKELY(SOUPEN_SUCCESS != (ret = trienode->init()))) { \
      }\
    }\
  }\
  if (SOUPEN_FAILED) { \
    soupen_reclaim(string);\
    soupen_reclaim(trie);\
    soupen_reclaim(trienode);\
  }

#define DECLARE_SOUPEN_DEL_DS_NODE(TYPE, member_field) \
  void soupen_ds_node_del(TYPE *curr);
#define DEFINE_SOUPEN_DEL_DS_NODE(TYPE, member_field) \
  void soupen_ds_node_del(TYPE *curr) \
  { \
    TYPE *prev = ydbe[SoupenServerInfoManager::get_db_id()].member_field; \
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
      ydbe[SoupenServerInfoManager::get_db_id()].member_field = curr->next; \
    } \
    reclaim_soupen_ds_type_node(curr); \
  }

#define DECLARE_SOUPEN_INSERT_DS_NODE(TYPE, RAW_TYPE, member_field) \
  void soupen_ds_node_insert(TYPE *p, RAW_TYPE *q, SoupenString *string);
#define DEFINE_SOUPEN_INSERT_DS_NODE(TYPE, RAW_TYPE, member_field) \
  void soupen_ds_node_insert(TYPE *p, RAW_TYPE *q, SoupenString *string) \
  {         \
    p->val = q; \
    p->key = string; \
    p->next = ydbe[SoupenServerInfoManager::get_db_id()].member_field; \
    ydbe[SoupenServerInfoManager::get_db_id()].member_field = p; \
  }


  //standard bloom filter routine
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

  //trie routine
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

  int select(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums);
  int flushdb(char *out_buffer,
      char **params,
      int *param_lens,
      int param_nums);


  DECLARE_SOUPEN_GET_DS_NODE(SoupenBloomFilterDSNode, bf)
  DECLARE_SOUPEN_DEL_DS_NODE(SoupenBloomFilterDSNode, bf)
  DECLARE_SOUPEN_INSERT_DS_NODE(SoupenBloomFilterDSNode, SoupenBloomFilter, bf)
  DECLARE_SOUPEN_GET_DS_NODE(SoupenTrieDSNode, tn)
  DECLARE_SOUPEN_DEL_DS_NODE(SoupenTrieDSNode, tn)
  DECLARE_SOUPEN_INSERT_DS_NODE(SoupenTrieDSNode, SoupenTrie<SoupenTrieNode>, tn)

}

#endif /* SOUPEN_ORDER_H_ */

/*
 * Copyright (C) 2015, Leo Ma <begeekmyfriend@gmail.com>
 */

#ifndef _SKIPLIST_H
#define _SKIPLIST_H

#include <stdlib.h>
#include <string.h>
#define ZSKIPLIST_MAXLEVEL 32

#define ZSKIPLIST_P 0.25

struct sdshdr {
    long len;
    long free;
    char buf[];
};

typedef char *sds;

sds sdsnewlen(const void *init, size_t initlen) {
    struct sdshdr *sh;

    sh = (sdshdr *)malloc(sizeof(struct sdshdr)+initlen+1);
    if (sh == NULL) return NULL;
    sh->len = initlen;
    sh->free = 0;
    if (initlen) {
        if (init) memcpy(sh->buf, init, initlen);
        else memset(sh->buf,0,initlen);
    }
    sh->buf[initlen] = '\0';
    return (char*)sh->buf;
}

struct robj {
    unsigned type:4;
    unsigned notused:2;     /* Not used */
    unsigned encoding:4;
    unsigned lru:22;        /* lru time (relative to server.lruclock) */
    int refcount;
    void *ptr;
};

robj *createObject(int type, void *ptr)
{
    robj *o = (robj*)malloc(sizeof(*o));
    o->type = type;
    o->encoding = 1;
    o->ptr = ptr;
    o->refcount = 1;
    return o;
}

robj *createRawStringObject(const char *ptr, size_t len) {
    return createObject(1, sdsnewlen(ptr,len));
}

struct zskiplistNode;

struct zskiplistLevel
{
        zskiplistNode*forward;//后继
        unsigned int span;//该层跨越的节点数量
};

int compareStringObjects(robj *a, robj *b) {
  return strcasecmp((char*)a->ptr,(char*)b->ptr);
}

int equalStringObjects(robj *a, robj *b) {
  return strcasecmp((char*)a->ptr,(char*)b->ptr) == 0;
}

struct zskiplistNode {
    robj *obj; //节点数据
    double score;
    zskiplistNode*backward; //前驱
    zskiplistLevel level[0];
};

struct zskiplist {
    zskiplistNode*header, *tail;
    unsigned long length;//节点的数目
    int level;//目前表的最大层数
};

zskiplistNode *zslCreateNode(int level, double score, robj *obj) {
    zskiplistNode *zn = (zskiplistNode*)malloc(sizeof(*zn)+level*sizeof(struct zskiplistLevel));
    zn->score = score;
    zn->obj = obj;
    return zn;
}

zskiplist *zslCreate(void) {
    int j;
    zskiplist *zsl;

    zsl = (zskiplist *)malloc(sizeof(*zsl));
    zsl->level = 1;
    zsl->length = 0;
    zsl->header = zslCreateNode(ZSKIPLIST_MAXLEVEL,0,NULL);//ZSKIPLIST_MAXLEVEL = 32
    for (j = 0; j < ZSKIPLIST_MAXLEVEL; j++) {
        zsl->header->level[j].forward = NULL;//后继
        zsl->header->level[j].span = 0;
    }
    zsl->header->backward = NULL;//前驱
    zsl->tail = NULL;//尾指针
    return zsl;
}

int zslRandomLevel(void) {//为新的skiplist节点生成该节点level数目
    int level = 1;
    while ((random()&0xFFFF) < (ZSKIPLIST_P * 0xFFFF))//0.25
        level += 1;
    return (level<ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
}

zskiplistNode *zslInsert(zskiplist *zsl, double score, robj *obj) {
    zskiplistNode *update[ZSKIPLIST_MAXLEVEL], *x;
    unsigned int rank[ZSKIPLIST_MAXLEVEL];
    int i, level;

    x = zsl->header;//header不存储数据
    //从高向低
    for (i = zsl->level-1; i >= 0; i--) {
        /* store rank that is crossed to reach the insert position */
        //rank[i]用来记录第i层达到插入位置的所跨越的节点总数,也就是该层最接近(小于)给定score的排名
        //rank[i]初始化为上一层所跨越的节点总数
        rank[i] = i == (zsl->level-1) ? 0 : rank[i+1];
        //后继节点不为空，并且后继节点的score比给定的score小
        while (x->level[i].forward &&
            (x->level[i].forward->score < score ||
                //score相同，但节点的obj比给定的obj小
                (x->level[i].forward->score == score &&
                compareStringObjects(x->level[i].forward->obj,obj) < 0))) {
            rank[i] += x->level[i].span;//记录跨越了多少个节点
            x = x->level[i].forward;//继续向右走
        }
        update[i] = x;//保存访问的节点，并且将当前x移动到下一层
    }
    /* we assume the key is not already inside, since we allow duplicated
     * scores, and the re-insertion of score and redis object should never
     * happen since the caller of zslInsert() should test in the hash table
     * if the element is already inside or not. */
    level = zslRandomLevel();//计算新的level
    if (level > zsl->level) {//新的level > zsl->level，需要进行升级
        for (i = zsl->level; i < level; i++) {
            rank[i] = 0;
            update[i] = zsl->header;//需要更新的节点就是header
            update[i]->level[i].span = zsl->length;
            //在未添加新节点之前，需要更新的节点跨越的节点数目自然就是zsl->length，
        }
        zsl->level = level;
    }
    x = zslCreateNode(level,score,obj);//建立新节点
    //开始插入节点
    for (i = 0; i < level; i++) {
        //新节点的后继就是插入位置节点的后继
        x->level[i].forward = update[i]->level[i].forward;
        //插入位置节点的后继就是新节点
        update[i]->level[i].forward = x;

        /* update span covered by update[i] as x is inserted here */
        /**
            rank[i]: 在第i层，update[i]->score的排名
            rank[0] - rank[i]: update[0]->score与update[i]->score之间间隔了几个数，即span数目
            对于update[i]->level[i].span值的更新由于在update[i]与update[i]->level[i]->forward之间又添加了x，
            update[i]->level[i].span = 从update[i]到x的span数目，
            由于update[0]后面肯定是新添加的x，所以自然新的update[i]->level[i].span = (rank[0] - rank[i]) + 1;
            x->level[i].span = 从x到update[i]->forword的span数目，
            原来的update[i]->level[i].span = 从update[i]到update[i]->level[i]->forward的span数目
            所以x->level[i].span = 原来的update[i]->level[i].span - (rank[0] - rank[i]);

            另外需要注意当level > zsl->level时，update[i] = zsl->header的span处理
        */
        x->level[i].span = update[i]->level[i].span - (rank[0] - rank[i]);
        update[i]->level[i].span = (rank[0] - rank[i]) + 1;
    }

    /* increment span for untouched levels */
    //如果新节点的level小于原来skiplist的level，那么在上层没有insert新节点的span需要加1
    for (i = level; i < zsl->level; i++) {
        update[i]->level[i].span++;
    }

    x->backward = (update[0] == zsl->header) ? NULL : update[0];//前驱指针
    if (x->level[0].forward)
        x->level[0].forward->backward = x;
    else
        zsl->tail = x;
    zsl->length++;
    return x;
}

unsigned long zslGetRank(zskiplist *zsl, double score, robj *o) {
    zskiplistNode *x;
    unsigned long rank = 0;
    int i;

    x = zsl->header;
    for (i = zsl->level-1; i >= 0; i--) {
        while (x->level[i].forward &&
            (x->level[i].forward->score < score ||
                (x->level[i].forward->score == score &&
                compareStringObjects(x->level[i].forward->obj,o) <= 0))) {
            rank += x->level[i].span;//排名，加上该层跨越的节点数目
            x = x->level[i].forward;
        }

        /* x might be equal to zsl->header, so test if obj is non-NULL */
        if (x->obj && equalStringObjects(x->obj,o)) {// 找到目标元素
            return rank;
        }
    }
    return 0;
}

#endif  /* _SKIPLIST_H */

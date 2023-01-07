/**
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <string.h>
#include "c_map.h"

#define FuncAlloc malloc
#define FuncReAlloc realloc
#define FuncFree free
static void* c_map_realloc(void* ptr, size_t size)
{
 
    void* new_ptr = FuncReAlloc(ptr, size);
    if (new_ptr == NULL)
    {
        new_ptr = FuncAlloc(size);
        if(new_ptr)
            FuncFree(ptr);
    }
    return new_ptr;
}
struct map_node_t {
    unsigned hash;
    void* value;
    map_node_t* next;
};

static unsigned map_hash(const CStrView* key) {
    unsigned hash = 5381;
    const char* str = key->c_str;

    for (int i = 0; i < key->len; i++)
    {
        hash = ((hash << 5) + hash) ^ *str++;
    }
    return hash;
}


static map_node_t* map_newnode(const CStrView* key, void* value, int vsize) {
    map_node_t* node;
    int ksize = key->len + sizeof(size_t) + 1;
    int k_mod = ksize % sizeof(void*);
    int voffset = ksize + (k_mod == 0 ? 0 : sizeof(void*) - k_mod);
    //int voffset = ksize + ((sizeof(void*) - ksize) % sizeof(void*));
    node = FuncAlloc(sizeof(*node) + voffset + vsize);
    if (!node) return NULL;

    char* p_cpy = (char*)(node + 1);
    memcpy(p_cpy, &key->len, sizeof(size_t));
    p_cpy += sizeof(size_t);
    memcpy(p_cpy, key->c_str, key->len);
    p_cpy[key->len] = 0;
    node->hash = map_hash(key);
    node->value = ((char*)(node + 1)) + voffset;
    memcpy(node->value, value, vsize);
    return node;
}


static int map_bucketidx(const map_base_t* m, unsigned hash) {
    /* If the implementation is changed to allow a non-power-of-2 bucket count,
     * the line below should be changed to use mod instead of AND */
    return hash & (m->nbuckets - 1);
}


static void map_addnode(map_base_t* m, map_node_t* node) {
    int n = map_bucketidx(m, node->hash);
    node->next = m->buckets[n];
    m->buckets[n] = node;
}


static int map_resize(map_base_t* m, int nbuckets) {
    map_node_t* nodes, * node, * next;
    map_node_t** buckets;
    int i = m->nbuckets;
    /* Chain all nodes together */
    nodes = NULL;
    while (i--) {
        node = (m->buckets)[i];
        while (node) {
            next = node->next;
            node->next = nodes;
            nodes = node;
            node = next;
        }
    }
    /* Reset buckets */
    buckets = c_map_realloc(m->buckets, sizeof(*m->buckets) * nbuckets);
    if (buckets != NULL) {
        m->buckets = buckets;
        m->nbuckets = nbuckets;
    }
    if (m->buckets) {
        memset(m->buckets, 0, sizeof(*m->buckets) * m->nbuckets);
        /* Re-add nodes to buckets */
        node = nodes;
        while (node) {
            next = node->next;
            map_addnode(m, node);
            node = next;
        }
    }
    /* Return error code if realloc() failed */
    return (buckets == NULL) ? -1 : 0;
}


static map_node_t** map_getref(const map_base_t* m, const CStrView* key)
{
    unsigned hash = map_hash(key);
    map_node_t** next;
    if (m->nbuckets > 0) {
        next = &m->buckets[map_bucketidx(m, hash)];
        while (*next) {
            if ((*next)->hash == hash)
            {
                size_t str_len = *(size_t*)(*next + 1);
                char* str = (char*)(*next + 1) + sizeof(size_t);
                if (str_len == key->len && !memcmp(str, key->c_str, key->len))
                {
                    return next;
                }
            }

            next = &(*next)->next;
        }
    }
    return NULL;
}


void map_deinit_(map_base_t* m) {
    map_node_t* next, * node;
    int i;
    i = m->nbuckets;
    while (i--) {
        node = m->buckets[i];
        while (node) {
            next = node->next;
            FuncFree(node);
            node = next;
        }
    }
    FuncFree(m->buckets);
}


void* map_get_(const map_base_t* m, CStrView key) {
    map_node_t** next = map_getref(m, &key);
    return next ? (*next)->value : NULL;
}


int map_set_(map_base_t* m, CStrView key, const void* value, int vsize) {
    map_node_t** next, * node;
    /* Find & replace existing node */
    next = map_getref(m, &key);
    if (next) {
        memcpy((*next)->value, value, vsize);
        return 0;
    }
    /* Add new node */
    node = map_newnode(&key, value, vsize);
    if (node == NULL) goto fail;
    if (m->nnodes >= m->nbuckets) 
    {
        int n = (m->nbuckets > 0) ? (m->nbuckets << 1) : 16;
        map_resize(m, n); // 若本次resize 失败则map_addnode 仍会正常插入剩余空间或链表, 若一直resize 失败则会退化为链表
        if (m->nbuckets == 0)
        {
            goto fail;
        }
    }
    map_addnode(m, node);
    m->nnodes++;
    return 0;
fail:
    if (node) FuncFree(node);
    return -1;
}

int map_remove_(map_base_t* m, CStrView key)
{
    map_node_t* node;
    map_node_t** next = map_getref(m, &key);
    if (next) {
        node = *next;
        *next = (*next)->next;
        FuncFree(node);
        m->nnodes--;
        return 0;
    }
    else
    {
        return -1;
    }
}
map_iter_t map_erase_(map_base_t* m, map_iter_t iter)
{
    if (iter.second == NULL)
    {
        return iter;
    }
    map_iter_t ret_iter = map_next_(m, iter);
    if (ret_iter.next != &m->buckets[ret_iter.bucketidx])
    {
        ret_iter.next = iter.next;
    }
    map_node_t* node = *iter.next;
    *iter.next = (*iter.next)->next;
    FuncFree(node);
    m->nnodes--;
    return ret_iter;
}


map_iter_t map_begin_(const map_base_t* m) {
    map_iter_t iter;
    iter.bucketidx = 0;
    iter.next = NULL;
    iter.first.c_str = NULL;
    iter.first.len = 0;
    iter.second = NULL;
    return map_next_(m, iter);
}

map_iter_t map_next_(const map_base_t* m, map_iter_t iter) {


    if (iter.next == NULL)
    {
        if (iter.bucketidx >= m->nbuckets) {
            iter.second = NULL;
            return iter;
        }
        iter.next = &m->buckets[iter.bucketidx];
    }
    else if (iter.second == NULL)
    {
        return iter;
    }
    else
    {
        iter.next = &(*iter.next)->next;
    }
    while (*iter.next == NULL)
    {
        if (++iter.bucketidx >= m->nbuckets) {
            iter.second = NULL;
            return iter;
        }
        iter.next = &m->buckets[iter.bucketidx];
    }
    iter.first.len = *(size_t*)(*iter.next + 1);
    iter.first.c_str = (char*)(*iter.next + 1) + sizeof(size_t);
    iter.second = (*iter.next)->value;
    return iter;
}
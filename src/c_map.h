/**
 * Copyright (c) 2014 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See LICENSE for details.
 */

#pragma once

#include <string.h>
#include <stdint.h>

#include "c_str_view.h"

#define MAP_VERSION "0.1.0"

struct map_node_t;



typedef struct map_node_t map_node_t;

typedef struct {
    map_node_t** buckets;
    unsigned nbuckets, nnodes;
} map_base_t;

typedef struct {
    int32_t bucketidx;
    map_node_t** next;
    CStrView first;
    void* second;
} map_iter_t;


#define map_t(T)\
  struct { map_base_t base; T *ref; T tmp; }


#define map_init(m)\
  memset(m, 0, sizeof(*(m)))


#define map_deinit(m)\
  map_deinit_(&(m)->base)


#define map_get(m, key)\
    map_get_(&(m)->base, key)


#define map_set(m, key, value)\
  ( (m)->tmp = (value),\
    map_set_(&(m)->base, key, &(m)->tmp, sizeof((m)->tmp)) )


#define map_remove(m, key)\
  map_remove_(&(m)->base, key)

#define map_erase(m, iter)\
  map_erase_(&(m)->base, iter)

#define map_begin(m)\
  map_begin_(&(m)->base)

#define map_end(m, iter) ((iter).second == NULL)

#define map_next(m, iter)\
  map_next_(&(m)->base, iter)

#define map_empty(m)\
  ((m)->base.nnodes == 0)

#define map_iter_first(m, iter) \
    (iter.first)
 
#define map_iter_second(m, iter) \
    ( (m)->ref = iter.second )

void map_deinit_(map_base_t* m);

void* map_get_(const map_base_t* m, CStrView key);

int map_set_(map_base_t* m, CStrView key, const void* value, int vsize);

int map_remove_(map_base_t* m, CStrView key);

map_iter_t map_erase_(map_base_t* m, map_iter_t iter);

map_iter_t map_begin_(const map_base_t* m);

map_iter_t map_next_(const map_base_t* m, map_iter_t iter);


typedef map_t(void*) map_void_t;
typedef map_t(char*) map_str_t;
typedef map_t(int) map_int_t;
typedef map_t(char) map_char_t;
typedef map_t(float) map_float_t;
typedef map_t(double) map_double_t;

# map
a  hash map implementation for C, based at https://github.com/rxi/map.git

## Installation 
The [map.c](src/c_map.c?raw=1) and [map.h](src/c_map.h?raw=1) files can be dropped
into an existing C project and compiled along with it.



## Usage
```c
typedef struct
{
    size_t len;
    char* c_str;
}CStrView;
```
The `key` parameter should always be a `CStrView` value, so it can be any type. 
Implementation iterator similar to C++. all api is not thread safe.

Before using a map it should first be initialised using the `map_init()`
function.
```c
map_int_t m;
map_init(&m);
```

Values can added to a map using the `map_set()` function.
```c
map_set(&m, c_str_view("testkey"), 123);
```

To retrieve a value from a map, the `map_get()` function can be used.
`map_get()` will return a pointer to the key's value, or `NULL` if no mapping
for that key exists.
```c
int *val = map_get(&m, c_str_view("testkey"));
if (val) {
  printf("value: %d\n", *val);
} else {
  printf("value not found\n");
}
```

When you are done with a map the `map_deinit()` function should be called on
it. This will free any memory the map allocated during use.
```c
map_deinit(&m);
```


## Types
map.h provides the following predefined map types:

Contained Type  | Type name
----------------|----------------------------------
void*           | map_void_t
char*           | map_str_t
int             | map_int_t
char            | map_char_t
float           | map_float_t
double          | map_double_t

To define a new map type the `map_t()` macro should be used:
```c
/* Creates the type uint_map_t for storing unsigned ints */
typedef map_t(unsigned int) uint_map_t;
```

## Functions
All map functions are macro functions. The parameter `m` in each function
should be a pointer to the map struct which the operation is to be performed
on. The `key` parameter should always be a `CStrView` value.



### map\_t(T)
Creates a map struct for containing values of type `T`.
```c
/* Typedefs the struct `fp_map_t` as a container for type FILE* */
typedef map_t(FILE*) fp_map_t;
```

### map\_init(m)
Initialises the map, this must be called before the map can be used. 

### map\_deinit(m)
Deinitialises the map, freeing the memory the map allocated during use;
this should be called when we're finished with a map.

### map\_get(m, key)
Returns a pointer to the value of the given `key`. If no mapping for the `key`
exists then `NULL` will be returned.

### map\_set(m, key, value)
Sets the given `key` to the given `value`. Returns `0` on success, otherwise
`-1` is returned and the map remains unchanged.

### map\_remove(m, key)
Removes the mapping of the given `key` from the map. If the `key` does not
exist in the map then the function has no effect.

### map_begin(m)
Returns a `map_iter_t` which can be used with `map_next()` to iterate all the
keys in the map.

### map\_next(m, iter)
Uses the `map_iter_t` returned by `map_begin()` to iterate all the keys in the
map. `map_next()` returns a `map_iter_t` with each call.

### map\_end(m, iter)
When the `map_iter_t` returned by `map_begin()` or `map_next()` is not available ,
`map_end()` returns true else returns false.
### map\_empty(m)
When the map is empty , returns true.

### map\_iter\_first(m, iter)
Get key from `map_iter_t`.

### map\_iter\_second(m, iter)
Get pointer of value from `map_iter_t`.

### map\_erase(m, iter)
Removes the mapping of the given iter from the map. the next iter is returned.
```c
for (map_iter_t iter = map_begin(&m); !map_end(&m, iter); )
{
    int val = *map_iter_second(&m, iter);
    if (val % 3 == 2)
    {
        iter = map_erase(&m, iter);
    }
    else
    {
        iter = map_next(&m, iter);
    }
}
```

## License
This library is free software; you can redistribute it and/or modify it under
the terms of the MIT license. See [LICENSE](LICENSE) for details.

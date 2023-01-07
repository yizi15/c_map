#pragma once

typedef struct
{
    size_t len;
    char* c_str;
}CStrView;

static inline CStrView c_str_view(const char* str)
{
    CStrView ret = { .c_str = (char*)str, .len = strlen(str) };
    return ret;
}

static inline CStrView c_str_view_arr(void* str, size_t str_len)
{
    CStrView ret = { .c_str = str, .len = str_len };
    return ret;
}

#include<stdio.h>
#include<stdint.h>
#include<stdbool.h>
#include <math.h>
#include "c_map.h"
// Used to measure intervals and absolute times
typedef int64_t msec_t;

// Get current time in milliseconds from the Epoch (Unix)
// or the time the system started (Windows).
msec_t time_ms(void);

#if defined(_WIN32)

#include <windows.h>

static inline msec_t get_cur_time_ms()
{
   /* SYSTEMTIME tv_now;
    GetSystemTime(&tv_now);
    return  (msec_t)tv_now.wMilliseconds;*/
    FILETIME f;
    GetSystemTimeAsFileTime(&f);
    (long long)f.dwHighDateTime;
    __int64 nano = ((__int64)f.dwHighDateTime << 32LL) + (__int64)f.dwLowDateTime;
    return (nano - 116444736000000000LL) / 10000;
}
#else

#include <sys/time.h>

msec_t get_ime_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (msec_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

#endif

CStrView int_str(int val)
{
    static char buf[100];
    static CStrView str = { .c_str = buf, .len = 10 };
    str.len = 0;
    for (int i = 0; val > 0; i++)
    {
        str.c_str[i] = (val & 0xF) + 'a';
        str.len++;
        val /= 10;
    }
    return str;
}
typedef struct
{
    char buf[64];
}T_Test;

int main()
{   
    map_t(int) m;
    T_Test test_val = {.buf = {0}};
    map_init(&m);
    int64_t start_time = get_cur_time_ms();

    for (int i = 0; i < 1000; i++)
    {
        map_set(&m, int_str(i), i);
        if (i % 10 == 1)
        {
            map_remove(&m, int_str(i));
        }
    }
    for (int i = 0; i < 1000; i++)
    {
        if (i % 3 == 1)
        {
            map_remove(&m, int_str(i));
        }
    }
    int i = 0;
    for (map_iter_t iter = map_begin(&m); !map_end(&m, iter); )
    {
        i++;
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
    for (map_iter_t iter = map_begin(&m); !map_end(&m, iter); iter = map_next(&m, iter))
    {
        i++;
        int val = *map_iter_second(&m, iter);
        printf("VAL: %d, key_len %d\n", val, map_iter_first(&m, iter).len);

    }
    int64_t use_time = get_cur_time_ms() - start_time;

    printf("map size %d %d %lld %d", m.base.nnodes,m.base.nbuckets ,use_time, i);
}
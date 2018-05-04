#pragma once
#include <assert.h>

#define UDP_BUF_MAX_SIZE (1000)

typedef unsigned long long unique_id_t;
typedef unique_id_t unid_t;

#define ASSERT(x) \
    assert((x))

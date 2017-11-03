#ifndef PLEUNE_H
#define PLEUNE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MODULO(a, b) (((a) % (b) + (b)) % (b))

#ifdef __cplusplus
extern "C" {
#endif

#include "mpool_static.h"
#include "mpool_grow.h"
#include "mpool_dynamic.h"
#include "ntorus.h"
#include "plog.h"
#include "ringbuff.h"
#include "stack.h"
#include "tpool.h"
#include "voxtree.h"

#ifdef __cplusplus
}
#endif

#endif

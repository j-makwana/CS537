#ifndef _MUTEX_H_
#define _MUTEX_H_

#include "types.h"
#include "spinlock.h"

typedef struct {
    uint locked;
    struct spinlock lk;
    struct proc *holder;
} mutex;

#endif // _MUTEX_H_

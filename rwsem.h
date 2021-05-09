/*
** pem 2021-05-08
**
** Read/write locks using unnamed POSIX semaphores.
**
*/

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct rwsem_s rwsem_t;

size_t rwsem_size(void);
bool rwsem_init(rwsem_t *rwsp);
bool rwsem_destroy(rwsem_t *rwsp);
bool rwsem_writelock(rwsem_t *rwsp);
bool rwsem_writeunlock(rwsem_t *rwsp);
bool rwsem_readlock(rwsem_t *rwsp);
bool rwsem_readunlock(rwsem_t *rwsp);
void rwsem_status(rwsem_t *rwsp, int *mutexp, int *writep, uint64_t *readersp);

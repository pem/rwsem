/*
** pem 2021-05-08
**
** Read/write locks using unnamed POSIX semaphores.
**
*/

#include <stdlib.h>
#include "rwsem.h"

void
rwsem_status(rwsem_t *rwsp, int *mutexp, int *writep, uint64_t *readersp)
{
    sem_getvalue(&rwsp->mutex, mutexp);
    sem_getvalue(&rwsp->write, writep);
    *readersp = rwsp->readers;
}

bool
rwsem_init(rwsem_t *rwsp)
{
    if (sem_init(&rwsp->write, 1, 1) < 0)
        return false;
    if (sem_init(&rwsp->mutex, 1, 1) < 0)
    {
        sem_destroy(&rwsp->write);
        return false;
    }
    rwsp->readers = 0;
    return true;
}

bool
rwsem_destroy(rwsem_t *rwsp)
{
    if (sem_trywait(&rwsp->write) < 0)
        return false;
    if (sem_trywait(&rwsp->mutex) < 0)
        return false;
    sem_destroy(&rwsp->write);
    sem_destroy(&rwsp->mutex);
    return true;
}

bool
rwsem_writelock(rwsem_t *rwsp)
{
    if (sem_wait(&rwsp->write) < 0)
        return false;
    return true;
}

bool
rwsem_writeunlock(rwsem_t *rwsp)
{
    if (sem_post(&rwsp->write) < 0)
        return false;
    return true;
}

bool
rwsem_readlock(rwsem_t *rwsp)
{
    if (sem_wait(&rwsp->mutex) < 0) /* Get mutex */
        return false;
    rwsp->readers += 1;
    if (rwsp->readers == 1 && sem_wait(&rwsp->write) < 0)
    {
        (void)sem_post(&rwsp->mutex); /* Reset mutex */
        return false;
    }
    if (sem_post(&rwsp->mutex) < 0) /* Release mutex */
        return false;
    return true;
}

bool
rwsem_readunlock(rwsem_t *rwsp)
{
    if (sem_wait(&rwsp->mutex) < 0) /* Get mutex */
        return false;
    rwsp->readers -= 1;
    if (rwsp->readers == 0 && sem_post(&rwsp->write) < 0)
    {
        (void)sem_post(&rwsp->mutex); /* Reset mutex */
        return false;
    }
    if (sem_post(&rwsp->mutex) < 0) /* Release mutex */
        return false;
    return true;
}

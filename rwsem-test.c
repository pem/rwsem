/*
** pem 2021-05-08
**
** Test program for librwsem, read/write locks using unnamed POSIX semaphores.
**
** Call with one or more digits as arguments. It will loop once for each digit,
** and acquire a read lock if it's an odd numer, a write lock if it's an even
** number, and sleep that many seconds while holding the lock.
**
** For example:
**   ./rwsem-test 1 2 3 4
** will hold a read lock for one second, then a write lock for 2 seconds, and
** so on. It will sleep 1 second before each locking attempt.
**
** It will always take a write lock at the start and at the end, updating a
** counter of the number of processes using the locks. The last process remaining
** will destroy the locks.
**
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "rwsem.h"

static void
perr(char *f)
{
    fprintf(stderr, "%s failed: %s: \n", f, strerror(errno));
}

static void
perrex(char *f)
{
    perr(f);
    exit(1);
}

int
main(int argc, char **argv)
{
    size_t mapsize = sizeof(uint64_t) + rwsem_size();

    int fd = shm_open("rwsem-test", O_CREAT | O_RDWR, S_IRWXU);
    if (fd < 0)
        perrex("shm_open");
    if (ftruncate(fd, mapsize))
        perrex("ftruncate");

    void *map = mmap(0, mapsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED)
    {
        close(fd);
        perrex("mmap");
    }
    close(fd);
    uint64_t *users = (uint64_t *)map;
    rwsem_t *rws = (rwsem_t *)(((char *)map) + sizeof(uint64_t));

    if (! rwsem_init(rws))
        perrex("rwsem_init");

    int m, w;
    uint64_t  r;
    rwsem_status(rws, &m, &w, &r);
    printf("Intialized: %d %d %lu\n", m, w, (unsigned long)r);

    /* Signing in... */
    if (! rwsem_writelock(rws))
        perrex("rwsem_writelock");
    *users += 1;
    if (! rwsem_writeunlock(rws))
        perrex("rwsem_writeunlock");

    for (int i = 1 ; i < argc ; i++)
    {
        unsigned s = strtoul(argv[i], NULL, 10);

        sleep(1);
        if (s & 1)
        {
            printf("--> Read LOCK...\n");
            if (! rwsem_readlock(rws))
            {
                perr("rwsem_readlock");
                break;
            }
            rwsem_status(rws, &m, &w, &r);
            printf("  ### Reading: m=%d w=%d r=%lu u=%lu\n",
                   m, w, (unsigned long)r, (unsigned long)(*users));
            sleep(s);
            printf("<-- Read UNLOCK\n");
            if (! rwsem_readunlock(rws))
            {
                perr("rwsem_readunlock");
                break;
            }
        }
        else
        {
            printf("--> Write LOCK...\n");
            if (! rwsem_writelock(rws))
            {
                perr("rwsem_writelock");
                break;
            }
            rwsem_status(rws, &m, &w, &r);
            printf("  ### Writing: m=%d w=%d r=%lu u=%lu\n",
                   m, w, (unsigned long)r, (unsigned long)(*users));
            sleep(s);
            printf("<-- Write UNLOCK\n");
            if (! rwsem_writeunlock(rws))
            {
                perr("rwsem_writeunlock");
                break;
            }
        }
    }

    rwsem_status(rws, &m, &w, &r);
    printf("Done: %d %d %lu\n", m, w, (unsigned long)r);

    bool destroy = false;
    /* Signing out... */
    if (! rwsem_writelock(rws))
        perrex("rwsem_writelock");
    *users -= 1;
    if (*users == 0)
        destroy = true;
    if (! rwsem_writeunlock(rws))
        perrex("rwsem_writeunlock");

    if (destroy)
    {
        printf("Destroying...\n");
        if (! rwsem_destroy(rws))
            perrex("rwsem_destroy");
    }

    munmap(rws, mapsize);

    if (destroy)
    {
        printf("Unlinking...\n");
        if (shm_unlink("rwsem-test") < 0)
            perrex("shm_unlink");
    }

    exit(0);
}

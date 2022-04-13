#include <unistd.h>
#include <stdio.h>

struct eqstr
{
    bool operator()(long s1, long s2) const
    {
        return (s1 == s2);
    }
};

struct ProcMemory
{
    long size;     // total program size
    long resident; // resident set size
    long share;    // shared pages
    long trs;      // text (code)
    long lrs;      // library
    long drs;      // data/stack
    long dt;       // dirty pages
};

static long read_proc_memory()
{
    ProcMemory m;
    FILE *fp = NULL;
    fp = fopen("/proc/self/statm", "r");
    if (NULL == fp)
    {
        return -1;
    }
    if (fscanf(fp, "%ld %ld %ld %ld %ld %ld %ld",
               &m.size, &m.resident, &m.share,
               &m.trs, &m.lrs, &m.drs, &m.dt) != 7)
    {

        return -1;
    }
    return m.resident * getpagesize();
}
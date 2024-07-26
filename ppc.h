#ifndef PPC_H
#define PPC_H

#include <stdint.h>

struct ppc_state {
    uint32_t cr;
    uint64_t lr;
    uint64_t ctr;
    uint64_t g[32];
    uint64_t xer;
};

#endif
#ifndef PPC_DIS_H
#define PPC_DIS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

struct ppc_dis_state {
    uint32_t pc;
    int print_address;
    int print_opcode;
};

char* ppc_disassemble(struct ppc_dis_state* state, char* buf, uint32_t opcode);

#ifdef __cplusplus
}
#endif

#endif
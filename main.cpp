extern "C" {
#include "ppc.h"
}

#include "elf.hpp"
#include "mm.hpp"

int main(int argc, const char* argv[]) {
    allity::memory_manager mm;

    mm.load(argv[1]);

    struct ppc_bus bus;

    bus.read8 = allity::mm_read8;
    bus.read16 = allity::mm_read16;
    bus.read32 = allity::mm_read32;
    bus.read64 = allity::mm_read64;
    bus.write8 = allity::mm_write8;
    bus.write16 = allity::mm_write16;
    bus.write32 = allity::mm_write32;
    bus.write64 = allity::mm_write64;
    bus.udata = &mm;

    struct ppc_state* ppu = ppc_create(bus);

    ppu->pc = mm.read(mm.file().entry_point(), allity::MM_64);

    mm.alloc(0xd0000000, 0x10000000);

    ppu->r[1] = 0xdf000000;
    ppu->r[2] = mm.read(mm.file().entry_point() + 8, allity::MM_64);

    while (true)
        ppc_execute(ppu);

    ppc_destroy(ppu);
}
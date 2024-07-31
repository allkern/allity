#include "elf.hpp"

namespace allity {

enum : int {
    MM_8,
    MM_16,
    MM_32,
    MM_64
};

struct memory_block {
    void* buf;
    uint64_t attributes;
    uint64_t start;
    uint64_t end;
};

class memory_manager {
    elf::file m_file;
    std::vector <memory_block> m_blocks;

public:
    memory_block* alloc(uint64_t start, uint64_t size);
    memory_block* alloc(uint64_t size);
    void free(uint64_t ptr);
    uint64_t read(uint32_t addr, int size);
    void write(uint32_t addr, uint64_t data, int size);
    bool load(const char* path);
    const elf::file& file() const;
};

uint8_t mm_read8(void* udata, uint32_t addr);
uint16_t mm_read16(void* udata, uint32_t addr);
uint32_t mm_read32(void* udata, uint32_t addr);
uint64_t mm_read64(void* udata, uint32_t addr);
void mm_write8(void* udata, uint32_t addr, uint8_t data);
void mm_write16(void* udata, uint32_t addr, uint16_t data);
void mm_write32(void* udata, uint32_t addr, uint32_t data);
void mm_write64(void* udata, uint32_t addr, uint64_t data);

}
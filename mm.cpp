#include <stdlib.h>
#include <stdio.h>
#include <algorithm>

#include "mm.hpp"
#include "platform.hpp"

namespace allity {

uint64_t memory_manager::read(uint32_t addr, int size) {
    auto s = std::find_if(
        m_blocks.begin(),
        m_blocks.end(),
        [addr](const memory_block& mb) {
            return (addr >= mb.start) && (addr < mb.end);
        }
    );

    if (s == m_blocks.end()) {
        printf("allity: segv when reading from %016x\n", addr);

        exit(1);
    }

    uint64_t offset = addr - s->start;

    switch (size) {
        case 0: return *((uint8_t*)(s->buf + offset));
        case 1: return be(*(uint16_t*)(s->buf + offset));
        case 2: return be(*(uint32_t*)(s->buf + offset));
        case 3: return be(*(uint64_t*)(s->buf + offset));
    }

    printf("allity: invalid size when reading from %016x\n", addr);

    return 0;
}

void memory_manager::write(uint32_t addr, uint64_t data, int size) {
    auto s = std::find_if(
        m_blocks.begin(),
        m_blocks.end(),
        [addr](const memory_block& mb) {
            return (addr >= mb.start) && (addr < mb.end);
        }
    );

    if (s == m_blocks.end()) {
        printf("allity: segv when writing %x to %016x\n", data, addr);

        exit(1);
    }

    uint64_t offset = addr - s->start;

    switch (size) {
        case 0: *((uint8_t*)(s->buf + offset)) = data; return;
        case 1: *((uint16_t*)(s->buf + offset)) = be(data); return;
        case 2: *((uint32_t*)(s->buf + offset)) = be(data); return;
        case 3: *((uint64_t*)(s->buf + offset)) = be(data); return;
    }

    printf("allity: invalid size when writing to %016x\n", addr);

    return;
}

const elf::file& memory_manager::file() const {
    return m_file;
}

memory_block* memory_manager::alloc(uint64_t start, uint64_t size) {
    memory_block mb;

    mb.start = start;
    mb.end = mb.start + size;
    mb.buf = malloc(size);

    m_blocks.push_back(mb);

    return &m_blocks.back();
}

memory_block* memory_manager::alloc(uint64_t size) {
    return alloc(0x100000000ull | rand(), size);
}

void memory_manager::free(uint64_t ptr) {
    // To-do
}

bool memory_manager::load(const char* path) {
    if (!m_file.open(path))
        return false;

    for (const auto& s : m_file.segments()) {
        uint64_t size = s.file_size();

        memory_block* mb;

        if (size) {
            mb = alloc(s.virtual_address(), size);

            s.store(mb->buf, 0);
        } else {
            mb = alloc(s.virtual_address(), s.mem_size());
        }

        mb->attributes =
            (s.is_executable() ? PF_X : 0) |
            (s.is_writeable() ? PF_W : 0) |
            (s.is_readable() ? PF_R : 0);
    }

    return true;
}

uint8_t mm_read8(void* udata, uint32_t addr) {
    memory_manager* mm = (memory_manager*)udata;

    return mm->read(addr, 0);
}

uint16_t mm_read16(void* udata, uint32_t addr) {
    memory_manager* mm = (memory_manager*)udata;

    return mm->read(addr, 1);
}

uint32_t mm_read32(void* udata, uint32_t addr) {
    memory_manager* mm = (memory_manager*)udata;

    return mm->read(addr, 2);
}

uint64_t mm_read64(void* udata, uint32_t addr) {
    memory_manager* mm = (memory_manager*)udata;

    return mm->read(addr, 3);
}

void mm_write8(void* udata, uint32_t addr, uint8_t data) {
    memory_manager* mm = (memory_manager*)udata;

    if (addr == 0xa0000000) {
        putchar(data);

        return;
    }

    mm->write(addr, data, 0);
}

void mm_write16(void* udata, uint32_t addr, uint16_t data) {
    memory_manager* mm = (memory_manager*)udata;

    mm->write(addr, data, 1);
}

void mm_write32(void* udata, uint32_t addr, uint32_t data) {
    memory_manager* mm = (memory_manager*)udata;

    mm->write(addr, data, 2);
}

void mm_write64(void* udata, uint32_t addr, uint64_t data) {
    memory_manager* mm = (memory_manager*)udata;

    mm->write(addr, data, 3);
}

}
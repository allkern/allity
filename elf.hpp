#pragma once

#include <iostream>
#include <cstdint>
#include <fstream>
#include <vector>
#include <elf.h>

class elf_segment {
    std::vector <uint8_t> m_buf;
    Elf64_Phdr m_phdr;

public:
    void load(Elf64_Phdr phdr, std::ifstream file);
    void store(void* buf, Elf64_Addr base = 0);
    Elf64_Word type() const;
    bool is_executable() const;
    bool is_writeable() const;
    bool is_readable() const;
    Elf64_Off file_offset() const;
    Elf64_Addr virt_addr() const;
    Elf64_Addr phys_addr() const;
    Elf64_Xword size() const;
    Elf64_Xword mem_size() const;
    Elf64_Xword alignment() const;
};

class elf_file {
    std::ifstream m_file;
    Elf64_Ehdr m_ehdr;
    std::vector <Elf64_Phdr> m_phdr_table;

public:
    bool open(const char* path);
    Elf64_Addr entry_point() const;
    Elf64_Half type() const;
    elf_segment load_segment(void* buf);
};

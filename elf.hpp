#pragma once

#include <iostream>
#include <cstdint>
#include <fstream>
#include <vector>
#include <elf.h>

#include "platform.hpp"

namespace elf {

struct ehdr {
    unsigned char e_ident[EI_NIDENT];
    be_t <Elf64_Half> e_type;
    be_t <Elf64_Half> e_machine;
    be_t <Elf64_Word> e_version;
    be_t <Elf64_Addr> e_entry;
    be_t <Elf64_Off> e_phoff;
    be_t <Elf64_Off> e_shoff;
    be_t <Elf64_Word> e_flags;
    be_t <Elf64_Half> e_ehsize;
    be_t <Elf64_Half> e_phentsize;
    be_t <Elf64_Half> e_phnum;
    be_t <Elf64_Half> e_shentsize;
    be_t <Elf64_Half> e_shnum;
    be_t <Elf64_Half> e_shstrndx;
};

struct phdr {
    be_t <Elf64_Word> p_type;
    be_t <Elf64_Word> p_flags;
    be_t <Elf64_Off> p_offset;
    be_t <Elf64_Addr> p_vaddr;
    be_t <Elf64_Addr> p_paddr;
    be_t <Elf64_Xword> p_filesz;
    be_t <Elf64_Xword> p_memsz;
    be_t <Elf64_Xword> p_align;
};

struct shdr {
    be_t <Elf64_Word> sh_name;
    be_t <Elf64_Word> sh_type;
    be_t <Elf64_Xword> sh_flags;
    be_t <Elf64_Addr> sh_addr;
    be_t <Elf64_Off> sh_offset;
    be_t <Elf64_Xword> sh_size;
    be_t <Elf64_Word> sh_link;
    be_t <Elf64_Word> sh_info;
    be_t <Elf64_Xword> sh_addralign;
    be_t <Elf64_Xword> sh_entsize;
};

class segment {
    std::vector <uint8_t> m_buf;
    elf::phdr m_phdr;

public:
    void load(elf::phdr phdr, std::ifstream& file);
    void store(void* buf, Elf64_Addr base = 0) const;
    Elf64_Word type() const;
    bool is_executable() const;
    bool is_writeable() const;
    bool is_readable() const;
    Elf64_Off offset() const;
    Elf64_Addr virtual_address() const;
    Elf64_Addr physical_address() const;
    Elf64_Xword file_size() const;
    Elf64_Xword mem_size() const;
    Elf64_Xword alignment() const;
    uint8_t* data();
};

class section {
    std::string m_name;
    elf::shdr m_shdr;

public:
    std::string name() const;
    Elf64_Word type() const;
    Elf64_Xword flags() const;
    Elf64_Addr address() const;
    Elf64_Off offset() const;
    Elf64_Xword size() const;
    Elf64_Word link() const;
    Elf64_Word info() const;
    Elf64_Xword alignment() const;
    Elf64_Xword entry_size() const;
};

class file {
    std::ifstream m_file;
    elf::ehdr m_ehdr;
    std::vector <elf::segment> m_segments;

public:
    bool open(const char* path);
    Elf64_Addr entry_point() const;
    Elf64_Half type() const;
    Elf64_Half machine() const;
    Elf64_Word version() const;
    Elf64_Word flags() const;
    Elf64_Half phnum() const;
    Elf64_Half shnum() const;
    elf::segment segment(int i) const;
    const std::vector <elf::segment>& segments() const;
};

}

#include <stdio.h>
#include <string.h>

#include "elf.hpp"

bool elf::file::open(const char* path) {
    m_file.open(path, std::ios::binary);

    if (!m_file.is_open())
        return false;

    m_file.read((char*)&m_ehdr, sizeof(elf::ehdr));

    if (strncmp((char*)m_ehdr.e_ident, "\x7f" "ELF", 4))
        return false;

    elf::segment seg;
    elf::phdr phdr;

    Elf64_Half phnum = m_ehdr.e_phnum;
    Elf64_Addr phoff = m_ehdr.e_phoff;
    Elf64_Half phentsize = m_ehdr.e_phentsize;

    for (int i = 0; i < phnum; i++) {
        m_file.seekg(phoff + (i * phentsize));
        m_file.read((char*)&phdr, sizeof(elf::phdr));

        if ((Elf64_Word)phdr.p_type == PT_LOAD)
            seg.load(phdr, m_file);

        m_segments.push_back(seg);
    }

    return true;
}

Elf64_Addr elf::file::entry_point() const {
    return m_ehdr.e_entry;
}

Elf64_Half elf::file::type() const {
    return m_ehdr.e_type;
}

Elf64_Half elf::file::machine() const {
    return m_ehdr.e_machine;
}

Elf64_Word elf::file::version() const {
    return m_ehdr.e_version;
}

Elf64_Word elf::file::flags() const {
    return m_ehdr.e_flags;
}

Elf64_Half elf::file::phnum() const {
    return m_ehdr.e_phnum;
}

Elf64_Half elf::file::shnum() const {
    return m_ehdr.e_shnum;
}

elf::segment elf::file::segment(int i) const {
    return m_segments.at(i);
}

const std::vector <elf::segment>& elf::file::segments() const {
    return m_segments;
}

void elf::segment::load(elf::phdr phdr, std::ifstream& file) {
    m_phdr = phdr;

    Elf64_Addr size = m_phdr.p_filesz;
    Elf64_Addr offset = m_phdr.p_offset;
    Elf64_Word type = m_phdr.p_type;

    if (type != PT_LOAD)
        return;

    if (!size)
        size = m_phdr.p_memsz;

    m_buf.resize(size);

    for (int i = 0; i < m_buf.size(); i++)
        m_buf[i] = 0;

    if (!(Elf64_Addr)m_phdr.p_filesz)
        return;

    file.seekg(offset);
    file.read((char*)m_buf.data(), size);
}

void elf::segment::store(void* buf, Elf64_Addr base) const {
    memcpy(buf, m_buf.data(), (Elf64_Addr)m_phdr.p_filesz);
}

uint8_t* elf::segment::data() {
    return m_buf.data();
}

Elf64_Word elf::segment::type() const {
    return m_phdr.p_type;
}

bool elf::segment::is_executable() const {
    return m_phdr.p_flags & 1;
}

bool elf::segment::is_writeable() const {
    return m_phdr.p_flags & 2;
}

bool elf::segment::is_readable() const {
    return m_phdr.p_flags & 4;
}

Elf64_Off elf::segment::offset() const {
    return m_phdr.p_offset;
}

Elf64_Addr elf::segment::virtual_address() const {
    return m_phdr.p_vaddr;
}

Elf64_Addr elf::segment::physical_address() const {
    return m_phdr.p_paddr;
}

Elf64_Xword elf::segment::file_size() const {
    return m_phdr.p_filesz;
}

Elf64_Xword elf::segment::mem_size() const {
    return m_phdr.p_memsz;
}

Elf64_Xword elf::segment::alignment() const {
    return m_phdr.p_align;
}

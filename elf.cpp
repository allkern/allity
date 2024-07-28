#include <stdio.h>
#include <string.h>

#include "elf.hpp"

bool elf_file::open(const char* path) {
    m_file.open(path, std::ios::binary);

    if (!m_file.is_open())
        return false;

    m_file.read((char*)&m_ehdr, sizeof(Elf64_Ehdr));

    if (strncmp((char*)m_ehdr.e_ident, "\x7f" "ELF", 4))
        return false;

    Elf64_Phdr phdr;

    for (int i = 0; i < m_ehdr.e_phnum; i++) {
        m_file.seekg(m_ehdr.e_phoff + (i * m_ehdr.e_phentsize));
        m_file.read((char*)&phdr, sizeof(Elf64_Phdr));

        m_phdr_table.push_back(phdr);
    }

    return true;
}

Elf64_Addr elf_file::entry_point() const {
    return m_ehdr.e_entry;
}

Elf64_Half elf_file::type() const {
    return m_ehdr.e_type;
}

void elf_segment::load(Elf64_Phdr phdr, std::ifstream file) {
    m_phdr = phdr;
    m_buf.resize(m_phdr.p_filesz);

    file.seekg(m_phdr.p_offset);
    file.read((char*)m_buf.data(), m_phdr.p_filesz);
}

void elf_segment::store(void* buf, Elf64_Addr base = 0) {
    memcpy(buf, m_buf.data(), m_phdr.p_memsz);
}

Elf64_Word elf_segment::type() const {
    return m_phdr.p_type;
}

bool elf_segment::is_executable() const {
    return m_phdr.p_flags & 1;
}

bool elf_segment::is_writeable() const {
    return m_phdr.p_flags & 2;
}

bool elf_segment::is_readable() const {
    return m_phdr.p_flags & 4;
}

Elf64_Off elf_segment::file_offset() const {
    return m_phdr.p_offset;
}

Elf64_Addr elf_segment::virt_addr() const {
    return m_phdr.p_vaddr;
}

Elf64_Addr elf_segment::phys_addr() const {
    return m_phdr.p_paddr;
}

Elf64_Xword elf_segment::size() const {
    return m_phdr.p_filesz;
}

Elf64_Xword elf_segment::mem_size() const {
    return m_phdr.p_memsz;
}

Elf64_Xword elf_segment::alignment() const {
    return m_phdr.p_align;
}

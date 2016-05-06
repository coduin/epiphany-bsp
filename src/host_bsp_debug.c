/*
This file is part of the Epiphany BSP library.

Copyright (C) 2014-2015 Buurlage Wits
Support e-mail: <info@buurlagewits.nl>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License (LGPL)
as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
and the GNU Lesser General Public License along with this program,
see the files COPYING and COPYING.LESSER. If not, see
<http://www.gnu.org/licenses/>.
*/
#ifdef DEBUG
#include "host_bsp_private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>

void _read_elf(const char* filename);
void _parse_elf(char* buffer, size_t fsize);
void _parse_symbols(char* buffer, size_t fsize, Elf32_Shdr* shdr,
                    size_t symtab_index);
Symbol* _get_symbol_by_addr(void* addr);
Symbol* _get_symbol_by_name(const char* symbol);

void _read_elf(const char* filename) {
    state.e_symbols = 0;
    state.num_symbols = 0;

    FILE* file = fopen(filename, "r");
    
    if (!file) {
        fprintf(stderr, "ERROR: Could not open %s\n", filename);
        return;
    }

    size_t fsize;
    fseek(file, 0L, SEEK_END);
    fsize = ftell(file);
    fseek(file, 0L, SEEK_SET);

    char* buffer = malloc(fsize);
    size_t read = fread(buffer, 1, fsize, file);

    if (read < fsize)
        fprintf(stderr, "ERROR: Could not read full file %s\n", filename);
    else
        _parse_elf(buffer, fsize);

    free(buffer);
    fclose(file);

    return;
}

#define EM_ADAPTEVA_EPIPHANY 0x1223 /* Adapteva's Epiphany architecture.  */
int is_epiphany_exec_elf(Elf32_Ehdr* ehdr) {
    return ehdr && memcmp(ehdr->e_ident, ELFMAG, SELFMAG) == 0 &&
           ehdr->e_ident[EI_CLASS] == ELFCLASS32 && ehdr->e_type == ET_EXEC &&
           ehdr->e_version == EV_CURRENT &&
           ehdr->e_machine == EM_ADAPTEVA_EPIPHANY;
}

void _parse_elf(char* buffer, size_t fsize) {
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*)buffer;
    Elf32_Shdr* shdr;

    if (fsize < sizeof(Elf32_Ehdr) || !is_epiphany_exec_elf(ehdr) ||
        (ehdr->e_shoff + ehdr->e_shnum * sizeof(Elf32_Shdr) > fsize)) {
        fprintf(stderr, "ERROR: File is not an Epiphany executable.");
        return;
    }

    shdr = (Elf32_Shdr*)&buffer[ehdr->e_shoff];
    for (size_t i = 0; i < ehdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_SYMTAB)
            _parse_symbols(buffer, fsize, shdr, i);
    }
    return;
}

void _parse_symbols(char* buffer, size_t fsize, Elf32_Shdr* shdr,
                    size_t symtab_index) {
    Elf32_Shdr* symtab = &shdr[symtab_index];

    size_t count = symtab->sh_size / symtab->sh_entsize;

    const char* symstr = &buffer[shdr[symtab->sh_link].sh_offset];

    Elf32_Sym* symbol = (Elf32_Sym*)&buffer[symtab->sh_offset];

    // First count the number of symbols that we want to save
    state.num_symbols = 0;
    for (size_t i = 0; i < count; i++) {
        if (ELF32_ST_BIND(symbol[i].st_info) == STB_GLOBAL &&
            symbol[i].st_shndx != SHN_ABS) {
            state.num_symbols++;
        }
    }

    // Now save them in the array
    state.e_symbols = (Symbol*)malloc(state.num_symbols * sizeof(Symbol));
    size_t j = 0;
    for (size_t i = 0; i < count; i++) {
        if (ELF32_ST_BIND(symbol[i].st_info) != STB_GLOBAL ||
            symbol[i].st_shndx == SHN_ABS)
            continue;
        Symbol* sym = &state.e_symbols[j++];
        sym->index = i;
        sym->value = symbol[i].st_value;
        sym->size = symbol[i].st_size;
        sym->type = ELF32_ST_TYPE(symbol[i].st_info);
        sym->bind = ELF32_ST_BIND(symbol[i].st_info);
        sym->section = symbol[i].st_shndx;
        memcpy(sym->name, &symstr[symbol[i].st_name], sizeof(sym->name));
        sym->name[sizeof(sym->name) - 1] = 0;
    }
}

Symbol* _get_symbol_by_addr(void* addr) {
    for (size_t i = 0; i < state.num_symbols; i++) {
        if (state.e_symbols[i].value <= ((unsigned int)addr) &&
            ((unsigned int)addr) < state.e_symbols[i].value + state.e_symbols[i].size) {
            return &state.e_symbols[i];
        }
    }
    return 0;
}

Symbol* _get_symbol_by_name(const char* symbol) {
    for (size_t i = 0; i < state.num_symbols; i++) {
        if (!strncmp(state.e_symbols[i].name, symbol, sizeof(state.e_symbols[i].name))) {
            return &state.e_symbols[i];
        }
    }
    return 0;
}
#endif

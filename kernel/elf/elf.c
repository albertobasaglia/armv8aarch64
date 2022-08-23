#include <elf/filebuffer.h>
#include <elf/elf.h>
#include <stddef.h>
#include <string.h>

ELF elf_fromfilewrap(struct filebuffer file)
{
	ELF elf = {
	    .file = file,
	    .header_loaded = 0,
	    .section_headers_loaded = 0,
	    .strings_section_loaded = 0,
	    .program_headers_loaded = 0,
	};

	return elf;
}

int elf_parseheader(ELF* elf)
{
	size_t hdr_len = sizeof(ElfN_Ehdr);
	read_at_position(&elf->header, 0, hdr_len, 1, elf->file);
	if (elf->header.e_ident[0] != ELF_MAGIC_NUMBER) {
		return 0;
	}
	elf->header_loaded = 1;
	return 1;
}

int elf_get_sectionentries(ELF* elf)
{
	if (!elf->header_loaded) {
		return -1;
	}

	return elf->header.e_shnum;
}

int elf_get_programentries(ELF* elf)
{
	if (!elf->header_loaded) {
		return -1;
	}

	return elf->header.e_phnum;
}

size_t elf_get_sectionheaders_bytes(ELF* elf)
{
	if (!elf->header_loaded) {
		return -1;
	}

	return sizeof(ElfN_Shdr) * elf->header.e_shnum;
}

size_t elf_get_programheaders_bytes(ELF* elf)
{
	if (!elf->header_loaded) {
		return -1;
	}

	return sizeof(ElfN_Phdr) * elf->header.e_phnum;
}

int elf_load_sectionheaders(ELF* elf)
{
	if (!elf->header_loaded) {
		return 0;
	}
	if (elf->section_headers == NULL) {
		return 0;
	}

	read_at_position(elf->section_headers, elf->header.e_shoff,
			 elf->header.e_shentsize, elf->header.e_shnum,
			 elf->file);

	elf->section_headers_loaded = 1;
	return 1;
}

int elf_load_programheaders(ELF* elf)
{
	if (!elf->header_loaded) {
		return 0;
	}
	if (elf->program_headers == NULL) {
		return 0;
	}

	read_at_position(elf->program_headers, elf->header.e_phoff,
			 elf->header.e_phentsize, elf->header.e_phnum,
			 elf->file);

	elf->program_headers_loaded = 1;
	return 1;
}

size_t elf_get_sectionsize(ELF* elf, int index)
{
	if (!elf->section_headers_loaded) {
		return -1;
	}

	if (index < 0 || index >= elf->header.e_shnum) {
		return -1;
	}

	return elf->section_headers[index].sh_size;
}

size_t elf_get_stringsectionsize(ELF* elf)
{
	return elf_get_sectionsize(elf, elf->header.e_shstrndx);
}

int elf_load_strings_section(ELF* elf)
{
	if (!elf->section_headers_loaded) {
		return 0;
	}

	if (elf->strings_section == NULL) {
		return 0;
	}

	ElfN_Shdr* strings_section = elf_get_sectionheader_byid(
	    elf, elf->header.e_shstrndx);

	size_t offset = strings_section->sh_offset;
	size_t size = strings_section->sh_size;

	elf->strings_section_loaded = 1;

	read_at_position(elf->strings_section, offset, size, 1, elf->file);

	return 1;
}

ElfN_Shdr* elf_get_sectionheader_byid(ELF* elf, int index)
{
	if (!elf->section_headers_loaded) {
		return NULL;
	}

	if (index < 0 || index >= elf->header.e_shnum) {
		return NULL;
	}

	return elf->section_headers + index;
}

ElfN_Shdr* elf_get_sectionheader_bynameoffset(ELF* elf, int index)
{
	if (!elf->section_headers_loaded) {
		return NULL;
	}

	ElfN_Shdr* ptr = elf->section_headers;
	int size = elf->header.e_shnum;
	int i = 0;

	while (i < size) {
		if (ptr->sh_name == index) {
			return ptr;
		}
		i++;
		ptr++;
	}

	return NULL;
}

ElfN_Shdr* elf_get_sectionheader_byname(ELF* elf, const char* name)
{
	if (!elf->strings_section_loaded) {
		return NULL;
	}

	int current_string_first_index = 0;
	size_t current_char_index = 0;
	size_t section_len = elf_get_stringsectionsize(elf);

	while (current_char_index < section_len) {
		if (elf->strings_section[current_char_index] != 0) {
			current_char_index++;
			continue;
		}
		current_char_index++;
		if (strcmp(name, elf->strings_section +
				     current_string_first_index) == 0) {
			return elf_get_sectionheader_bynameoffset(
			    elf, current_string_first_index);
		}
		current_string_first_index = current_char_index;
	}

	return NULL;
}

void elf_dump_section_content(ELF* elf, ElfN_Shdr* section, void* ptr)
{
	size_t size = section->sh_size;
	size_t offset = section->sh_offset;
	read_at_position(ptr, offset, size, 1, elf->file);
}

ElfN_Phdr* elf_get_programheader_byid(ELF* elf, int index)
{
	if (!elf->program_headers_loaded) {
		return NULL;
	}

	return elf->program_headers + index;
}

size_t elf_get_programheader_count(ELF* elf)
{
	return elf->header.e_phnum;
}

void elf_dump_program_content(ELF* elf, ElfN_Phdr* program, void* ptr)
{
	size_t size = program->p_memsz;
	size_t offset = program->p_offset;
	read_at_position(ptr, offset, size, 1, elf->file);
}

ElfN_Addr elf_get_entrypoint(ELF* elf)
{
	return elf->header.e_entry;
}

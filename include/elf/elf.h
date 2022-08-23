#ifndef ELF_H
#define ELF_H

#include <elf/filebuffer.h>
#include <stddef.h>
#include <stdint.h>

// general defininitions:

#define EI_NIDENT        16
#define EI_DATA          5
#define ELF_MAGIC_NUMBER 0x7f

#define ET_NONE          0
#define ET_REL           1
#define ET_EXEC          2
#define ET_DYN           3

typedef uint64_t ElfN_Off;
typedef uint64_t ElfN_Addr;
typedef ElfN_Off Elf64_Off;
typedef ElfN_Addr Elf64_Addr;
typedef uint64_t uintN_t;

// ELF header:

typedef struct {
	/*
	 * How to interpret the file
	 * */
	unsigned char e_ident[EI_NIDENT];
	/*
	 * Type of ELF file
	 * */
	uint16_t e_type;
	/*
	 * Type of the machine this is compiler for
	 * */
	uint16_t e_machine;
	/*
	 * Version
	 * */
	uint32_t e_version;
	/*
	 * Entry address
	 * */
	ElfN_Addr e_entry;
	/*
	 * Program header table offset in bytes
	 * */
	ElfN_Off e_phoff;
	/*
	 * Section header table offset in bytes
	 * */
	ElfN_Off e_shoff;
	/*
	 * Processor-specific flags
	 * */
	uint32_t e_flags;
	/*
	 * ELF-header size in bytes
	 * */
	uint16_t e_ehsize;
	/*
	 * Size of entries in file program header table
	 * */
	uint16_t e_phentsize;
	/*
	 * Number of entries in the program header table
	 * */
	uint16_t e_phnum;
	/*
	 * Size of entries in file section header table
	 * */
	uint16_t e_shentsize;
	/*
	 * Number of entries in the section header table
	 * */
	uint16_t e_shnum;
	/*
	 * Section header table index associated with the string table
	 * */
	uint16_t e_shstrndx;
} __attribute__((packed)) ElfN_Ehdr;

// Program header:

typedef struct {
	uint32_t p_type;
	uint32_t p_flags;
	Elf64_Off p_offset;
	Elf64_Addr p_vaddr;
	Elf64_Addr p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
} __attribute__((packed)) ElfN_Phdr;

// Section header:

typedef struct {
	uint32_t sh_name;
	uint32_t sh_type;
	uintN_t sh_flags;
	ElfN_Addr sh_addr;
	ElfN_Off sh_offset;
	uintN_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uintN_t sh_addralign;
	uintN_t sh_entsize;
} __attribute__((packed)) ElfN_Shdr;

// String and symble table:

typedef struct {
	uint32_t st_name;
	unsigned char st_info;
	unsigned char st_other;
	uint16_t st_shndx;
	Elf64_Addr st_value;
	uint64_t st_size;
} __attribute__((packed)) ElfN_Sym;

typedef struct {

} ELF_string;

typedef struct {
	struct filebuffer file;
	int header_loaded;
	ElfN_Ehdr header;
	int section_headers_loaded;
	ElfN_Shdr* section_headers;
	int strings_section_loaded;
	char* strings_section;
	int program_headers_loaded;
	ElfN_Phdr* program_headers;
} ELF;

/*
 * Create an ELF file parser from a file buffer
 * */
ELF elf_fromfilebuffer(struct filebuffer file);

/*
 * Parse (from file) the ELF header structure
 *
 * @return 1 if successful, 0 otherwise
 * */
int elf_parseheader(ELF* elf);

/*
 * @return -1 if header has not been loaded yet
 * */
int elf_get_sectionentries(ELF* elf);

/*
 * @return -1 if header has not been loaded yet
 * */
int elf_get_programentries(ELF* elf);

/*
 * @return bytes needed by section headers
 * */
size_t elf_get_sectionheaders_bytes(ELF* elf);

/*
 * @return bytes needed by program headers
 * */
size_t elf_get_programheaders_bytes(ELF* elf);

/*
 * Loads the section headers.
 * Expects the section_header elf field to be pointing an allocated
 * region of memory. @see elf_get_sectionheaders_bytes
 * @return 1 if successful
 * */
int elf_load_sectionheaders(ELF* elf);

/*
 * Loads the program headers.
 * Expects the program_header elf field to be pointing an allocated
 * region of memory. @see elf_get_programheaders_bytes
 * @return 1 if successful
 * */
int elf_load_programheaders(ELF* elf);

/*
 * Gets the size (bytes) of the section with index
 * */
size_t elf_get_sectionsize(ELF* elf, int index);

/*
 * Gets the size (char) of the sting section
 * */
size_t elf_get_stringsectionsize(ELF* elf);

/*
 * Expects the memory for the strings section to be allocated
 * */
int elf_load_strings_section(ELF* elf);

/*
 * Returns the section header corresponding to the index
 * */
ElfN_Shdr* elf_get_sectionheader_byid(ELF* elf, int index);

/*
 * Returns a section header by the offset of its name in the strings
 * section
 * */
ElfN_Shdr* elf_get_sectionheader_bynameoffset(ELF* elf, int index);

/*
 * Returns the section header corresponding to the given string
 * */
ElfN_Shdr* elf_get_sectionheader_byname(ELF* elf, const char* name);

/*
 * Writes the section content at memory pointed by ptr
 * */
void elf_dump_section_content(ELF* elf, ElfN_Shdr* section, void* ptr);

/*
 * Returns the program header corresponding to an id
 * */
ElfN_Phdr* elf_get_programheader_byid(ELF* elf, int index);

/*
 * Returns the count of program headers
 * */
size_t elf_get_programheader_count(ELF* elf);

/*
 * Writes the content of a program section to a memory location
 * */
void elf_dump_program_content(ELF* elf, ElfN_Phdr* program, void* ptr);

/*
 * Get entry point address
 * */
ElfN_Addr elf_get_entrypoint(ELF* elf);

/*
 * Allocates and parses all headers
 * */
void elf_alloc_and_parse(ELF* elf);

/*
 * Frees allocated memory
 * */
void elf_free(ELF* elf);

#endif

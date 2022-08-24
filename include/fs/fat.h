#ifndef FS_FAT_H
#define FS_FAT_H

#include <block.h>
#include <stddef.h>
#include <stdint.h>

struct fat16_header {
	uint8_t pad1[11];
	uint16_t bytes_per_sector;
	uint8_t sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t fat_copies;
	uint16_t possible_root_entries;
	uint16_t small_sectors;
	uint8_t descriptor;
	uint16_t sectors_per_fat;
	uint16_t sectors_per_track;
	uint16_t number_of_heads;
	uint32_t hidden_sectors;
	uint32_t large_sectors;
	uint8_t pad2[476];
} __attribute__((packed));

struct fat16_dir_entry {
	char filename[8];
	char extension[3];
	uint8_t attribute;
	uint8_t windowsnt;
	uint8_t creation_ms;
	uint16_t creation_time;
	uint16_t creation_date;
	uint16_t lastaccess_date;
	uint16_t fat32_reserved;
	uint16_t lastwrite_time;
	uint16_t lastwrite_date;
	uint16_t starting_cluster;
	uint32_t filesize_bytes;
} __attribute__((packed));

struct fat_handle {
	struct fat16_header header;
	size_t sectors;
	// FAT
	size_t fat_size_bytes;
	size_t fat_offset;
	uint16_t* fat;
	// DIRECTORY
	size_t dir_entry_offset;
	size_t dir_entry_size_bytes;
	struct fat16_dir_entry* entries;
	// DATA
	size_t data_offset;
	// BLOCK
	struct block* block;
};

void print_dir_entry(struct fat16_dir_entry* entry);

/*
 * Abstraction utility function to read data from disk.
 * */
void fat_load_data(struct fat_handle* handle,
		   void* ptr,
		   size_t offset,
		   size_t n);

/*
 * Creates struct fat_handle on block device.
 *
 * Returns handle after reading fat header.
 * */
struct fat_handle fat_load(struct block* block);

/*
 * Loads file allocation table.
 * */
void fat_load_table(struct fat_handle* handle);

/*
 * Loads root directory entries.
 * */
void fat_load_entries(struct fat_handle* handle);

/*
 * Clears allocated data.
 * */
void fat_clear(struct fat_handle* handle);

/*
 * Reads a single cluster.
 * */
void fat_read_cluster(struct fat_handle* handle,
		      size_t cluster,
		      int bytes,
		      void* ptr);

/*
 * Reads a fat entry to ptr.
 * */
void fat_read_entry(struct fat_handle* handle,
		    struct fat16_dir_entry* entry,
		    void* ptr);

/*
 * Logs fat debug informations.
 * */
void fat_debug_info(struct fat_handle* handle);

/*
 * Returns uppercase.
 * */
char fat_char_get_uppercase(char c);

/*
 * Utility to convert names.
 * */
void fat_uppercase_and_pad(char* dest, const char* src, int dim);

/*
 * Returns name following FAT standards.
 * TODO Does not implement long names yet.
 * */
void fat_get_fat_name(char* dest, const char* src);

/*
 * Returns extension following FAT standards.
 * TODO Does not implement long names yet.
 * */
void fat_get_fat_extension(char* dest, const char* src);

/*
 * Returns an handle to a fat directory entry given its name.
 * TODO Only works in root directory.
 * */
struct fat16_dir_entry* fat_get_entry_by_file(struct fat_handle* handle,
					      const char* name,
					      const char* ext);

#endif

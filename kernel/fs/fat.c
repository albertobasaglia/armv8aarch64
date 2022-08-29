#include "math.h"
#include <block.h>
#include <fs/fat.h>
#include <log.h>
#include <sysutils.h>

#include <stdint.h>
#include <string.h>

#define SECTOR_SIZE 512

int fat_char_array_compare(const char* a, const char* b, int len)
{
	for (int i = 0; i < len; i++) {
		if (a[i] == b[i])
			continue;
		return a[i] - b[i];
	}
	return 0;
}

char fat_char_get_uppercase(char c)
{
	if (c >= 'a' && c <= 'z')
		return c - ('a' - 'A');
	return c;
}

void fat_uppercase_and_pad(char* dest, const char* src, int dim)
{
	char* dest_last = dest + dim;
	if (strlen(src) >= 0 && strlen(src) <= dim) {
		while (*src != 0) {
			*dest = fat_char_get_uppercase(*src);
			src++;
			dest++;
		}
		while (dest < dest_last) {
			*dest = ' ';
			dest++;
		}
	}
}

void fat_get_fat_name(char* dest, const char* src)
{
	fat_uppercase_and_pad(dest, src, 8);
}

void fat_get_fat_extension(char* dest, const char* src)
{
	fat_uppercase_and_pad(dest, src, 3);
}

void fat_load_data(struct fat_handle* handle,
		   void* ptr,
		   size_t offset,
		   size_t n)
{
	int sector = offset / SECTOR_SIZE;
	int sector_count = n / SECTOR_SIZE + ((n % SECTOR_SIZE == 0) ? 0 : 1) +
			   ((offset % SECTOR_SIZE == 0) ? 0 : 1);
	int not_complete_offset = offset % SECTOR_SIZE;
	char buffer[SECTOR_SIZE];

	// load first sector
	block_read(handle->block, buffer, sector);
	memcpy(ptr, buffer + not_complete_offset,
	       SECTOR_SIZE - not_complete_offset);
	ptr += SECTOR_SIZE - not_complete_offset;

	if (sector_count < 2)
		return;

	// load middle sectors
	for (int s = sector + 1; s < sector_count + sector - 1; s++) {
		block_read(handle->block, buffer, s);
		memcpy(ptr, buffer, SECTOR_SIZE);
		ptr += SECTOR_SIZE;
	}

	// load last sector
	block_read(handle->block, buffer, sector + sector_count - 1);
	memcpy(ptr, buffer,
	       not_complete_offset != 0 ? not_complete_offset : SECTOR_SIZE);
}

struct fat_handle fat_load(struct block* block)
{
	struct fat_handle handle;
	handle.block = block;
	fat_load_data(&handle, &handle.header, 0, sizeof(struct fat16_header));
	handle.sectors = (handle.header.small_sectors != 0)
			     ? handle.header.small_sectors
			     : handle.header.large_sectors;

	handle.fat_size_bytes = sizeof(uint16_t) * handle.sectors;
	handle.fat_offset = handle.header.reserved_sectors *
			    handle.header.bytes_per_sector;

	handle.dir_entry_offset = handle.fat_offset +
				  handle.header.sectors_per_fat *
				      handle.header.fat_copies *
				      handle.header.bytes_per_sector;
	handle.dir_entry_size_bytes = handle.header.possible_root_entries *
				      sizeof(struct fat16_dir_entry);

	handle.data_offset = handle.dir_entry_offset +
			     handle.dir_entry_size_bytes;

	fat_load_table(&handle);
	fat_load_entries(&handle);

	return handle;
}

void fat_load_table(struct fat_handle* handle)
{
	handle->fat = kalloc(handle->fat_size_bytes);

	if (handle->fat == NULL) {
		klog("Couldn't allocate fat");
		return;
	}

	fat_load_data(handle, handle->fat, handle->fat_offset,
		      handle->fat_size_bytes);
}

void fat_load_entries(struct fat_handle* handle)
{
	handle->entries = kalloc(handle->dir_entry_size_bytes);

	if (handle->entries == NULL) {
		klog("Couldn't allocate entries directory");
		return;
	}

	fat_load_data(handle, handle->entries, handle->dir_entry_offset,
		      handle->dir_entry_size_bytes);
}

void fat_clear(struct fat_handle* handle)
{
	if (handle->fat == NULL)
		kfree(handle->fat);
	handle->fat = NULL;

	if (handle->entries == NULL)
		kfree(handle->entries);
	handle->entries = NULL;
}

void fat_read_cluster(struct fat_handle* handle,
		      size_t cluster,
		      int bytes,
		      void* ptr)
{
	size_t read_offset = handle->data_offset +
			     handle->header.sectors_per_cluster *
				 handle->header.bytes_per_sector *
				 (cluster - 2);
	fat_load_data(handle, ptr, read_offset, bytes);
}

void fat_debug_info(struct fat_handle* handle)
{
	klogf("sectors: %q", handle->sectors);
	klogf("sectors per cluster: %q", handle->header.sectors_per_cluster);
	klogf("bytes per sector: %q", handle->header.bytes_per_sector);
}

void fat_read_entry(struct fat_handle* handle,
		    struct fat16_dir_entry* entry,
		    void* ptr)
{
	size_t cluster_size = fat_get_cluster_size(handle);

	int bytes_left = entry->filesize_bytes;
	int bytes_processed = 0;
	int cluster = entry->starting_cluster;

	while (cluster != 0xffff) {
		fat_read_cluster(handle, cluster,
				 bytes_left > cluster_size ? cluster_size
							   : bytes_left,
				 ptr + bytes_processed);
		cluster = handle->fat[cluster];
		bytes_left -= cluster_size;
		bytes_processed += cluster_size;
	}
}

void fat_read_entry_offset_size(struct fat_handle* handle,
				struct fat16_dir_entry* entry,
				void* ptr,
				int offset,
				int size)
{
	/* klogf("offset: %q, size: %q", offset, size); */
	/*
	 * TODO debug!
	 * */
	size_t cluster_size = fat_get_cluster_size(handle);

	char* buffer = kalloc(cluster_size);

	int clusters_read = size / cluster_size +
			    ((size % cluster_size == 0) ? 0 : 1) +
			    ((offset % cluster_size == 0) ? 0 : 1);

	int first_cluster_index = offset / cluster_size;
	int first_cluster_offset = offset % cluster_size;

	int cluster = entry->starting_cluster;
	for (int i = 0; i < first_cluster_index; i++) {
		cluster = handle->fat[cluster];
	}

	int out_index = 0;
	int remaining_bytes = size;

	// Read the first sector

	int first_read_size = min(size, cluster_size - first_cluster_offset);

	fat_read_cluster(handle, cluster, cluster_size, buffer);
	memcpy(ptr + out_index, buffer + first_cluster_offset, first_read_size);
	remaining_bytes -= first_read_size;
	out_index += first_read_size;

	cluster = handle->fat[cluster];

	while (remaining_bytes > cluster_size) {
		fat_read_cluster(handle, cluster, cluster_size, buffer);
		memcpy(buffer + out_index, buffer, cluster_size);
		remaining_bytes -= cluster_size;
		out_index += cluster_size;
		cluster = handle->fat[cluster];
	}

	if (remaining_bytes > 0) {
		fat_read_cluster(handle, cluster, remaining_bytes, buffer);
		memcpy(buffer + out_index, buffer, remaining_bytes);
	}

	kfree(buffer);
}

void fat_read_entry_cluster_offset(struct fat_handle* handle,
				   struct fat16_dir_entry* entry,
				   void* ptr,
				   int cluster_offset)
{
	size_t cluster_size = fat_get_cluster_size(handle);

	int cluster_index = 0;
	int cluster = entry->starting_cluster;
	while (cluster != 0xffff && cluster_index < cluster_offset) {
		cluster = handle->fat[cluster];
		cluster_index++;
	}
	if (cluster == 0xffff) {
		// failed, not enought clusters
		return;
	}
	fat_read_cluster(handle, cluster, cluster_size, ptr);
}

struct fat16_dir_entry* fat_get_entry_by_file(struct fat_handle* handle,
					      const char* name,
					      const char* ext)
{
	char fat_name[8], fat_ext[3];
	fat_get_fat_extension(fat_ext, ext);
	fat_get_fat_name(fat_name, name);

	for (int i = 0; i < handle->header.possible_root_entries; i++) {
		if (fat_char_array_compare(
			fat_name, handle->entries[i].filename, 8) == 0 &&
		    fat_char_array_compare(
			fat_ext, handle->entries[i].extension, 3) == 0) {
			return handle->entries + i;
		}
	}
	return NULL;
}

int fat_get_cluster_size(struct fat_handle* handle)
{
	return handle->header.bytes_per_sector *
	       handle->header.sectors_per_cluster;
}

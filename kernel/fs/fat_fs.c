#include <block.h>
#include <fs/fat.h>
#include <fs/fat_fs.h>
#include <fs/fs.h>
#include <log.h>
#include <stdint.h>
#include <sysutils.h>
#include <uart.h>

int fatfs_get(struct inode* inode, char* ptr);
int fatfs_put(struct inode* inode, char c);

struct filesystem* fatfs_createfs(struct block* block)
{
	struct fat_handle* fat = kalloc(sizeof(struct fat_handle));
	*fat = fat_load(block);
	struct filesystem* fs = kalloc(sizeof(struct filesystem));
	fs->arguments = (void*)fat;
	fs->open = fatfs_open;
	return fs;
}

void fatfs_deletefs(struct filesystem* filesystem)
{
	kfree(filesystem->arguments);
	kfree(filesystem);
}

struct inode* fatfs_open(struct filesystem* filesystem, const char* filename)
{
	struct fat_handle* fat_handle = filesystem->arguments;
	struct inode* inode = fs_allocate_inode();

	// TODO this should be in some string function
	char name[10];
	char ext[10];
	int index = 0;
	while (*filename != '.') {
		name[index++] = *filename;
		filename++;
	}
	name[index] = 0;
	filename++;
	index = 0;
	while (*filename != 0) {
		ext[index++] = *filename;
		filename++;
	}
	ext[index] = 0;

	struct fat16_dir_entry* entry = fat_get_entry_by_file(fat_handle, name,
							      ext);

	struct fat_inode* fat_inode = kalloc(sizeof(struct fat_inode));

	inode->impl_inode = (void*)fat_inode;

	fat_inode->fat_handle = fat_handle;
	fat_inode->fat16_dir_entry = entry;
	fat_inode->position = 0;
	fat_inode->loaded_buffer_index = -1;

	inode->vtable.close = fatfs_close;

	inode->vtable.get = fatfs_get;
	inode->vtable.put = NULL; // TODO read only now!
	inode->vtable.left = fatfs_left;

	return inode;
}

int fatfs_read(struct inode* inode, void* ptr)
{
	struct fat_inode* fat_inode = (struct fat_inode*)inode->impl_inode;
	fat_read_entry(fat_inode->fat_handle, fat_inode->fat16_dir_entry, ptr);
	return 0;
}

int fatfs_get(struct inode* inode, char* ptr)
{

	struct fat_inode* fat_inode = (struct fat_inode*)inode->impl_inode;
	struct fat_handle* fat_handle = fat_inode->fat_handle;

	if (fat_inode->position >= fat_inode->fat16_dir_entry->filesize_bytes)
		return 0;

	int req_buffer_index = fat_inode->position / BUFFERSIZE;
	int req_buffer_offset = fat_inode->position % BUFFERSIZE;

	if (req_buffer_index != fat_inode->loaded_buffer_index) {
		fat_inode->loaded_buffer_index = req_buffer_index;

		// TODO
		// this is should fix it.
		// fat_read_entry_offset_size is not yet implemented
		fat_read_entry_offset_size(
		    fat_handle, fat_inode->fat16_dir_entry, fat_inode->buffer,
		    BUFFERSIZE * req_buffer_index, BUFFERSIZE);
	}

	*ptr = fat_inode->buffer[req_buffer_offset];
	fat_inode->position++;
	return 1;
}

int fatfs_close(struct inode* inode)
{
	struct fat_inode* fat_inode = (struct fat_inode*)inode->impl_inode;
	kfree(fat_inode);
	fs_free_inode(inode);
	return 0;
}

int fatfs_left(struct inode* inode)
{
	struct fat_inode* fat_inode = (struct fat_inode*)inode->impl_inode;
	return fat_inode->fat16_dir_entry->filesize_bytes - fat_inode->position;
}

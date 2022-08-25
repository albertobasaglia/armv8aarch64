#include "block.h"
#include "uart.h"
#include <fs/fat.h>
#include <fs/fat_fs.h>
#include <fs/fs.h>
#include <stdint.h>
#include <sysutils.h>

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

	/*
	 * Set up the vtable
	 * */
	inode->vtable.read = fatfs_read;
	inode->vtable.close = fatfs_close;

	return inode;
}

int fatfs_read(struct inode* inode, void* ptr)
{
	struct fat_inode* fat_inode = (struct fat_inode*)inode->impl_inode;
	fat_read_entry(fat_inode->fat_handle, fat_inode->fat16_dir_entry, ptr);
	return 0;
}

int fatfs_close(struct inode* inode)
{
	struct fat_inode* fat_inode = (struct fat_inode*)inode->impl_inode;
	kfree(fat_inode);
	fs_free_inode(inode);
	return 0;
}

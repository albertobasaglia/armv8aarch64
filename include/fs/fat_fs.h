#ifndef FS_FAT_FS_H
#define FS_FAT_FS_H

#include <fs/fat.h>
#include <fs/fs.h>

#define BUFFERSIZE 512

struct fat_inode {
	struct fat16_dir_entry* fat16_dir_entry;
	struct fat_handle* fat_handle;
	char buffer[BUFFERSIZE];
	int position;
	int loaded_buffer_index;
};

struct filesystem* fatfs_createfs(struct block* block);

void fatfs_deletefs(struct filesystem* filesystem);

/*
 * Opens a file.
 * Allocates its inode.
 * */
struct inode* fatfs_open(struct filesystem* filesystem, const char* filename);

int fatfs_get(struct inode* inode, char* ptr);

int fatfs_read(struct inode* inode, void* ptr);

int fatfs_close(struct inode* inode);

int fatfs_left(struct inode* inode);

#endif

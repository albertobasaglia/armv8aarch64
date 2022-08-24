#ifndef FS_FAT_FS_H
#define FS_FAT_FS_H

#include <fs/fat.h>
#include <fs/fs.h>

struct fat_inode {
	struct fat16_dir_entry* fat16_dir_entry;
	struct fat_handle* fat_handle;
};

/*
 * Opens a file.
 * Allocates its inode.
 * */
struct inode* fatfs_open(struct fat_handle* fat_handle, const char* filename);

int fatfs_read(struct inode* inode, void* ptr);

int fatfs_close(struct inode* inode);

#endif

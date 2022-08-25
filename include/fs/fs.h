#ifndef FS_FS_H
#define FS_FS_H

#define FS_INODE_MAX 1024

#include <stddef.h>
#include <stdint.h>

struct inode;

struct inode_vtable {
	/*
	 * This reads data from the file. At the moment all data is dumped.
	 * */
	int (*read)(struct inode* impl_inode, void* ptr);

	/*
	 * This should close the file (and free the impl_inode memory)
	 * */
	int (*close)(struct inode* impl_inode);
	/*
	 * - write
	 * - create
	 * - delete
	 * */
};

struct inode {
	void* impl_inode;
	struct inode_vtable vtable;
};

struct filesystem {
	struct inode* (*open)(struct filesystem* filesystem,
			      const char* filename);
	void* arguments;
};

void fs_init_slab();

struct inode* fs_allocate_inode();

void fs_free_inode(struct inode* inode);

int fs_inode_read(struct inode* inode, const char* ptr);

int fs_inode_close(struct inode* inode);

#endif

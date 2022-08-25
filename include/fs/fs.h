#ifndef FS_FS_H
#define FS_FS_H

#define FS_INODE_MAX 1024

#include <stddef.h>
#include <stdint.h>

struct inode;

struct inode_vtable {
	/*
	 * Reads char form file.
	 * */
	int (*get)(struct inode* impl_inode, char* ptr);

	/*
	 * Puts a char in a file.
	 * */
	int (*put)(struct inode* impl_inode, char c);

	/*
	 * This should close the file (and free the impl_inode memory)
	 * */
	int (*close)(struct inode* impl_inode);
};

struct inode {
	void* impl_inode;
	struct inode_vtable vtable;
};

struct filesystem {
	struct inode* (*open)(struct filesystem* filesystem,
			      const char* filename);

	struct inode* (*create)(struct filesystem* filesystem,
				const char* filename);
	void* arguments;
};

void fs_init_slab();

struct inode* fs_allocate_inode();

void fs_free_inode(struct inode* inode);

int fs_inode_close(struct inode* inode);

int fs_inode_get(struct inode* inode, char* ptr);

int fs_inode_put(struct inode* inode, char c);

#endif

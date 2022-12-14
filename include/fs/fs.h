#ifndef FS_FS_H
#define FS_FS_H

#define FS_INODE_MAX 32

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

	/*
	 * Depends on the implementation.
	 * Returns total bytes left to read if possible.
	 * Elsewhere 0 if "buffer" is empty, 1 if chars available.
	 * */
	int (*left)(struct inode* impl_inode);
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

int fs_inode_left(struct inode* inode);

void fs_filesystem_setmain(struct filesystem* filesystem);

struct filesystem* fs_filesystem_getmain();

#endif

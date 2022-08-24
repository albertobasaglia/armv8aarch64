#include <fs/fat.h>
#include <fs/fat_fs.h>
#include <fs/fs.h>
#include <stdint.h>
#include <sysutils.h>

struct inode* fatfs_open(struct fat_handle* fat_handle, const char* filename)
{
	struct inode* inode = fs_allocate_inode();

	struct fat16_dir_entry* entry = fat_get_entry_by_file(fat_handle,
							      "init", "elf");

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

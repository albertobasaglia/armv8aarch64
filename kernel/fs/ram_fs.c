#include <fs/fs.h>
#include <fs/ram_fs.h>
#include <string.h>
#include <sysutils.h>

int ramfs_get(struct inode* inode, char* ptr);
int ramfs_close(struct inode* inode);
int ramfs_put(struct inode* inode, char c);

struct inode* ramfs_allocate_inode(struct ram_file* ram_file)
{
	struct inode* inode = fs_allocate_inode();
	struct ram_inode* ram_inode = kalloc(sizeof(struct ram_inode));

	inode->impl_inode = ram_inode;
	inode->vtable.close = ramfs_close;
	inode->vtable.get = ramfs_get;
	inode->vtable.put = ramfs_put;

	ram_inode->ram_file = ram_file;
	ram_inode->position = 0;

	return inode;
}

struct inode* ramfs_create_internal(struct filesystem* filesystem,
				    const char* filename)
{
	struct ram_filesystem* ram_filesystem = filesystem->arguments;
	struct ram_file* ptr = ram_filesystem->head;
	struct ram_file* setup;

	if (ptr == NULL) {
		ptr = kalloc(sizeof(struct ram_file));
		ram_filesystem->head = ptr;
		setup = ptr;
	} else {
		while (ptr->next != NULL) {
			ptr = ptr->next;
		}
		ptr->next = kalloc(sizeof(struct ram_file));
		setup = ptr->next;
	}

	setup->next = NULL;
	strcpy(setup->name, filename);
	setup->content = kalloc(RAMFILE_STARTSIZE);
	setup->content_alloc = RAMFILE_STARTSIZE;
	setup->content_fill = 0;

	return ramfs_allocate_inode(setup);
}

struct inode* ramfs_open_internal(struct filesystem* filesystem,
				  const char* filename)
{
	struct ram_filesystem* ram_filesystem = filesystem->arguments;
	struct ram_file* ptr = ram_filesystem->head;

	while (ptr != NULL) {
		if (strcmp(filename, ptr->name) == 0) {
			return ramfs_allocate_inode(ptr);
		}
	}

	return NULL;
}

struct filesystem* ramfs_createfs()
{
	struct filesystem* filesystem = kalloc(sizeof(struct filesystem));

	struct ram_filesystem* ram_filesystem = kalloc(
	    sizeof(struct ram_filesystem));

	filesystem->create = ramfs_create_internal;
	filesystem->open = ramfs_open_internal;

	filesystem->arguments = ram_filesystem;

	return filesystem;
}

void ramfs_deletefs(struct filesystem* filesystem)
{
	kfree(filesystem->arguments);
	kfree(filesystem);
}

int ramfs_get(struct inode* inode, char* ptr)
{
	struct ram_inode* ram_inode = (struct ram_inode*)inode->impl_inode;
	struct ram_file* ram_file = ram_inode->ram_file;

	if (ram_inode->position < ram_file->content_fill) {
		*ptr = ram_file->content[ram_inode->position];
		ram_inode->position++;
		return 1;
	}

	return 0;
}

int ramfs_close(struct inode* inode)
{

	struct ram_inode* ram_inode = (struct ram_inode*)inode->impl_inode;
	struct ram_file* ram_file = ram_inode->ram_file;

	kfree(ram_inode);
	fs_free_inode(inode);

	return 0;
}

int ramfs_put(struct inode* inode, char c)
{
	struct ram_inode* ram_inode = (struct ram_inode*)inode->impl_inode;
	struct ram_file* ram_file = ram_inode->ram_file;

	if (ram_inode->position >= ram_file->content_alloc)
		return 0; // TODO We need to allocate a bigger buffer

	ram_file->content[ram_inode->position] = c;
	ram_inode->position++;
	ram_file->content_fill++;

	return 1;
}

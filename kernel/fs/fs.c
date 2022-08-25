#include <fs/fs.h>
#include <slab.h>
#include <sysutils.h>

static struct slab inode_slab;

void fs_init_slab()
{
	void* slab_memory = kalloc(
	    slab_get_needed_size(sizeof(struct inode), FS_INODE_MAX));

	inode_slab = slab_create(sizeof(struct inode), FS_INODE_MAX,
				 slab_memory);
}

struct inode* fs_allocate_inode()
{
	return slab_allocate(&inode_slab);
}

void fs_free_inode(struct inode* inode)
{
	slab_free(&inode_slab, inode);
}

int fs_inode_get(struct inode* inode, char* ptr)
{
	return inode->vtable.get(inode, ptr);
}

int fs_inode_put(struct inode* inode, char c)
{
	return inode->vtable.put(inode, c);
}

int fs_inode_close(struct inode* inode)
{
	return inode->vtable.close(inode);
}

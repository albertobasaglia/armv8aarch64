#ifndef FS_RAM_FS
#define FS_RAM_FS

#include <fs/fs.h>

#define RAMFILE_MAXNAME   30
#define RAMFILE_STARTSIZE 4096

struct ram_file {
	char name[RAMFILE_MAXNAME];
	char* content;
	int content_alloc;
	int content_fill;
	struct ram_file* next;
};

struct ram_filesystem {
	struct ram_file* head;
};

struct ram_inode {
	struct ram_file* ram_file;
	int position;
};

struct filesystem* ramfs_createfs();
void ramfs_deletefs(struct filesystem* filesystem);

#endif

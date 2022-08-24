#include "mp/sched.h"
#include <block.h>
#include <elf/elf.h>
#include <elf/filebuffer.h>
#include <fs/fat.h>
#include <mp/job.h>
#include <paging.h>
#include <stddef.h>
#include <stdint.h>
#include <sysutils.h>
#include <virtioblk.h>

#include <gic.h>
#include <heap.h>
#include <log.h>
#include <slab.h>
#include <string.h>
#include <timer.h>
#include <uart.h>

extern char USERSTACK_END;
extern char EXCEPTION_TABLE;
extern char HEAP_START;

static struct slab paging_slab;
static struct paging_manager paging_kernel;

void handle_timer_int(int id, void* arg)
{
	klog("Ack TIMER");
	timer_write_tval(timer_getfrequency());
}

void enable_gic()
{
	gic_distributor_enable();
	gic_redistributor_wake();
	gic_interface_init();
	gic_interface_enablegroups();
	gic_interface_setprioritymask(0xff);
	gic_redistributor_enable_id(30);
	// 79 is the virtioblk interrupt
	gic_distributor_set_priority(79, 0);
	gic_distributor_set_target(79, 1);
	gic_distributor_enable_id(79);
	gic_distributor_set_group(79, 0);
	gic_redistributor_set_handler(30, handle_timer_int, NULL);
}

void setup_heap()
{
	int heap_size_blocks = 0x1000; // 16MB
	size_t table_address = (size_t)&HEAP_START;
	size_t heap_address = table_address + heap_table_size(heap_size_blocks);
	sysutils_kernel_heap_create(table_address, heap_address,
				    heap_size_blocks);
}

void enable_paging_test()
{
	paging_manager_init(&paging_kernel, &paging_slab);
	paging_manager_map_kernel(&paging_kernel);
	paging_manager_apply(&paging_kernel);
	klog("Paging enabled");
}

void usermode()
{
	// Read the elf file from the disk
	struct virtioblk disk;
	if (disk_init(&disk, 31) == 0) {
		klog("Disk init is successful");
	} else {
		klog("Disk init error!");
	}

	struct block disk_block = disk_register_block_device(&disk);

	struct fat_handle fat = fat_load(&disk_block);
	/* fat_debug_info(&fat); */

	struct fat16_dir_entry* entry = fat_get_entry_by_file(&fat, "init",
							      "elf");
	/* klogf("INIT.ELF filesize is %q", entry->filesize_bytes); */

	char* init_mem = kalloc(entry->filesize_bytes);
	fat_read_entry(&fat, entry, init_mem);

	struct filebuffer fb = filebuffer_frombuffer(init_mem);

	ELF elf = elf_fromfilebuffer(fb);
	elf_alloc_and_parse(&elf);

	int ph_count = elf_get_programheader_count(&elf);
	/* klogf("Program header count is %q", ph_count); */
	ElfN_Phdr* program_header = elf_get_programheader_byid(&elf, 0);
	/* klogf("Program header 0 size is %q", program_header->p_filesz); */

	// Create the job
	struct paging_manager pm;
	paging_manager_init(&pm, &paging_slab);
	paging_manager_map_kernel(
	    &pm); // every userprocess has the kernel mapped in!
	paging_manager_map_1gb(&pm, 0x80000000, 0x80000000, 1, 0);

	paging_manager_map_1gb(&paging_kernel, 0x80000000, 0x80000000, 0, 0);
	elf_dump_program_content(&elf, program_header, (void*)0x80000000);
	elf_free(&elf);

	klogf("Jumping to 0x%x (entry point)", elf.header.e_entry);
	job_init_slab(32);
	struct job* init_job = job_init_and_create(elf.header.e_entry,
						   0x80001000, "init", &pm);
	klogf("Free blocks: %q/%q", sysutils_kernel_heap_get_free_count(),
	      sysutils_kernel_heap_get_total_count());
	scheduling_register_routine();
	sysutils_jump_eret_usermode(init_job);
}

void debug_paging()
{
	paging_manager_map_page(&paging_kernel, 0x100000000, 0x40000000, 0, 0);
	paging_manager_map_page(&paging_kernel, 0x100001000, 0x40000000, 0, 0);
	char* a = (char*)0x100001005;
	char* b = (char*)0x40000005;
	*a = 'a';
	put_char(*b);
	put_char('\n');
}

void start()
{
	sysutils_set_vbar((uint64_t)&EXCEPTION_TABLE);
	setup_heap();

	void* slab_memory = kalloc(
	    slab_get_needed_size(sizeof(union page_table_store), 128));

	klogf("Slab memory allocated at 0x%x", slab_memory);

	paging_slab = slab_create(sizeof(union page_table_store), 128,
				  slab_memory);

	sysutils_mask_fiq(false);
	sysutils_mask_irq(false);

	klog("Kernel started");
	enable_gic();
	klog("GIC enabled");

	timer_write_tval(timer_getfrequency());
	timer_enable();

	enable_paging_test();

	/* debug_paging(); */
	usermode();
}

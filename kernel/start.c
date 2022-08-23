#include "block.h"
#include "fs/fat.h"
#include "job.h"
#include "paging.h"
#include "sysutils.h"
#include "virtioblk.h"
#include <elf/elf.h>
#include <elf/filebuffer.h>
#include <stddef.h>
#include <stdint.h>

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

struct slab paging_slab;
struct paging_manager paging_kernel;

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
	gic_distributor_set_priority(79, 0);
	gic_distributor_set_target(79, 1);
	gic_distributor_enable_id(79);
	gic_distributor_set_group(79, 0);
	gic_redistributor_set_handler(30, handle_timer_int, NULL);
}

void setup_heap()
{
	int heap_size_blocks = 1024; // 4MB
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

void jump_usermode()
{
	struct paging_manager pm;
	paging_manager_init(&pm, &paging_slab);
	paging_manager_map_kernel(
	    &pm); // every userprocess has the kernel mapped in!
	paging_manager_map_1gb(&pm, 0x80000000, 0x80000000, 1, 0);
	// TODO:
	// - load executable section in memory (in user memory possibly!)
	// - map it
	// - execute it
	/* struct job user_job = job_create((uint64_t)init, 0, "init", &pm); */
	/* sysutils_jump_eret_usermode(&user_job); */
}

void usermode()
{
	struct virtioblk disk;
	if (disk_init(&disk, 31) == 0) {
		klog("Disk init is successful");
	} else {
		klog("Disk init error!");
	}

	struct block disk_block = disk_register_block_device(&disk);

	struct fat_handle fat = fat_load(&disk_block);
	fat_debug_info(&fat);

	struct fat16_dir_entry* entry = fat_get_entry_by_file(&fat, "init",
							      "elf");
	klogf("INIT.ELF filesize is %q", entry->filesize_bytes);

	char* init_mem = kalloc(entry->filesize_bytes);
	fat_read_entry(&fat, entry, init_mem);

	struct filebuffer fb = filebuffer_frombuffer(init_mem);

	ELF elf = elf_fromfilebuffer(fb);
	elf_parseheader(&elf);
	elf.section_headers = kalloc(elf_get_sectionheaders_bytes(&elf));
	elf_load_sectionheaders(&elf);
	elf.program_headers = kalloc(elf_get_programheaders_bytes(&elf));
	elf_load_programheaders(&elf);

	int ph_count = elf_get_programheader_count(&elf);
	klogf("Program header count is %q", ph_count);
	ElfN_Phdr* program_header = elf_get_programheader_byid(&elf, 0);
	klogf("Program header 0 size is %q", program_header->p_filesz);

	struct paging_manager pm;
	paging_manager_init(&pm, &paging_slab);
	paging_manager_map_kernel(
	    &pm); // every userprocess has the kernel mapped in!
	paging_manager_map_1gb(&pm, 0x80000000, 0x80000000, 1, 0);

	paging_manager_map_1gb(&paging_kernel, 0x80000000, 0x80000000, 0, 0);

	elf_dump_program_content(&elf, program_header, (void*)0x80000000);
	klog("Loaded program");

	klogf("Jumping to 0x%x (entry point)", elf.header.e_entry);
	struct job init_job = job_create(elf.header.e_entry, 0x80001000, "init",
					 &pm);
	sysutils_jump_eret_usermode(&init_job);
}

void start()
{
	sysutils_set_vbar((uint64_t)&EXCEPTION_TABLE);
	setup_heap();

	void* slab_memory = kalloc(
	    slab_get_needed_size(sizeof(struct page_table_store), 128));

	klogf("Slab memory allocated at 0x%x", slab_memory);

	paging_slab = slab_create(sizeof(struct page_table_store), 128,
				  slab_memory);

	sysutils_mask_fiq(false);
	sysutils_mask_irq(false);

	klog("Kernel started");
	enable_gic();
	klog("GIC enabled");

	timer_write_tval(timer_getfrequency());
	timer_enable();

	enable_paging_test();

	klog("Kernel finished");

	usermode();
}

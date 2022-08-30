#include <fs/fs.h>
#include <sysutils.h>
#include <gic.h>
#include <log.h>
#include <mp/job.h>
#include <stdbool.h>
#include <timer.h>
#include <uart.h>

#include <exceptions.h>
#include <stddef.h>
#include <stdint.h>

#define REG(x) x30[30 - x]

/*
 * TODO
 * This should be moved to the syscall table.
 * */
void exceptions_internal_open(uint64_t* x30)
{
	struct filesystem* fs = fs_filesystem_getmain();
	struct inode* inode = fs->open(fs, (char*)REG(0));

	struct job* job = job_get_current();

	int fd = job_add_file(job, inode);
	REG(0) = fd;
}

void exceptions_internal_get(uint64_t* x30)
{
	struct filesystem* fs = fs_filesystem_getmain();
	struct job* job = job_get_current();

	int fd = REG(0);

	struct inode* inode = job_get_file(job, fd);

	if (inode == NULL)
		goto err;

	char c;
	fs_inode_get(inode, &c);

	REG(0) = c;

	return;
err:
	REG(0) = -1;
}

bool exceptions_handle_syscall(uint16_t imm, uint64_t* x30)
{
	int res;
	uint64_t lr, spsr;
	asm volatile("mrs %0, ELR_EL1\n"
		     "mrs %1, SPSR_EL1"
		     : "=r"(lr), "=r"(spsr));

	sysutils_mask_fiq(false);
	if (imm == 10) {
		// syscall exit: hang
		klog("HANG");
		while (1)
			;
	} else if (imm == 11) {
		klogf("'%s': '%s'", job_get_current()->name, REG(0));
		res = 1;
		goto exit;
	} else if (imm == 20) {
		exceptions_internal_open(x30);
		res = 1;
		goto exit;
	} else if (imm == 21) {
		exceptions_internal_get(x30);
		res = 1;
		goto exit;
	}
exit:
	asm volatile("msr ELR_EL1, %0\n"
		     "msr SPSR_EL1, %1" ::"r"(lr),
		     "r"(spsr));
	return res;
}

void exceptions_distributor(uint64_t* x30)
{
	/* for (int i = 0; i < 31; i++) { */
	/* 	klogf("x%q: %x", i, x30[30 - i]); */
	/* } */
	uint64_t pc;
	asm volatile("mrs %0, ELR_EL1" : "=r"(pc));
	/* klogf("Program counter: 0x%x", pc); */
	uint64_t esr = exceptions_getesr();
	/* klogf("ESR: 0x%x", esr); */
	uint64_t ec = (esr >> ESR_EC_OFFSET) & ESR_EC_MASK;
	if (ec == ESR_EC_DATABT_SAME) {
		klog("Data abort from same EL");
	} else if (ec == ESR_EC_DATABT_LOWER) {
		klog("Data abort from lower EL");
	} else if (ec == ESR_EC_SVC64) {
		uint16_t imm16 = esr & ESR_ISS_SVC_IMM16;
		/* klogf("Requested system call imm16: %q", imm16); */
		bool res = exceptions_handle_syscall(imm16, x30);
		if (!res) {
			klogf("Unhandled syscall (imm=%q)", imm16);
		}
	} else {
		klogf("Unhandled exception (esr=0x%x)", esr);
		while (1)
			;
	}
}

uint64_t* exceptions_context_switch_x30;

void exceptions_handle_fiq(uint64_t* x30)
{
	int intid = gic_interface_read_and_ack_group0();

	if (intid == 30)
		exceptions_context_switch_x30 = x30;

	Handler* handler = gic_redistributor_get_handler(intid);
	if (handler != NULL) {
		void* param = gic_redistributor_get_handler_param(intid);
		handler(intid, param);
	} else {
		put_string("Unhandled interrupt\n");
	}
	gic_interface_end_of_interrupt_group0(intid);
}

uint64_t exceptions_getesr()
{
	uint64_t esr;
	asm volatile("mrs %0, ESR_EL1" : "=r"(esr));
	return esr;
}

#include <gic.h>
#include <heap.h>
#include <stddef.h>
#include <sysutils.h>
#include <uart.h>
#include <virtioblk.h>

#include <stdint.h>
#include <string.h>

#define mb() asm volatile("dsb 0b1111" ::: "memory");

uint32_t disk_read_reg(struct virtioblk* disk, int offset)
{
	return *((uint32_t*)(disk->mmio + offset));
}

void disk_set_reg(struct virtioblk* disk, int offset, uint32_t value)
{
	*((volatile uint32_t*)(disk->mmio + offset)) = value;
}

int disk_check_magic(struct virtioblk* disk)
{
	return disk_read_reg(disk, VIRTIO_MAGIC_OFFSET) ==
	       VIRTIO_DISK_MAGIC_VALUE;
}

int disk_get_version(struct virtioblk* disk)
{
	return disk_read_reg(disk, VIRTIO_VERSION_OFFSET);
}

int disk_get_device_id(struct virtioblk* disk)
{
	return disk_read_reg(disk, VIRTIO_DEVICEID_OFFSET);
}

int disk_init(struct virtioblk* disk, int offset)
{
	disk->mmio = (char*)(VIRTIO_DISK_BASE_ADDRESS +
			     offset * VIRTIO_DISK_SIZE);
	if (!disk_check_magic(disk)) {
		// no magic number found
		return -1;
	}
	if (disk_get_version(disk) != 2) {
		// device version
		return -2;
	}
	if (disk_get_device_id(disk) == 0) {
		// no device id
		return -3;
	}

	disk_set_reg(disk, VIRTIO_DEVICE_STATUS_OFFSET, 0);
	mb();
	disk_set_reg(disk, VIRTIO_DEVICE_STATUS_OFFSET, VIRTIO_STATUS_ACK);
	mb();
	disk_set_reg(disk, VIRTIO_DEVICE_STATUS_OFFSET, VIRTIO_STATUS_DRIVER);
	mb();
	uint32_t feat = disk_read_reg(disk, VIRTIO_DEV_FEATURES_OFFSET);
	mb();

	disk_set_reg(disk, VIRTIO_DEV_FEATURESSELECT_OFFSET,
		     feat); // set all the features
	mb();
	disk_set_reg(disk, VIRTIO_DEVICE_STATUS_OFFSET,
		     VIRTIO_STATUS_FEATURES_OK);
	if (disk_read_reg(disk, VIRTIO_DEVICE_STATUS_OFFSET) !=
	    VIRTIO_STATUS_FEATURES_OK) {
		// device did not accept our features
		return -4;
	}
	mb();
	// queue 0x0
	disk_set_reg(disk, VIRTIO_QUEUE_INDEX_OFFSET, 0);
	if (disk_read_reg(disk, VIRTIO_QUEUE_READY_OFFSET) != 0x0) {
		// queue already in use
		return -5;
	}
	int queue_size = QUEUE_SIZE;
	disk_set_reg(disk, VIRTIO_QUEUE_SIZE_OFFSET,
		     queue_size); // set queue size to 8
	disk->descriptors = kalloc(sizeof(Virtq_desc) * queue_size);
	disk->avail = kalloc(sizeof(Virtq_avail));
	disk->used = kalloc(sizeof(Virtq_used));

	// TODO split the addresses!
	disk_set_reg(disk, VIRTIO_QUEUE_DESCRIPTOR_LOW_OFFSET,
		     (uint64_t)disk->descriptors);
	disk_set_reg(disk, VIRTIO_QUEUE_DESCRIPTOR_HIGH_OFFSET, 0);
	disk_set_reg(disk, VIRTIO_QUEUE_AVAILABLE_LOW_OFFSET,
		     (uint64_t)disk->avail);
	disk_set_reg(disk, VIRTIO_QUEUE_AVAILABLE_HIGH_OFFSET, 0);
	disk_set_reg(disk, VIRTIO_QUEUE_USED_LOW_OFFSET, (uint64_t)disk->used);
	disk_set_reg(disk, VIRTIO_QUEUE_USED_HIGH_OFFSET, 0);

	mb();

	disk_set_reg(disk, VIRTIO_QUEUE_READY_OFFSET, 1); // queue is ready
	// setup virtqueues:
	disk_set_reg(disk, VIRTIO_DEVICE_STATUS_OFFSET,
		     VIRTIO_STATUS_DRIVER_OK);

	mb();
	return 0;
}

/*
 * This can process only one request at the time.
 * To make this work with a buffer, a strategy to keep track of empty spaces
 * in the descriptor table is needed. Now I use only the first 3 slots.
 * */
int disk_create_request_sync(struct virtioblk* disk,
			     int write,
			     char* data,
			     int sector)
{
	Virtio_blk_req req;
	req.sector = sector;
	req.type = write;

	// get this from the current descriptor
	int desc1, desc2, desc3;
	desc1 = 0;
	desc2 = desc1 + 1;
	desc3 = desc2 + 1;

	// header descriptor
	disk->descriptors[desc1].addr = (le64)&req;
	disk->descriptors[desc1].len = 16;
	disk->descriptors[desc1].flags = VIRTQ_DESC_F_NEXT;
	disk->descriptors[desc1].next = desc2;

	// data descriptor
	disk->descriptors[desc2].addr = (le64)(data);
	disk->descriptors[desc2].len = 512;
	int data_flags = VIRTQ_DESC_F_NEXT;
	if (!write)
		data_flags |= VIRTQ_DESC_F_WRITE;
	disk->descriptors[desc2].flags = data_flags;
	disk->descriptors[desc2].next = desc3;

	// status descriptor
	disk->descriptors[desc3].addr = ((le64)((char*)&req) + 16);
	disk->descriptors[desc3].len = 1;
	disk->descriptors[desc3].flags = VIRTQ_DESC_F_WRITE;

	disk->avail->ring[disk->avail->idx] = desc1;
	disk->avail->idx += 1;
	disk->avail->flags = 0;

	mb();

	volatile Sync_read* sync = kalloc(sizeof(Sync_read));
	sync->disk = disk;
	sync->done = 0;

	Handler* handler = disk_handle_interrupt_sync;
	void* param = (void*)sync;

	gic_redistributor_set_handler(79, handler, param);

	disk_set_reg(disk, VIRTIO_QUEUE_NOTIFY_OFFSET, 0);
	mb();

	while (!sync->done) {
	}

	kfree((void*)sync);
	return 0;
}

void disk_handle_used(struct virtioblk* disk)
{
	// TODO handle used ring
}

void disk_handle_interrupt_sync(int id, void* argument)
{
	Sync_read* sync = (Sync_read*)argument;
	struct virtioblk* disk = sync->disk;
	int interrupt = disk_read_reg(disk, VIRTIO_INTERRUPT_STATUS_OFFSET);
	disk_set_reg(disk, VIRTIO_INTERRUPT_ACK_OFFSET, interrupt);
	disk_handle_used(disk);
	sync->done = 1;
	disk_handle_used(disk);
}

int disk_block_read(void* ptr, size_t n, void* args)
{
	struct virtioblk* virtioblk = (struct virtioblk*)args;
	int res = disk_create_request_sync(virtioblk, 0, (char*)ptr, n);
	return res;
}

int disk_block_write(const void* ptr, size_t n, void* args)
{
	struct virtioblk* virtioblk = (struct virtioblk*)args;
	int res = disk_create_request_sync(virtioblk, 1, (char*)ptr, n);
	return res;
}

struct block disk_register_block_device(struct virtioblk* disk)
{
	struct block block = {
	    .args = (void*)disk,
	    .read = disk_block_read,
	    .write = disk_block_write,
	    .read_only = false,
	    .sector_size = 512,
	};
	return block;
}

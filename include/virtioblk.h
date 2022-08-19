#ifndef VIRTIO_BLK
#define VIRTIO_BLK

#include <stdint.h>

#define VIRTIO_DISK_BASE_ADDRESS            0xa000000ull
#define VIRTIO_DISK_SIZE                    0x200
#define VIRTIO_DISK_MAGIC_VALUE             0x74726976

#define QUEUE_SIZE                          1024

/*
 * Memory mapped control registers, from paragraph 4.4.2
 * */
#define VIRTIO_MAGIC_OFFSET                 0x0
#define VIRTIO_VERSION_OFFSET               0x4
#define VIRTIO_DEVICEID_OFFSET              0x8
#define VIRTIO_VENDORID_OFFSET              0xc
#define VIRTIO_DEV_FEATURES_OFFSET          0x10
#define VIRTIO_DEV_FEATURESSELECT_OFFSET    0x14
#define VIRTIO_DRIV_FEATURES_OFFSET         0x20
#define VIRTIO_DRIV_FEATURESSELECT_OFFSET   0x24
#define VIRTIO_QUEUE_INDEX_OFFSET           0x30
#define VIRTIO_MAX_QUEUE_SIZE_OFFSET        0x34
#define VIRTIO_QUEUE_SIZE_OFFSET            0x38
#define VIRTIO_QUEUE_READY_OFFSET           0x44
#define VIRTIO_QUEUE_NOTIFY_OFFSET          0x50
#define VIRTIO_INTERRUPT_STATUS_OFFSET      0x60
#define VIRTIO_INTERRUPT_ACK_OFFSET         0x64
#define VIRTIO_DEVICE_STATUS_OFFSET         0x70
#define VIRTIO_QUEUE_DESCRIPTOR_LOW_OFFSET  0x80
#define VIRTIO_QUEUE_DESCRIPTOR_HIGH_OFFSET 0x84
#define VIRTIO_QUEUE_AVAILABLE_LOW_OFFSET   0x90
#define VIRTIO_QUEUE_AVAILABLE_HIGH_OFFSET  0x94
#define VIRTIO_QUEUE_USED_LOW_OFFSET        0xa0
#define VIRTIO_QUEUE_USED_HIGH_OFFSET       0xa4
#define VIRTIO_CONFIG_OFFSET                0x100

#define VIRTIO_STATUS_ACK                   1
#define VIRTIO_STATUS_DRIVER                2
#define VIRTIO_STATUS_FAILED                128
#define VIRTIO_STATUS_FEATURES_OK           8
#define VIRTIO_STATUS_DRIVER_OK             4
#define VIRTIO_STATUS_NEEDS_RESET           64

typedef uint32_t le32;
typedef uint64_t le64;
typedef uint16_t le16;
typedef uint8_t u8;

// structures extracted from the reference
typedef struct {
	le64 capacity;
	le32 size_max;
	le32 seg_max;
	struct virtio_blk_geometry {
		le16 cylinders;
		u8 heads;
		u8 sectors;
	} __attribute__((packed)) geometry;
	le32 blk_size;
	struct virtio_blk_topology {
		// # of logical blocks per physical block (log2)
		u8 physical_block_exp;
		// offset of first aligned logical block
		u8 alignment_offset;
		// suggested minimum I/O size in blocks
		le16 min_io_size;
		// optimal (suggested maximum) I/O size in blocks
		le32 opt_io_size;
	} __attribute__((packed)) topology;
	u8 writeback;
} __attribute__((packed)) Virtio_blk_config;

typedef struct {
	/* Address (guest-physical). */
	le64 addr;
	/* Length. */
	le32 len;
/* This marks a buffer as continuing via the next field. */
#define VIRTQ_DESC_F_NEXT     1
/* This marks a buffer as device write-only (otherwise device read-only). */
#define VIRTQ_DESC_F_WRITE    2
/* This means the buffer contains a list of buffer descriptors. */
#define VIRTQ_DESC_F_INDIRECT 4
	/* The flags as indicated above. */
	le16 flags;
	/* Next field if flags & NEXT */
	le16 next;
} __attribute__((packed)) Virtq_desc;

typedef struct {
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
	le16 flags;
	le16 idx;
	le16 ring[QUEUE_SIZE];
	le16 used_event; /* Only if VIRTIO_F_EVENT_IDX */
} __attribute__((packed)) Virtq_avail;

typedef struct {
	/* Index of start of used descriptor chain. */
	le32 id;
	/* Total length of the descriptor chain which was used (written to) */
	le32 len;
} __attribute__((packed)) Virtq_used_elem;

typedef struct {
#define VIRTQ_USED_F_NO_NOTIFY 1
	le16 flags;
	le16 idx;
	Virtq_used_elem ring[QUEUE_SIZE];
	le16 avail_event; /* Only if VIRTIO_F_EVENT_IDX */
} __attribute__((packed)) Virtq_used;

typedef struct {
	uint32_t type;
	uint32_t reserved;
	uint64_t sector;
	/* uint8_t data[512]; */
	uint8_t status;
} __attribute__((packed)) Virtio_blk_req;

struct virtioblk {
	char* mmio;
	Virtq_desc* descriptors;
	Virtq_avail* avail;
	Virtq_desc* used;
};

typedef struct {
	struct virtioblk* disk;
	volatile uint8_t done;
} Sync_read;

/*
 * Driver functions
 * */
uint32_t read_reg(struct virtioblk* disk, int offset);

void set_reg(struct virtioblk* disk, int offset, uint32_t value);

int disk_check_magic(struct virtioblk* disk);

int disk_get_version(struct virtioblk* disk);

int disk_get_device_id(struct virtioblk* disk);

int disk_init(struct virtioblk* disk, int offset);

/*
 * write - 1 if write / 0 if read
 * data - pointer to sector
 * sector - sector on disk
 * */
int disk_create_request_sync(struct virtioblk* disk,
			     int write,
			     char* data,
			     int sector);

void disk_handle_interrupt_sync(int id, void* argument);

#endif

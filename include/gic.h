#ifndef GIC_H
#define GIC_H

// TODO read these from the DeviceTree
#include <stdint.h>
#define GIC_DIST       0x08000000
#define GIC_CPU        0x08010000
#define GIC_REDIST     0x080a0000
#define GIC_REDIST_SGI 0x080b0000

// Interrupt handler
typedef void(Handler)(int);

/*
 * Enables the distributor interface GICD
 * Enables groups 0 and 1
 * */
void gic_distributor_enable();

/*
 * Enables and wakes the redistributor interface GICR
 * */
void gic_redistributor_wake();

/*
 * Enables an interrupt on a redistributor
 *
 * @param id interrupt id
 * */
void gic_redistributor_enable_id(int id);

/*
 * Enables the interface to be controlled from
 * the system registers
 * */
void gic_interface_init();

/*
 * Sets the binary point register for group 0
 * */
void gic_interface_setbinarypoint0(uint8_t bp);

/*
 * Sets the binary point register for group 1
 * */
void gic_interface_setbinarypoint1(uint8_t bp);

/*
 * Sets the priority mask for the interface
 * */
void gic_interface_setprioritymask(uint8_t pmr);

/*
 * Enables groups 0 and 1 on the interface
 * */
void gic_interface_enablegroups();

uint32_t gic_interface_read_and_ack_group0();

uint32_t gic_interface_read_and_ack_group1();

void gic_interface_end_of_interrupt_group0(uint32_t intid);

void gic_interface_end_of_interrupt_group1(uint32_t intid);

/*
 * Sets handler for interrupt id
 *
 * NULL value is used to indicate unassigned handler
 * */
void gic_redistributor_set_handler(int intid, Handler* handler);

/*
 * Gets handler for interrupt id
 *
 * NULL value is used to indicate unassigned handler
 * */
Handler* gic_redistributor_get_handler(int intid);

void gic_redistributor_erase_handlers();

#endif

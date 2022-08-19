#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdint.h>

#define ESR_EC_OFFSET       26
#define ESR_EC_MASK         0x3f
#define ESR_EC_SVC64        0b010101
#define ESR_EC_INSABT_LOWER 0b100000
#define ESR_EC_INSABT_SAME  0b100001
#define ESR_EC_DATABT_LOWER 0b100100
#define ESR_EC_DATABT_SAME  0b100101
#define ESR_EC_UNK          0b000000

#define ESR_ISS_OFFSET      0
#define ESR_ISS_MASK        0x1ffffff

void exceptions_distributor();

uint64_t exceptions_getesr();

/*
 * At the current state this handles both the exceptions coming from the current
 * level and the ones coming from the lower (EL0) level.
 *
 * There are cases where the two should have different behaviors.
 * E.g: FIQ is generated from the generic timer:
 *  - Coming from EL0: trigger the context switch mechanism
 *  - Coming from EL1: just reload the timer since there is no usermode process
 *                     to switch
 * */
void exceptions_handle_fiq();

#endif

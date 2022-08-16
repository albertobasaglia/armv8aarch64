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

#endif

#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/*
 * @return generic timer frequency
 * */
uint32_t timer_getfrequency();

/*
 * Enables the generic timer
 * */
void timer_enable();

/*
 * Writes the TVAL for the timer
 * */
void timer_write_tval(uint32_t tval);

#endif

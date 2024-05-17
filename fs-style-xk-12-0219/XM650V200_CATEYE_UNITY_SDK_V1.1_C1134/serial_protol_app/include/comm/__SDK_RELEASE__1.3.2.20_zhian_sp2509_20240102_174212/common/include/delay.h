/*
 * Kneron Delay driver
 *
 * Copyright (C) 2019 Kneron, Inc. All rights reserved.
 *
 */

#ifndef __DELAY_H__
#define __DELAY_H__

/**
 * @brief microseconds delay for any cpu
 *
 * @param [in] usec number of microseconds
 * @return No
 */
void delay_us(unsigned int usec);

void delay_ms_enable(void);
void delay_ms_disable(void);
void delay_ms(unsigned int msec);

#endif

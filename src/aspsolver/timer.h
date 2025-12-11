/**
 * @file timer.h
 *
 * @brief Timers
 *
 * This header defines the timers which the simulator uses to monitor its internal behaviour
 *
 * SPDX-FileCopyrightText: 2008-2025 HPCS Group <rootsim@googlegroups.com>
 * SPDX-License-Identifier: GPL-3.0-only
 */
#pragma once

#include <stdint.h>

/** The type used to store results of timer related calls */
typedef uint_fast64_t timer_uint;

/**
 * @brief Get a new starting point for an time interval measure
 * @return a timer_uint value, a not meaningful value by itself
 *
 * The returned value can be used in conjunction with timer_value() to measure a time interval with microsecond
 * resolution
 */
static inline timer_uint timer_new(void);

/**
 * @brief Compute a time interval measure using a previous timer_uint value
 * @param start a timer_uint value obtained from a previous timer_new() call
 * @return a timer_uint value, the count of microseconds of the time interval
 */
static inline timer_uint timer_value(timer_uint start);


#include <sys/time.h>

static inline timer_uint timer_new(void)
{
	struct timeval tmptv;
	gettimeofday(&tmptv, NULL);
	return (timer_uint)tmptv.tv_sec * 1000000U + tmptv.tv_usec;
}

static inline timer_uint timer_value(timer_uint start)
{
	return timer_new() - start;
}

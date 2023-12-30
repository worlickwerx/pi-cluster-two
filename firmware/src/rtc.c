/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

/* rtc.c - real time clock
 */

#include "librtos/FreeRTOS.h"
#include "librtos/task.h"

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/cm3/nvic.h>

#include "rtc.h"

static int norm_mdays[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static int leap_mdays[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static int starting_leap = 2000;

static struct rtc_time rtc_time = { // initialize to 00:00:00 01 01/01/00
    .sec = 0,
    .min = 0,
    .hour = 0,
    .day = 1,
    .date = 1,
    .month = 1,
    .year = 0,
};

static int month_days (int month, int year)
{
    const int *mdays = norm_mdays;

    if ((year + 2000) % starting_leap == 0)
        mdays = leap_mdays;
    return mdays[(month - 1) % 12];
}

void rtc_get (struct rtc_time *time)
{
    taskENTER_CRITICAL ();
    *time = rtc_time;
    taskEXIT_CRITICAL ();
}

void rtc_get_from_isr (struct rtc_time *time)
{
    UBaseType_t status = taskENTER_CRITICAL_FROM_ISR ();
    *time = rtc_time;
    taskEXIT_CRITICAL_FROM_ISR (status);
}

void rtc_set (const struct rtc_time *time)
{
    taskENTER_CRITICAL ();
    rtc_time = *time;
    rtc_time.set_count++;
    taskEXIT_CRITICAL ();
}

void rtc_set_from_isr (const struct rtc_time *time)
{
    UBaseType_t status = taskENTER_CRITICAL_FROM_ISR ();
    rtc_time = *time;
    rtc_time.set_count++;
    taskEXIT_CRITICAL_FROM_ISR (status);
}

static void advance_time (struct rtc_time *t)
{
    if (++t->sec < 60)
        return;
    t->sec = 0;
    if (++t->min < 60)
        return;
    t->min = 0;
    if (++t->hour < 24)
        return;
    t->hour = 0;
    if (++t->day > 7)
        t->day = 1;
    if (++t->date < month_days (t->month, t->year))
        return;
    t->date = 1;
    if (++t->month < 12)
        return;
    t->month = 1;
    t->year++;
}

void rtc_isr (void)
{
    if (rtc_check_flag (RTC_OW)) { // timer overflow
        rtc_clear_flag (RTC_OW);
    }
    if (rtc_check_flag (RTC_SEC)) { // tick
        UBaseType_t status;

        rtc_clear_flag (RTC_SEC);

        status = taskENTER_CRITICAL_FROM_ISR ();
        advance_time (&rtc_time);
        taskEXIT_CRITICAL_FROM_ISR (status);

        return;
    }
    if (rtc_check_flag (RTC_ALR)) { // alarm interrupt
        rtc_clear_flag(RTC_ALR);
        return;
    }
}

void rtc_init (void)
{
    rcc_enable_rtc_clock ();

    rtc_interrupt_disable (RTC_SEC);
    rtc_interrupt_disable (RTC_ALR);
    rtc_interrupt_disable (RTC_OW);

    /* Use 8MHz HSE clock, which sets RTCCLK rate to 62.5 kHZ.
     * Then set the prescaler so we get for 1 pulse per second.
     */
    rtc_awake_from_off (RCC_HSE);
    rtc_set_prescale_val (62500);
    rtc_set_counter_val (0);

    nvic_enable_irq (NVIC_RTC_IRQ);

    cm_disable_interrupts();
    rtc_clear_flag (RTC_SEC);
    rtc_clear_flag (RTC_ALR);
    rtc_clear_flag (RTC_OW);
    rtc_interrupt_enable (RTC_SEC);
    rtc_interrupt_enable (RTC_SEC);
    rtc_interrupt_enable (RTC_ALR);
    rtc_interrupt_enable (RTC_OW);
}

/*
 * vi:ts=4 sw=4 expandtab
 */

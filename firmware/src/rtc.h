/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _FIRMWARE_RTC_H
#define _FIRMWARE_RTC_H

struct rtc_time {
    uint8_t sec;    // 00-59
    uint8_t min;    // 00-59
    uint8_t hour;   // 00-23
    uint8_t day;    // 01-07
    uint8_t date;   // 01-31
    uint8_t month;  // 01-12
    uint8_t year;   // 00-99

    uint8_t set_count; // for debugging
};

void rtc_set (const struct rtc_time *time);
void rtc_set_from_isr (const struct rtc_time *time);

void rtc_get (struct rtc_time *time);
void rtc_get_from_isr (struct rtc_time *time);

void rtc_init (void);

#endif /* !_FIRMWARE_RTC_H */

/*
 * vi:ts=4 sw=4 expandtab
 */

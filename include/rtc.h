#ifndef _RTC_H
#define _RTC_H

#include "stdint.h"

/* RTC时间结构体 */
struct rtc_time {
    int tm_sec;   /* 秒 - [0,59] */
    int tm_min;   /* 分 - [0,59] */
    int tm_hour;  /* 时 - [0,23] */
    int tm_mday;  /* 日 - [1,31] */
    int tm_mon;   /* 月 - [0,11] */
    int tm_year;  /* 年 - 从1900年开始 */
    int tm_wday;  /* 星期几 - [0,6] (星期日=0) */
    int tm_msec;  /* 毫秒 - [0,9] */
};

/* RTC地址常量 */
#define BAR_BASE               0x1FE2ULL  /* 0b1111111100010 */
#define RTC_REGISTER_BASE      ((BAR_BASE << 16) | (0x7 << 12) | (0x1 << 11))


/* 函数声明 */
void                            rtc_init(void);
int                             rtc_read_time(struct rtc_time *tm);
int                             rtc_set_time(struct rtc_time *tm);
uint64_t                        rtc_tm_to_sec(struct rtc_time *tm);
void                            rtc_sec_to_tm(uint64_t seconds, struct rtc_time *tm);

#endif /* _RTC_H */ 

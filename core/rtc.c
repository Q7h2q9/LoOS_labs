#include "rtc.h"
#include "klib.h"
#include "memlayout.h"

/* 初始化RTC */
void rtc_init(void) {
    return;
}

/* 从RTC读取时间 */
int rtc_read_time(struct rtc_time *tm) {
    return 0;
}

/* 设置RTC时间 */
int rtc_set_time(struct rtc_time *tm) {
    return 0;
}

/* 将RTC时间结构体转换为从1970年1月1日开始的秒数 */
uint64_t rtc_tm_to_sec(struct rtc_time *tm) {
    return 0;
}

/* 将秒数转换为RTC时间结构体 */
void rtc_sec_to_tm(uint64_t seconds, struct rtc_time *tm) {
    return;
}

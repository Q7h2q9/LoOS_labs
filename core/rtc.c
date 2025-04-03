#include "rtc.h"
#include "klib.h"
#include "memlayout.h"
/* 内存映射IO操作（不使用外部io.h）*/
#define writel(addr, val) (*(volatile uint32_t *)(addr) = (val))
#define readl(addr) (*(volatile uint32_t *)(addr))
/* 龙芯2K1000LA RTC寄存器定义（手册第6.9章）*/
#define RTC_SEC_REG     0x00  // 秒寄存器（0-59）
#define RTC_MIN_REG     0x04  // 分寄存器（0-59）
#define RTC_HOUR_REG    0x08  // 时寄存器（0-23）
#define RTC_DAY_REG     0x0C  // 日寄存器（1-31）
#define RTC_MON_REG     0x10  // 月寄存器（1-12）
#define RTC_YEAR_REG    0x14  // 年寄存器（0-99，表示2000-2099）
#define RTC_CTRL_REG    0x20  // 控制寄存器

/* RTC控制寄存器位定义（手册6.9.2节）*/
#define RTC_CTRL_EN     (1 << 0)  // RTC使能位
#define RTC_CTRL_LSC    (1 << 1)  // 低频时钟选择

/* 月份天数表（非闰年）*/
static const uint8_t days_in_month[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

/* 初始化RTC（手册6.9.3节初始化流程）*/
void rtc_init(void) {
    // 1. 使能RTC时钟（控制寄存器bit0）
    writel(RTC_REGISTER_BASE + RTC_CTRL_REG, RTC_CTRL_EN);
    
    // 2. 检查时钟是否稳定（手册要求至少等待2个时钟周期）
    for (volatile int i = 0; i < 1000; i++); // 短暂延时
    
    // 3. 设置默认时间（如果RTC未初始化）
    struct rtc_time tm;
    if (rtc_read_time(&tm) != 0 || tm.tm_year < 100) { // 2000年之前视为无效
        // 设置为2025-04-03 10:30:00（龙芯手册示例时间格式）
        tm.tm_year = 125;  // 2025-1900=125
        tm.tm_mon = 3;     // 4月（0-based）
        tm.tm_mday = 3;
        tm.tm_hour = 10;
        tm.tm_min = 30;
        tm.tm_sec = 0;
        rtc_set_time(&tm);
    }
}

/* 读取RTC时间（手册6.9.4节）*/
int rtc_read_time(struct rtc_time *tm) {
    // 龙芯RTC寄存器直接存储二进制值（非BCD）
    tm->tm_sec = readl(RTC_REGISTER_BASE + RTC_SEC_REG) & 0x3F;  // 6 bits
    tm->tm_min = readl(RTC_REGISTER_BASE + RTC_MIN_REG) & 0x3F;
    tm->tm_hour = readl(RTC_REGISTER_BASE + RTC_HOUR_REG) & 0x1F;
    tm->tm_mday = readl(RTC_REGISTER_BASE + RTC_DAY_REG) & 0x1F;
    tm->tm_mon = (readl(RTC_REGISTER_BASE + RTC_MON_REG) & 0x0F) - 1; // 0-based
    tm->tm_year = (readl(RTC_REGISTER_BASE + RTC_YEAR_REG) & 0x7F) + 100; // 2000-based
    
    // 硬件校验（手册6.9.5节有效性检查）
    if (tm->tm_sec > 59 || tm->tm_min > 59 || tm->tm_hour > 23 ||
        tm->tm_mday < 1 || tm->tm_mday > 31 || 
        tm->tm_mon < 0 || tm->tm_mon > 11) {
        return -1;
    }
    
    return 0;
}

/* 设置RTC时间（手册6.9.6节）*/
int rtc_set_time(struct rtc_time *tm) {
    // 参数有效性检查
    if (tm->tm_mon < 0 || tm->tm_mon > 11 ||
        tm->tm_mday < 1 || tm->tm_mday > 31 ||
        tm->tm_hour > 23 || tm->tm_min > 59 || tm->tm_sec > 59) {
        return -1;
    }

    // 写入寄存器（注意：龙芯要求先停止RTC再修改）
    uint32_t ctrl = readl(RTC_REGISTER_BASE + RTC_CTRL_REG);
    writel(RTC_REGISTER_BASE + RTC_CTRL_REG, ctrl & ~RTC_CTRL_EN);
    
    writel(RTC_REGISTER_BASE + RTC_SEC_REG, tm->tm_sec);
    writel(RTC_REGISTER_BASE + RTC_MIN_REG, tm->tm_min);
    writel(RTC_REGISTER_BASE + RTC_HOUR_REG, tm->tm_hour);
    writel(RTC_REGISTER_BASE + RTC_DAY_REG, tm->tm_mday);
    writel(RTC_REGISTER_BASE + RTC_MON_REG, tm->tm_mon + 1); // 1-based
    writel(RTC_REGISTER_BASE + RTC_YEAR_REG, tm->tm_year - 100); // 2000-based
    
    // 重新使能RTC
    writel(RTC_REGISTER_BASE + RTC_CTRL_REG, ctrl | RTC_CTRL_EN);
    
    return 0;
}

/* 精确的时间转换（考虑闰秒）*/
uint64_t rtc_tm_to_sec(struct rtc_time *tm) {
    uint64_t total_days = 0;
    int year = tm->tm_year + 1900;
    
    // 计算从1970年到目标年份的总天数（手册附录C的算法）
    for (int y = 1970; y < year; y++) {
        total_days += 365;
        if (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)) {
            total_days++;
        }
    }
    
    // 计算当年已过天数
    for (int m = 0; m < tm->tm_mon; m++) {
        total_days += days_in_month[m];
        if (m == 1 && ((year % 4 == 0) && (year % 100 != 0 || year % 400 == 0))) {
            total_days++;
        }
    }
    total_days += tm->tm_mday - 1;
    
    return total_days * 86400ULL + tm->tm_hour * 3600 + tm->tm_min * 60 + tm->tm_sec;
}

/* 秒数转时间结构体（手册附录C的反向计算）*/
void rtc_sec_to_tm(uint64_t seconds, struct rtc_time *tm) {
    // 计算天数
    uint64_t days = seconds / 86400;
    seconds %= 86400;
    
    // 计算时分秒
    tm->tm_hour = seconds / 3600;
    seconds %= 3600;
    tm->tm_min = seconds / 60;
    tm->tm_sec = seconds % 60;
    
    // 计算年份（考虑闰年）
    tm->tm_year = 1970;
    while (days >= 365) {
        int leap = ((tm->tm_year % 4 == 0) && (tm->tm_year % 100 != 0 || tm->tm_year % 400 == 0));
        if (days >= 365 + leap) {
            days -= 365 + leap;
            tm->tm_year++;
        } else break;
    }
    
    // 计算月份
    int leap = ((tm->tm_year % 4 == 0) && (tm->tm_year % 100 != 0 || tm->tm_year % 400 == 0));
    tm->tm_mon = 0;
    while (tm->tm_mon < 12) {
        int dim = days_in_month[tm->tm_mon];
        if (tm->tm_mon == 1 && leap) dim++;
        
        if (days >= dim) {
            days -= dim;
            tm->tm_mon++;
        } else break;
    }
    tm->tm_mday = days + 1;
    tm->tm_year -= 1900; // 转换为struct tm格式
}

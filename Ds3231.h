//
// Created by yufu on 2021/1/25.
//

#ifndef PICO_EXAMPLES_DS3231_H
#define PICO_EXAMPLES_DS3231_H
#include "stdio.h"
#include "hardware/i2c.h"
#include "define.h"
#include "string.h"

#define DS3231_REG_SECOND 	0x00
#define DS3231_REG_MINUTE 	0x01
#define DS3231_REG_HOUR 		0x02
#define DS3231_REG_DAY 			0x03
#define DS3231_REG_DATE 		0x04
#define DS3231_REG_MONTH 		0x05
#define DS3231_REG_YEAR 		0x06
#define DS3231_REG_A1S			0x07
#define DS3231_REG_A1M 			0x08
#define DS3231_REG_A1H 			0x09
#define DS3231_REG_A1D 			0x0A
#define	DS3231_REG_A2M 			0x0B
#define	DS3231_REG_A2H			0x0C
#define DS3231_REG_A2D			0x0D
#define DS3231_REG_CONTROL  0X0E
#define DS3231_REG_HTEMP		0x11
#define DS3231_REG_LTEMP		0x12
#define DS3231_REG_STATUS       0x0F
#define DS3231_STA_A1F          0x01
#define DS3231_STA_A2F          0x02
#define	Control_default	        0x20
//启动定时器，关闭电池供电方波使能，强制启动温度转换，SQW引脚输出1Hz方波，关闭闹钟中断使能
#define	Status_default		0x00	//关闭32.768K方波输出，清除闹钟标志位

TIME_RTC Read_RTC();

extern bool hourMode;
extern char meridiem[2][3];
extern uint8_t byteData[16];
extern char stateOfTime[3];
extern uint8_t DS3231_ReadReg[17];
void init_DS3231();
void Set_Time(uint8_t sec,uint8_t min,uint8_t hour, uint8_t dow,uint8_t dom,uint8_t month,uint8_t year);
void Show_Time();
void Set_alarm1_clock(uint8_t mode,uint8_t sec,uint8_t min,uint8_t hour,uint8_t data);
void  Set_alarm2_clock(uint8_t min,uint8_t hour,uint8_t date);
void DS3231_REG_Read();
uint8_t BCD_to_Byte(uint8_t value);
uint8_t decToBcd(int val);
void setClockMode(bool h12);
void FormatTime_mode();
void Ds3231_set_alarm_s(uint8_t hour, uint8_t min, uint8_t sec);
void Ds3231_SQW_enable(bool enable);
bool Ds3231_check_alarm();
void set_min(uint8_t min);
void set_hour(uint8_t hour);
void set_year(uint8_t year);
void set_dayofweekday(uint8_t dayofweek);
void set_dayofmouth(uint8_t dayofmouth);
void set_month(uint8_t mouth);
#endif //PICO_EXAMPLES_DS3231_H

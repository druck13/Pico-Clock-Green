//
// Created by yufu on 2021/1/25.
//
#include "Ds3231.h"

uint8_t DS3231_ReadReg[17];
bool hourMode;
char meridiem[2][3] = {"AM", "PM"};
char stateOfTime[3] = "";
uint8_t byteData[16];
uint8_t REG_ADDRESSES[17] = {	DS3231_REG_SECOND,
                                 DS3231_REG_MINUTE,
                                 DS3231_REG_HOUR,
                                 DS3231_REG_DAY,
                                 DS3231_REG_DATE,
                                 DS3231_REG_MONTH,
                                 DS3231_REG_YEAR,
                                 DS3231_REG_A1S,
                                 DS3231_REG_A1M,
                                 DS3231_REG_A1H,
                                 DS3231_REG_A1D,
                                 DS3231_REG_A2M,
                                 DS3231_REG_A2H,
                                 DS3231_REG_A2D,
                                 DS3231_REG_CONTROL,
                                 DS3231_REG_HTEMP,
                                 DS3231_REG_LTEMP	};
uint8_t BCD_to_Byte(uint8_t value)
{
    return ((((value & 0xf0) >> 4) * 10) + (value & 0x0f));
}
uint8_t decToBcd(int val)
{
    return (uint8_t)((val/10*16) + (val%10));
}
void ByteData()
{
    DS3231_REG_Read();
    for (int i = 0; i < 7; i++)
    {
        byteData[i] = BCD_to_Byte(DS3231_ReadReg[i]);
    }
}

void init_DS3231()
{
    unsigned char i;
    uint8_t val[2];
    val[0] = DS3231_REG_CONTROL;
    val[1] = Control_default;
    i2c_write_blocking(I2C_PORT,Address,val,2,false);
    val[0] = DS3231_REG_STATUS;
    val[1] = Status_default;
    i2c_write_blocking(I2C_PORT,Address,val,2,false);
}
void set_min(uint8_t min)
{
    uint8_t setMin[3] = {0x00,0x00,0x00};
    setMin[2] = decToBcd(min);
    i2c_write_blocking(I2C_PORT,Address,setMin,3,false);

}

void set_hour(uint8_t hour)
{
    uint8_t setHour[2] = {0x02,0x00};
    setHour[1] = decToBcd(hour);
    i2c_write_blocking(I2C_PORT,Address,setHour,2,false);

}
void set_year(uint8_t year)
{
    uint8_t setYear[2] = {0x06,0x00};
    setYear[1] = decToBcd(year);
    i2c_write_blocking(I2C_PORT,Address,setYear,2,false);

}

void set_dayofmouth(uint8_t dayofmouth)
{
    uint8_t setDom[2] = {0x04,0x00};
    setDom[1] = decToBcd(dayofmouth);
    i2c_write_blocking(I2C_PORT,Address,setDom,2,false);
}
void set_dayofweekday(uint8_t dayofweek)
{
    uint8_t setDow[2] = {0x03,0x00};
    setDow[1] = decToBcd(dayofweek);
    i2c_write_blocking(I2C_PORT,Address,setDow,2,false);
}
void set_month(uint8_t mouth)
{
    uint8_t setMouth[2] = {0x05,0x00};
    setMouth[1] = decToBcd(mouth);
    i2c_write_blocking(I2C_PORT,Address,setMouth,2,false);
}

void Set_Time(uint8_t sec,uint8_t min,uint8_t hour, uint8_t dow,uint8_t dom,uint8_t month,uint8_t year){
    uint8_t set_time[8];
    set_time[0]=0x00;
    set_time[1]=decToBcd(sec);
    set_time[2]=decToBcd(min);
    set_time[3]=decToBcd(hour);
    set_time[4]=decToBcd(dow);
    set_time[5]=decToBcd(dom);
    set_time[6]=decToBcd(month);
    set_time[7]=decToBcd(year);

    i2c_write_blocking(I2C_PORT,Address,set_time,8,false);


}
TIME_RTC Read_RTC()
{
    TIME_RTC timeRtc;
    unsigned  char RTC_buf[7];
    uint8_t val = 0x00;
    i2c_write_blocking(I2C_PORT, Address, &val, 1, true);
    i2c_read_blocking(I2C_PORT, Address, RTC_buf, 7, false);
    timeRtc.seconds=RTC_buf[0];
    timeRtc.minutes=RTC_buf[1];
    timeRtc.hour=RTC_buf[2];
    timeRtc.dayofweek=RTC_buf[3];
    timeRtc.dayofmonth=RTC_buf[4];
    timeRtc.month=RTC_buf[5];
    timeRtc.year=RTC_buf[6];
    return timeRtc;
}
void Ds3231_SQW_enable(bool enable)
{
    uint8_t control;
    uint8_t val[2];
    val[0] = DS3231_REG_CONTROL;
    i2c_write_blocking(I2C_PORT, Address, &val[0], 1, true);
    i2c_read_blocking(I2C_PORT,Address, &control,1,false);
    if (enable) {
        control |=  0b01000000; // set BBSQW to 1
        control &=  ~0b00000100; // set INTCN to 1
    }
    else {
        control &= ~0b01000000; // set BBSQW to 0
    }

    val[1] = control;
    i2c_write_blocking(I2C_PORT,Address,val,2,false);

}


void Set_alarm1_clock(uint8_t mode,uint8_t sec,uint8_t min,uint8_t hour,uint8_t date)
{

    uint8_t alarmSecond = decToBcd(sec);
    uint8_t alarmMinute = decToBcd(min);
    uint8_t alarmHour = decToBcd(hour);
    uint8_t alarmDate = decToBcd(date);
    switch(mode)
    {
        case ALARM_MODE_ALL_MATCHED:
            break;
        case ALARM_MODE_HOUR_MIN_SEC_MATCHED:
            alarmDate |= 0x80;
            break;
        case ALARM_MODE_MIN_SEC_MATCHED:
            alarmDate |= 0x80;
            alarmHour |= 0x80;
            break;
        case ALARM_MODE_SEC_MATCHED:
            alarmDate |= 0x80;
            alarmHour |= 0x80;
            alarmMinute |= 0x80;
            break;
        case ALARM_MODE_ONCE_PER_SECOND:
            alarmDate |= 0x80;
            alarmHour |= 0x80;
            alarmMinute |= 0x80;
            alarmSecond |= 0x80;
            break;
    }
    uint8_t start_address = DS3231_REG_A1S;
    uint8_t val[5]={start_address,alarmSecond,alarmMinute,alarmHour,alarmDate};
    i2c_write_blocking(I2C_PORT,Address,val,5,false);
    uint8_t addr_reg_val[2]= {DS3231_REG_CONTROL,0x00};

    i2c_write_blocking(I2C_PORT, Address,&addr_reg_val[0], 1, true);
    i2c_read_blocking(I2C_PORT,Address, &addr_reg_val[1],1,false);
    addr_reg_val[1] |= 0x01;
    addr_reg_val[1] |= 0x04;
    i2c_write_blocking(I2C_PORT, Address,addr_reg_val, 2, false);

}
void  Set_alarm2_clock(uint8_t min,uint8_t hour,uint8_t date)
{
    uint8_t alarmMinute = decToBcd(min);
    uint8_t alarmHour = decToBcd(hour);
    uint8_t alarmDate = decToBcd(date);
    alarmDate |= 0x80;
    uint8_t start_address = DS3231_REG_A2M;
    uint8_t val[4]={start_address,alarmMinute,alarmHour,alarmDate};
    i2c_write_blocking(I2C_PORT,Address,val,4,false);
    uint8_t addr_reg_val[2]= {DS3231_REG_CONTROL,0x00};

    i2c_write_blocking(I2C_PORT, Address,&addr_reg_val[0], 1, true);
    i2c_read_blocking(I2C_PORT,Address, &addr_reg_val[1],1,false);
    addr_reg_val[1] |= 0x02;
    addr_reg_val[1] |= 0x04;
    i2c_write_blocking(I2C_PORT, Address,addr_reg_val, 2, false);
}
bool Ds3231_check_alarm()
{
    uint8_t regVal[2] = {DS3231_REG_STATUS,0x00};
    bool res = false;
    i2c_write_blocking(I2C_PORT, Address,&regVal[0], 1, true);
    i2c_read_blocking(I2C_PORT,Address, &regVal[1],1,false);
    if(regVal[1] & DS3231_STA_A1F)
    {
        res = true;
        regVal[1] &= ~DS3231_STA_A1F;
        i2c_write_blocking(I2C_PORT, Address,regVal, 2, false);
    }if(regVal[1] & DS3231_STA_A2F)
    {
        res = true;
        regVal[1] &= ~DS3231_STA_A2F;
        i2c_write_blocking(I2C_PORT, Address,regVal, 2, false);
    }
    return res;
}
void DS3231_REG_Read()
{
    for (int i = 0; i < 16; i++)
    {
        i2c_write_blocking(I2C_PORT, Address, &REG_ADDRESSES[i], 1, true);
        i2c_read_blocking(I2C_PORT,Address, &DS3231_ReadReg[i],1,false);
    }
}
void FormatTime_mode() {
    ByteData();

    if ((DS3231_ReadReg[2] & 0x40) != 0) //判断12小时或24小时模式，bit6高位为12小时模式
    {
        hourMode = true;

        if ((DS3231_ReadReg[2] & 0x20) != 0) //判断AM/PM，bit5高位为PM
        {
            strcpy(stateOfTime, meridiem[1]);
        } else
            strcpy(stateOfTime, meridiem[0]);
    } else
        hourMode = false;

}

void setClockMode(bool h12)
{
    uint8_t val[2];
    val[0]= DS3231_REG_HOUR;
    if (h12)
    {

        i2c_write_blocking(I2C_PORT, Address, &val[0], 1, true);
        i2c_read_blocking(I2C_PORT, Address, &val[1], 1, false);
        val[1] = (val[1] | 0b01000000);
        i2c_write_blocking(I2C_PORT,Address,val,2,false);
    }
    else
    {
        i2c_write_blocking(I2C_PORT, Address, &val[0], 1, true);
        i2c_read_blocking(I2C_PORT, Address, &val[1], 1, false);
        val[1] = (val[1] & 0b10111111);
        i2c_write_blocking(I2C_PORT,Address,val,2,false);
    }
}







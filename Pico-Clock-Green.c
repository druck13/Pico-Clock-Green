/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "Ds3231.h"
#include "ziku.h"
unsigned char month_date[2][12]={{31,29,31,30,31,30,31,31,30,31,30,31},
                                {31,28,31,30,31,30,31,31,30,31,30,31}};//判断闰年和非闰年每个月的天数
unsigned char disp_buf[112];//缓冲区字节，对应24x8个点及滚动字节
unsigned char CS_cnt;//行选计数
unsigned char UP_id=0,UP_Key_flag=0,KEY_Set_flag=0,No_operation_flag=0,No_operation_count;//触发按键检测
unsigned char adc_light_flag = 0,adc_light_time_flag = 0,light_set = 0;
uint16_t adc_light,adc_light_count = 0; //设置自动亮度
unsigned char set_id=0,update_time = 0,scroll_start_count = 0,scroll_show_flag = 0,scroll_show_start =0;
unsigned char alarm_id = 0,alarm_flag = 0,beep_sta = 1,beep_flag=0,beep_on_flag = 0,beep_on_count = 0,scroll_flag = 0,scroll_sta = 0,scroll_count = 0,scroll_start = 0;//蜂鸣器及滚动
unsigned  char alarm_hour_temp = 0,alarm_min_temp = 0,alarm_hour_flag = 0,alarm_min_flag = 0,alarm_day_select_flag = 0,alarm_day_select = 1;
unsigned char Set_time_hour_flag = 0,Set_time_min_flag = 0,Set_time_year_flag = 0,Set_time_month_flag = 0,Set_time_dayofmonth_flag = 0,Set_hour_temp = 0,Set_min_temp = 0,change_time_flag = 0;
unsigned char alarm_select_flag = 0,alarm_open_flag = 0,alarm_select_sta = 0,alarm_open_sta = 0,hour_temp,min_temp,year_temp,month_temp,dayofmonth_temp,year_high_temp = 20;//闹钟及时间
unsigned char Min_count=0,alarm_star_flag = 0,Timing_show_count = 0,Timing_show_sec = 0;
uint16_t KEY_cnt=0,UP_cnt=0,Exit_cnt = 0,Flashing_count = 0,whole_year,adc_count = 0,write_flag = 0;
unsigned char Timing_mode_flag = 0,Timing_mode_sta = 2,Timing_min_flag = 0,Timing_sec_flag = 0,Timing_min_temp = 0,Timing_sec_temp = 0,Timing_DN_flag = 0,Timing_UP_Key_flag = 0,Timing_DN_close_flag = 0;//计时
unsigned char Time_set_mode_flag = 0,Time_set_mode_sta = 0,Full_time_flag = 0,Full_time_sta = 0,Full_time_alarm_count = 5;//整点报时、时间模式
char Time_buf[4];
unsigned char i,jr,save_buf,adc_show_flag = 0,adc_show_time = 6;
TIME_RTC Time_RTC;
#define BAUD_RATE 115200
unsigned char flag_Flashing[11]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
unsigned char temp_high,temp_low,temp_sta = 0,get_add_high = 0x11,get_add_low = 0x12;
void get_temperature();
void display_char(unsigned char x,unsigned char dis_char); //对需要显示的数据进行存放
void send_data(unsigned char data); 
void cls_disp(unsigned char x); //清数据
void select_weekday(unsigned char x); 
bool repeating_timer_callback_ms(struct repeating_timer *t); //1ms回调函数
bool repeating_timer_callback_s(struct repeating_timer *t);//1s 回调函数
bool repeating_timer_callback_us(struct repeating_timer *t);
void dis_SetMode(); //普通模式设置
void dis_Timing();  //计时模式设置
void dis_alarm(); //闹钟模式设置
void dis_scroll(); //滚动显示
uint16_t get_ads1015();
unsigned char get_month_date(uint16_t year,uint8_t month_cnt);//获取当年的月份天数
unsigned char get_weekday(uint16_t year,uint8_t month_cnt,uint8_t date_cnt);//基姆拉尔森计算公式:Weekday= (day+2*mon+3*(mon+1)/5+year+year/4-year/100+year/400) % 7
void show_adc();
void Alarm_set(uint8_t UP_DOWN_flag);
void Timing_set(uint8_t UP_DOWN_flag);
void Time_set(uint8_t UP_DOWN_flag);
void scroll_show_judge();
void beep_stop_judge();
void Flashing_start_judge();
void adc_show_count();
void beep_start_judge();
void EXIT();
void Special_Exit();
struct repeating_timer timer2;
int port_init(void) //GPIO初始化
{
    stdio_init_all();
    gpio_init(A0);
    gpio_init(A1);
    gpio_init(A2);

    gpio_init(SDI);
    gpio_init(LE);
    gpio_init(OE);
    gpio_init(CLK);
    gpio_init(SQW);
    gpio_init(BUZZ);
    gpio_init(SET_FUNCTION);
    gpio_init(UP);
    gpio_init(DOWN);


    gpio_set_dir(A0, GPIO_OUT);
    gpio_set_dir(A1, GPIO_OUT);
    gpio_set_dir(A2, GPIO_OUT);
    gpio_set_dir(SDI, GPIO_OUT);
    gpio_set_dir(OE, GPIO_OUT);
    gpio_set_dir(LE, GPIO_OUT);
    gpio_set_dir(CLK, GPIO_OUT);

    gpio_set_dir(SQW,GPIO_IN);
    gpio_set_dir(BUZZ,GPIO_OUT);

  

    //iic config

    i2c_init(I2C_PORT, 100000);
    gpio_set_function(SDA, GPIO_FUNC_I2C);
    gpio_set_function(SCL, GPIO_FUNC_I2C);
    gpio_pull_up(SDA);
    gpio_pull_up(SCL);
    
    


    //adc config
    adc_init();

    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(ADC_Light);
    adc_gpio_init(ADC_VCC);
    // Select ADC input 0 (GPIO26)
    adc_select_input(3);

}
int main(void)
{
    port_init();
    dis_C_flag;
	
    struct repeating_timer timer;
    struct repeating_timer timer1;

    add_repeating_timer_ms(1, repeating_timer_callback_ms, NULL, &timer);
    add_repeating_timer_ms(1000, repeating_timer_callback_s, NULL, &timer1);

    while(1)
    {
        if(adc_show_flag == 1)
        {
            show_adc();
            adc_show_time --;
            adc_show_flag = 0;
            if(adc_show_time == 0)
            {
                update_time = 1;
                gpio_set_dir(SET_FUNCTION,GPIO_IN);
                gpio_set_dir(UP,GPIO_IN);
                gpio_set_dir(DOWN,GPIO_IN);
                gpio_pull_up(SET_FUNCTION);
                gpio_pull_up(UP);
                gpio_pull_up(DOWN);
            }
            
        }
        if(KEY_Set_flag == 1) //设置按钮被单击，进入普通设置模式
        {
            No_operation_flag = 1;
            dis_SetMode();
            KEY_Set_flag = 0;
        }
        if(alarm_flag == 1)//长按进入闹钟设置
        {
            No_operation_flag = 1;
            dis_alarm();
            alarm_flag = 0;
        }
        if(UP_Key_flag == 1)
        {
            No_operation_flag = 1;
            dis_Timing();
            UP_Key_flag = 0;
        }
        if(update_time == 1)//刷新时间
        {
            Show_Time();
            update_time = 0;
        }
    }
    return 0;

}
bool repeating_timer_callback_us(struct repeating_timer *t) //us
{
    if(adc_light >2800)
    {
        light_set ++;
        if(light_set == 3)
        {
            OE_OPEN;
            light_set = 0;
        }
        else
        {
            OE_CLOSE;
        }
    }
    else
    {
        OE_OPEN;
    }
}


bool repeating_timer_callback_ms(struct repeating_timer *t) { //1ms进入一次
    unsigned char i;
	adc_show_count(); 
    beep_stop_judge();
    Flashing_start_judge(); 
    scroll_show_judge();
    if(gpio_get(SET_FUNCTION) == 0) //检测设置按钮是否被按下
    {
        KEY_cnt++;
    }
    else
    {
        if(KEY_cnt>50&&KEY_cnt < 300)//少于300ms判断为短按,设置键短按为模式切换
        {
            KEY_cnt = 0;
            if(alarm_id == 1)
                alarm_flag = 1;    //进入闹钟设置模式
            else if(UP_id ==1)
                UP_Key_flag = 1;  //进入计时设置模式
            else
                KEY_Set_flag = 1; //进入普通设置模式 （外部时钟设置、按键声音开关设置、滚动开关设置、时钟显示模式设置）
            beep_start_judge();
            EXIT();
            set_id++;	
        }
        else if(KEY_cnt >300&&set_id == 0) //大于300ms判断为长按
        {
            if(set_id == 0)
            {
                set_id ++;
                alarm_flag = 1;  //设置按键长按进入闹钟设置
                alarm_id = 1;
            }
            KEY_cnt = 0;
            beep_start_judge();
        }
        else KEY_cnt = 0;
    }
    if(gpio_get(UP) == 0) //+标号
    {
        UP_cnt++;
    }
    else
    {
        if(UP_cnt>50 && UP_cnt < 300)//少于300ms判断为短按,短按+、-键可以改变功能的状态
        {
            UP_cnt = 0;
            No_operation_count = 0;
            if(set_id == 0)
            {
                temp_sta = !temp_sta;
                if(temp_sta == 0)
                {
                    dis_C_flag;
                    dis_F_flag_close;
                }
                else
                {
                    dis_C_flag_close;
                    dis_F_flag;
                }
            }
            if(beep_flag == 1) //按键声音可设置标志位
            {
                beep_sta = !beep_sta;
            }
            
            beep_start_judge();
            if(scroll_flag == 1)//滚动开关可设置标志位
            {
                scroll_sta = !scroll_sta;
                if(scroll_sta != 0)
                {
                    dis_move_open;
                }
                else
                {
                    dis_move_close;
                }
            }
            Alarm_set(UP_flag);
			Timing_set(UP_flag);
			Time_set(UP_flag);
            if(Full_time_flag == 1)
            {
                Full_time_sta = !Full_time_sta;
            }
        }
        else if(UP_cnt >300&&set_id == 0) //大于300ms判断为长按
        {
            if(set_id == 0)
            {
                UP_Key_flag = 1;
                UP_id = 1;
                set_id ++ ;
            }
            UP_cnt = 0;
            beep_start_judge();
        }

        else UP_cnt = 0;
    }
    if(gpio_get(DOWN) == 0) //-标号
    {
       Exit_cnt++;
    }
    else
    {
        if(Exit_cnt>0&&Exit_cnt< 300)//少于300ms判断为短按,短按+、-键可以改变功能的状态
        {
            No_operation_count = 0;
            if(set_id == 0 )//自动亮度开关
            {
                adc_light_flag = !adc_light_flag;
                if(adc_light_flag !=0)
                {
                    dis_Auto_light;
                }
                else
                {
                    dis_Auto_light_close;
                }
            }
            if(beep_flag == 1)//按键声音可设置标志位
            {

                beep_sta = !beep_sta;
            }
            beep_start_judge();
            if(scroll_flag == 1)//滚动开关可设置标志位
            {
                scroll_sta = !scroll_sta;
                if(scroll_sta != 0)
                {
                    dis_move_open;
                }
                else
                {
                    dis_move_close;
                }
            }
            if(Full_time_flag == 1)
            {
                Full_time_sta = !Full_time_sta;
            }
			Alarm_set(DOWN_flag);
			Timing_set(DOWN_flag);
			Time_set(DOWN_flag);
            Exit_cnt = 0;
        }
        else if(Exit_cnt >300&&set_id != 0) //大于300ms判断为长按 ,任何设置模式下,可以退出设置模式
        {
            beep_start_judge();
            Special_Exit();
            //alarm_day_select_flag = 0;
            EXIT();
			set_id = 0;
            Exit_cnt = 0;
        }
        else  Exit_cnt = 0;
    }
    CS_cnt++;
    if(CS_cnt>7)
    {
        CS_cnt=0;
    }
    if(adc_light_flag != 0 && adc_light_time_flag == 1)
    {
        cancel_repeating_timer(&timer2);
        OE_CLOSE;

    }
    else
    {
        if(adc_light_time_flag == 1)
        {
            adc_light_time_flag = 0;
            cancel_repeating_timer(&timer2);
        }
        OE_CLOSE;
    }
    for(i=0;i<4;i++)
    {
        send_data(disp_buf[8*i+CS_cnt]);
    }
    LE_HIGH;
    LE_LOW;
    if(CS_cnt&0x01)A0_HIGH; else A0_LOW;

    if(CS_cnt&0x02)A1_HIGH; else A1_LOW;

    if(CS_cnt&0x04)A2_HIGH; else A2_LOW;

    if(adc_light_flag != 0)
    {
        adc_light_time_flag = 1;
        add_repeating_timer_us(150, repeating_timer_callback_us, NULL, &timer2);
    }
    else
    {
        OE_OPEN;
    }
    return true;
}
bool repeating_timer_callback_s(struct repeating_timer *t)//1s 进入一次
{

    Min_count++;
    if(alarm_star_flag == 1)
    {
        gpio_put(BUZZ,0);
        alarm_star_flag = 0;
    }

    if(Min_count==60)//每分钟刷新一次时间
    {
        if(alarm_open_sta != 0)
        {
            if (Time_RTC.dayofweek == alarm_day_select && Ds3231_check_alarm() == true)
            {
                alarm_star_flag = 1;
                gpio_put(BUZZ, 1);
            }
        }

        if(No_operation_flag == 0 && scroll_start_count == 0 && adc_show_time == 0){
            update_time = 1;
        }
        Min_count = 0;
        Full_time_alarm_count = 5;
    }
    if(No_operation_flag == 1)
    {
        No_operation_count++;
        if(No_operation_count == 10 )//10秒内无任何操作则退出设置模式
        {
            Special_Exit();
            EXIT();
			set_id = 0;
            if(alarm_select_sta == 0 && alarm_open_sta !=0 && alarm_day_select_flag == 1)
            {
                Set_alarm1_clock(ALARM_MODE_HOUR_MIN_SEC_MATCHED,00,alarm_min_temp,alarm_hour_temp,alarm_day_select);
            }
            if(alarm_select_sta != 0 && alarm_open_sta !=0 && alarm_day_select_flag == 1)
            {
                Set_alarm2_clock(alarm_min_temp,alarm_hour_temp,alarm_day_select);
            }
            alarm_day_select_flag = 0;
        }
    }
    if(scroll_sta != 0)
    {
        scroll_count ++;
    }
    else
    {
        scroll_count = 0;
    }
    if(scroll_count == 120 && set_id == 0) //滚动间隔时间
    {
        scroll_show_flag = 1;
        scroll_count = 0;
        scroll_start = 1;
    }
    if(Timing_DN_flag == 1)
    {
        if(Timing_sec_temp == 0 && Timing_min_temp == 0)
        {
            Timing_DN_flag = 0;
            Timing_DN_close_flag = 1;
            gpio_put(BUZZ,1);
            beep_on_flag = 1;
        }
        else
        {
            Timing_sec_temp --;
            Timing_show_count ++;
            if(Timing_sec_temp == 255)
            {
                Timing_sec_temp = 0;
            }
            if(Timing_sec_temp == 0 && Timing_min_temp != 0)
            {
                Timing_sec_temp = 59;
                Timing_min_temp --;
                Timing_show_sec = 0;
            }
            if(Timing_show_count % 3 == 0)
            {
                Timing_show_count = 0;
                Timing_show_sec = Timing_sec_temp;
            }
        }
    }
    if(Timing_UP_Key_flag == 1)
    {
        Timing_sec_temp ++;
        Timing_show_count ++;
        if(Timing_sec_temp == 60)
        {
            Timing_sec_temp = 0;
            Timing_min_temp ++;
            if(Timing_min_temp == 60)Timing_min_temp = 0;
        }
        if(Timing_show_count % 3 == 0)
        {
            Timing_show_count = 0;
            Timing_show_sec = Timing_sec_temp;
        }
    }
    if(Full_time_sta == 1)
    {
        if(min_temp == 0 && Full_time_alarm_count >0)
        {
            
            if(Full_time_alarm_count != 5)
            {
                gpio_put(BUZZ,1);
                beep_on_flag = 1;
            }
            Full_time_alarm_count -- ;
        }
    }
    return true;
}


void show_adc()
{
    const float conversion_factor = 3.3f / (1 << 12);//计算电压
    uint16_t result = adc_read();
    float voltage = 3 *result * conversion_factor;
    uint8_t Single_digit = (int)voltage;//个位数
    uint8_t Decile = (int)(voltage*10)%10;//十分位
    uint8_t Percentile = (int)(voltage*100)%10;//百分位
    display_char(0,Single_digit+'0');
    display_char(5,'.');
    display_char(7,Decile+'0');
    display_char(12,Percentile+'0');
    display_char(17,'U');

}

void select_weekday(unsigned char x)//显示星期几
{
    switch(x)
    {
        case 0: Monday;DisTuesday;DisWednesday;DisThursday;DisFriday;DisSaturday;DisSunday;break;
        case 1:DisMonday;Tuesday;DisWednesday;DisThursday;DisFriday;DisSaturday; DisSunday;break;
        case 2:DisMonday;DisTuesday; Wednesday;DisThursday; DisFriday;DisSaturday;DisSunday;break;
        case 3:DisMonday;DisTuesday;DisWednesday;Thursday;DisFriday;DisSaturday;DisSunday;break;
        case 4: DisMonday;DisTuesday;DisWednesday;DisThursday;Friday;DisSaturday;DisSunday;break;
        case 5:DisMonday;DisTuesday;DisWednesday;DisThursday;DisFriday;Saturday;DisSunday;break;
        case 6:DisMonday;DisTuesday;DisWednesday; DisThursday;DisFriday;DisSaturday;Sunday;break;
    }
}

void cls_disp(unsigned char x)//清除x位置后的显示内容
{
    do
    {
        display_char(x,' ');
        x+=8;
    }while(x<sizeof(disp_buf));
}
void send_data(unsigned char data)//发送数据函数
{
    unsigned char i;
    for(i=0;i<8;i++)
    {
        CLK_LOW;

        SDI_LOW;

        if(data&0x01)
            SDI_HIGH;
        data>>=1;

        CLK_HIGH;

    }
}

void display_char(unsigned char x,unsigned char dis_char)
{
    unsigned char i,j,k;
    x+=disp_offset;//加上状态指示灯的偏移
    j=x/8;//要显示是第几个点阵序号
    k=x%8;//在第几个bit开始显示
    if((dis_char>='0')&&(dis_char<='9'))
        dis_char-=0x30;
    else if((dis_char>='A')&&(dis_char<='F'))
        dis_char-=0x37;
    else switch(dis_char)
        {
            case 'H':dis_char=16;break;
            case 'L':dis_char=17;break;
            case 'N':dis_char=18;break;
            case 'P':dis_char=19;break;
            case 'U':dis_char=20;break;
            case ':':dis_char=21;break;
            case ' ':dis_char=24;break;
            case 0:  dis_char=24;break;
            case 'T':dis_char=25;break;
                //  case '-':dis_char=26;break;
            case '.':dis_char=26;break;
            case '-':dis_char=27;break;
            case 'M':dis_char=28;break;
            case '/':dis_char=29;break;
                //case 'V':dis_char=28;break;
                //case 'W':dis_char=29;break;
        }
    for(i=1;i<8;i++)
    {
        if(k>0)
        {

            disp_buf[8*j+i]=(disp_buf[8*j+i]&(0xff>>(8-k)))|((ZIKU[dis_char*7+i-1])<<k);//保留需要的数据位
            if(j<(sizeof(disp_buf)/8)-1){
                disp_buf[8*j+8+i]=(disp_buf[8*j+8+i]&(0xff<<(8-k)))|((ZIKU[dis_char*7+i-1])>>(8-k));

            }

        }

        else
            disp_buf[8*j+i]=(ZIKU[dis_char*7+i-1]);

    }
}

void Show_Time() //显示时间
{
    Time_RTC=Read_RTC(); //获取RTC的值
    display_char(0,'1');
    Time_RTC.seconds = Time_RTC.seconds&0x7F;
    Time_RTC.minutes = Time_RTC.minutes&0x7F;
    Time_RTC.hour = Time_RTC.hour&0x3F;
    Time_RTC.dayofweek = Time_RTC.dayofweek&0x07;
    Time_RTC.dayofmonth = Time_RTC.dayofmonth&0x3F;
    Time_RTC.month = Time_RTC.month&0x1F;
    Set_hour_temp = BCD_to_Byte(Time_RTC.hour);
    min_temp = BCD_to_Byte(Time_RTC.minutes);
    dayofmonth_temp = BCD_to_Byte(Time_RTC.dayofmonth);
    month_temp = BCD_to_Byte(Time_RTC.month);
    year_temp = BCD_to_Byte(Time_RTC.year);
    if(Time_set_mode_sta != 0)  //改变时间时需要注意当前的时间模式
    {
        if(Set_hour_temp > 12)
        {
            hour_temp = Set_hour_temp -12;
            dis_PM;
            dis_AM_close;
        }
        else if(Set_hour_temp == 12)
        {
            dis_PM;
            dis_AM_close;
        }
        else
        {
            hour_temp = Set_hour_temp;
            dis_AM;
            dis_PM_close;
        }        
    }
    else
    {
        hour_temp = Set_hour_temp;
    }

    Time_buf[0]=((hour_temp/10)+'0');
    Time_buf[1]=((hour_temp%10)+'0');
    Time_buf[2]=((Time_RTC.minutes/16)+'0');
    Time_buf[3]=((Time_RTC.minutes%16)+'0');
    Min_count=((float)Time_RTC.seconds)/1.5; //计算当前RTC的秒数
    
    if(scroll_start == 0)
    {
        display_char(0,Time_buf[0]);
        display_char(5,Time_buf[1]);
        display_char(10,':');
        display_char(13,Time_buf[2]);
        display_char(18,Time_buf[3]);
    }
    if(Time_RTC.dayofweek==1){select_weekday(0);}
    else if (Time_RTC.dayofweek==2){select_weekday(1);}
    else if (Time_RTC.dayofweek==3){select_weekday(2);}
    else if (Time_RTC.dayofweek==4){select_weekday(3);}
    else if (Time_RTC.dayofweek==5){select_weekday(4);}
    else if (Time_RTC.dayofweek==6){select_weekday(5);}
    else select_weekday(6);

}
void dis_SetMode() 
{
    if(set_id < 3) //设置小时和分钟
    {
        if(set_id == 1)
        {
            if(Set_time_hour_flag == 0)
            {
                Set_hour_temp = BCD_to_Byte(Time_RTC.hour);
            }
            Set_time_hour_flag = 1;
        }
        if(set_id == 2)
        {
            if(Set_time_min_flag == 0)
            {
                Set_min_temp = BCD_to_Byte(Time_RTC.minutes);
            }
            Set_time_min_flag = 1;
        }
        display_char(0,(hour_temp/10+'0')&flag_Flashing[1]);
        display_char(5,(hour_temp%10+'0')&flag_Flashing[1]);
        display_char(10,':');
        display_char(13,(min_temp/10+'0')&flag_Flashing[2]);
        display_char(18,(min_temp%10+'0')&flag_Flashing[2]);
    }
    else if(set_id == 3)
    {
        whole_year = year_high_temp*100 + year_temp;
        Set_time_year_flag = 1;
        display_char(0,year_high_temp/10+0x30);
        display_char(5,year_high_temp%10+0x30);
        display_char(10,((year_temp/10+0x30)&flag_Flashing[3]));
        display_char(15,((year_temp%10+0x30)&flag_Flashing[3]));
        display_char(23,' ');
    }
    else if(set_id == 4 || set_id == 5)
    {
        whole_year = year_high_temp*100 + year_temp;
        if(set_id == 4)
        {
            Set_time_month_flag  = 1;
        }
        if(set_id == 5)
        {
            
            Set_time_dayofmonth_flag = 1;
        }
        display_char(0,((month_temp/10+0x30)&flag_Flashing[4]));
        display_char(5,((month_temp%10+0x30)&flag_Flashing[4]));
        display_char(10,'-');
        display_char(13,((dayofmonth_temp/10+0x30)&flag_Flashing[5]));
        display_char(18,((dayofmonth_temp%10+0x30)&flag_Flashing[5]));
    }
    else if(set_id == 6)
    {
        beep_flag  = 1;
        display_char(0,'B');
        display_char(5,'P');
        display_char(10,':');
        display_char(13,('0'&flag_Flashing[6]));
        if(beep_sta == 1)
        {
            display_char(18,('N'&flag_Flashing[6]));

        }
        else
        {
            display_char(18,('F'&flag_Flashing[6]));
        }
        cls_disp(26);

    }
    else if(set_id == 7)
    {
        scroll_flag = 1;
        display_char(0,'D');
        display_char(5,'P');
        display_char(10,':');
        display_char(13,('0'&flag_Flashing[7]));
        if(scroll_sta != 0)
        {
            display_char(18,('N'&flag_Flashing[7]));
        }
        else
        {
            display_char(18,('F'&flag_Flashing[7]));
        }
        cls_disp(26);

    }
    else if(set_id == 8)
    {
        if(Time_set_mode_flag == 0)
        {
            Set_hour_temp = BCD_to_Byte(Time_RTC.hour);
        }
        Time_set_mode_flag = 1;
        display_char(0,'M');
        display_char(6,'D');
        display_char(11,':');
        if(Time_set_mode_sta == 0)
        {
            display_char(14,'2'&flag_Flashing[8]);
        }
        else
        {
            display_char(14,'1'&flag_Flashing[8]);
        }
        
        cls_disp(23);
    }
    else if(set_id == 9)
    {
        Full_time_flag = 1;
        display_char(0,'F');
        display_char(5,'T');
        display_char(10,':');
        display_char(13,'0'&flag_Flashing[9]);
        if(Full_time_sta != 0)
        {
             display_char(18,'N'&flag_Flashing[9]);
             dis_hourly_chime;
        }
        else
        {
             display_char(18,'F'&flag_Flashing[9]);
             dis_hourly_chime_close;
        }
        cls_disp(26);
    }
    else
    {
        No_operation_count = 0;
        No_operation_flag = 0;
        KEY_Set_flag = 0;
        set_id = 0;
        update_time = 1;
        beep_flag = 0;
        scroll_flag = 0;
    }
}

void dis_alarm()
{
    if(set_id == 1 || set_id == 2)
    {
        if(set_id == 1)
        {
          alarm_select_flag = 1;  
        }
        if(set_id == 2)
        {
            alarm_open_flag = 1;
        }
        display_char(0,'A');
        if(alarm_select_sta == 0)
        {
            display_char(5,'0'&flag_Flashing[1]);
        }
        else
        {
            display_char(5,'1'&flag_Flashing[1]);
        }

        display_char(10,':');
        display_char(13,'0'&flag_Flashing[2]);
        if(alarm_open_sta != 0)
        {
            display_char(18,'N'&flag_Flashing[2]);          
        }
        else
        {
            display_char(18,'F'&flag_Flashing[2]);
        }

        cls_disp(26);
    }
    else if(set_id == 3 || set_id == 4)
    {
        if(set_id == 3)
        {
            alarm_hour_flag = 1;
        }
        if(set_id == 4)
        {
            alarm_min_flag = 1;
        }
        display_char(0,(alarm_hour_temp/10+'0') &flag_Flashing[3]);
        display_char(5,(alarm_hour_temp%10+'0')&flag_Flashing[3]);
        display_char(10,':');
        display_char(13,(alarm_min_temp/10+'0')&flag_Flashing[4]);
        display_char(18,(alarm_min_temp%10+'0')&flag_Flashing[4]);
        cls_disp(26);

    }
    else if(set_id == 5)
    {
        alarm_day_select_flag = 1;
        if(alarm_day_select == 1)
        {
            select_weekday(0);
        }
        else if(alarm_day_select == 2)
        {
            select_weekday(1);
        }
        else if(alarm_day_select == 3)
        {
            select_weekday(2);
        }
        else if(alarm_day_select == 4)
        {
            select_weekday(3);
        }
        else if(alarm_day_select == 5)
        {
            select_weekday(4);
        }
        else if(alarm_day_select == 6)
        {
            select_weekday(5);
        }
        else
        {
            select_weekday(6);
        }

    }
    else{
        if(alarm_select_sta == 0 && alarm_open_sta !=0)
        {
            Set_alarm1_clock(ALARM_MODE_HOUR_MIN_SEC_MATCHED,00,alarm_min_temp,alarm_hour_temp,alarm_day_select);
        }
        if(alarm_select_sta !=0 && alarm_open_sta !=0)
        {
            Set_alarm2_clock(alarm_min_temp,alarm_hour_temp,alarm_day_select);
        }
        No_operation_count = 0;
        No_operation_flag = 0;
        set_id = 0;
        alarm_flag = 0;
        alarm_id = 0;
        update_time = 1;
        beep_flag = 0;
        scroll_flag = 0;
        alarm_select_flag = 0;
        alarm_open_flag = 0;
        alarm_hour_flag = 0;
        alarm_min_flag = 0;
        alarm_day_select_flag = 0;
        scroll_count = 0;
    }
}

void dis_Timing()
{
    if(set_id == 1)
    {
        Timing_mode_flag = 1;
        display_char(0,'T');
        display_char(5,'M');
        display_char(11,'.');
        if(Timing_mode_sta == 0)
        {
            display_char(13,'U'&flag_Flashing[1]);
            display_char(18,'P'&flag_Flashing[1]);
            dis_CountUp;
            dis_CountDown_close;
        }
        else if(Timing_mode_sta == 1)
        {
            display_char(13,'D'&flag_Flashing[1]);
            display_char(18,'N'&flag_Flashing[1]);
            dis_CountDown;
            dis_CountUp_close;
        }
        else if(Timing_mode_sta == 2)
        {
            display_char(13,'0'&flag_Flashing[1]);
            display_char(18,'F'&flag_Flashing[1]);
            dis_CountDown_close;
            dis_CountUp_close;
        }
        cls_disp(26);

    }
    else if(set_id == 2 || set_id == 3 && Timing_mode_sta != 2)
    {
        if(set_id == 2&& Timing_mode_sta == 1)
        {
            Timing_min_flag  = 1;
        }
        else if (set_id == 2 && Timing_mode_sta == 0)//正计时设计
        {
            if(Timing_UP_Key_flag == 0)
            {
                Timing_min_temp = 0;
                Timing_sec_temp = 0;
            }
            No_operation_count = 0;
            Timing_UP_Key_flag = 1;
            display_char(0,Timing_min_temp/10+'0');
            display_char(5,Timing_min_temp%10+'0');
            display_char(10,':');
            display_char(13,Timing_show_sec/10+'0');
            display_char(18,Timing_show_sec%10+'0');
            cls_disp(26);
        }
        if(set_id == 3&& Timing_mode_sta == 1)
        {
            Timing_sec_flag = 1;
            Timing_DN_close_flag = 0;
        }
        if(Timing_mode_sta == 1) //倒计时设计
        {
            display_char(0,Timing_min_temp/10+'0'&flag_Flashing[2]);
            display_char(5,Timing_min_temp%10+'0'&flag_Flashing[2]);
            display_char(10,':');
            display_char(13,Timing_sec_temp/10+'0'&flag_Flashing[3]);
            display_char(18,Timing_sec_temp%10+'0'&flag_Flashing[3]);
            cls_disp(26);
            Timing_show_sec = Timing_sec_temp;
        }
        if(Timing_mode_sta == 0 && set_id == 3) //关闭正计时
        {
            display_char(0,Timing_min_temp/10+'0');
            display_char(5,Timing_min_temp%10+'0');
            display_char(10,':');
            display_char(13,Timing_sec_temp/10+'0');
            display_char(18,Timing_sec_temp%10+'0');
            cls_disp(26);
            Timing_show_sec = 0;
            Timing_show_count = 0;
            Timing_UP_Key_flag = 0;
        }

    }
    else if(set_id == 4 && Timing_mode_sta == 1 && Timing_DN_close_flag == 0 && Timing_mode_sta != 2) //倒计时显示
    {
        No_operation_count = 0;
        Timing_DN_flag = 1;
        display_char(0,Timing_min_temp/10+'0');
        display_char(5,Timing_min_temp%10+'0');
        display_char(10,':');
        display_char(13,Timing_show_sec/10+'0');
        display_char(18,Timing_show_sec%10+'0');
        cls_disp(26);
        
            
    }
    else{
        if(Timing_mode_sta == 0)
        {
            Timing_mode_sta = 2;
            Timing_min_temp = 0;
            Timing_sec_temp = 0;
            dis_CountUp_close;
        }
        No_operation_count = 0;
        No_operation_flag = 0;
        set_id = 0;
        UP_id = 0;
        UP_Key_flag = 0;
        update_time = 1;
        beep_flag = 0;
        scroll_flag = 0;
        scroll_count = 0;
        Timing_sec_flag = 0;
    }

}

void get_temperature()
{
    unsigned char start_tran[2] = {0x0E,0x20};
    i2c_write_blocking(I2C_PORT,Address,start_tran,2,false);
    i2c_write_blocking(I2C_PORT, Address, &get_add_high, 1, true);
    i2c_read_blocking(I2C_PORT,Address, &temp_high,1,false);
    i2c_write_blocking(I2C_PORT, Address, &get_add_low, 1, true);
    i2c_read_blocking(I2C_PORT,Address, &temp_low,1,false);
    temp_low = (temp_low >> 6)*25;//放大分辨率
}
void dis_scroll()
{
    
    
    if(scroll_show_flag == 1)
    {   
        get_temperature();
        display_char(32,'2');
        display_char(37,'0');
        display_char(42,(year_temp/10+0x30));
        display_char(47,(year_temp%10+0x30));
        display_char(52,'-');
        display_char(55,Time_RTC.month/16+'0');
        display_char(60,Time_RTC.month%16+'0');
        display_char(65,'-');
        display_char(68,Time_RTC.dayofmonth/16+'0');
        display_char(73,Time_RTC.dayofmonth%16+'0');
        if(temp_sta == 0)
        {
            display_char(80, temp_high/10+0x30);
            display_char(85, temp_high%10+0x30);
            display_char(90, '.');
            display_char(92, temp_low/10+0x30);
            display_char(97, temp_low%10+0x30);
            display_char(102,'C');
        }
        else
        {
            unsigned char T = temp_high*9/5 +32;
            if(T>=100)T=99;
             display_char(80, T/10+0x30);
            display_char(85, T%10+0x30);
            display_char(90, '.');
            display_char(92, temp_low/10+0x30);
            display_char(97, temp_low%10+0x30);
            display_char(102,'T');
        }
        
        scroll_show_flag = 0;
    }
    if(scroll_show_start == sizeof(disp_buf)-36)
    {
        display_char(32,' ');
        display_char(40,' ');
        display_char(48,' ');
        display_char(36,hour_temp/10+0x30);
        display_char(41,hour_temp%10+0x30);
        display_char(46,':');
        display_char(49,Time_RTC.minutes/16+0x30);
        display_char(54,Time_RTC.minutes%16+0x30);
    }
    for(i=1;i<8;i++)
    {
       save_buf = disp_buf[i] & 0x03; //保留功能位
        for(jr=0;jr<sizeof(disp_buf)/8;jr++)
        {
        
        if(jr<sizeof(disp_buf)/8-1)
            disp_buf[8*jr+i]=disp_buf[8*jr+i]>>1|disp_buf[8*jr+i+8]<<7;
        else
            disp_buf[8*jr+i]=disp_buf[8*jr+i]>>1;
       
        }
        disp_buf[i] = (disp_buf[i] & (~0x03)) | save_buf;//复原功能位
    } 
}
unsigned char get_month_date(uint16_t year_cnt,uint8_t month_cnt)//判断一个月的最大天数
{
    if((year_cnt%4==0&&year_cnt%100!=0)||year_cnt%400==0)
    {
        switch(month_cnt)
        {
            case 1:return month_date[0][0];break;
            case 2:return month_date[0][1];break;
            case 3:return month_date[0][2];break;
            case 4:return month_date[0][3];break;
            case 5:return month_date[0][4];break;
            case 6:return month_date[0][5];break;
            case 7:return month_date[0][6];break;
            case 8:return month_date[0][7];break;
            case 9:return month_date[0][8];break;
            case 10:return month_date[0][9];break;
            case 11:return month_date[0][10];break;
            case 12:return month_date[0][11];break;
        }    
           
    }
    else
    {
        switch(month_cnt)
        {
            case 1:return month_date[1][0];break;
            case 2:return month_date[1][1];break;
            case 3:return month_date[1][2];break;
            case 4:return month_date[1][3];break;
            case 5:return month_date[1][4];break;
            case 6:return month_date[1][5];break;
            case 7:return month_date[1][6];break;
            case 8:return month_date[1][7];break;
            case 9:return month_date[1][8];break;
            case 10:return month_date[1][9];break;
            case 11:return month_date[1][10];break;
            case 12:return month_date[1][11];break;
        }    
    }
}
unsigned char get_weekday(uint16_t year_cnt,uint8_t month_cnt,uint8_t date_cnt)//根据年月日判断星期几
{
    uint8_t weekday = 8;
    if(month_cnt == 1 || month_cnt == 2)
    {
        month_cnt += 12;
        year_cnt--;
    }
    weekday = (date_cnt+1+2*month_cnt+3*(month_cnt+1)/5+year_cnt+year_cnt/4-year_cnt/100+year_cnt/400)%7;
    switch(weekday)
    {
        case 0 : return 7; break;
        case 1 : return 1; break;
        case 2 : return 2; break;
        case 3 : return 3; break;
        case 4 : return 4; break;
        case 5 : return 5; break;                                                             
        case 6 : return 6; break;
    }
}
uint16_t get_ads1015() //获取光敏传感器的值
{
    adc_select_input(0);
    uint16_t value = adc_read();
	return value;
}
void Alarm_set(uint8_t UP_DOWN_flag)
{
	if(alarm_hour_flag == 1) //闹钟时钟可设置标志位
	{
        if(UP_DOWN_flag == UP_flag)
        {
            alarm_hour_temp ++;
            if(alarm_hour_temp == 24)alarm_hour_temp = 0;
        }
		else
        {
            alarm_hour_temp --;
		    if(alarm_hour_temp == 255)alarm_hour_temp = 23;
        }   	
	}
	if(alarm_min_flag == 1)
	{
        if(UP_DOWN_flag == UP_flag)
        {
            alarm_min_temp ++;
		    if(alarm_min_temp == 60)alarm_min_temp = 0;
        }
        else
        {
            alarm_min_temp --;
		    if(alarm_min_temp == 255)alarm_min_temp = 59;
        }
	}
	if(alarm_open_flag == 1)//闹钟开关可设置标志位
	{
		alarm_open_sta = !alarm_open_sta;
		if(alarm_open_sta != 0)
		{
			dis_Alarm_en;
		}
		else
		{
			dis_Alarm_close;
		}
	}
	if(alarm_select_flag == 1)//闹钟选择可设置标志位
	{
		alarm_select_sta = ! alarm_select_sta;
	}
	if(alarm_day_select_flag == 1)
	{
        if(UP_DOWN_flag == UP_flag)
        {
            alarm_day_select++;
		    if(alarm_day_select == 8)alarm_day_select =1;
        }
        else
        {
            alarm_day_select --;
		    if(alarm_day_select == 0)alarm_day_select = 7;
        }
		
	}
}
void Timing_set(uint8_t UP_DOWN_flag)
{
	if(Timing_mode_flag == 1)
	{
        if(UP_DOWN_flag == UP_flag)
        {
            Timing_mode_sta++;
		    if(Timing_mode_sta == 3) Timing_mode_sta = 0;
        }
        else
        {
            Timing_mode_sta--;
            if(Timing_mode_sta == 255)Timing_mode_sta = 2;
        }
		
	}
	if(Timing_mode_sta == 1 && Timing_min_flag == 1)
	{
		if(UP_DOWN_flag == UP_flag)
		{
			Timing_min_temp ++;
			if(Timing_min_temp == 60)Timing_min_temp = 0;
		}
		else
		{
			Timing_min_temp --;
			if(Timing_min_temp == 255)Timing_min_temp = 59;
		}
	}
	if(Timing_mode_sta == 1 && Timing_sec_flag == 1)
	{
		if(UP_DOWN_flag == UP_flag)
		{
			Timing_sec_temp ++;
			if(Timing_sec_temp == 60)Timing_sec_temp = 0;
		}
		else
		{
			Timing_sec_temp --;
			if(Timing_sec_temp == 255)Timing_sec_temp = 59;
		}
	}
	if(Time_set_mode_flag == 1) //时间模式设置
	{
		Time_set_mode_sta = !Time_set_mode_sta;
		if(Time_set_mode_sta == 0)
		{
			dis_AM_close;
			dis_PM_close;
		}
		else
		{
			
			if(Set_hour_temp > 12)
			{
				hour_temp = Set_hour_temp - 12;
				dis_PM;
				dis_AM_close;
			}
			else if(Set_hour_temp == 12)
			{
				dis_PM;
				dis_AM_close;
			}
			else
			{
				dis_AM;
				dis_PM_close;
			}
		}
	}
}
void Time_set(uint8_t UP_DOWN_flag)
{
	if(Set_time_hour_flag == 1) //小时设置
	{
		change_time_flag = 1;
		if(UP_DOWN_flag == UP_flag)
		{
			Set_hour_temp++;
			if(Set_hour_temp == 24)Set_hour_temp = 0;
		}
		else
		{
			Set_hour_temp--;
			if(Set_hour_temp == 255)Set_hour_temp = 23;
		}
		if(Time_set_mode_sta != 0)  //改变时间时需要注意当前的时间模式
		{
			if(Set_hour_temp > 12)
			{
				hour_temp = Set_hour_temp -12;
				dis_PM;
				dis_AM_close;
			}
			else if(Set_hour_temp == 12)
			{
				hour_temp = Set_hour_temp;
				dis_PM;
				dis_AM_close;
			}
			else
			{
				hour_temp = Set_hour_temp;
				dis_AM;
				dis_PM_close;
			}
				
		}
		else
		{
			hour_temp = Set_hour_temp;
		}
	}
	if(Set_time_min_flag == 1) //分钟设计
	{
		change_time_flag = 1;
		if(UP_DOWN_flag == UP_flag)
		{
			Set_min_temp++;
			if(Set_min_temp == 60)Set_min_temp = 0;
		}
		else
		{
			Set_min_temp--;
			if(Set_min_temp == 255)Set_min_temp = 59;
		}
		min_temp = Set_min_temp;
	}
	if(Set_time_year_flag == 1)
	{
		change_time_flag = 1;
		if(UP_DOWN_flag == UP_flag)
		{
			year_temp ++;
			if(year_temp == 100)
			{
				year_temp = 0;
				year_high_temp ++;
			}
		}
		else
		{
			year_temp --;
			if(year_temp == 255)
			{
				year_temp = 99;
				year_high_temp --;
			}
		}
		
	}
	if(Set_time_month_flag == 1)
	{
		change_time_flag = 1;
		if(UP_DOWN_flag == UP_flag)
		{
			month_temp ++ ;
			if(month_temp == 13) month_temp = 1;
		}
		else
		{
			month_temp -- ;
			if(month_temp == 0) month_temp = 12;
		}
	}
	if(Set_time_dayofmonth_flag == 1)
	{
		change_time_flag = 1;
		if(UP_DOWN_flag == UP_flag)
		{
			dayofmonth_temp++;
			if(dayofmonth_temp > get_month_date(whole_year,month_temp))
			{
				dayofmonth_temp = 0;
			}
		}
		else
		{
			dayofmonth_temp--;
			if(dayofmonth_temp == 0)
			{
				dayofmonth_temp = get_month_date(whole_year,month_temp);
			}
		}
	}
}
void adc_show_count()
{
	if(adc_light_flag != 0)
    {
        adc_light_count++;
        if(adc_light_count == 1000)
        {
            adc_light_count = 0;
            adc_light = get_ads1015();
        }
    }
    if(adc_show_time != 0)
    {
        adc_count ++ ;
        if(adc_count == 500)
        {
            adc_count = 0;
            adc_show_flag = 1;
        }
    }
}
void beep_stop_judge()
{
	if(beep_on_flag == 1)
    {
        beep_on_count++;
        if(beep_on_count == 80)
        {
            gpio_put(BUZZ,0);
            beep_on_count = 0;
            beep_on_flag = 0;
        }
    }
}
void Flashing_start_judge()
{
	if(set_id != 0)
	{
        Flashing_count ++;
        if(Flashing_count == 600)
        {
            Flashing_count = 0;
            flag_Flashing[set_id] = ~flag_Flashing[set_id];

            if(alarm_id == 1)
                alarm_flag = 1;
            else if(UP_id == 1)
                UP_Key_flag = 1;
            else
                KEY_Set_flag = 1;
        }
    }
}
void scroll_show_judge()
{
	if(scroll_start == 1) //每3分钟滚动一次
    {
        scroll_start_count ++ ;
    }
    if(scroll_start_count == 130)
    {
        if(scroll_show_start <sizeof(disp_buf)){
            dis_scroll(); 
            scroll_show_start++;
        }
        else
        {
            display_char(24,' ');
            
            scroll_start = 0;
            scroll_show_start = 0;
            update_time = 1;
        }    
        
        scroll_start_count = 0;
    }
}
void EXIT()
{
	if(Set_time_hour_flag ==1 && change_time_flag == 1)
	{
		set_hour(Set_hour_temp);
	}
	if(Set_time_min_flag == 1 && change_time_flag == 1)
	{
		set_min(Set_min_temp);
	}
	if(Set_time_year_flag == 1 && change_time_flag == 1)
	{
		set_year(year_temp);
		set_dayofweekday(get_weekday(whole_year,month_temp,dayofmonth_temp));
		select_weekday(get_weekday(whole_year,month_temp,dayofmonth_temp)-1);
	}
	if(Set_time_month_flag == 1 && change_time_flag == 1)
	{
		set_month(month_temp);
		set_dayofweekday(get_weekday(whole_year,month_temp,dayofmonth_temp));
		select_weekday(get_weekday(whole_year,month_temp,dayofmonth_temp)-1);
	}
	if(Set_time_dayofmonth_flag == 1 && change_time_flag ==1)
	{
		set_dayofmouth(dayofmonth_temp);
		set_dayofweekday(get_weekday(whole_year,month_temp,dayofmonth_temp));
		select_weekday(get_weekday(whole_year,month_temp,dayofmonth_temp)-1);
	}
    flag_Flashing[set_id] = 0xff;
    if(alarm_min_flag == 1) //防止显示为空
    {
        display_char(13,(alarm_min_temp/10+'0')&flag_Flashing[4]);
        display_char(18,(alarm_min_temp%10+'0')&flag_Flashing[4]);
    }
    if(Timing_mode_flag == 1 && Timing_mode_sta == 2)
    {
        display_char(13,'0'&flag_Flashing[1]);
        display_char(18,'F'&flag_Flashing[1]);
    }
	No_operation_count = 0;
    scroll_flag = 0;
	beep_flag = 0;
	alarm_select_flag = 0;
	alarm_open_flag = 0;
	alarm_hour_flag = 0;
	alarm_min_flag = 0;
	Timing_mode_flag = 0;
	Timing_min_flag  = 0;
	Timing_sec_flag = 0;
	Time_set_mode_flag = 0;
	Full_time_flag = 0;
	Set_time_hour_flag = 0;
	Set_time_min_flag = 0;
	Set_time_year_flag = 0;
	Set_time_month_flag = 0;
	Set_time_dayofmonth_flag = 0;
	change_time_flag = 0;
}
void Special_Exit()
{
	No_operation_flag = 0;
	KEY_Set_flag = 0;
	alarm_flag = 0;
	alarm_id = 0;
	UP_id = 0;
	UP_Key_flag = 0;
	update_time = 1;
	scroll_count = 0;
}
void beep_start_judge()
{
	if(beep_sta == 1)
	{
		gpio_put(BUZZ,1);
		beep_on_flag = 1;
	}
}
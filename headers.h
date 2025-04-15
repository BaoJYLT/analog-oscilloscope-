// 头文件
// 全局变量
// 函数声明

#ifndef __HEADERS_H__
#define __HEADERS_H__

#include <STC12C5A60S2.H>
#include <INTRINS.H>
#include <REG51.H>
#include <ABSACC.H>

#endif
// 上面的头文件位置 E:/Downloads/C51/INC STC那个在D盘的学科文件夹

#define true 1
#define false 0
// #define T_flop 10   //10ms按键延时去抖
#define REALTIME 8
#define GENERATOR 7
#define RECALL 6
#define MEASURE 5
// submode每个宏变量结尾都有一个M表示MODE
#define SINM 4
#define TRIANNGULARM 3
#define RECTANGULARM 2
#define SAWTOOTHM 1
// 地址数据复用
#define ADDR_HIGH P2
#define ADDR_LOW P0


sbit KEY1 = P3^4;
sbit KEY2 = P3^5;
// 与595芯片相关的引脚 两个clk都是上升沿有效
sbit SDAT = P1^0;   // DS
sbit SCLK = P1^1;   // SH_CP 数据移位时钟 shift_clock
sbit RCLK = P1^2;   // ST_CP 数据存储时钟（所存） storage_clock

sbit ad_in = P1^3;
sbit WR = P3^6;
sbit RD = P3^7;
sbit ALE = P4^5; 

// SFR寄存器名称地址映射
//P1ASF 模拟输入引脚
// ADC_CONTR CSF则设置AD转换中的输入信号来源
sfr P1ASF = 0x9d;
sfr ADC_CONTR = 0xbc;
sfr ADC_RES = 0xbd;
sfr ADC_RESL = 0xbe;
sfr AUXR1 = 0xa2;

extern unsigned int workmode;  //工作模式
extern unsigned int submode ;   //波形发生模式
extern volatile bit Timer0Flag ;
extern void initIE();
extern void initTimer0();
extern void interruptTimer0() interrupt 0xb;
extern void delay_T(unsigned int ms);

extern void realtime_mode();// mode
extern void generator_mode();
extern void recall_mode();
extern void measure_mode();

extern void init_ad();// ad
extern void saveAD_6264(unsigned int address, unsigned char data);
extern void realout_0832_1(unsigned char data);

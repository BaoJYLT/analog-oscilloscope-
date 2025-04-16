// 头文件
// 全局变量
// 函数声明

#ifndef __HEADERS_H__
#define __HEADERS_H__

#include <stdint.h>
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
// submode 测量模式下的测量对象选择
#define MEASUREFREQ 2   // 频率测量
#define MEASUREAMP 1    // 幅值测量
// 地址数据复用
#define ADDR_HIGH P2
#define ADDR_LOW P0
#define WRITE_RAM 1
#define READ_RAM 2  //关于6264RAM的 读写控制信号宏定义只适用于区分，大概是先写后读
// 测量抖动阈值
#define SIGNAL_JITTER_THRESHOLD 5

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

// 数码管码表


// 数码管模式表


extern unsigned int workmode;  //工作模式
extern unsigned int submode ;   //波形发生模式
extern volatile bit Timer0Flag ;

extern void initIE();
extern void initTimer0();
extern void interruptTimer0() interrupt 0xb;
extern void initTimer1();
extern void interruptTimer1() interrupt 0x1b;
extern void delay_T(unsigned int ms);
extern void sendByte_595(unsigned char byte_595);
extern void keyDetection();
extern void modeSelection();

extern void realtime_mode();// mode
extern void generator_mode();
extern void recall_mode();
extern void measure_mode();
// extern void default_mode();

extern void init_ad();// ad
extern void writeRAM_6264(unsigned int address, unsigned char data);
// extern void readRAM_6264(unsigned int address,unsigned char data);
extern void realtime_out_0832(unsigned char outputNum);
extern void readRAM_6264(unsigned int address);
extern void display_digits();

#define WAVE_TABLE_LEN 100

extern const unsigned char code sinWaveTable[WAVE_TABLE_LEN];
extern const unsigned char code triWaveTable[WAVE_TABLE_LEN];
extern const unsigned char code rectWaveTable[WAVE_TABLE_LEN];
extern const unsigned char code sawWaveTable[WAVE_TABLE_LEN];
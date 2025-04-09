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

sbit KEY1 = P3^4;
sbit KEY2 = P3^5;
// 与595芯片相关的引脚 两个clk都是上升沿有效
sbit SDAT = P1^0;   // DS
sbit SCLK = P1^1;   // SH_CP 数据移位时钟 shift_clock
sbit RCLK = P1^2;   // ST_CP 数据存储时钟（所存） storage_clock

extern unsigned int workmode = 0;  //工作模式
extern unsigned int submode = 0;   //波形发生模式

extern void realtime_mode();
extern void generator_mode();
extern void recall_mode();
extern void measure_mode();

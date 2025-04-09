#include "headers.h"
void init_ad(){
    P1ASF = 0b00001000; // 模拟输入口 p1.3
    ADC_CONTR = 0b11100011;    // 设置 speed11 90T 300k 将开关留到后面打开
    // 完成AD转换之后需要软件清除ADC_FLAG标志位
    AUXR1 &= 0xfb;  // ADRJ=0，取高八位结果，不影响寄存器设置的其他功能
    P2= 0;
}

/**
 * 将数据存储到6264存储器
 * @param address 13位地址；
 * @param data 8位数据。
 * @return 
 * 将数据存储到6264的特定位置
 */
void saveAD_6264(unsigned int address, unsigned char data){
    // ADDRESS设置与锁存
    unsigned char addr_high, addr_low;
    addr_high = (unsigned char)(address >> 8);//地址高5位（或者说是高八位 但地址只有13位 所以最高位000）
    addr_low = (unsigned char)(address & 0xff);//地址低八位
    ADDR_HIGH = addr_high;
    ADDR_LOW = addr_low;
    ALE = 1;
    delay_T();
    ALE = 0;
    // 处理后一个数据存储
    P0 = data;//
    P2 |= 0x20; //CS0使能
    WR = 0;
    RD = 0;
    delay_T();
    WR = 1;
}
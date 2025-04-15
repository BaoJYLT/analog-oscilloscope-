//dataprocess.c 与数据处理相关，包括AD转化，数据传输，数据流控制等
#include "headers.h"
/**
 * 单片机内AD转换相关SFR初始化
 * - 保证AD转换的顺利进行
 * - 转换结果存放在ADC_RES寄存器上，是结果的高八位
 */
void init_ad(){
    P1ASF = 0b00001000; // 模拟输入口 p1.3
    ADC_CONTR = 0b11100011;    // 设置 speed11 90T 300k 将开关留到后面打开
    // 完成AD转换之后需要软件清除ADC_FLAG标志位
    AUXR1 &= 0xfb;  // ADRJ=0，取高八位结果，不影响寄存器设置的其他功能
    P2= 0b00000111; // 初始化不打开任何一个片选信号
}

/**
 * 将数据存储到6264存储器
 * @param address 13位地址；
 * @param data 8位数据。
 * @return 
 * 将数据存储到6264的特定位置
 */
void writeRAM_6264(unsigned int address, unsigned char data){
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
    P0 = data;//处理写操作的硬件数据总线上的赋值
    P2 &= 0b00011111; //CS0使能 
    WR = 0;
    RD = 0;
    delay_T();
    WR = 1;

}

/**
 * 实时数据输出OUTPUT 1
 * 【应用场景】
 * - REALTIME模式（一路数据实时输出并且进行存储，另一路数据波形发生）
 * - RECALL模式（一路数据实时输出，但是不存储，另一路数据换成读取6264存储器）
 * @param data 传到DAC的数据
 * @param outputNum 选择DAC的哪一路输出 OUTPUT1 还是OUTPUT2
 */
void realtime_out_0832(unsigned char outputNum){
    P2 &= 0b00011111;
    if(outputNum == 1){
        P2 |= 0b00100000;   // CS1片选
    }else if(outputNum == 2){
        P2 |= 0b01000000;   // CS2片选
    }
    // P0 = data;
}

/**
 * 输出6264存储器中数据
 * - 注意：使用该函数之前需要在P0端口上先设置好数据总线
 * ? void readRAM_6264(unsigned int address, unsigned char data)
 */
void readRAM_6264(unsigned int address){
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
    P2 &= 0b00011111; //CS0使能 
    WR = 0;
    RD = 0;
    delay_T();
    RD = 1;
}
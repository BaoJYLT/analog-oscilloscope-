#include "headers.h"
/**
 * 【控制寄存器初始化】
 * - 中断初始化
 * -计数/定时器初始化
 * - ADC控制器件初始化（ADC_CONTR P1ASF——>ADC_RES ADC_RESL）
 * 【工作模式初始化】
 * - modeSelection 4大
 * - submodeSelection 4小波形发生
 */
unsigned int workmode = 0;  //工作模式
unsigned int submode = 0;   //波形发生模式
volatile bit Timer0Flag = 0;
/**
 * 中断使能初始化 中断允许寄存器IE A8H
 * 【面向采样率的设计？？】
 * EA中断使能总开关； ET0 ET1两个定时Timer使能 T0优先级高于T1；
 * T0 定时按键消抖，T1 定时其他
 * 12MHz时钟频率，单片机频率为1MHz，T=1us，10ms=10000*1us
 * mode-KEY2，submode-KEY1。
 * 虽然INT0 INT1也是定时计数器，但是没有接线，所以没有读数的必要
 * ES使能UART；
 */
void initIE(){
    EA = 1;
    //ES = 1;
    ET1 = 1;//其他定时
    EX1 = 0;
    ET0 = 1;//按键消抖定时
    EX0 = 0;
}

/**
 * 【TMOD设置 T1T0】
 *  [T0-按键消抖]
 * GATE=0不受外部中断信号控制，CT=0 timer，M1M0=01 16位计数器
 * TR0与TR1才是定时器开始工作与否的开关
 * 
 */
void initTimer0(){
    TMOD &= 0xf0;   //  保留T1设置，修改T0延时定时器
    TMOD |= 0b00000001; // 设定T0对应TMOD
    TH0 = 0xfc;
    TL0 = 0x18;// 定时1000个  即1ms 作为延时的基本时间单元
    //TR0 = 1;
}

/**
 * T0定时器中断执行程序
 */
void interruptTimer0() interrupt 0xb{
    // 执行T0定时结束后的中断——wait？
    Timer0Flag = TF0;
    TF0 = 0;
}
/**
 * 延时
 * - T：延时时长 
 * - 去抖时，常见设置10ms
 */
void delay_T(unsigned int ms){
    unsigned int i;
    initTimer0();
    TR0 = 1;
    for(i=0; i< ms;i++){
        while(!Timer0Flag);
    }
    TR0 = 0;        
}

/**
 * 双排按键状态检测
 * - 先行再列
 * - 第一行：波形发生具体形状选择
 * - 第二行：具体功能选择
 */
void keyDetection_bjy(){
    int i;
    unsigned int keyEvent = false;      // 按键事件发生标志
    unsigned int rowKeyFlag = false;   // 行按键状态 false 1 2 分别代表没有，第一行，第二行
    unsigned int colKeyFlag = false;   // 列按键状态
    unsigned int colEnable = 0B11111;  // 列扫描使能 低四位用于四列按键状态的列扫描，比如000001用扫描右边第一列的按键KEY1 KEY5
    
    // KEY UP按键抬起事件判断
    if(keyEvent){
        if (!KEY1 && !KEY2){
            keyEvent = false;
        }
    }
    // KEY DOWN按键按下事件判断
    // GPT建议 for循环实现行列扫描

    // 我写的
    else{
        if(KEY1 && KEY2 == 0){
            // Delay remove flop
            keyEvent = true;
            rowKeyFlag = 1;
        }else if (KEY2 && KEY1 == 0){
            // Delay 10ms remove flop
            keyEvent = true;
            rowKeyFlag = 2;
        }
        else{
            // 上下两行同时有按键按下 无效状态判断
            keyEvent = false;
            rowKeyFlag = false;
            colEnable = false
        }
    } 
}

/**
 * 向74HC595发送一个字节数据
 */
void sendByte_595(unsigned char byte_595){
    unsigned char i;
    for(i= 0; i < 8; i++){  //8位二进制数据从8052中串行发出
        SDAT = (byte_595 & 0x80) ? 1 : 0;   // 取最高位作为串行口发出的数据
        SCLK = 0;
        SCLK = 1;   // 移位时钟上升沿
        byte_595 << 1; // 左移字节数据 
    }
    RCLK = 0;
    RCLK = 1;   // 所存时钟上升沿
}

void keyDetection(){
    unsigned char rowEnable   = 0b1111;   // 行扫描控制字节，所有的都使能 detect行 
    unsigned char colEnable[] = {0b00000001, 0b00000010, 0b00000100, 0b00001000};   // 列扫描使能控制信号
    unsigned char rowKeyFlag = false;    // 标记行坐标
    unsigned char colKeyFlag =  false;   // 标记列
    unsigned char key1Event = false, key2Event = false; // KEY1  KEY2按键是否按下的状态标记
    unsigned char col_scanned = 0;// 被检测列 用于for循环 从原理图右到左扫描
    unsigned int keyNum = 0;  // 被按下的按键编号

    // 行扫描
    sendByte_595(rowEnable);    // 将行扫描时的控制信号设置，并通过595
    if (KEY2) {
        delay_T(10); // 调用延时函数去抖
        if (KEY2){
            key2Event = true;
        }
        else{
            key2Event = false;
        }
    }
    else if (KEY1){
        delay_T(10);
        if(KEY1){
            key1Event = true;
        }
        else{
            key1Event = false;
        }
    }
    delay_T(10); //等待10ms，确保行扫描的稳定

    // 列扫描
    for(col_scanned = 0; col_scanned < 4; col_scanned ++ ){
        sendByte_595(colEnable[col_scanned]);
        if (KEY2) {
            delay_T(10); // 调用延时函数去抖
            if (KEY2){
                key2Event = true;
                keyNum = 5 + col_scanned;
            }
            
        }
        else if (KEY1){
            delay_T(10);
            if(KEY1){
                key1Event = true;
                keyNum = 1 + col_scanned;
            }
        }
    }
    delay_T(10);
    /*
    if(keyNum){
        modeSelection(keyNum);
    }
    */
    //modeSelection(keyNum);

    // 工作模式切换 + 发生波形切换
    if(keyNum >= 5 && keyNum <= 8){
        workmode = keyNum;
    }
    else if (workmode == GENERATOR){
        if (1 <= keyNum && keyNum <= 4){
            submode = keyNum;
        }
    } 
}

/**
 * 按键模式选择 KEY2 T0
 * - 实时显示
 * - 波形发生
 * - 波形回放
 * - 测量模式
 */
void modeSelection(){
    switch (workmode){
        case REALTIME:
            realtime_mode();
            break;
        case GENERATOR:
            generator_mode();// 需要有默认的波生成波形
            break;
        case RECALL:
            recall_mode();
            break;
        case MEASURE:
            measure_mode();
            break;
        default:
            default_mode();
            break;
    }
}

/**
 * 波形发生状态下的子模式选择 KEY1 T1
 * - 正弦波
 * - 矩形波
 * - 三角波
 * - 锯齿波（注意波形变化过程的直上直下）
 */
void submodeSelection(unsigned int keyNum){

}
void initSCOM(){

}

void initSMOD(){

}
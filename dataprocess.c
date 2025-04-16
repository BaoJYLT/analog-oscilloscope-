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

void measure_parameters(){
    typedef enum{ FALLING, RISING} SingnalState;
    unsigned short Sig_data, Sig_pre_data;
    unsigned short Sig_highest, Sig_lowest;
    unsigned short Sig_amp, Sig_freq, Sig_Period;
    SignalState Sig_trend = RISING;
    static unsigned char stable_count = 0;
    unsigned int measure_timer =  0;
    // 初始化 得到转换后的一个字节
    if(workmode == MEASURE){ //【尚不确定需要while循环，应该是在时间中断里面不断被keydetection调用，可能不需要这个循环】
        measure_timer ++;
        init_ad();
        ADC_CONTR |= 0X08;  //start ADC工作
        while(!(ADC_CONTR & 0x10)); // 等待转换完成的flag
        Sig_pre_data = Sig_data;
        Sig_data = ADC_RES;

        if((Sig_data > Sig_pre_data) && (Sig_trend == FALLING)){
            if(stable_count == 0){
                Sig_lowest = Sig_data;
            }
            if(++ stable_count >= 5){
                Sig_trend = RISING;
                stable_count = 0;
                Sig_freq = 2000 / measure_timer;//需要根据我们的采样率重新设计
                measure_timer = 0;
            }
        }else if((Sig_data < Sig_pre_data) && (Sig_trend == RISING)){
            if(stable_count == 0){
                Sig_highest = Sig_data;
            }
            if(++ stable_count >= 5){
                Sig_trend = FALLING;
                stable_count = 0;
                Sig_amp = (int)(Sig_highest - Sig_lowest);  
            }
        }
        Sig_pre_data = Sig_data;

       /*
        
        if (Sig_data > Sig_highest){
            Sig_highest = Sig_data;
        }
        if (Sig_data < Sig_lowest){
            Sig_lowest = Sig_data;
        }
        Sig_amp = Sig_highest - Sig_lowest;
        // 【需要与基准电压进行对比转换，结合数据手册】 
        
        if ((Sig_data - Sig_pre_data < SIGNAL_JITTER_THRESHOLD) || 
            (Sig_pre_data - Sig_data < SIGNAL_JITTER_THRESHOLD)){    // 信号抖动处理
            stableCount++;
        }else{
            stableCount = 0;    //stableCount用于记录稳定count的数量
        }

        if (stableCount > 5){
            if(Sig_trend == RISING){
                if(Sig_data < Sig_pre_data){
                    Sig_trend = FALLING;
                    Sig_Period = getTime() - Sig_last_Time;
                    Sig_freq = 1000 / Sig_Period;
                    Sig_last_time = getTime();
                }
            }else if(Sig_trend == FALLING){
                if(Sig_data > Sig_pre_data){
                    Sig_trend = RISING;
                }
            }
        }
        
        
        if (Sig_trend == RISING){
            if (Sig_data > Sig_pre_data){
                if (stableCount == 0){
                    Sig_lowest = Sig_data;
                    // 上升沿开始时间
                }
                if (++stableCount >= 5){ // 5个采样点的稳定状态
                    Sig_highest = Sig_data;
                    Sig_trend = FALLING; // 转换为下降沿
                    // 上升沿结束时间
                    Sig_Period = 2 * (Sig_highest - Sig_lowest);
                    Sig_amp = Sig_highest - Sig_lowest; 
                    Sig_freq = 1 / Sig_Period; // 频率计算
                }
            }
            else{ // 极值点
                stableCount = 0;
            }
        }
        else{//FALLING
            if (Sig_data < Sig_pre_data){
                if (stableCount == 0){
                    Sig_highest = Sig_data;
                    // 下降沿开始时间
                }
                if (++stableCount >= 5){ // 5个采样点的稳定状态
                    Sig_lowest = Sig_data;
                    Sig_trend = RISING; // 转换为上升沿
                    // 下降沿结束时间
                    Sig_Period = 2 * (Sig_highest - Sig_lowest);
                    Sig_amp = Sig_highest - Sig_lowest; 
                    Sig_freq = 1 / Sig_Period; // 频率计算
                }
            }
            else{ // 极值点
                stableCount = 0;
            }
        }
    }
    // 用一个变量返回两个值
    return Sig_freq; // 频率计算
    // 是否需要设置输入参数，选择输结果；还是返回数组？？
    */
}

/**
 * 数码管显示
 * display_digits();
 * 
 */
void display_digits(){
    
}

 /**
  * 波形发生查表
  */
 const unsigned char code sinWaveTable[WAVE_TABLE_LEN] = {
    128,136,143,150,158,165,172,179,185,191,197,202,207,211,215,218,220,222,223,224,
    224,224,223,222,220,218,215,211,207,202,197,191,185,179,172,165,158,150,143,136,
    128,120,113,106, 98, 91, 84, 77, 71, 65, 59, 54, 49, 45, 41, 38, 36, 34, 33, 32,
     32, 32, 33, 34, 36, 38, 41, 45, 49, 54, 59, 65, 71, 77, 84, 91, 98,106,113,120
  };
  
  const unsigned char code triWaveTable[WAVE_TABLE_LEN] = {
    // 前50个线性上升，后50个线性下降
    0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,
    100,105,110,115,120,125,130,135,140,145,150,155,160,165,170,175,180,185,190,195,
    200,205,210,215,220,225,230,235,240,245,
    250,245,240,235,230,225,220,215,210,205,200,195,190,185,180,175,170,165,160,155,
    150,145,140,135,130,125,120,115,110,105,100,95,90,85,80,75,70,65,60,55,50,45,40,35,30,25,20,15,10,5
  };
  
  const unsigned char code rectWaveTable[WAVE_TABLE_LEN] = {
    // 前半为高电平，后半为低电平
    [0 ... 49] = 255,
    [50 ... 99] = 0
  };
  
  const unsigned char code sawWaveTable[WAVE_TABLE_LEN] = {
    // 线性上升
    0,3,5,8,11,13,16,18,21,23,26,28,31,33,36,38,41,43,46,48,
    51,53,56,58,61,63,66,68,71,73,76,78,81,83,86,88,91,93,96,98,
    101,103,106,108,111,113,116,118,121,123,
    126,128,131,133,136,138,141,143,146,148,151,153,156,158,161,163,166,168,171,173,
    176,178,181,183,186,188,191,193,196,198,201,203,206,208,211,213,216,218,221,223,
    226,228,231,233,236,238,241,243,246,248
  };
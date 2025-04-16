// 各种模式的具体实现
#include "headers.h"
/**
 * 实时显示
 * - ad_in，经过ADC特殊功能寄存器；
 * - 地址数据复用完成6264存储
 * - 数据传出完成DAC 0832的一路输出
 */
void realtime_mode(){
    unsigned char ad_result;
    unsigned int address_6264 = 0;
    while(address_6264 <= 0x1fff){  // 6264存储
        init_ad();
        ADC_CONTR |= 0X08;  //start ADC工作
        while(!(ADC_CONTR & 0x10)); // 等待转换完成的flag
        ad_result = ADC_RES;
        writeRAM_6264(address_6264, ad_result);
        realtime_out_0832(1);    //实时输出ad_in OUTPUT 1 
        address_6264 ++;        
    }
}

/**
 * 波形发生
 */
void generator_mode(){
    if(workmode == GENERATOR)
    if(submode == SINM){
		sinusoidalWave();
	}else if(submode == TRIANNGULARM){
		triangularWave();
	}else if(submode == RECTANGULARM){
		rectangularWave();
	}else if(submode == SAWTOOTHM){
		sawtoothWave();
	}
}
/*
void chooseGenerator(void){
	if(working_parameter == 0){
		sinusoidalWave();
	}else if(working_parameter == 1){
		triangularWave();
	}else if(working_parameter == 2){
		rectangularWave();
	}else if(working_parameter == 3){
		sawtoothWave();
	}
}
*/

void sinusoidalWave(void){
	generator_timer += working_frequency;
	if(generator_timer >= WAVE_TABLE_LEN){
		generator_timer = 0;
	}
	generator_result = sinWaveTable[generator_timer];
	generator_result = BASE_LINE + generator_result * working_amplitude / 255;
}

void triangularWave(void){
	generator_timer += working_frequency;
	if(generator_timer >= WAVE_TABLE_LEN){
		generator_timer = 0;
	}
	generator_result = triWaveTable[generator_timer];
	generator_result = BASE_LINE + generator_result * working_amplitude / 255;
}

void rectangularWave(void){
	generator_timer += working_frequency;
	if(generator_timer >= WAVE_TABLE_LEN){
		generator_timer = 0;
	}
	generator_result = rectWaveTable[generator_timer];
	generator_result = BASE_LINE + generator_result * working_amplitude / 255;
}

void sawtoothWave(void){
	generator_timer += working_frequency;
	if(generator_timer >= WAVE_TABLE_LEN){
		generator_timer = 0;
	}
	generator_result = sawWaveTable[generator_timer];
	generator_result = BASE_LINE + generator_result * working_amplitude / 255;
}

/**
 * 波形回放
 * - 保留realtime_mode下的实时输出部分
 * - 6264存储器控制信号配置，改为波形回放
 */
void recall_mode(){
    unsigned int address_6264 = 0;
    while(address_6264 <= 0x1fff){  // 在6264范围内寻址，保障recall不溢出
        init_ad();
        ADC_CONTR |= 0X08;  //start ADC工作
        while(!(ADC_CONTR & 0x10)); // 等待转换完成的flag
        P0 = ADC_RES;
        realtime_out_0832(1);    //实时输出ad_in OUTPUT 1
        readRAM_6264(address_6264);
        realout_0832(2); // 将6264中的数据输出 OUTPUT 2
        address_6264 ++;        
    }
}

/**
 * 测量模式
 * - amplitude测量
 * - frequency测量
 * 会需要调用到void sendByte_595(unsigned char byte_595) 向74HC595发送一个字节的数据
 * 
 */
void measure_mode(){
    typedef enum{ FALLING, RISING} SingnalState;
    unsigned char Sig_data, Sig_pre_data;
    unsigned char Sig_highest, Sig_lowest;
    unsigned char Sig_amp, Sig_freq, Sig_Period;
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
        
        if(workmode == MEASURE){
            if (submode == MEASUREAMP){
                sendByte_595(Sig_amp);
                display_digits();
            }
            else if(submode == MEASUREFREQ){
                sendByte_595(Sig_freq);
                display_digits();
            }
        }
}

/**
 * 默认状态（存储器内容+数码管显示）
 */
void default_mode(){
    display_digits();
}

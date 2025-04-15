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
        ADC_CONTR |= 0X08;  //start
        while(!(ADC_CONTR & 0x10)); // 等待转换完成的flag
        ad_result = ADC_RES;
        saveAD_6264(address_6264, ad_result);
        realout_0832_1(ad_result);    //将转换完成之后的数据实时输出 OUTPUT1 
        address_6264 ++;        
    }
}

void generator_mode(){
    switch (submode){
        case SINM:
            
            break;
        case TRIANNGULARM:
            break;
        case RECTANGULARM:
            break;
        case SAWTOOTHM:
            break;
        default:
            break;
    }
}
void recall_mode(){
    
}
void measure_mode(){

}

/**
 * 默认状态（存储器内容+数码管显示）
 */
void default_mode(){

}


// by yr
typedef enum {FALLING_PHASE, RISING_PHASE} SignalPhase;

void processADData(void) {
    static uint8_t stableCount = 0;
    
    if (AD_trend == RISING_PHASE) {
        if (AD_data > AD_pre_data) {
            if (stableCount == 0) AD_lowest = AD_data;
            if (++stableCount >= 5) {
                AD_trend = FALLING_PHASE;
                AD_frequency = 2000 / AD_timer;
                AD_timer = stableCount = 0;
            }
        } else {
            stableCount = 0;
        }
    } else {
        if (AD_data < AD_pre_data) {
            if (stableCount == 0) AD_highest = AD_data;
            if (++stableCount >= 5) {
                AD_trend = RISING_PHASE;
                AD_amplitude = (int)((AD_highest - AD_lowest) * 1.95);
                stableCount = 0;
            }
        } else {
            stableCount = 0;
        }
    }
}

void getAD(void) {
    /* ADC启动配置 */
    ADC_CONTR |= (1 << 3);  // 使用位偏移替代十六进制值
    for(volatile uint8_t dly = 4; dly > 0; dly--);  // 改变循环写法
    
    /* 等待转换完成 */
    while (!(ADC_CONTR & (1 << 4)) {  // 使用位检测替代等于判断
        // 空等待
    }
    
    /* 清除标志位 */
    ADC_CONTR &= ~((1 << 4) | (1 << 3));  // 显式清除两个标志位
    AD_data = ADC_RES;  // 保持数据读取不变
    
    /* 状态切换 */
    CHECK_4 ^= 1;  // 使用异或操作简化取反
    
    if(working_mode == ANALYZE_MODE) {  // 使用有意义的模式定义
        AD_timer++;
        
        /* 重构趋势检测逻辑 */
        processADData();
        
        AD_pre_data = AD_data;  // 最后更新前值
    }
}
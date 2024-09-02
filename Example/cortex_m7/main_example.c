#include <includes.h>
#include <MTKnob.h>

MT_KONB  knob1;

uint8_t  Knob1GetA(void);
uint8_t  Knob1GetB(void);
uint32_t Knob1EventCallback(MT_KONB *MTKB);

/**
  * @brief  程序入口
  * @param  None
  * @retval None
*/
void main(int argc, char *argv[])
{
    InitCpu( ); // 初始化芯片

    MTKnobInit(&knob1, &Knob1GetA, &Knob1GetB, &Knob1EventCallback);
    MTKnobStart(&knob1);

    while(1)
    {
        MTKnobTicks(1);
        HAL_Delay(1);
    }
}

/**
 * @brief 依据 MTKB_GET 类型格式，编写旋钮A相相态函数
 * @param  无
 * @return 相态返回
 */
uint8_t Knob1GetA(void)
{
    if((GPIOB->IDR & GPIO_PIN_0) != 0x00) // 检测GPIO B0电平
        return MTKB_PHASE_HIGH;
    else
        return MTKB_PHASE_LOW;
}

/**
 * @brief 依据 MTKB_GET 类型格式，编写旋钮B相相态函数
 * @param  无
 * @return 相态返回
 */
uint8_t Knob1GetB(void)
{
    if((GPIOB->IDR & GPIO_PIN_1) != 0x00) // 检测GPIO B1电平
        return MTKB_PHASE_HIGH;
    else
        return MTKB_PHASE_LOW;
}

/**
 * @brief 依据 MTKB_CALLBACK 类型格式，编写旋钮事件回调函数
 * @param  MTKB 回调触发的旋钮对象指针
 * @return 标准返回 回调处理状态
 */
uint32_t Knob1EventCallback(MT_KONB *MTKB)
{
    uint16_t event = MTKnobEventGet(MTKB);
    switch(event)
    {
    case MTKB_EVENT_A_FIRST:
        SEGGER_RTT_Write(0, "A相先发生了跳变", sizeof("A相先发生了跳变"));
        break;
    case MTKB_EVENT_B_FIRST:
        SEGGER_RTT_Write(0, "B相先发生了跳变", sizeof("B相先发生了跳变"));
        break;
    case MTKB_EVENT_A_END:
        SEGGER_RTT_Write(0, "发生了正旋CW", sizeof("发生了正旋CW"));
        break;
    case MTKB_EVENT_B_END:
        SEGGER_RTT_Write(0, "发生了逆旋CCW", sizeof("发生了逆旋CCW"));
        break;
    case MTKB_EVENT_A_TIMEOUT:
        SEGGER_RTT_Write(0, "硬件异常 A相长时间异常电平", sizeof("硬件异常 A相长时间异常电平"));
        break;
    case MTKB_EVENT_B_TIMEOUT:
        SEGGER_RTT_Write(0, "硬件异常 B相长时间异常电平", sizeof("硬件异常 B相长时间异常电平"));
        break;
    default:
        SEGGER_RTT_Write(0, "异常事件 可能存在RAM溢出", sizeof("异常事件 可能存在RAM溢出"));
        break;
    }
    return MTKB_API_RETURN_DEFAULT;
}
/********************************************************************************


 **** Copyright (C), 2024, Yuanlong Xu <Yono233@outlook.com>    ****
 **** All rights reserved                                       ****

 ********************************************************************************
 * File Name     : MTKnob.c
 * Author        : yono
 * Date          : 2024-08-23
 * Version       : 1.0
********************************************************************************/
/**************************************************************************/

/* Includes ------------------------------------------------------------------*/

#include <MTKnob.h>

/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static MT_KONB *MTKBChainRoot = NULL; // 旋钮对象链头指针
/* Private Constants ---------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/

/* 状态机定义 */
#define MTKB_STATE_IDLE    (0) /*!< 空闲态 */
#define MTKB_STATE_A_FIRST (1) /*!< A先动作的确认 */
#define MTKB_STATE_B_FIRST (2) /*!< B先动作的确认 */

/**
 * @brief 置事件并触发callback
 * @param ev 事件值
 */
#define MTKB_EVENT_CB(ev) \
    MTKB->Event = ev;     \
    MTKB->Callback(MTKB);

/**
 * @brief 状态机流转
 * @param st 状态值
 */
#define MTKB_STATE_FLOW(st) \
    MTKB->Ticks = 0;        \
    MTKB->State = st;

/* Private function prototypes -----------------------------------------------*/
static void MTKnobEngine(MT_KONB *MTKB, uint8_t cycle);
/* Private functions ---------------------------------------------------------*/

/**
 * @brief 初始化旋钮对象
 * @param MTKB 旋钮对象指针
 * @param GetA 获取A相态的函数
 * @param GetB 获取B相态的函数
 * @param Callback 事件回调函数
 */
uint32_t MTKnobInit(MT_KONB *MTKB, MTKB_GET GetA, MTKB_GET GetB, MTKB_CALLBACK Callback)
{
    /* 检查传参 */
    if(MTKB == NULL || GetA == NULL || GetB == NULL || Callback == NULL)
        return MTKB_API_RETURN_ERR_PARAM;

    uint8_t isFound = 0;
    for(MT_KONB *target = MTKBChainRoot; target; target = target->next)
    {
        if(target == MTKB)
            isFound = 1;
    }

    if(isFound == 0) // 并未存在链表中，进行memset内存初始化；
        memset(MTKB, 0, sizeof(MT_KONB));

    MTKB->GetA     = GetA;
    MTKB->GetB     = GetB;
    MTKB->Callback = Callback;
    return MTKB_API_RETURN_DEFAULT;
}

/**
 * @brief 启动旋钮工作，添加旋钮对象指针到工作链表
 * @param MTKB 旋钮对象指针
 * @return 标准返回
 */
uint32_t MTKnobStart(MT_KONB *MTKB)
{
    MT_KONB *target = MTKBChainRoot;
    while(target)
    {
        if(target == MTKB)
            return MTKB_API_RETURN_ERR_PARAM; // 已开启，无需重复
        target = target->next;
    }
    MTKB->next    = MTKBChainRoot;
    MTKBChainRoot = MTKB;
    return MTKB_API_RETURN_DEFAULT;
}

/**
 * @brief 停止旋钮工作，删除旋钮对象指针从工作链表。
 * @param MTKB 旋钮对象指针
 * @return 标准返回
 */
uint32_t MTKnobStop(MT_KONB *MTKB)
{
    MT_KONB **curr;
    for(curr = &MTKBChainRoot; *curr;)
    {
        MT_KONB *entry = *curr;
        if(entry == MTKB)
        {
            *curr = entry->next;
            return MTKB_API_RETURN_DEFAULT;
        }
        else
        {
            curr = &entry->next;
        }
    }
    return MTKB_API_RETURN_ERR_PARAM; // 未开启，无需关闭
}

/**
 * @brief 获得当前的旋钮事件
 * @param MTKB 旋钮对象指针
 * @return 旋钮对象对应的当前事件 参考 "支持的事件集"
 */
uint16_t MTKnobEventGet(MT_KONB *MTKB)
{
    return MTKB->Event;
}

/**
 * @brief 核心引擎，驱动状态机流转
 * @param MTKB 旋钮对象指针
 * @param cycle ms调用周期
 */
static void MTKnobEngine(MT_KONB *MTKB, uint8_t cycle)
{
    uint8_t A = MTKB->GetA( );
    uint8_t B = MTKB->GetB( );

    switch(MTKB->State)
    {
    case MTKB_STATE_IDLE:
        if(A == B) // 未有相差，不允许流转
            break;

        if(A != MTKB->PastA && B == MTKB->PastB) // A相先跳变
        {
            MTKB_EVENT_CB(MTKB_EVENT_A_FIRST);
            MTKB_STATE_FLOW(MTKB_STATE_A_FIRST);
        }
        else if(B != MTKB->PastB && A == MTKB->PastA) // B相先跳变
        {
            MTKB_EVENT_CB(MTKB_EVENT_B_FIRST);
            MTKB_STATE_FLOW(MTKB_STATE_B_FIRST);
        }
        break;
    case MTKB_STATE_A_FIRST:
        MTKB->Ticks += cycle;
        if(MTKB->PastB != B) // B相跳变，发生完整旋转事件)
        {
            MTKB_EVENT_CB(MTKB_EVENT_A_END);
            MTKB_STATE_FLOW(MTKB_STATE_IDLE);
        }
        if(MTKB->PastA != A) // A相脱离触发态，无论如何解除A确认态(可能是毛刺电平)
        {
            MTKB_STATE_FLOW(MTKB_STATE_IDLE);
        }
        if(MTKB->Ticks > MTKB_TIMEOUT_TICKS) // 检查异常超时
        {
            MTKB_EVENT_CB(MTKB_EVENT_A_TIMEOUT);
        }
        break;
    case MTKB_STATE_B_FIRST:
        MTKB->Ticks += cycle;
        if(MTKB->PastA != A) // A相跳变，发生完整旋转事件)
        {
            MTKB_EVENT_CB(MTKB_EVENT_B_END);
            MTKB_STATE_FLOW(MTKB_STATE_IDLE);
        }
        if(MTKB->PastB != B) // B相脱离触发态，无论如何解除B确认态(可能是毛刺电平)
        {
            MTKB_STATE_FLOW(MTKB_STATE_IDLE);
        }
        if(MTKB->Ticks > MTKB_TIMEOUT_TICKS) // 检查异常超时
        {
            MTKB_EVENT_CB(MTKB_EVENT_B_TIMEOUT);
        }
        break;
    default:
        MTKB->State = MTKB_STATE_IDLE;
        break;
    }

    MTKB->PastA = A;
    MTKB->PastB = B;
}

/**
 * @brief 必须周期调用，驱动旋钮系统的关键函数
 * @param  cycle ms调用周期
 */
void MTKnobTicks(uint32_t cycle)
{
    for(MT_KONB *target = MTKBChainRoot; target; target = target->next)
    {
        MTKnobEngine(target, cycle);
    }
}

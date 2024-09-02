/*
 * Copyright (C), 2024, Yuanlong Xu <Yono233@outlook.com> 
 * All rights reserved
 */

/**
 * @file MTKnob_api.h
 *     MTKonb Component
 *     Application Interface (API)
 * 巧简单易用的事件驱动型旋钮驱动模块
 */

#ifndef _MT_KONB_H_
#define _MT_KONB_H_
#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/

/* 简易配置 */
#define MTKB_TIMEOUT_TICKS (200) // 200ms未出现完整旋转事件意味着电平异常

/* 支持的事件集 */
#define MTKB_EVENT_A_FIRST   (1) /*!< 旋钮A先动作事件(B未变化) */
#define MTKB_EVENT_B_FIRST   (2) /*!< 旋钮B先动作事件(A未变化) */
#define MTKB_EVENT_A_END     (3) /*!< 旋钮A先动作,且AB相均回归静默(通常意味着一次正旋) */
#define MTKB_EVENT_B_END     (4) /*!< 旋钮B先动作,且AB相均回归静默(通常意味着一次反旋)  */
#define MTKB_EVENT_A_TIMEOUT (5) /*!< 旋钮A先动作,但AB没有全部回归静默，不完整的旋转事件(通常意味着硬件异常)*/
#define MTKB_EVENT_B_TIMEOUT (6) /*!< 旋钮B先动作,但AB没有全部回归静默，不完整的旋转事件(通常意味着硬件异常)*/

/* A或B相的状态 */
#define MTKB_PHASE_LOW  (0) /*!< 相态为低 */
#define MTKB_PHASE_HIGH (1) /*!< 相态为高 */

/* MTKB空参数定义 */
#define MTKB_PARA_NULL 0

/* API返回集 */
/* 通用返回 */
#define MTKB_API_RETURN_DEFAULT        0x00 //默认的无错误
#define MTKB_API_RETURN_ERR_INDEFINITE 0x01 //默认的未指定错误
#define MTKB_API_RETURN_ERR_PARAM      0x02 //传递参数错误

/* Exported types ------------------------------------------------------------*/

/* 获取相态的函数类型 */
typedef uint8_t (*MTKB_GET)(void);

/**
 * @brief 旋钮对象结构体
 */
typedef struct MT_KONB
{
    uint16_t Ticks; // tick标准计数器

    uint8_t  Event:4; // 事件寄存器
    uint8_t  State:3; // 驱动状态机寄存器
    uint8_t  First:1; // 先出现变化的相寄存 0:A相首先变化 1:B相首先变化

    uint8_t  PastA:1; // A旧相值寄存
    uint8_t  PastB:1; // B旧相值寄存
    uint8_t  reg  :6; // 备用

    /* 需绑定的port */
    MTKB_GET GetA;                              // 绑定的获取A相态函数
    MTKB_GET GetB;                              // 绑定的获取B相态函数
    uint32_t (*Callback)(struct MT_KONB *MTKB); // 绑定的处理旋钮事件函数

    /* 链表支持 */
    struct MT_KONB *next;
} MT_KONB;

/* 处理事件的函数类型 */
typedef uint32_t (*MTKB_CALLBACK)(MT_KONB *MTKB);
/* Exported variables ---------------------------------------------------------*/
/* Exported functions ---------------------------------------------------------*/

extern uint32_t MTKnobInit(MT_KONB *MTKB, MTKB_GET GetA, MTKB_GET GetB, MTKB_CALLBACK Callback);
extern uint32_t MTKnobStart(MT_KONB *MTKB);
extern uint32_t MTKnobStop(MT_KONB *MTKB);
extern uint16_t MTKnobEventGet(MT_KONB *MTKB);

extern void     MTKnobTicks(uint32_t Cycle);

#ifdef __cplusplus
}
#endif
#endif /* _MT_KONB_H_ */

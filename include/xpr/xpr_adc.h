#ifndef XPR_ADC_H
#define XPR_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

/// @brief 初始化 XPR_ADC 库
/// @retval 0   成功
/// @retval -1  失败
/// @retval 1   已经初始化过了
int XPR_ADC_Init(void);

/// @brief 释放 XPR_ADC 库内已分配的资源
void XPR_ADC_Fini(void);

/// @brief 获取支持的通道数
/// @return 返回通道数量
int XPR_ADC_GetChannelCount(void);

/// @brief 获取指定通道的采样值
/// @return 返回通道当前采样值，当失败时返回 0
int XPR_ADC_GetValue(int channel);

#ifdef __cplusplus
}
#endif

#endif // XPR_ADC_H


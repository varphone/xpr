#ifndef XPR_DRM_H
#define XPR_DRM_H

#include <stdint.h>

/// @defgroup xprdrm 数字版权管理
///
/// 提供各种数字版权管理接口, 例如: 硬件验证, 获取序列号等等
///
/// @{
///

#ifdef __cplusplus
extern "C"
{
#endif

///
/// 序列号位宽
///
#define XPR_DRM_SERIAL_BTIS  64

///
/// 序列号字节数
///
#define XPR_DRM_SERIAL_SIZE  8

#define XPR_DRM_SERIAL_STRING_NULL  "0000000000000000"
#define XPR_DRM_UUID_STRING_NULL    "00000000-0000-0000-0000-000000000000"

#ifndef XPR_DRM_CONFIGURATION_TYPE_DEFINED
#define XPR_DRM_CONFIGURATION_TYPE_DEFINED
///
/// DRM 配置类型
///
enum XPR_DRM_Configuration {
    XPR_DRM_CFG_I2C_DEV, ///< I2C 设备, 数据类型: const char*
    XPR_DRM_CFG_I2C_SLAVE_ADDR, ///< I2C 设备从地址, 数据类型: int
};
#endif // XPR_DRM_CONFIGURATION_TYPE_DEFINED

/// @brief 配置 DRM
/// @retval XPR_ERR_SUCCESS 配置成功
/// @retval XPR_ERR_ERROR 配置失败
/// @note 此接口需要在 XPR_DRM_Init() 之前调用
int XPR_DRM_Config(int request, const void* data, int size);

/// @brief 初始化 DRM
/// @retval XPR_ERR_SUCCESS 初始化成功
/// @retval XPR_ERR_ERROR 初始化失败
int XPR_DRM_Init(void);

/// @brief 释放 DRM 资源
/// @return 无
void XPR_DRM_Fini(void);

/// @brief 验证 DRM 数据
/// @retval XPR_ERR_SUCCESS 验证成功
/// @retval XPR_ERR_ERROR 验证失败
int XPR_DRM_Verify(void);

/// @brief 获取 DRM 序列号
/// @param [in,out] buffer  用于接受序列号数据的缓冲区
/// @param [in,out] length  用于接受序列号数据的缓冲区长度
/// @retval XPR_ERR_SUCCESS 获取成功
/// @retval XPR_ERR_ERROR 获取失败
int XPR_DRM_GetSerial(uint8_t* buffer, int* length);

/// @brief 获取 DRM 序列号字符串
/// @retval NULL 获取失败
/// @retval Other 字符串格式的序列号
const char* XPR_DRM_GetSerialString(void);

/// @brief 获取 DRM 全球唯一序列号字符串
/// @retval NULL 获取失败
/// @retval Other 字符串格式的全球唯一序列号
const char* XPR_DRM_GetUuidString(void);

/// @brief 安装 DRM 序列号
/// @param [in,out] data    用于安装的序列号数据
/// @param [in,out] length  用于安装的序列号数据的长度
/// @retval XPR_ERR_SUCCESS 安装成功
/// @retval XPR_ERR_ERROR 安装失败
int XPR_DRM_InstallSerial(const uint8_t* data, int length);

#ifdef __cplusplus
}
#endif

#endif // XPR_DRM_H


/*
 * File: xpr_serial.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 串口操作
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2017-05-10 07:22:04 Wednesday, 10 May
 * Last Modified : 2019-07-03 04:39:40 Wednesday, 3 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2012 - 2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2012 - 2019 Varphone Wong, Varphone.com.
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-03   varphone    更新版权信息
 * 2014-11-21   varphone    初始版本建立
 */
#ifndef XPR_SERIAL_H
#define XPR_SERIAL_H

#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief 打开串口库
/// @param [in] portAddr     端口地址：/dev/ttyS0
/// @retval -1  失败
/// @retval 0   文件描述符
/// @sa close_port()
XPR_API int XPR_OpenPort(const char* portAddr);

/// @brief 关闭串口库
/// @param [in] fd           文件描述符
/// @retval -1  失败
/// @retval 0   成功
/// @sa open_port()
XPR_API int XPR_ClosePort(int fd);

/// @brief 配置串口信息
/// @param [in] fd           文件描述符
/// @param [in] baudRate     波特率
/// @param [in] dataBit      数据位
/// @param [in] parity       校验
/// @param [in] stopBit      停止位
/// @retval -1  失败
/// @retval 0   成功
XPR_API int XPR_SetPort(int fd, int baudRate, int dataBit, char parity,
                        int stopBit);

/// @brief 读取串口数据
/// @param [in] fd           文件描述符
/// @param [in,out] buf      数据缓存
/// @param [in] length       数据长度
/// @retval -1  失败
/// @retval 0   成功
XPR_API int XPR_ReadPort(int fd, void* buf, int length);

/// @brief 串口写入数据
/// @param [in] fd           文件描述符
/// @param [in] buf          数据缓存
/// @param [in] length       数据长度
/// @retval -1  失败
/// @retval 0   成功
XPR_API int XPR_WritePort(int fd, void* buf, int length);

#ifdef __cplusplus
}
#endif

#endif // XPR_SERIAL_H

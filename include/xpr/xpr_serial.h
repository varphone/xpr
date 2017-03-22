#ifndef XPR_DEVCAPS_H
#define XPR_DEVCAPS_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>   

#ifdef __cplusplus
extern "C"
{
#endif

/// @brief 打开串口库
/// @param [in] portAddr 端口地址：/dev/ttyS0
/// @retval -1  失败
/// @retval 0   文件描述符
/// @sa close_port()
int XPR_OpenPort(const char *portAddr);  

/// @brief 关闭串口库
/// @param [in] fd           文件描述符
/// @retval -1  失败
/// @retval 0   成功
/// @sa open_port()
int XPR_ClosePort(int fd); 

/// @brief 配置串口信息
/// @param [in] fd           文件描述符
/// @param [in] baudRate    波特率
/// @param [in]dataBit    数据位
/// @param [in] parity       校验
/// @param [in] stopBit      停止位
/// @retval -1  失败
/// @retval 0   成功
int XPR_SetPort(int fd,int baudRate ,int dataBit ,char parity ,int stopBit );  

/// @brief 读取串口数据
/// @param [in] fd           文件描述符
/// @param [in] buf          数据缓存
/// @param [in] length       数据长度 
/// @retval -1  失败
/// @retval 0   成功
int XPR_ReadPort(int fd,void *buf,int length);

/// @brief 串口写入数据
/// @param [in] fd           文件描述符
/// @param [in] buf          数据缓存
/// @param [in] length       数据长度 
/// @retval -1  失败
/// @retval 0   成功
int XPR_WritePort(int fd,void *buf,int length);  

#ifdef __cplusplus
}
#endif

#endif // XPR_DEVCAPS_H

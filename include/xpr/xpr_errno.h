#ifndef XPR_ERRNO_H
#define XPF_ERRNO_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_ERR_APPID  0x80000000

typedef enum XPR_ERR_LEVEL_E
{
    EN_ERR_LEVEL_DEBUG = 0,  /* debug-level                                  */
    EN_ERR_LEVEL_INFO,       /* informational                                */
    EN_ERR_LEVEL_NOTICE,     /* normal but significant condition             */
    EN_ERR_LEVEL_WARNING,    /* warning conditions                           */
    EN_ERR_LEVEL_ERROR,      /* error conditions                             */
    EN_ERR_LEVEL_CRIT,       /* critical conditions                          */
    EN_ERR_LEVEL_ALERT,      /* action must be taken immediately             */
    EN_ERR_LEVEL_FATAL,      /* just for compatibility with previous version */
    EN_ERR_LEVEL_BUTT
}XPR_ERR_LEVEL_E;


typedef enum XPR_MOD_ID_E
{
    XPR_ID_UPS      = 0,
	XPR_ID_PLUGIN   = 1

}XPR_MOD_ID_E;

/******************************************************************************
|----------------------------------------------------------------|
| 1 |   APP_ID   |   MOD_ID    | ERR_LEVEL |   ERR_ID            |
|----------------------------------------------------------------|
|<--><--7bits----><----8bits---><--3bits---><------13bits------->|
******************************************************************************/

#define XPR_DEF_ERR( module, level, errid) \
    ((int32_t)( (XPR_ERR_APPID) | ((module) << 16 ) | ((level)<<13) | (errid) ))

/* NOTE! 下面定义的为通用错误号，每个模块都有这些错误，具体的可以需要那些就定义那些
** 所有的模块必须预留0~63的错误号，要定义通用错误外的错误请使用63+以上的错误号
*/
typedef enum XPR_EN_ERR_CODE_E
{
    EN_ERR_INVALID_DEVID = 1, /* 无效的设备ID                 */
    EN_ERR_INVALID_CHNID = 2, /* 无效的通道 ID                */
    EN_ERR_ILLEGAL_PARAM = 3, /* 至少一个参数是无效的
                               * eg, 例如无效的IP地址          */
    EN_ERR_EXIST         = 4, /* 资源已经存在
								 eg, 挂在相同的目录            */
    EN_ERR_UNEXIST       = 5, /* 资源不存在           
					             eg, 目录没有挂在就挂在节点    */
    
    EN_ERR_NULL_PTR      = 6, /* 传入的指针为空       */
    
    EN_ERR_NOT_CONFIG    = 7, /* 某些操作需要事先配置 */
                              
    EN_ERR_NOT_SUPPORT   = 8, /* 现在海不支持当前操作    */
    EN_ERR_NOT_PERM      = 9, /* 操作不允许修改
                              ** eg, 修改静态属性       */

    EN_ERR_NOMEM         = 12,/* 申请内存失败 malloc  */
    EN_ERR_NOBUF         = 13,/* failure caused by malloc buffer  */

    EN_ERR_BUF_EMPTY     = 14,/* 缓冲区为空            */
    EN_ERR_BUF_FULL      = 15,/* 缓冲区已满            */

    EN_ERR_SYS_NOTREADY  = 16,/* 系统未初始化 ，需要先调用Init或Load   */

    EN_ERR_BADADDR       = 17,/* 错误的地址, 
                              ** eg. used for copy_from_user & copy_to_user   */
    EN_ERR_BUSY          = 18,/* 资源正忙 */
                             
    EN_ERR_BUTT          = 63,/* 保留错误好的最大码，所有模块必须大于此 */
                              
}XPR_EN_ERR_CODE_E;


/* 
** 下面演示了ups模块无效参数的错误吗定义
** #define XPR_ERR_UPS_ILLEGAL_PARAM XPR_DEF_ERR(XPR_ID_UPS, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
**
*/


#ifdef __cplusplus
}
#endif

#endif //XPR_ERRNO_H

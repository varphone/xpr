#ifndef XPR_STREAMBLOCK_H
#define XPR_STREAMBLOCK_H

#include <stdint.h>

/// @defgroup xprstreamblock Stream Block Data
/// @brief     Stream Block Data.
/// @author    Varphone Wong [varphone@163.com]
/// @version   1.0.0
/// @date      2013/12/1
///
/// @{
///

/// @defgroup xprstreamblock-changes Changes
/// @{
///
/// @par 2013/12/1
///   - Initial version crated
///
/// @}
///

/// @addtogroup xprstreamblock-macros Macros
/// @{
///

/// @}
///

#ifdef __cplusplus
extern "C" {
#endif

/// @addtogroup xprstreamblock-structs 数据结构
/// @{
///

#ifndef XPR_STREAMBLOCK_TYPE_DEFINED
#define XPR_STREAMBLOCK_TYPE_DEFINED
#define XPR_STREAMBLOCK_FLAG_DISCONTINUITY        0x0001      ///< 非连续块
#define XPR_STREAMBLOCK_FLAG_TYPE_I               0x0002      ///< I帧
#define XPR_STREAMBLOCK_FLAG_TYPE_P               0x0004      ///< P帧
#define XPR_STREAMBLOCK_FLAG_TYPE_B               0x0008      ///< B帧
#define XPR_STREAMBLOCK_FLAG_TYPE_PB              0x0010      ///< P/B帧
#define XPR_STREAMBLOCK_FLAG_HEADER               0x0020      ///< 含有头部信息
#define XPR_STREAMBLOCK_FLAG_END_OF_FRAME         0x0040      ///< 帧结束
#define XPR_STREAMBLOCK_FLAG_NO_KEYFRAME          0x0080      ///< 非关键帧
#define XPR_STREAMBLOCK_FLAG_END_OF_SEQUENCE      0x0100      ///< 序列结束
#define XPR_STREAMBLOCK_FLAG_CLOCK                0x0200      ///< 参考时钟
#define XPR_STREAMBLOCK_FLAG_SCRAMBLED            0x0400      ///< 已加扰或受扰
#define XPR_STREAMBLOCK_FLAG_PREROLL              0x0800      ///< 已经解码但未显示
#define XPR_STREAMBLOCK_FLAG_CORRUPTED            0x1000      ///< 数据损坏或丢失
#define XPR_STREAMBLOCK_FLAG_TOP_FIELD_FIRST      0x2000      ///< 顶场在前
#define XPR_STREAMBLOCK_FLAG_BOTTOM_FIELD_FIRST   0x4000      ///< 底场在先
#define XPR_STREAMBLOCK_FLAG_EOF                  0x00010000     ///< End of frame (Bitmask)

// 前置声明
/// 数据流输出的结构体
struct XPR_StreamBlock;
typedef struct XPR_StreamBlock XPR_StreamBlock;

/// 流数据块资源释放回调函数
/// @param [in] block       已分配的流数据块指针
/// @return 无返回值.
typedef void (*XPR_StreamBlockDeallocator)(XPR_StreamBlock* block);

/// 流数据块
#pragma pack(8)
struct XPR_StreamBlock {
    XPR_StreamBlock* next;           ///< 后续数据块

    uint8_t*        data;           ///< 负载数据起始位置
    uint32_t        dataSize;       ///< 负载数据长度

    uint8_t*        buffer;         ///< 缓冲区起始位置
    uint32_t        bufferSize;     ///< 缓冲区长度

    uint32_t        flags;          ///< 数据包类型，是I帧，P帧
    uint32_t        samples;        ///< 音频样本数量
    uint32_t        track;          ///< 数据轨道
    uint32_t
    codec;          ///< 编码类型FOURCC('a','b','c','d')

    int64_t         pts;            ///< 数据包时戳
    int64_t         dts;            ///< 解码时戳
    int64_t         duration;       ///< 持续时长
    XPR_StreamBlockDeallocator pf_release;   ///< 数据块释放回调
};
#pragma pack()
#endif // XPR_STREAMBLOCK_TYPE_DEFINED

/// @}
///

/// @addtogroup xprstreamblock-funs Functions
/// @{
///

/// @brief 分配流数据块资源
/// @param [in] size    数据块大小，以字节为单位
/// @retval NULL    分配失败
/// @retval !NULL   已分配到的数据块内存地址
/// @note 本接口分配的数据块并不会设定释放回调函数，除非用户手工设定，否则要释放资源必须调用 XPR_StreamBlockFree() 来完成
/// @sa XPR_StreamBlockFree(), XPR_StreamBlockRelease()
XPR_StreamBlock* XPR_StreamBlockAlloc(size_t size);

/// @brief 重新分配流数据块资源
/// @param [in] size    数据块大小，以字节为单位
/// @retval NULL    分配失败
/// @retval !NULL   已分配到的数据块内存地址
/// @note 本接口分配的数据块并不会设定释放回调函数，除非用户手工设定，否则要释放资源必须调用 XPR_StreamBlockFree() 来完成
/// @warning 此接口只能用于 XPR_StreamBlockAlloc() 分配出来的对象，切勿用于用户自定义分配的对象
/// @sa XPR_StreamBlockFree(), XPR_StreamBlockRelease()
XPR_StreamBlock* XPR_StreamBlockRealloc(size_t size);

/// @brief 释放流数据块资源
/// @param [in] blk     已分配到的数据块内存地址
/// @return 无返回值
/// @note 此接口只能用于 XPR_StreamBlockAlloc() 分配出来的对象
/// @sa XPR_StreamBlockAlloc()
void XPR_StreamBlockFree(XPR_StreamBlock* blk);

/// @brief 释放流数据块资源
/// @param [in] blk     已分配到的数据块内存地址
/// @return 无返回值
/// @note 可用于 XPR_StreamBlockAlloc() 或其他自定义方式分配的数据块，当数据块内不指定释放回调时将不会执行任何操作
/// @sa XPR_StreamBlockAlloc(), XPR_StreamBlockFree()
void XPR_StreamBlockRelease(XPR_StreamBlock* blk);

/// @brief 向数据块追加数据
/// @param [in] blk     已分配到的数据块内存地址
/// @param [in] data    数据地址
/// @param [in] length  数据长度
/// @return 返回包含了新数据的数据块地址
/// @note 当数据块内缓冲长度足够时返回原来的数据块地址， 否则将返回调用 XPR_StreamBlockRealloc() 重新分配空间后的数据块地址
/// @note XPR_StreamBlockFree()
XPR_StreamBlock* XPR_StreamBlockAppend(XPR_StreamBlock* blk,
                                       uint8_t* data, size_t length);

/// @brief 清空数据块
/// @param [in] blk     已分配到的数据块内存地址
/// @return 无返回值
/// @warning 仅用于 XPR_StreamBlockAlloc()， XPR_StreamBlockRealloc() 分配的对象
/// @sa XPR_StreamBlockAlloc(), XPR_StreamBlockRealloc()
void XPR_StreamBlockClear(XPR_StreamBlock* blk);

/// @brief 复制数据块
///        复制除容量及缓冲区地址外的一切数据
/// @param [in] from    源数据块
/// @param [in] to      目的数据块
/// @retval -1  失败
/// @retval 0   成功
/// @note 当目的数据块缓冲区容量不足时会拷贝失败
int XPR_StreamBlockCopy(const XPR_StreamBlock* from, XPR_StreamBlock* to);

/// @brief 复制数据块头部信息
///        复制除缓冲区地址、缓冲区长度、负载数据地址、负载数据长度外的一切信息
/// @param [in] from    源数据块
/// @param [in] to      目的数据块
/// @return 无返回值
void XPR_StreamBlockCopyHeader(const XPR_StreamBlock* from, XPR_StreamBlock* to);

/// @brief 复制数据块
///        复制除缓冲区地址外的一切数据
/// @param [in] blk     已分配到的数据块内存地址
/// @return 已复制好的数据块内存地址
XPR_StreamBlock* XPR_StreamBlockDuplicate(const XPR_StreamBlock* blk);

/// @brief 获取数据块的缓冲区地址
uint8_t* XPR_StreamBlockBuffer(const XPR_StreamBlock* blk);

/// @brief 获取数据块的负载数据地址
uint8_t* XPR_StreamBlockData(const XPR_StreamBlock* blk);

/// @brief 获取数据块的负载数据长度
size_t XPR_StreamBlockLength(const XPR_StreamBlock* blk);

/// @brief 获取数据块的缓冲区长度
size_t XPR_StreamBlockSize(const XPR_StreamBlock* blk);

/// @brief 获取数据块的缓冲区剩余空间
size_t XPR_StreamBlockSpace(const XPR_StreamBlock* blk);

/// @brief 获取数据块的负载数据的编码格式
uint32_t XPR_StreamBlockCodec(const XPR_StreamBlock* blk);

/// @brief 获取数据块的标志值
uint32_t XPR_StreamBlockFlags(const XPR_StreamBlock* blk);

/// @brief 获取数据块的负载数据的轨道标识
uint32_t XPR_StreamBlockTrack(const XPR_StreamBlock* blk);

/// @brief 获取数据块的负载数据的时戳
int64_t XPR_StreamBlockPTS(const XPR_StreamBlock* blk);

/// @brief 获取数据块的负载数据的解码时戳
int64_t XPR_StreamBlockDTS(const XPR_StreamBlock* blk);

/// @brief 获取数据块的负载数据的内容持续时长
int64_t XPR_StreamBlockDuration(const XPR_StreamBlock* blk);

/// @}
///

/// @}
///

#ifdef __cplusplus
}
#endif

#endif // XPR_STREAMBLOCK_H


#ifndef XPR_FIFO_H
#define XPR_FIFO_H

#include <stdint.h>
#include <xpr/xpr_common.h>

/// @defgroup xpr-fifo 先进先出队列
/// @brief 通用先进先出队列操作接口
/// @{
///

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_FIFO_TYPE_DEFINED
#define XPR_FIFO_TYPE_DEFINED
struct XPR_Fifo;
typedef struct XPR_Fifo XPR_Fifo;
#endif // XPR_FIFO_TYPE_DEFINED

/// @brief Create an fifo
/// @param [in] elementSize     Size of the element hold in the fifo
/// @param [in] maxElements     Number of elements can be hold in the fifo
/// @return Address of the fifo, NULL on failure
XPR_API XPR_Fifo* XPR_FifoCreate(int elementSize, int maxElements);

/// @brief Destroy an fifo
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_FifoDestroy(XPR_Fifo* f);

/// @brief Get and remove an element from the fifo
/// @param [in] f               Address of the fifo to be used.
/// @param [in,out] buffer      Buffer to receive the element data.
/// @param [in] size            Number of elements to get.
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_FifoGet(XPR_Fifo* f, void* buffer, int size);

/// @brief Put element(s) to fifo
/// @param [in] f               Address of the fifo to be used.
/// @param [in] data            Element(s) data address.
/// @param [in] size            Number of elements to put.
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_FifoPut(XPR_Fifo* f, const void* data, int size);

/// @brief Get data from the fifo without removing
/// @param [in] f           Address of the fifo to be used
/// @param [in] buffer      Buffer to receive the element data
/// @param [in] size        Number of elements to get
/// @param [in] offset      Offset to seek
/// @return returns 0 if the fifo was empty. Otherwise it returns the number processed elements
XPR_API int XPR_FifoPeek(XPR_Fifo* f, void* buffer, int size, int offset);

/// @brief Get one element data as atomic value from the fifo with removing.
/// @param [in] f           Address of the fifo to be used.
/// @param [in] size        Number of elements to get.
/// @param [in] offset      Offset to seek.
/// @return returns 0 if the fifo was empty.
XPR_API uintptr_t XPR_FifoGetAsAtomic(XPR_Fifo* f);

/// @brief Put data as atomic
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_FifoPutAsAtomic(XPR_Fifo* f, uintptr_t data);

XPR_API uintptr_t XPR_FifoPeekAsAtomic(XPR_Fifo* f, int offset);

/// @brief Removes the elements at front of the fifo
/// @param [in] f           Address of the fifo to be used
XPR_API int XPR_FifoDrain(XPR_Fifo* f, int size);

/// @brief Removes the entire fifo content
/// @param [in] f           Address of the fifo to be used
XPR_API void XPR_FifoReset(XPR_Fifo* f);

/// @brief Returns the available size of the fifo in elements
/// @param [in] f           Address of the fifo to be used
XPR_API int XPR_FifoGetAvailableSize(const XPR_Fifo* f);

/// @brief Returns the size of the element managed by the fifo
/// @param [in] f           Address of the fifo to be used
XPR_API int XPR_FifoGetElementSize(const XPR_Fifo* f);

/// @brief Returns the size of the fifo in elements
/// @param [in] f           Address of the fifo to be used
XPR_API int XPR_FifoGetSize(const XPR_Fifo* f);

/// @brief Returns the number of used elements in the fifo
/// @param [in] f           Address of the fifo to be used
XPR_API int XPR_FifoGetLength(const XPR_Fifo* f);

/// @brief Returns true if the fifo is empty
/// @param [in] f           Address of the fifo to be used
XPR_API int XPR_FifoIsEmpty(const XPR_Fifo* f);

/// @brief Returns true if the fifo is full
/// @param [in] f           Address of the fifo to be used
XPR_API int XPR_FifoIsFull(const XPR_Fifo* f);

#ifdef __cplusplus
}
#endif

/// @}
///

#endif // XPR_FIFO_H


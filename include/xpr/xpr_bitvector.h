#ifndef XPR_BITVECTOR_H
#define XPR_BITVECTOR_H

#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

///
/// 使用 NOB 方式创建对象时所需缓存区大小
/// @sa XPR_BitVectorNOB()
///
#define XPR_BITVECTOR_NOB_SIZE  128

#ifndef XPR_BITVECTOR_TYPE_DEFINED
#define XPR_BITVECTOR_TYPE_DEFINED
struct XPR_BitVector;
typedef struct XPR_BitVector XPR_BitVector;
#endif // XPR_BITVECTOR_TYPE_DEFINED

XPR_API XPR_BitVector* XPR_BitVectorNew(unsigned char* data, unsigned int offset,
				                        unsigned int totalBits);

XPR_API XPR_BitVector* XPR_BitVectorNOB(void* buffer, unsigned int bufferSize,
				                        unsigned char* data, unsigned int offset,
						                unsigned int totalBits);

XPR_API void XPR_BitVectorDestroy(XPR_BitVector* bv);

XPR_API void XPR_BitVectorSetup(XPR_BitVector* bv, unsigned char* data,
				                unsigned int offset, unsigned int totalBits);

XPR_API void XPR_BitVectorPutBits(XPR_BitVector* bv, unsigned int bits, unsigned int nb);

XPR_API void XPR_BitVectorPut1Bit(XPR_BitVector* bv, unsigned int bit);

XPR_API unsigned int XPR_BitVectorGetBits(XPR_BitVector* bv, unsigned int nb);

XPR_API unsigned int XPR_BitVectorGet1Bit(XPR_BitVector* bv);

#define XPR_BitVectorGetBoolean(x) XPR_BitVectorGet1Bit(x)

XPR_API unsigned int XPR_BitVectorGetExpGolomb(XPR_BitVector* bv);

XPR_API void XPR_BitVectorRollback(XPR_BitVector* bv, unsigned int nb);

XPR_API void XPR_BitVectorSkip(XPR_BitVector* bv, unsigned int nb);

XPR_API unsigned int XPR_BitVectorCurPos(const XPR_BitVector* bv);

XPR_API unsigned int XPR_BitVectorTotalBits(const XPR_BitVector* bv);

XPR_API unsigned int XPR_BitVectorRemainBits(const XPR_BitVector* bv);

#ifdef __cplusplus
}
#endif

#endif // XPR_BITVECTOR_H


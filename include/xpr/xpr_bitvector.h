﻿#ifndef XPR_BITVECTOR_H
#define XPR_BITVECTOR_H

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

XPR_BitVector* XPR_BitVectorNew(unsigned char* data, unsigned int offset,
                                unsigned int totalBits);

XPR_BitVector* XPR_BitVectorNOB(void* buffer, unsigned int bufferSize,
                                unsigned char* data, unsigned int offset,
                                unsigned int totalBits);

void XPR_BitVectorDestroy(XPR_BitVector* bv);

void XPR_BitVectorSetup(XPR_BitVector* bv, unsigned char* data,
                        unsigned int offset, unsigned int totalBits);

void XPR_BitVectorPutBits(XPR_BitVector* bv, unsigned int bits, unsigned int nb);

void XPR_BitVectorPut1Bit(XPR_BitVector* bv, unsigned int bit);

unsigned int XPR_BitVectorGetBits(XPR_BitVector* bv, unsigned int nb);

unsigned int XPR_BitVectorGet1Bit(XPR_BitVector* bv);

#define XPR_BitVectorGetBoolean(x) XPR_BitVectorGet1Bit(x)

unsigned int XPR_BitVectorGetExpGolomb(XPR_BitVector* bv);

void XPR_BitVectorRollback(XPR_BitVector* bv, unsigned int nb);

void XPR_BitVectorSkip(XPR_BitVector* bv, unsigned int nb);

unsigned int XPR_BitVectorCurPos(const XPR_BitVector* bv);

unsigned int XPR_BitVectorTotalBits(const XPR_BitVector* bv);

unsigned int XPR_BitVectorRemainBits(const XPR_BitVector* bv);

#ifdef __cplusplus
}
#endif

#endif // XPR_BITVECTOR_H


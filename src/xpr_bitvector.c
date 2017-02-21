#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_bitvector.h>

#define XPR_BITVECTOR_FLAG_NOB      0x0001

struct XPR_BitVector {
    unsigned int flags;
    unsigned char* data;
    unsigned int offset;
    unsigned int totalBits;
    unsigned int curPos;
};

static const unsigned char singleBitMask[8] = 
    {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

#define MAX_LENGTH 32

static void ShiftBits(unsigned char* toBasePtr, unsigned int toBitOffset,
                      const unsigned char* fromBasePtr,
                      unsigned int fromBitOffset, unsigned int numBits)
{
    /* Note that from and to may overlap, if from>to */
    unsigned char const* fromBytePtr = fromBasePtr + fromBitOffset / 8;
    unsigned int fromBitRem = fromBitOffset % 8;
    unsigned char* toBytePtr = toBasePtr + toBitOffset / 8;
    unsigned int toBitRem = toBitOffset % 8;
    unsigned char fromBitMask = 0;
    unsigned char fromBit = 0;
    unsigned char toBitMask = 0;
    //
    if (numBits == 0)
        return;
    //
    while (numBits-- > 0) {
        fromBitMask = singleBitMask[fromBitRem];
        fromBit = (*fromBytePtr)&fromBitMask;
        toBitMask = singleBitMask[toBitRem];
        if (fromBit != 0) {
            *toBytePtr |= toBitMask;
        }
        else {
            *toBytePtr &= ~ toBitMask;
        }
        if (++fromBitRem == 8) {
            ++fromBytePtr;
            fromBitRem = 0;
        }
        if (++toBitRem == 8) {
            ++toBytePtr;
            toBitRem = 0;
        }
    }
}

XPR_BitVector* XPR_BitVectorNew(unsigned char* data,
                                unsigned int offset,
                                unsigned int totalBits)
{
    XPR_BitVector* bv = calloc(sizeof(*bv), 1);
    XPR_BitVectorSetup(bv, data, offset, totalBits);
    return bv;
}

XPR_BitVector* XPR_BitVectorNOB(void* buffer, unsigned int bufferSize,
                                unsigned char* data, unsigned int offset,
                                unsigned int totalBits)
{
    XPR_BitVector* bv = (XPR_BitVector*)buffer;
    if (bufferSize < XPR_BITVECTOR_NOB_SIZE)
        return 0;
    bv->flags |= XPR_BITVECTOR_FLAG_NOB;
    XPR_BitVectorSetup(bv, data, offset, totalBits);
    return bv;
}

void XPR_BitVectorDestroy(XPR_BitVector* bv)
{
    if (bv && (bv->flags & XPR_BITVECTOR_FLAG_NOB))
        free((void*)bv);
}

void XPR_BitVectorSetup(XPR_BitVector* bv, unsigned char* data,
                        unsigned int offset, unsigned int totalBits)
{
    if (bv) {
        bv->data = data;
        bv->offset = offset;
        bv->totalBits = totalBits;
        bv->curPos = 0;
    }
}

void XPR_BitVectorPutBits(XPR_BitVector* bv, unsigned int bits,
                          unsigned int numBits)
{
    unsigned char tmpBuf[4];
    unsigned int overflowingBits = 0;
    if (numBits == 0)
        return;
    if (numBits > MAX_LENGTH) {
        numBits = MAX_LENGTH;
    }
    if (numBits > bv->totalBits - bv->curPos) {
        overflowingBits = numBits - (bv->totalBits - bv->curPos);
    }
    tmpBuf[0] = (unsigned char)(bits >> 24);
    tmpBuf[1] = (unsigned char)(bits >> 16);
    tmpBuf[2] = (unsigned char)(bits >> 8);
    tmpBuf[3] = (unsigned char)bits;
    ShiftBits(bv->data, bv->offset + bv->curPos, /* to */
              tmpBuf, MAX_LENGTH - numBits, /* from */
              numBits - overflowingBits /* num bits */);
    bv->curPos += numBits - overflowingBits;
}

void XPR_BitVectorPut1Bit(XPR_BitVector* bv, unsigned int bit)
{
    // The following is equivalent to "putBits(..., 1)", except faster:
    unsigned int totBitOffset = bv->offset + bv->curPos++;
    unsigned char mask = singleBitMask[totBitOffset % 8];
    if (bv->curPos >= bv->totalBits) { /* overflow */
        return;
    }
    //
    if (bit) {
        bv->data[totBitOffset / 8] |= mask;
    }
    else {
        bv->data[totBitOffset / 8] &= ~mask;
    }
}

unsigned int XPR_BitVectorGetBits(XPR_BitVector* bv, unsigned int numBits)
{
    unsigned char tmpBuf[4];
    unsigned int overflowingBits = 0;
    unsigned int result = 0;
    //
    if (numBits == 0)
        return 0;
    if (numBits > MAX_LENGTH) {
        numBits = MAX_LENGTH;
    }
    if (numBits > bv->totalBits - bv->curPos) {
        overflowingBits = numBits - (bv->totalBits - bv->curPos);
    }
    ShiftBits(tmpBuf, 0, /* to */
              bv->data, bv->offset + bv->curPos, /* from */
              numBits - overflowingBits /* num bits */);
    bv->curPos += numBits - overflowingBits;

    result = (tmpBuf[0] << 24) | (tmpBuf[1] << 16) | (tmpBuf[2] << 8) | tmpBuf[3];
    result >>= (MAX_LENGTH - numBits); // move into low-order part of word
    result &= (0xFFFFFFFF << overflowingBits); // so any overflow bits are 0
    return result;
}

unsigned int XPR_BitVectorGet1Bit(XPR_BitVector* bv)
{
    // The following is equivalent to "getBits(1)", except faster:
    unsigned int totBitOffset = bv->offset + bv->curPos++;
    unsigned char curFromByte = bv->data[totBitOffset / 8];
    unsigned int result = (curFromByte >> (7 - (totBitOffset % 8))) & 0x01;
    if (bv->curPos >= bv->totalBits) { /* overflow */
        return 0;
    }
    return result;
}

unsigned int XPR_BitVectorGetExpGolomb(XPR_BitVector* bv)
{
    unsigned int numLeadingZeroBits = 0;
    unsigned int codeStart = 1;
    while (XPR_BitVectorGet1Bit(bv) == 0 && bv->curPos < bv->totalBits) {
        ++numLeadingZeroBits;
        codeStart *= 2;
    }
    return codeStart - 1 + XPR_BitVectorGetBits(bv, numLeadingZeroBits);
}

void XPR_BitVectorRollback(XPR_BitVector* bv, unsigned int numBits)
{
    if (numBits > bv->curPos) { /* overflow */
        bv->curPos = 0;
    }
    else {
        bv->curPos -= numBits;
    }
}

void XPR_BitVectorSkip(XPR_BitVector* bv, unsigned int numBits)
{
    if (numBits > bv->totalBits - bv->curPos) { /* overflow */
        bv->curPos = bv->totalBits;
    }
    else {
        bv->curPos += numBits;
    }
}

unsigned int XPR_BitVectorCurPos(const XPR_BitVector* bv)
{
    return bv ? bv->curPos : 0;
}

unsigned int XPR_BitVectorTotalBits(const XPR_BitVector* bv)
{
    return bv ? bv->totalBits : 0;
}

unsigned int XPR_BitVectorRemainBits(const XPR_BitVector* bv)
{
    return bv ? bv->totalBits - bv->curPos : 0;
}


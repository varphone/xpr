#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_streamblock.h>
#include <xpr/xpr_utils.h>

XPR_StreamBlock* XPR_StreamBlockAlloc(int size)
{
    XPR_StreamBlock* blk = 0;
    if (size > XPR_STREAMBLOCK_MAX_SIZE) {
        DBG(DBG_L2, "stream block is too large to alloc (%u > %u)", size, XPR_STREAM_BLOCK_MAX_SIZE);
        return 0;
    }
    size = XPR_AlignedUpTo(size, 256);
    blk = (XPR_StreamBlock*)XPR_Alloc(sizeof(*blk)+size);
    if (blk) {
        memset(blk, 0, sizeof(*blk));
        blk->buffer = (uint8_t*)blk + sizeof(*blk);
        blk->bufferSize = (uint32_t)size;
        blk->data = blk->buffer;
        blk->dataSize = 0;
    }
    return blk;
}

XPR_StreamBlock* XPR_StreamBlockRealloc(XPR_StreamBlock* blk, int size)
{
    XPR_StreamBlock* nblk = 0;
    if (size > XPR_STREAMBLOCK_MAX_SIZE) {
        DBG(DBG_L2, "stream block is too large to realloc (%u > %u)", size, XPR_STREAM_BLOCK_MAX_SIZE);
        return blk;
    }
    size = XPR_AlignedUpTo(size, 256);
    nblk = (XPR_StreamBlock*)realloc(blk, sizeof(*nblk) + size);
    if (nblk) {
        nblk->buffer = (uint8_t*)nblk + sizeof(*nblk);
        nblk->bufferSize = (uint32_t)size;
        nblk->data = nblk->buffer;
    }
    return nblk;
}

void XPR_StreamBlockFree(XPR_StreamBlock* blk)
{
    if (blk) {
        XPR_Free((void*)blk);
    }
}

void XPR_StreamBlockRelease(XPR_StreamBlock* blk)
{
    if (blk) {
        if (blk->pf_release)
            blk->pf_release(blk);
    }
}

XPR_StreamBlock* XPR_StreamBlockAppend(XPR_StreamBlock* blk, uint8_t* data, int length)
{
    size_t space = 0;
    if (!blk || !data || !length)
        return blk;
    if (length > XPR_STREAMBLOCK_MAX_SIZE) {
        DBG(DBG_L2, "data block [%p, %u] is too large to append to [%p]", data, length, blk);
        return blk;
    }
    space = blk->bufferSize - blk->dataSize;
    if (space < length)
        blk = XPR_StreamBlockRealloc(blk, blk->bufferSize + (length - space));
    if (blk) {
        memcpy(blk->data + blk->dataSize, data, length);
        blk->dataSize += (uint32_t)length;
    } else {
        DBG(DBG_L2, "data block [%p, %u] unable append to stream block [%p], realloc failure", blk);
    }
    return blk;
}

void XPR_StreamBlockClear(XPR_StreamBlock* blk)
{
    if (blk) {
        blk->dataSize = 0;
        blk->flags = 0;
        blk->samples = 0;
        blk->codec = 0;
        blk->track = 0;
        blk->dts = 0;
        blk->pts = 0;
        blk->duration = 0;
    }
}

int XPR_StreamBlockCopy(const XPR_StreamBlock* from, XPR_StreamBlock* to)
{
    if (!from || !to)
        return -1;
    if (to->bufferSize < from->dataSize)
        return -1;
    memcpy(to->data, from->data, from->dataSize);
    to->dataSize = from->dataSize;
    to->flags = from->flags;
    to->samples = from->samples;
    to->codec = from->codec;
    to->track = from->track;
    to->pts = from->pts;
    to->dts = from->dts;
    to->duration = from->duration;
    return 0;
}

void XPR_StreamBlockCopyHeader(const XPR_StreamBlock* from, XPR_StreamBlock* to)
{
    if (from && to) {
        to->flags = from->flags;
        to->samples = from->samples;
        to->codec = from->codec;
        to->track = from->track;
        to->pts = from->pts;
        to->dts = from->dts;
        to->duration = from->duration;
    }
}

XPR_StreamBlock* XPR_StreamBlockDuplicate(const XPR_StreamBlock* blk)
{
    size_t size = MIN(blk->bufferSize ? blk->bufferSize : blk->dataSize, blk->dataSize);
    XPR_StreamBlock* nblk = 0;

    if (!blk)
        return NULL;
    nblk = XPR_StreamBlockAlloc(size);
    if (nblk) {
        //*nblk = *blk;
        //nblk->buffer = (uint8_t*)nblk + sizeof(*nblk);
        //nblk->bufferSize = size;
        //nblk->data = nblk->buffer;
        XPR_StreamBlockCopyHeader(blk, nblk);
        nblk->dataSize = blk->dataSize;
        memcpy(nblk->data, blk->data, blk->dataSize);
    }
    return nblk;
}

uint8_t* XPR_StreamBlockBuffer(const XPR_StreamBlock* blk)
{
    return blk->buffer;
}

uint8_t* XPR_StreamBlockData(const XPR_StreamBlock* blk)
{
    return blk->data;
}

int XPR_StreamBlockLength(const XPR_StreamBlock* blk)
{
    return blk->dataSize;
}

int XPR_StreamBlockSize(const XPR_StreamBlock* blk)
{
    return blk->bufferSize;
}

int XPR_StreamBlockSpace(const XPR_StreamBlock* blk)
{
    return blk->bufferSize - blk->dataSize;
}

uint32_t XPR_StreamBlockCodec(const XPR_StreamBlock* blk)
{
    return blk->codec;
}

uint32_t XPR_StreamBlockFlags(const XPR_StreamBlock* blk)
{
    return blk->flags;
}

uint32_t XPR_StreamBlockSamples(const XPR_StreamBlock* blk)
{
    return blk->samples;
}

uint32_t XPR_StreamBlockTrack(const XPR_StreamBlock* blk)
{
    return blk->track;
}

int64_t XPR_StreamBlockPTS(const XPR_StreamBlock* blk)
{
    return blk->pts;
}

int64_t XPR_StreamBlockDTS(const XPR_StreamBlock* blk)
{
    return blk->dts;
}

int64_t XPR_StreamBlockDuration(const XPR_StreamBlock* blk)
{
    return blk->duration;
}

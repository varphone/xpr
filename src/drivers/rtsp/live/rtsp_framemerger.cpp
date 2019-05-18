#include "rtsp_framemerger.hpp"
#include <xpr/xpr_utils.h>
#include <algorithm>
#include <cstring>
#include <memory>
#include <string>

namespace xpr
{
namespace rtsp
{

GenericFrameMerger::GenericFrameMerger(void)
    : mBuffer(nullptr)
    , mMaxFrameSize(64 * 1024)
    , mMergedSize(0)
    , mMergedCallback(nullptr)
    , mMergedCallbackOpaque(nullptr)
    , mStreamBlock()
{
    mBuffer = new uint8_t[mMaxFrameSize];
}

GenericFrameMerger::~GenericFrameMerger(void)
{
    if (mBuffer) {
        delete[] mBuffer;
        mBuffer = nullptr;
    }
}

void GenericFrameMerger::merge(XPR_StreamBlock const& stb)
{
    if (mMergedSize > 0 && mStreamBlock.pts != stb.pts) {
        mStreamBlock.buffer = mBuffer;
        mStreamBlock.bufferSize = mMaxFrameSize;
        mStreamBlock.data = mBuffer;
        mStreamBlock.dataSize = mMergedSize;
        if (mMergedCallback)
            mMergedCallback(mMergedCallbackOpaque, mStreamBlock);
        mMergedSize = 0;
    }
    if (mMergedSize == 0) {
        mStreamBlock = stb;
    }
    size_t avail = mMaxFrameSize - mMergedSize;
    size_t ncopy = std::min(avail, static_cast<size_t>(stb.dataSize));
    std::memcpy(mBuffer + mMergedSize, stb.data, ncopy);
    mMergedSize += ncopy;
    // Use flags of the last fragment as merged flags.
    mStreamBlock.flags = stb.flags;
}

void GenericFrameMerger::setMaxFrameSize(size_t size)
{
    if (mMaxFrameSize < size) {
        uint8_t* newBuffer = new uint8_t[size];
        if (newBuffer == nullptr) {
            DBG(DBG_L1, "XPR_RTSP: GenericFrameMerger: Not enough memory!");
        }
        else {
            std::memcpy(newBuffer, mBuffer, mMergedSize);
            delete[] mBuffer;
            mBuffer = newBuffer;
            mMaxFrameSize = size;
        }
    }
}

void GenericFrameMerger::setMergedCallback(MergedCallback cb, void* opaque)
{
    mMergedCallback = cb;
    mMergedCallbackOpaque = opaque;
}

} // namespace rtsp
} // namespace xpr
#ifndef XPR_RTSP_DRIVER_LIVE_RTSP_FRAMEMERGER_HPP
#define XPR_RTSP_DRIVER_LIVE_RTSP_FRAMEMERGER_HPP

#include "rtsp.hpp"

namespace xpr
{
namespace rtsp
{

typedef void (*MergedCallback)(void*, XPR_StreamBlock const& stb);

class GenericFrameMerger {
public:
    GenericFrameMerger(void);

    void setMaxFrameSize(size_t size);
    void setMergedCallback(MergedCallback cb, void* opaque);

    virtual ~GenericFrameMerger(void);
    virtual void merge(XPR_StreamBlock const& stb);

protected:
    uint8_t* mBuffer;
    size_t mMaxFrameSize;
    size_t mMergedSize;
    MergedCallback mMergedCallback;
    void* mMergedCallbackOpaque;
    XPR_StreamBlock mStreamBlock;
};

} // namespace rtsp
} // namespace xpr

#endif // XPR_RTSP_DRIVER_LIVE_RTSP_FRAMEMERGER_HPP
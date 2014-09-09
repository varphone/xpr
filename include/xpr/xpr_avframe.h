#ifndef XPR_AVFRAME_H
#define XPR_AVFFAME_H

#include <stdint.h>

/// @defgroup xpravframe Audio/Video raw data
/// @brief     Audio/Video raw data library.
/// @author    Varphone Wong [varphone@163.com]
/// @version   1.0.0
/// @date      2013/12/1
///
/// @{
///

/// @defgroup xpravframe-changes Changes
/// @{
///
/// @par 2013/12/1
///   - Initial version crated
///
/// @}
///

#ifdef __cplusplus
extern "C" {
#endif

/// @addtogroup xpravframe-macros Macros
/// @{
///

#define XPR_AVFRAME_PLANES   8

/// @}
///

/// @addtogroup xpravframe-structs Structs
/// @{
///

#ifndef XPR_AVFRAME_TYPE_DEFINED
#define XPR_AVFRAME_TYPE_DEFINED
// Forwards
struct XPR_AVFrame;

/// @brief AVFrame release callback routine
/// @param [in] frame       AVFrame instance
/// @return the reference count after decreased
typedef int (*XPR_AVFrameReleaseCallback)(XPR_AVFrame* frame);

/// @brief This structure describes decoded (raw) audio or video data
struct XPR_AVFrame {
    uint8_t* datas[XPR_AVFRAME_PLANES];         ///< Pointer to the picture/channel planes
    int pitches[XPR_AVFRAME_PLANES];            ///< For video, size in bytes of each picture line
    int format;                 ///< Format of the frame, -1 if unknown or unset Values, correspond to enum XPR_AVFrameFormat
    int width;                  ///< Width of the video frame
    int height;                 ///< Height of the video frame
    int samples;                ///< Number of audio samples (per channel) described by this frame
    int sampleRate;             ///< Sample rate of the audio data
    uint64_t channelLayout;     ///< Channel layout of the audio data
    int flags;                  ///< Frame flags, a combination of XPR_AVFRAME_FLAG_*
    int64_t pts;                ///< Packet/Present timestamp
    int64_t dts;                ///< Data/Decode/Display timestamp
    void* opaque;               ///< For some private data of the user
    XPR_AVFrameReleaseCallback release;  ///< Release callback routine
};
/// @brief Type of AVFrame
typedef struct XPR_AVFrame XPR_AVFrame;
#endif // XPR_AVFRAME_TYPE_DEFINED

/// @}
///

/// @addtogroup xpravframe-enums Enums
/// @{
///

#ifndef XPR_AVPIXELFORMAT_TYPE_DEFINED
#define XPR_AVPIXELFORMAT_TYPE_DEFINED
/// @brief Video Pixel Format
enum XPR_AVPixelFormat {
    XPR_AV_PIX_FMT_NONE = -1,
    XPR_AV_PIX_FMT_YUV420P,   ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    XPR_AV_PIX_FMT_YUYV422,   ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    XPR_AV_PIX_FMT_RGB24,     ///< packed RGB 8:8:8, 24bpp, RGBRGB...
    XPR_AV_PIX_FMT_BGR24,     ///< packed RGB 8:8:8, 24bpp, BGRBGR...
    XPR_AV_PIX_FMT_YUV422P,   ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    XPR_AV_PIX_FMT_YUV444P,   ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    XPR_AV_PIX_FMT_YUV410P,   ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    XPR_AV_PIX_FMT_YUV411P,   ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    XPR_AV_PIX_FMT_GRAY8,     ///<        Y        ,  8bpp
    XPR_AV_PIX_FMT_MONOWHITE, ///<        Y        ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
    XPR_AV_PIX_FMT_MONOBLACK, ///<        Y        ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
    XPR_AV_PIX_FMT_PAL8,      ///< 8 bit with PIX_FMT_RGB32 palette
    XPR_AV_PIX_FMT_YUVJ420P,  ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV420P and setting color_range
    XPR_AV_PIX_FMT_YUVJ422P,  ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV422P and setting color_range
    XPR_AV_PIX_FMT_YUVJ444P,  ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of PIX_FMT_YUV444P and setting color_range
    XPR_AV_PIX_FMT_XVMC_MPEG2_MC,///< XVideo Motion Acceleration via common packet passing
    XPR_AV_PIX_FMT_XVMC_MPEG2_IDCT,
    XPR_AV_PIX_FMT_UYVY422,   ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    XPR_AV_PIX_FMT_UYYVYY411, ///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    XPR_AV_PIX_FMT_BGR8,      ///< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    XPR_AV_PIX_FMT_BGR4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    XPR_AV_PIX_FMT_BGR4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    XPR_AV_PIX_FMT_RGB8,      ///< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    XPR_AV_PIX_FMT_RGB4,      ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    XPR_AV_PIX_FMT_RGB4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
    XPR_AV_PIX_FMT_NV12,      ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    XPR_AV_PIX_FMT_NV21,      ///< as above, but U and V bytes are swapped

    XPR_AV_PIX_FMT_ARGB,      ///< packed ARGB 8:8:8:8, 32bpp, ARGBARGB...
    XPR_AV_PIX_FMT_RGBA,      ///< packed RGBA 8:8:8:8, 32bpp, RGBARGBA...
    XPR_AV_PIX_FMT_ABGR,      ///< packed ABGR 8:8:8:8, 32bpp, ABGRABGR...
    XPR_AV_PIX_FMT_BGRA,      ///< packed BGRA 8:8:8:8, 32bpp, BGRABGRA...

    XPR_AV_PIX_FMT_GRAY16BE,  ///<        Y        , 16bpp, big-endian
    XPR_AV_PIX_FMT_GRAY16LE,  ///<        Y        , 16bpp, little-endian
    XPR_AV_PIX_FMT_YUV440P,   ///< planar YUV 4:4:0 (1 Cr & Cb sample per 1x2 Y samples)
    XPR_AV_PIX_FMT_YUVJ440P,  ///< planar YUV 4:4:0 full scale (JPEG), deprecated in favor of PIX_FMT_YUV440P and setting color_range
    XPR_AV_PIX_FMT_YUVA420P,  ///< planar YUV 4:2:0, 20bpp, (1 Cr & Cb sample per 2x2 Y & A samples)
    XPR_AV_PIX_FMT_VDPAU_H264,///< H.264 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    XPR_AV_PIX_FMT_VDPAU_MPEG1,///< MPEG-1 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    XPR_AV_PIX_FMT_VDPAU_MPEG2,///< MPEG-2 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    XPR_AV_PIX_FMT_VDPAU_WMV3,///< WMV3 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    XPR_AV_PIX_FMT_VDPAU_VC1, ///< VC-1 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    XPR_AV_PIX_FMT_RGB48BE,   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as big-endian
    XPR_AV_PIX_FMT_RGB48LE,   ///< packed RGB 16:16:16, 48bpp, 16R, 16G, 16B, the 2-byte value for each R/G/B component is stored as little-endian

    XPR_AV_PIX_FMT_RGB565BE,  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), big-endian
    XPR_AV_PIX_FMT_RGB565LE,  ///< packed RGB 5:6:5, 16bpp, (msb)   5R 6G 5B(lsb), little-endian
    XPR_AV_PIX_FMT_RGB555BE,  ///< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), big-endian, most significant bit to 0
    XPR_AV_PIX_FMT_RGB555LE,  ///< packed RGB 5:5:5, 16bpp, (msb)1A 5R 5G 5B(lsb), little-endian, most significant bit to 0

    XPR_AV_PIX_FMT_BGR565BE,  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), big-endian
    XPR_AV_PIX_FMT_BGR565LE,  ///< packed BGR 5:6:5, 16bpp, (msb)   5B 6G 5R(lsb), little-endian
    XPR_AV_PIX_FMT_BGR555BE,  ///< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), big-endian, most significant bit to 1
    XPR_AV_PIX_FMT_BGR555LE,  ///< packed BGR 5:5:5, 16bpp, (msb)1A 5B 5G 5R(lsb), little-endian, most significant bit to 1

    XPR_AV_PIX_FMT_VAAPI_MOCO, ///< HW acceleration through VA API at motion compensation entry-point, Picture.data[3] contains a vaapi_render_state struct which contains macroblocks as well as various fields extracted from headers
    XPR_AV_PIX_FMT_VAAPI_IDCT, ///< HW acceleration through VA API at IDCT entry-point, Picture.data[3] contains a vaapi_render_state struct which contains fields extracted from headers
    XPR_AV_PIX_FMT_VAAPI_VLD,  ///< HW decoding through VA API, Picture.data[3] contains a vaapi_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers

    XPR_AV_PIX_FMT_YUV420P16LE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    XPR_AV_PIX_FMT_YUV420P16BE,  ///< planar YUV 4:2:0, 24bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    XPR_AV_PIX_FMT_YUV422P16LE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    XPR_AV_PIX_FMT_YUV422P16BE,  ///< planar YUV 4:2:2, 32bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    XPR_AV_PIX_FMT_YUV444P16LE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    XPR_AV_PIX_FMT_YUV444P16BE,  ///< planar YUV 4:4:4, 48bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    XPR_AV_PIX_FMT_VDPAU_MPEG4,  ///< MPEG4 HW decoding with VDPAU, data[0] contains a vdpau_render_state struct which contains the bitstream of the slices as well as various fields extracted from headers
    XPR_AV_PIX_FMT_DXVA2_VLD,    ///< HW decoding through DXVA2, Picture.data[3] contains a LPDIRECT3DSURFACE9 pointer

    XPR_AV_PIX_FMT_RGB444LE,  ///< packed RGB 4:4:4, 16bpp, (msb)4A 4R 4G 4B(lsb), little-endian, most significant bits to 0
    XPR_AV_PIX_FMT_RGB444BE,  ///< packed RGB 4:4:4, 16bpp, (msb)4A 4R 4G 4B(lsb), big-endian, most significant bits to 0
    XPR_AV_PIX_FMT_BGR444LE,  ///< packed BGR 4:4:4, 16bpp, (msb)4A 4B 4G 4R(lsb), little-endian, most significant bits to 1
    XPR_AV_PIX_FMT_BGR444BE,  ///< packed BGR 4:4:4, 16bpp, (msb)4A 4B 4G 4R(lsb), big-endian, most significant bits to 1
    XPR_AV_PIX_FMT_Y400A,     ///< 8bit gray, 8bit alpha
    XPR_AV_PIX_FMT_BGR48BE,   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as big-endian
    XPR_AV_PIX_FMT_BGR48LE,   ///< packed RGB 16:16:16, 48bpp, 16B, 16G, 16R, the 2-byte value for each R/G/B component is stored as little-endian
    XPR_AV_PIX_FMT_YUV420P9BE, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    XPR_AV_PIX_FMT_YUV420P9LE, ///< planar YUV 4:2:0, 13.5bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    XPR_AV_PIX_FMT_YUV420P10BE,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    XPR_AV_PIX_FMT_YUV420P10LE,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian
    XPR_AV_PIX_FMT_YUV422P10BE,///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    XPR_AV_PIX_FMT_YUV422P10LE,///< planar YUV 4:2:2, 20bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    XPR_AV_PIX_FMT_YUV444P9BE, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    XPR_AV_PIX_FMT_YUV444P9LE, ///< planar YUV 4:4:4, 27bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    XPR_AV_PIX_FMT_YUV444P10BE,///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), big-endian
    XPR_AV_PIX_FMT_YUV444P10LE,///< planar YUV 4:4:4, 30bpp, (1 Cr & Cb sample per 1x1 Y samples), little-endian
    XPR_AV_PIX_FMT_YUV422P9BE, ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), big-endian
    XPR_AV_PIX_FMT_YUV422P9LE, ///< planar YUV 4:2:2, 18bpp, (1 Cr & Cb sample per 2x1 Y samples), little-endian
    XPR_AV_PIX_FMT_VDA_VLD,    ///< hardware decoding through VDA
    XPR_AV_PIX_FMT_GBRP,      ///< planar GBR 4:4:4 24bpp
    XPR_AV_PIX_FMT_GBRP9BE,   ///< planar GBR 4:4:4 27bpp, big-endian
    XPR_AV_PIX_FMT_GBRP9LE,   ///< planar GBR 4:4:4 27bpp, little-endian
    XPR_AV_PIX_FMT_GBRP10BE,  ///< planar GBR 4:4:4 30bpp, big-endian
    XPR_AV_PIX_FMT_GBRP10LE,  ///< planar GBR 4:4:4 30bpp, little-endian
    XPR_AV_PIX_FMT_GBRP16BE,  ///< planar GBR 4:4:4 48bpp, big-endian
    XPR_AV_PIX_FMT_GBRP16LE,  ///< planar GBR 4:4:4 48bpp, little-endian
    XPR_AV_PIX_FMT_YUVA422P,  ///< planar YUV 4:2:2 24bpp, (1 Cr & Cb sample per 2x1 Y & A samples)
    XPR_AV_PIX_FMT_YUVA444P,  ///< planar YUV 4:4:4 32bpp, (1 Cr & Cb sample per 1x1 Y & A samples)
    XPR_AV_PIX_FMT_YUVA420P9BE,  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), big-endian
    XPR_AV_PIX_FMT_YUVA420P9LE,  ///< planar YUV 4:2:0 22.5bpp, (1 Cr & Cb sample per 2x2 Y & A samples), little-endian
    XPR_AV_PIX_FMT_YUVA422P9BE,  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), big-endian
    XPR_AV_PIX_FMT_YUVA422P9LE,  ///< planar YUV 4:2:2 27bpp, (1 Cr & Cb sample per 2x1 Y & A samples), little-endian
    XPR_AV_PIX_FMT_YUVA444P9BE,  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), big-endian
    XPR_AV_PIX_FMT_YUVA444P9LE,  ///< planar YUV 4:4:4 36bpp, (1 Cr & Cb sample per 1x1 Y & A samples), little-endian
    XPR_AV_PIX_FMT_YUVA420P10BE, ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    XPR_AV_PIX_FMT_YUVA420P10LE, ///< planar YUV 4:2:0 25bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    XPR_AV_PIX_FMT_YUVA422P10BE, ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    XPR_AV_PIX_FMT_YUVA422P10LE, ///< planar YUV 4:2:2 30bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    XPR_AV_PIX_FMT_YUVA444P10BE, ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    XPR_AV_PIX_FMT_YUVA444P10LE, ///< planar YUV 4:4:4 40bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    XPR_AV_PIX_FMT_YUVA420P16BE, ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, big-endian)
    XPR_AV_PIX_FMT_YUVA420P16LE, ///< planar YUV 4:2:0 40bpp, (1 Cr & Cb sample per 2x2 Y & A samples, little-endian)
    XPR_AV_PIX_FMT_YUVA422P16BE, ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, big-endian)
    XPR_AV_PIX_FMT_YUVA422P16LE, ///< planar YUV 4:2:2 48bpp, (1 Cr & Cb sample per 2x1 Y & A samples, little-endian)
    XPR_AV_PIX_FMT_YUVA444P16BE, ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, big-endian)
    XPR_AV_PIX_FMT_YUVA444P16LE, ///< planar YUV 4:4:4 64bpp, (1 Cr & Cb sample per 1x1 Y & A samples, little-endian)
    XPR_AV_PIX_FMT_Y8,      ///< planar YUV 4:0:0, 8bpp, 1 plane for Y without the UV components
    XPR_AV_PIX_FMT_NV16,      ///< planar YUV 4:2:2, 16bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    XPR_AV_PIX_FMT_BAY10,
    XPR_AV_PIX_FMT_NB,        ///< number of pixel formats, DO NOT USE THIS if you want to link with shared libav* because the number of formats might differ between versions
};
typedef enum XPR_AVPixelFormat XPR_AVPixelFormat;
#endif // XPR_AVPIXELFORMAT_TYPE_DEFINED

/// @}
///

/// @addtogroup xpravframe-funs Functions
/// @{
///

/// @brief Create an new frame without data buffers
/// @return the frame context, null on failure
XPR_AVFrame* XPR_AVFrameNew(void);

/// @brief Create an new frame with audio data buffers
/// @param [in] format          Frame format
/// @param [in] samples         Number of samples
/// @param [in] channelLayout   Channels layout
/// @return the frame context, null on failure
XPR_AVFrame* XPR_AVFrameNewAudio(int format, int samples,
                               int64_t channelLayout);

/// @brief Create an new frame with video data buffers
/// @param [in] format          Frame format
/// @param [in] width           Width of the picture
/// @param [in] height          Height of the picture
/// @return the frame context, null on failure
XPR_AVFrame* XPR_AVFrameNewVideo(int format, int width, int height);

/// @brief Force destroy an frame, ignore the references
/// @param [in] frame   Frame context
/// @return no value returns
void XPR_AVFrameDestroy(XPR_AVFrame* frame);

int XPR_AVFrameGetPlanes(const XPR_AVFrame* frame,
                         uint8_t** data[XPR_AVFRAME_PLANES],
                         int* lines[XPR_AVFRAME_PLANES]);

int XPR_AVFrameSetPlanes(XPR_AVFrame* frame,
                         uint8_t* data[XPR_AVFRAME_PLANES],
                         int lines[XPR_AVFRAME_PLANES]);

int XPR_AVFrameSetFormat(XPR_AVFrame* frame, int format);
int XPR_AVFrameGetFormat(const XPR_AVFrame* frame);

int XPR_AVFrameSetSamples(XPR_AVFrame* frame, int samples);
int XPR_AVFrameGetSamples(const XPR_AVFrame* frame);

int XPR_AVFrameSetSampleRate(XPR_AVFrame* frame, int sampleRate);
int XPR_AVFrameGetSampleRate(const XPR_AVFrame* frame);

int XPR_AVFrameSetChannelLayout(XPR_AVFrame* frame,
                               int64_t channelLayout);
int64_t XPR_AVFrameGetChannelLayout(const XPR_AVFrame* frame);

/// @brief Set the DTS of the frame
/// @param [in] frame   AVFrame instance
/// @param [in] dts     Data/Decode/Display timestamp
/// retval 0    success
/// @retval -1  failure
int XPR_AVFrameSetDTS(XPR_AVFrame* frame, int64_t dts);

/// @brief Get the DTS of the frame
/// @param [in] frame   AVFrame instance
/// @return Data/Decode/Display timestamp of the frame
int64_t XPR_AVFrameGetDTS(const XPR_AVFrame* frame);

/// @brief Increase the references
/// @param [in] frame   AVFrame instance
/// @return the reference count after increased
int XPR_AVFrameRetain(XPR_AVFrame* frame);

/// @brief Decrease the references
/// @param [in] frame   AVFrame instance
/// @return the reference count after decreased
/// @note if reference counter == 0, the AVFrame will be destroy
int XPR_AVFrameRelease(XPR_AVFrame* frame);

/// @}
///

/// @}
///

#ifdef __cplusplus
}
#endif

#endif // XPR_AVFRAME_H


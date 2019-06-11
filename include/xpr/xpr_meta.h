#ifndef XPR_META_H
#define XPR_META_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 前置声明
/// 音频元数据的结构体
struct XPR_AudioMeta;
typedef struct XPR_AudioMeta XPR_AudioMeta;

struct XPR_AudioMeta {
    uint32_t bitrate;
    uint8_t bitsPerSample;
    uint8_t channelConfig;
    uint8_t numOfChannels;
    uint8_t profile;
    uint32_t samplingFrequency;
    uint8_t sampleRateIndex;
};

#ifdef __cplusplus
}
#endif

#endif // XPR_META_H
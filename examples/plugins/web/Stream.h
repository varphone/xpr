#ifndef _WEB_STREAM_H_
#define _WEB_STREAM_H_

#include "Config.h"

typedef struct StreamingChannel {
    int constantBitRate; ///< FIXME: "4096"
    int fixedQuality; ///< FIXME: "60"
    int maxFrameRate; ///< FIXME: "2500"
    int keyFrameInterval; ///< FIXME: "1000"
    int rtspPortNo;
    int hfr;
    char videoCodecType[64];
    char videoResolution[64];
    char videoScanType[64]; ///< FIXME: "progressive"
    char videoQualityControlType[64]; ///< FIXME: "cbr"
} StreamingChannel;

void MainStreamInit(void);

void SecondaryStreamInit(void);

int GET_PSIA_Streaming_channels(struct mg_connection* conn,
                                const struct mg_request_info* request_info);

int GET_PSIA_Streaming_channels_101(struct mg_connection* conn,
                                    const struct mg_request_info* request_info);

int GET_PSIA_Streaming_channels_102(struct mg_connection* conn,
                                    const struct mg_request_info* request_info);

int PUT_PSIA_Streaming_channels_101(struct mg_connection* conn,
                                    const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Streaming_channels_102(struct mg_connection* conn,
                                    const struct mg_request_info* request_info, XML_Parser p);

#endif /* _WEB_STREAM_H_ */

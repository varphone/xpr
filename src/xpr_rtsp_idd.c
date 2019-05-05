#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_rtsp_idd.h>
#include <xpr/xpr_utils.h>

typedef struct XPR_RTSP_IDD_Params {
    void* opaque;
    XPR_RTSP_IDD_DataHandler dataHandler;
} XPR_RTSP_IDD_Params;

struct XPR_RTSP_IDD {
    XPR_RTSP_IDD_Params params;
    XPR_RTSP_IDD_DataType dataType;
    int channel;
    int packetSize;
    int remainSize;
    uint8_t hdr[4];
    uint8_t hdrSize;
    uint8_t buffer[1];
};

XPR_RTSP_IDD* XPR_RTSP_IDD_New(void)
{
    XPR_RTSP_IDD* idd = (XPR_RTSP_IDD*)XPR_Alloc(sizeof(*idd)+XPR_RTSP_IDD_MAX_BUFFER_SIZE);
    if (idd) {
        idd->params.dataHandler = 0;
        idd->params.opaque = 0;
        idd->channel = 0;
        idd->packetSize = 0;
        idd->remainSize = 0;
        idd->hdrSize = 0;
    }
    return idd;
}

int XPR_RTSP_IDD_Destroy(XPR_RTSP_IDD* idd)
{
    if (idd)
        XPR_Free((void*)idd);
    return 0;
}

static int DeliverPacket(XPR_RTSP_IDD* idd, int channel, int dataType,
                         const uint8_t* data, int length)
{
    if (idd->params.dataHandler)
        return idd->params.dataHandler(idd->params.opaque, channel, dataType, (uint8_t*)data, length);
    return 0;
}

#if 0
static void dump(const uint8_t* data, int len)
{
    int i = 0;
    for (; i<len; i++) {
        if ((i % 16) == 0)
            printf("\n%04d", i);
        printf(" %02hhx", data[i]);
    }
    printf("\n");
}
#endif

int XPR_RTSP_IDD_PushData(XPR_RTSP_IDD* idd, const uint8_t* data, int length)
{
    int channel = 0;
    int offset = 0;
    char* p = 0;
    uint8_t oc = 0;
    uint8_t* ocp = 0;
    uint16_t l = 0;
    //
    if (!idd || !data || length <= 0)
        return XPR_ERR_ERROR;
    //
    if (idd->hdrSize) {
        l =  MIN(4 - idd->hdrSize, length);
        switch (l) {
        case 3:
            idd->hdr[1] = data[0];
            idd->hdr[2] = data[1];
            idd->hdr[3] = data[2];
            break;
        case 2:
            idd->hdr[1] = data[0];
            idd->hdr[2] = data[1];
            break;
        case 1:
            idd->hdr[1] = data[0];
            break;
        default:
            break;
        }
        data += l;
        length -= l;
        idd->hdrSize += l;
        //
        if (idd->hdrSize == 0) {
            idd->channel = idd->hdr[1];
            idd->remainSize = (int)idd->hdr[2] << 8 | idd->hdr[3];
            idd->packetSize = idd->remainSize;
        }
    }
    //
    //printf("remain size: %d\n", idd->remainSize);
    if (idd->remainSize < 0) {
        p = strstr((char*)data, "\r\n\r\n");
        if (p)
            l = (uint16_t)(p + 4 - (char*)data);
        idd->remainSize = l;
    }
    //
    if (idd->remainSize > 0) {
        l = MIN(idd->remainSize, length);
        offset = idd->packetSize - idd->remainSize;
        memcpy(idd->buffer+offset, data, l);
        idd->remainSize -= l;
        data += l;
        length -= l;
        if (idd->remainSize == 0) {
            DeliverPacket(idd, idd->channel, idd->dataType, idd->buffer, idd->packetSize);
            idd->channel = 0;
            idd->packetSize = 0;
        }
    }
    //
    while (length > 0) {
        //printf("data: %d, length: %d\n", data, length);
        //if (length <= 6) {
        //    dump(data, length);
        //}
        if (data[0] == '$') {
            if (length < 4) {
                switch (l) {
                case 3:
                    idd->hdr[0] = data[0];
                    idd->hdr[1] = data[1];
                    idd->hdr[2] = data[2];
                    break;
                case 2:
                    idd->hdr[1] = data[0];
                    idd->hdr[2] = data[1];
                    break;
                case 1:
                    idd->hdr[1] = data[0];
                    break;
                default:
                    break;
                }
                //
                idd->hdrSize = length;
                //
                data += length;
                length -= length;
            }
            else {
                channel = data[1];
                l = (int)data[2] << 8 | data[3];
                idd->dataType = XPR_RTSP_IDD_DATA_TYPE_BIN;
                data += 4;
                length -= 4;
                if (length < l) {
                    idd->channel = channel;
                    idd->packetSize = l;
                    idd->remainSize = l - length;
                    memcpy(idd->buffer, data, length);
                }
                else {
                    DeliverPacket(idd, channel, idd->dataType, data, l);
                }
                data += l;
                length -= l;
            }
        }
        else {
            idd->dataType = XPR_RTSP_IDD_DATA_TYPE_HDR;
            p = strstr((char*)data, "\r\n\r\n");
            if (p) {
				l = (uint16_t)(p + 4 - (char*)data);
                ocp = (uint8_t*)p + 4;
                oc = *ocp;
                *ocp = '\0';
                DeliverPacket(idd, 0, idd->dataType, data, l);
                p = strstr((char*)data, "Content-Length:");
                *ocp = oc;
                data += l;
                length -= l;
                if (length > 0 && p) {
                    idd->dataType = XPR_RTSP_IDD_DATA_TYPE_SDP;
                    l = (uint16_t)strtoul(p+15, 0, 10);
                    if (length < l) {
                        idd->channel = 0;
                        idd->packetSize = l;
                        idd->remainSize = l - length;
                        memcpy(idd->buffer, data, length);
                    }
                    else {
                        DeliverPacket(idd, 0, idd->dataType, data, l);
                    }
                    data += l;
                    length -= l;
                }
            }
            else {
                idd->channel = 0;
                idd->packetSize = l;
                idd->remainSize = -1;
                memcpy(idd->buffer, data, length);
                length = 0;
            }
        }
    }
    return 0;
}

int XPR_RTSP_IDD_SetParam(XPR_RTSP_IDD* idd, int param, const void* data, int length)
{
    switch (param) {
    case XPR_RTSP_IDD_PARAM_DATA_HANDLER:
        idd->params.dataHandler = length > 0 ? *(XPR_RTSP_IDD_DataHandler*)data : (XPR_RTSP_IDD_DataHandler)data;
        break;
    case XPR_RTSP_IDD_PARAM_OPAQUE:
        idd->params.opaque = length > 0 ? *(void**)data : (void*)data;
        break;
    default:
        break;
    }
    return XPR_ERR_OK;
}

int XPR_RTSP_IDD_GetParam(XPR_RTSP_IDD* idd, int param, void* buffer, int* size)
{
    if (!buffer || !size)
        return XPR_ERR_ERROR;
    switch (param) {
    case XPR_RTSP_IDD_PARAM_DATA_HANDLER:
        if (*size == sizeof(idd->params.dataHandler))
            *(XPR_RTSP_IDD_DataHandler*)buffer = idd->params.dataHandler;
        * size = sizeof(idd->params.dataHandler);
        break;
    case XPR_RTSP_IDD_PARAM_OPAQUE:
        if (*size == sizeof(idd->params.opaque))
            *(void**)buffer = idd->params.opaque;
        * size = sizeof(idd->params.opaque);
        break;
    default:
        break;
    }
    return XPR_ERR_OK;
}

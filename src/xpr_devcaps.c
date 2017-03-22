#include <malloc.h>
#include <pthread.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_devcaps.h>
#include <xpr/xpr_json.h>
#include <xpr/xpr_utils.h>
#include <xpr/xpr_drm.h>

#pragma pack(1) 
struct XPR_DevCaps
{
    uint32_t   version; // (uint32_t){major[31..16],minor[15..8],rev:[7..0]}
    long long    dateTime;
    char       internalModel[32];
    char       model[32];
    char       sn[32];
    uint8_t    serial[8];   // 硬件序列号
    uint32_t   hwVer; // (uint32_t){major[31..16],minor[15..8],rev:[7..0]}
    uint32_t   swVer; // (uint32_t){major[31..16],minor[15..8],rev:[7..0]}
    uint8_t    lightSensor; //{0=无,1=普通光敏电阻}
    uint8_t    thermSensor; //{0=无,1=普通温度传感器}
    char       videoSensor[16];
    uint8_t    ircut; //{0=无,1=固定,2=可控}
    uint8_t    auxLight; //{0=无,1=白光灯,2=红外灯}
    uint8_t    af; //{0=无,1=普通自动聚集}
    uint8_t    lens; //{0=无,1=全手动,2=手动聚焦+电动光圈,
                     // 3=电动聚焦+手动光圈,4=电动聚焦+电动光圈,5=全自动聚焦+电动光圈}
    uint8_t    audioInput; //{0=无,1=支持}
    uint8_t    audioOutput; //{0=无,1=单通道,2=双通道}
    uint32_t   vencMaxResolution; //(width[31...16,height[15..0]])
    uint32_t   vencMaxFrame;
    uint32_t   vencMaxMinBitrate;// (max[31..16], min[15...0])
    char       reserved[96];  
};


typedef struct XPR_DevCaps XPR_DevCaps;

static XPR_DevCaps xpr_devcap = {
    .version = 0,
    .dateTime = 0,
    .internalModel = "100",
    .model = "ry",
    .sn = "xsa",
    .serial = "102032",
    .hwVer = 1,
    .swVer = 2,
    .lightSensor = 0,
    .thermSensor = 0,
    .videoSensor = "imx202",
    .ircut = 1,
    .auxLight = 2,
    .af = 1,
    .lens = 2,
    .audioInput = 0,
    .audioOutput = 0,
    .vencMaxResolution = 1920<<16|1080,
    .vencMaxFrame = 30,
    .vencMaxMinBitrate =  40000<<16|16
};

static int fd_d;
static XPR_JSON* root_json = 0, *js = 0;

#define MAX_DEVID 16
static const char** strings_list[MAX_DEVID] = {0};
static pthread_mutex_t mattr;

int XPR_DevCapsInit(void)
{
    int length;
    root_json = XPR_JSON_LoadFileName("./capabilities.json");
    if (root_json == NULL)
        printf("json faile\n");
#if 1
    if ((fd_d = open("./capabilities.bin", O_RDONLY)) < 0) {
        printf("open failed!\n");
        return XPR_ERR_ERROR;
    }
    if ((length = read(fd_d, &xpr_devcap , sizeof(XPR_DevCaps))) < 0) {
        printf("read failed!\n");
        return XPR_ERR_ERROR;
    }
#else
	length = sizeof(xpr_devcap);
	if(XPR_DRM_GetRawData((uint8_t*)&xpr_devcap, &length) != XPR_ERR_SUCCESS)
		printf("XPR_DRM_GetRawData failed\n");
	printf("videoSensor = %s, model=%s\n", xpr_devcap.videoSensor, xpr_devcap.model);
#endif
    pthread_mutex_init(&mattr,NULL);
    return XPR_ERR_OK;
}

int XPR_DevCapsFini(void)
{
    pthread_mutex_destroy(&mattr);
    int i=0, j=0;
    for(i=0; strings_list[i] != NULL ; i++)
    {
        for(j=0; strings_list[i][j] != NULL; ++j) {
            char* tmp = (char*)strings_list[i][j];
            free(tmp);
        }
        free(strings_list[i]);
        strings_list[i] = NULL;
    }
    XPR_JSON_DecRef(root_json);
    if (fd_d >= 0) {
        close(fd_d);
    }
    return XPR_ERR_OK;
}

int XPR_DevCapsGetInteger(int capId, int devId, uint32_t *value)
{
    int size = 0;
    if (devId > XPR_DEVCAPS_DEVID_3)
        return XPR_ERR_ERROR;
    switch (capId) {
        case XPR_DEVCAPS_ID_VERSION :
        {
            *value = xpr_devcap.version;
            break;
        }
        case XPR_DEVCAPS_ID_HW_VER :
        {
            *value = xpr_devcap.hwVer;
            break;
        }
        case XPR_DEVCAPS_ID_SW_VER :
        {
            *value = xpr_devcap.swVer;
            break;
        }
        case XPR_DEVCAPS_ID_LIGHT_SENSOR :
        {
            *value = xpr_devcap.lightSensor;
            break;
        }
        case XPR_DEVCAPS_ID_THERM_SENSOR :
        {
            *value = xpr_devcap.thermSensor;
            break;
        }
        case XPR_DEVCAPS_ID_IRCUT :
        {
            *value = xpr_devcap.ircut;
            break;
        }
        case XPR_DEVCAPS_ID_AUX_LIGHT :
        {
            *value = xpr_devcap.auxLight;
            break;
        }
        case XPR_DEVCAPS_ID_AF :
        {
            *value = xpr_devcap.af;
            break;
        }
        case XPR_DEVCAPS_ID_LENS :
        {
            *value = xpr_devcap.lens;
            break;
        }
        case XPR_DEVCAPS_ID_AUDIO_INPUT :
        {
            *value = xpr_devcap.audioInput;
            break;
        }
        case XPR_DEVCAPS_ID_AUDIO_OUTPUT :
        {
            *value = xpr_devcap.audioOutput;
            break;
        }
        case XPR_DEVCAPS_ID_VENC_MAX_RES :
        {
            *value = xpr_devcap.vencMaxResolution;
            break;
        }
        case XPR_DEVCAPS_ID_VENC_MAX_FRAME :
        {
            *value = xpr_devcap.vencMaxFrame;
            break;
        }
        case XPR_DEVCAPS_ID_VENC_MAX_BITRATES :
        {
            *value = xpr_devcap.vencMaxMinBitrate;
            break;
        }
        case XPR_DEVCAPS_ID_VENC_STREAM_COUNTS :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_VENC_STREAM_COUNTS");
                if(!js)
                    return XPR_ERR_ERROR;
                js = XPR_JSON_ObjectGet(js, "value");
                if(!js)
                    return XPR_ERR_ERROR;
                *value = XPR_JSON_IntegerValue(js);
                break;
            }
        case XPR_DEVCAPS_ID_VENC_QUALITY :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_VENC_QUALITY");
                js = XPR_JSON_ArrayGet(js, devId);
                js = XPR_JSON_ObjectGet(js, "descs");
                size = XPR_JSON_ArraySize(js);
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_VENC_FRAME :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_VENC_FRAME");
                js = XPR_JSON_ArrayGet(js, devId);
                js = XPR_JSON_ObjectGet(js, "range");
                int num = XPR_JSON_ArraySize(js);
                uint8_t tmpFrame[2];
                for (size = 0; size < num; size++) {
                    js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_VENC_FRAME");
                    js = XPR_JSON_ArrayGet(js, devId);
                    js = XPR_JSON_ObjectGet(js, "range");
                    js = XPR_JSON_ArrayGet(js, size);
                    tmpFrame[size] = XPR_JSON_IntegerValue(js);
                }
                *value = tmpFrame[1] << 16 | tmpFrame[0];
                break;
            }
        case XPR_DEVCAPS_ID_IRIS :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_IRIS");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_SHUTTER_TIME :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_SHUTTER_TIME");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_EXPOSURE_LEVEL :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_EXPOSURE_LEVEL");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_SHUTTER_MAX_TIME:
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_SHUTTER_MAX_TIME");
                js = XPR_JSON_ObjectGet(js, "range");
                int num = XPR_JSON_ArraySize(js);
                int tmpShutter[2];
                for (size = 0; size < num; size++) {
                    js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_SHUTTER_MAX_TIME");
                    js = XPR_JSON_ObjectGet(js, "range");
                    js = XPR_JSON_ArrayGet(js, size);
                    tmpShutter[size] = XPR_JSON_IntegerValue(js);
                }
                *value = (tmpShutter[1] << 16) | tmpShutter[0];
                break;
            }
        case XPR_DEVCAPS_ID_SHUTTER_MIN_TIME :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_SHUTTER_MIN_TIME");
                js = XPR_JSON_ObjectGet(js, "range");
                int num = XPR_JSON_ArraySize(js);
                int tmpShutter[2];
                for (size = 0; size < num; size++) {
                    js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_SHUTTER_MIN_TIME");
                    js = XPR_JSON_ObjectGet(js, "range");
                    js = XPR_JSON_ArrayGet(js, size);
                    tmpShutter[size] = XPR_JSON_IntegerValue(js);
                }
                *value = (tmpShutter[1] << 16) | tmpShutter[0];
                break;
            }
        case XPR_DEVCAPS_ID_NOISE_2D :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_NOISE_2D");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_NOISE_3D :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_NOISE_3D");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_GRAY_SCALE :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_GRAY_SCALE");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value =  (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_WHITE_BALANCE :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_WHITE_BALANCE");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_MIRROR :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_MIRROR");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_POWER_LINE :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_POWER_LINE");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_DAY_NIGHT :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_DAY_NIGHT");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value =  (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value =  (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_LIGHT :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_LIGHT");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value =  (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_LIGHT_MODE :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_LIGHT_MODE");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value =  (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_WDR :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_WDR");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value =  (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_FLOG :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_FLOG");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_METER :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_METER");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value = (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value =  (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_STABILIZATION :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_STABILIZATION");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value =  (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_LENS_CORRECTION :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_LENS_CORRECTION");
                js = XPR_JSON_ObjectGet(js, "value");
                size = XPR_JSON_ArraySize(js);
                if (size == 1) {
                    js = XPR_JSON_ArrayGet(js, 0);
                    *value =  (XPR_JSON_IntegerValue(js)?1:0);
                }
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_PORTS :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_PORTS");
                js = XPR_JSON_ObjectGet(js, "value");
                *value =  XPR_JSON_IntegerValue(js);
                break;
            }
        case XPR_DEVCAPS_ID_BAUD_RATE :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_BAUD_RATE");
                size = XPR_JSON_ArraySize(js);
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_DATA_BITS :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_DATA_BITS");
                size = XPR_JSON_ArraySize(js);
                *value = (size?1:0);
                break;
            }

        case XPR_DEVCAPS_ID_STOP_BITS :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_STOP_BITS");
                size = XPR_JSON_ArraySize(js);
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_PARITY_CHECK :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_PARITY_CHECK");
                size = XPR_JSON_ArraySize(js);
                *value =  (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_FLOW_CONTROL :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_FLOW_CONTROL");
                size = XPR_JSON_ArraySize(js);
                *value = (size?1:0);
                break;
            }
        case XPR_DEVCAPS_ID_EVENT_MOTION_SENSITIVE :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_EVENT_MOTION_SENSITIVE");
                js = XPR_JSON_ObjectGet(js, "range");
                int num = XPR_JSON_ArraySize(js);
                uint8_t tmpMotion[2];
                for (size = 0; size < num; size++) {
                    js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_EVENT_MOTION_SENSITIVE");
                    js = XPR_JSON_ObjectGet(js, "range");
                    js = XPR_JSON_ArrayGet(js, size);
                    tmpMotion[size] = XPR_JSON_IntegerValue(js);
                }

                *value =  tmpMotion[1] << 16 | tmpMotion[0];
                break;
            }
        case XPR_DEVCAPS_ID_EVENT_COVER_SENSITIVE :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_EVENT_COVER_SENSITIVE");
                js = XPR_JSON_ObjectGet(js, "range");
                int num = XPR_JSON_ArraySize(js);
                uint8_t tmpCover[2];
                for (size = 0; size < num; size++) {
                    js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_EVENT_COVER_SENSITIVE");
                    js = XPR_JSON_ObjectGet(js, "range");
                    js = XPR_JSON_ArrayGet(js, size);
                    tmpCover[size] = XPR_JSON_IntegerValue(js);
                }
                *value = tmpCover[1] << 16 | tmpCover[0];
                break;
            }
        case XPR_DEVCAPS_ID_VENC_CODECS :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_VENC_CODECS");
                js = XPR_JSON_ArrayGet(js, devId);
                js = XPR_JSON_ObjectGet(js, "descs");
                *value = XPR_JSON_ArraySize(js);
                break;
            }
        case XPR_DEVCAPS_ID_VENC_RESOLUTION :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_VENC_RESOLUTION");
                js = XPR_JSON_ArrayGet(js, devId);
                js = XPR_JSON_ObjectGet(js, "descs");
                *value =  XPR_JSON_ArraySize(js);
                break;
            }
        case XPR_DEVCAPS_ID_PTZ :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_PTZ");
                js = XPR_JSON_ObjectGet(js, "descs");
                *value =  XPR_JSON_ArraySize(js);
                break;
            }
        case XPR_DEVCAPS_ID_GIS :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_GIS");
                js = XPR_JSON_ObjectGet(js, "descs");
                *value =  XPR_JSON_ArraySize(js);
                break;
            } 
        case XPR_DEVCAPS_ID_EVENT :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_EVENT");
                js = XPR_JSON_ObjectGet(js, "descs");
                *value =  XPR_JSON_ArraySize(js);
                break;
            }
        case XPR_DEVCAPS_ID_AUDIO_CODECS :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_AUDIO_CODECS");
                js = XPR_JSON_ObjectGet(js, "descs");
                *value =  XPR_JSON_ArraySize(js);
                break;
            }
        case XPR_DEVCAPS_ID_PORT_TYPE :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_PORT_TYPE");
                js = XPR_JSON_ArrayGet(js, devId);
                js = XPR_JSON_ObjectGet(js, "descs");
                *value =  XPR_JSON_ArraySize(js);
                break;
            }
        default :
            return XPR_ERR_ERROR;
    }
    return XPR_ERR_SUCCESS;
}

int  XPR_DevCapsGetInt64(int capId, int devId, int64_t *value)
{
    if (devId > XPR_DEVCAPS_DEVID_3)
        return XPR_ERR_ERROR;

    switch (capId) {
        case XPR_DEVCAPS_ID_DATETIME :
            *value = xpr_devcap.dateTime;
        default:
            return XPR_ERR_ERROR;
    }
    return XPR_ERR_SUCCESS;
}

int XPR_DevCapsGetString(int capId, int devId, char* buffer, int len)
{
    int size = 0, num = 2;
    if (devId > XPR_DEVCAPS_DEVID_3)
        return XPR_ERR_ERROR;
    switch (capId) {
        case XPR_DEVCAPS_ID_INTERNAL_MODEL :
    {
            strcpy_s(buffer, len, xpr_devcap.internalModel);
            break;
    }
        case XPR_DEVCAPS_ID_MODEL :
    {
            strcpy_s(buffer, len, xpr_devcap.model);
            break;
    }
        case XPR_DEVCAPS_ID_SN :
    {
            strcpy_s(buffer, len, xpr_devcap.sn);
            break;
    }
        case XPR_DEVCAPS_ID_VIDEO_SENSOR :
    {
            strcpy_s(buffer, len, xpr_devcap.videoSensor);
            break;
    }
        default:
            return XPR_ERR_ERROR;
    }
    return XPR_ERR_OK;
}

const char** build_array(int capId, int devId)
{
    int size = 0,num = 0;
    switch (capId) {
        case XPR_DEVCAPS_ID_VENC_CODECS :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_VENC_CODECS");
                js = XPR_JSON_ArrayGet(js, devId);
                js = XPR_JSON_ObjectGet(js, "descs");
                num = XPR_JSON_ArraySize(js);
                char **str = (char **)malloc(sizeof(char*) * (num+1));
                for (size = 0; size < num+1; size++) {
                    js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_VENC_CODECS");
                    js = XPR_JSON_ArrayGet(js, devId);
                    js = XPR_JSON_ObjectGet(js, "descs");
                    js = XPR_JSON_ArrayGet(js, size);
                    char *tmpString = (char*)XPR_JSON_StringValue(js);
                    if (size+1 == num+1) {
                        str[size] = '\0';
                    } else {
                        str[size] = strdup(tmpString);
                    }
                }
                return (const char**)str;
            }
        case XPR_DEVCAPS_ID_VENC_RESOLUTION :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_VENC_RESOLUTION");
                js = XPR_JSON_ArrayGet(js, devId);
                js = XPR_JSON_ObjectGet(js, "descs");
                num = XPR_JSON_ArraySize(js);
                char **str = (char **)malloc(sizeof(char*) * (num+1));
                for (size = 0; size < num+1; size++) {
                    js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_VENC_RESOLUTION");
                    js = XPR_JSON_ArrayGet(js, devId);
                    js = XPR_JSON_ObjectGet(js, "descs");
                    js = XPR_JSON_ArrayGet(js, size);
                    char *tmpString = (char*)XPR_JSON_StringValue(js);
                    if (size+1 == num+1) {
                        str[size] = '\0';
                    } else {
                        str[size] = strdup(tmpString);
                    }
                }
                return (const char**)str;
            }
        case XPR_DEVCAPS_ID_PTZ :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_PTZ");
                js = XPR_JSON_ObjectGet(js, "descs");
                num = XPR_JSON_ArraySize(js);
                char **str = (char **)malloc(sizeof(char*) * (num+1));
                for (size = 0; size < num+1; size++) {
                    js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_PTZ");
                    js = XPR_JSON_ObjectGet(js, "descs");
                    js = XPR_JSON_ArrayGet(js, size);
                    char *tmpString = (char*)XPR_JSON_StringValue(js);
                    if (size+1 == num+1) {
                        str[size] = '\0';
                    } else {
                        str[size] = strdup(tmpString);
                    }
                }
                return (const char**)str;
            }
        case XPR_DEVCAPS_ID_GIS :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_GIS");
                js = XPR_JSON_ObjectGet(js, "descs");
                num = XPR_JSON_ArraySize(js);
                char **str = (char **)malloc(sizeof(char*) * (num+1));
                for (size = 0; size < num+1; size++) {
                    js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_GIS");
                    js = XPR_JSON_ObjectGet(js, "descs");
                    js = XPR_JSON_ArrayGet(js, size);
                    char *tmpString = (char*)XPR_JSON_StringValue(js);
                    if (size+1 == num+1) {
                        str[size] = '\0';
                    } else {
                        str[size] = strdup(tmpString);
                    }
                }
                return (const char**)str;
            }  
        case XPR_DEVCAPS_ID_EVENT :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_EVENT");
                js = XPR_JSON_ObjectGet(js, "descs");
                num = XPR_JSON_ArraySize(js);
                char **str = (char **)malloc(sizeof(char*) * (num+1));
                for (size = 0; size < num+1; size++) {
                    js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_EVENT");
                    js = XPR_JSON_ObjectGet(js, "descs");
                    js = XPR_JSON_ArrayGet(js, size);
                    char *tmpString = (char*)XPR_JSON_StringValue(js);
                    if (size+1 == num+1) {
                        str[size] = '\0';
                    } else {
                        str[size] = strdup(tmpString);
                    }
                }
                return (const char**)str;
            }
        case XPR_DEVCAPS_ID_AUDIO_CODECS :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_AUDIO_CODECS");
                js = XPR_JSON_ObjectGet(js, "descs");
                num = XPR_JSON_ArraySize(js);
                char **str = (char **)malloc(sizeof(char*) * (num+1));
                for (size = 0; size < num+1; size++) {
                    js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_AUDIO_CODECS");
                    js = XPR_JSON_ObjectGet(js, "descs");
                    js = XPR_JSON_ArrayGet(js, size);
                    char *tmpString = (char*)XPR_JSON_StringValue(js);
                    if (size+1 == num+1) {
                        str[size] = '\0';
                    } else {
                        str[size] = strdup(tmpString);
                    }
                }
                return (const char**)str;
            }
        case XPR_DEVCAPS_ID_PORT_TYPE :
            {
                js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_PORT_TYPE");
                js = XPR_JSON_ArrayGet(js, devId);
                js = XPR_JSON_ObjectGet(js, "descs");
                num = XPR_JSON_ArraySize(js);
                char **str = (char **)malloc(sizeof(char*) * (num+1));
                for (size = 0; size < num+1; size++) {
                    js = XPR_JSON_ObjectGet(root_json, "XPR_DEVCAPS_ID_PORT_TYPE");
                    js = XPR_JSON_ArrayGet(js, devId);
                    js = XPR_JSON_ObjectGet(js, "descs");
                    js = XPR_JSON_ArrayGet(js, size);
                    char *tmpString = (char*)XPR_JSON_StringValue(js);
                    if (size+1 == num+1) {
                        str[size] = '\0';
                    } else {
                        str[size] = strdup(tmpString);
                    }
                }
                return (const char**)str;
            }
    }
    return NULL;
}

const char** XPR_DevCapsGetStrings(int capId, int devId)
{
    pthread_mutex_lock(&mattr);
    int id = capId + devId;
    if(!strings_list[id%MAX_DEVID]) {
        strings_list[id%MAX_DEVID] = build_array(capId, devId);
    }
    pthread_mutex_unlock (&mattr);
    return strings_list[id%MAX_DEVID];

}

int XPR_DevCapsUpdate(const void* data, int size)
{
    return XPR_ERR_ERROR;
}

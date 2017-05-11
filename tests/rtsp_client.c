#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_streamblock.h>
#include <xpr/xpr_rtsp.h>


#define ANY_PORT XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, XPR_RTSP_PORT_MINOR_ANY, 0)

const char* url = "rtsp://admin:qq123456@192.168.1.64:554/Streaming/Channels/1?transportmode=unicast&profile=Profile_1";
//const char* url = "rtsp://192.168.0.7:8554/CH00_20150708102043.264";

static int ports[300];

static int data_handler(void* opaque, int port, const XPR_StreamBlock* stb)
{
    //printf("port %x, codec %x, flags %x, size %d, pts %lld\n", port, stb->codec, stb->flags, stb->dataSize, stb->pts);
    return 0;
}

static int event_handler(void* opaque, int port, const XPR_RTSP_EVD* evd)
{
    int idx = (int)opaque;
    if (evd) {
        printf("Event: %d, ", evd->event);
        if (evd->event == XPR_RTSP_EVT_TIMEOUT) {
            printf("XPR_RTSP_EVT_TIMEOUT");
        }
        printf("\n");
    }
    return 0;
}

static void simple_example(void)
{
    int port = XPR_RTSP_Open(ANY_PORT, url);
    printf("!!! PORT = %x\n", port);
    XPR_RTSP_AddDataCallback(port, data_handler, (void*)0);
    XPR_RTSP_AddEventCallback(port, event_handler, (void*)0);
    XPR_RTSP_SetAuth(port, "admin", "123456", 0);
    //XPR_RTSP_SetOutputFormat(port, AV_FOURCC_MP2P);
    XPR_RTSP_Start(port);
    printf("Press any key to stop!\n");
    int ch = getchar();
    XPR_RTSP_Stop(port);
    XPR_RTSP_Close(port);
}

int main(int argc, char** argv)
{
    XPR_RTSP_Init();
    simple_example();
    printf("Press any key to exit!\n");
    int ch = getchar();
    XPR_RTSP_Fini();
    return 0;
}


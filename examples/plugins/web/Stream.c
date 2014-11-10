#include "Stream.h"

#define VIDEOSVR_SOCK_PATH      "/tmp/videosvr.sock-0"
#define DEF_UN_RCVTMO           60000
#define DEF_UN_SNDTMO           1000
#define CHANNEL_XML_FILE   "/ambarella/channel.xml"

static StreamingChannel streamchannels[2];
static int streamdoor, channel;
char buffers[1024];
int Depth;

static int get_stream(char* buffers, int lens)
{
    if (lens != 0) {
        switch (streamdoor) {
        case 1: {
            int id = atoi(buffers);
            if (id == 101)
                channel = 0;
            if (id == 102)
                channel = 1;
            streamdoor = 0;
        }
        break;
        case 2:
            //streamchannels[channel].videoCodecType = atoi(buffers);
            sprintf(streamchannels[channel].videoCodecType, "%s", buffers);
            printf("codec :%s\n",streamchannels[channel].videoCodecType);
            streamdoor = 0;
            break;
        case 3:
            sprintf(streamchannels[channel].videoResolution, "%s", buffers);
            printf("videoResolution :%s\n",streamchannels[channel].videoResolution);
            streamdoor = 0;
            break;
        case 4:
            streamchannels[channel].constantBitRate = atoi(buffers);
            printf("constantBitRate :%d\n",streamchannels[channel].constantBitRate);
            streamdoor = 0;
            break;
        case 5:
            streamchannels[channel].fixedQuality = atoi(buffers);
            printf("fixedQuality :%d\n",streamchannels[channel].fixedQuality);
            streamdoor = 0;
            break;
        case 6:
            streamchannels[channel].maxFrameRate = atoi(buffers);
            printf("maxFrameRate :%d\n",streamchannels[channel].maxFrameRate);
            streamdoor = 0;
            break;
        case 7:
            streamchannels[channel].hfr = atoi(buffers);
            streamdoor = 0;
            break;
        default:
            break;
        }
    }
    return 1;
}


static void XMLCALL
chardatahandler(void* uerData, const char* target, int lens)
{
    memcpy(buffers, target, lens);
    buffers[lens] = '\0';
    get_stream(buffers, lens);
}

static void XMLCALL
start(void* data, const char* el, const char** attr)
{
    if (strcmp(el, "id") == 0)
        streamdoor = 1;
    if (strcmp(el, "videoCodecType") == 0)
        streamdoor = 2;
    if (strcmp(el, "videoResolution") == 0)
        streamdoor = 3;
    if (strcmp(el, "constantBitRate") == 0)
        streamdoor = 4;
    if (strcmp(el, "fixedQuality") == 0)
        streamdoor = 5;
    if (strcmp(el, "maxFrameRate") == 0)
        streamdoor = 6;
    if (strcmp(el, "hfrType") == 0)
        streamdoor = 7;
    Depth++;
}

static void XMLCALL
end(void* data, const char* el)
{
    Depth--;
}

void MainStreamInit(void)
{
    char value[128] = {0}; 
    int size = sizeof(value);

    if (XPR_UPS_GetString("/camera/video/encoder/0/codec", value, &size) != 0) {
        printf("get 0 codec error!\n");
    } else {
        printf("0 codec = %s\n", value);
        sprintf(streamchannels[0].videoCodecType, "%s", value); 
    }

    size = sizeof(value);
    if (XPR_UPS_GetString("/camera/video/encoder/0/resolution", value, &size) != 0) {
        printf("get 0 resolution error!\n");
    } else {
        printf("0 resolution = %s\n", value);
        sprintf(streamchannels[0].videoResolution, "%s", value); 
    }

    int ivalue = 0;
    if(XPR_UPS_GetInteger("/camera/video/encoder/0/bitrate", &ivalue) != 0) {
        printf("get 0 bitrate error!\n");
    } else {
		printf("0 constantBitRate = %d\n", ivalue);
        streamchannels[0].constantBitRate = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/video/encoder/0/fps", &ivalue) != 0) {
        printf("get 0 fps error!\n");
    } else{
  		printf("0 maxFrameRate = %d\n", ivalue);
        streamchannels[0].maxFrameRate = ivalue;
    }

    if (strcmp(streamchannels[0].videoCodecType, "H.264") == 0) {
	    if(XPR_UPS_GetInteger("/camera/video/encoder/0/h264/quality", &ivalue) != 0) {
			printf("get 0 h264 quality error!\n");
	    } else{
			printf("0 h264 quality = %d\n", ivalue);
			streamchannels[0].fixedQuality = ivalue;
	    }
   }

    if (strcmp(streamchannels[0].videoCodecType, "MJPEG") == 0) {
	    if(XPR_UPS_GetInteger("/camera/video/encoder/0/mjpeg/quality", &ivalue) != 0) {
			printf("get 0 mjpeg quality error!\n");
	    } else{
			printf("0 mjpeg quality = %d\n", ivalue);
			streamchannels[0].fixedQuality = ivalue;
	    }
    }
	
    if (XPR_UPS_GetInteger("/camera/video/encoder/0/frame_interval", &ivalue) != 0) {
        printf("get 0 frame_interval error!\n");
    } else{
  		printf("0 keyFrameInterval = %d\n", ivalue);
        streamchannels[0].keyFrameInterval = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/video/encoder/0/aisle_mode", &ivalue) != 0) {
        printf("get 0 aisle_mode error!\n");
    } else{
  		printf("0 hfr = %d\n", ivalue);
        streamchannels[0].hfr = ivalue;
    }
}

void SecondaryStreamInit(void)
{
    char value[128] = {0}; 
    int size = sizeof(value);

    if(XPR_UPS_GetString("/camera/video/encoder/1/codec", value, &size) != 0) {
        printf("get 1 codec error!\n");
    } else {
        printf("1 codec = %s\n", value);
        sprintf(streamchannels[1].videoCodecType, "%s", value); 
    }

    size = sizeof(value);
    if(XPR_UPS_GetString("/camera/video/encoder/1/resolution", value, &size) != 0) {
        printf("get 1 resolution error!\n");
    } else {
        printf("1 resolution = %s\n", value);
        sprintf(streamchannels[1].videoResolution, "%s", value); 
    }

    int ivalue = 0;
    if(XPR_UPS_GetInteger("/camera/video/encoder/1/bitrate", &ivalue) != 0)
        printf("get 1 bitrate error!\n");
    else{
	printf("1 constantBitRate = %d\n", ivalue);
        streamchannels[1].constantBitRate = ivalue;
    }

    if(XPR_UPS_GetInteger("/camera/video/encoder/1/fps", &ivalue) != 0)
        printf("get 0 fps error!\n");
    else{
	printf("1 maxFrameRate = %d\n", ivalue);
        streamchannels[1].maxFrameRate = ivalue;
    }

    if(strcmp(streamchannels[1].videoCodecType, "H.264") == 0) {
	    if(XPR_UPS_GetInteger("/camera/video/encoder/1/h264/quality", &ivalue) != 0)
		printf("get 1 h264 quality error!\n");
	    else{
		printf("1 h264 fixedQuality = %d\n", ivalue);
		streamchannels[1].fixedQuality = ivalue;
	    }
   }

    if(strcmp(streamchannels[1].videoCodecType, "MJPEG") == 0) {
	    if(XPR_UPS_GetInteger("/camera/video/encoder/1/mjpeg/quality", &ivalue) != 0)
		printf("get 1 mjpeg quality error!\n");
	    else{
		printf("1 mjpeg fixedQuality = %d\n", ivalue);
		streamchannels[1].fixedQuality = ivalue;
	    }
    }
	
    if(XPR_UPS_GetInteger("/camera/video/encoder/1/frame_interval", &ivalue) != 0)
        printf("get 1 frame_interval error!\n");
    else{
	printf("1 keyFrameInterval = %d\n", ivalue);
        streamchannels[1].keyFrameInterval = ivalue;
    }

    if(XPR_UPS_GetInteger("/camera/video/encoder/1/aisle_mode", &ivalue) != 0)
        printf("get 1 aisle_mode error!\n");
    else{
	printf("1 keyFrameInterval = %d\n", ivalue);
        streamchannels[1].hfr = ivalue;
    }
}

// GET PSIA/Streaming/channels/101
////////////////////////////////////////////////////// //////////////////////////
int GET_PSIA_Streaming_channels_101(struct mg_connection* conn,
                                    const struct mg_request_info* request_info)
{
    MainStreamInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<StreamingChannel version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<Video>\n"
              "<videoCodecType>%s</videoCodecType>\n"
              "<hfrType>%d</hfrType>\n"
              "<videoResolution>%s</videoResolution>\n"
              "<constantBitRate>%d</constantBitRate>\n"
              "<fixedQuality>%d</fixedQuality>\n"
              "<maxFrameRate>%d</maxFrameRate>\n"
              //"<keyFrameInterval>1000</keyFrameInterval>\n"
              "</Video>\n"
              "<Audio>\n"
              "<audioInputChannelID>1</audioInputChannelID>\n"
              "<audioCompressionType>G.711alaw</audioCompressionType>\n"
              "</Audio>\n"
              "</StreamingChannel>", streamchannels[0].videoCodecType, streamchannels[0].hfr, streamchannels[0].videoResolution,
		streamchannels[0].constantBitRate, streamchannels[0].fixedQuality, streamchannels[0].maxFrameRate);
    return 1;
}

// PUT PSIA/Streaming/channels/101
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Streaming_channels_101(struct mg_connection* conn,
                                    const struct mg_request_info* request_info, XML_Parser p)
{
    XML_SetCharacterDataHandler(p, chardatahandler);
    XML_SetElementHandler(p, start, end);
    // 解析客户端传递的xml
    char data[4096];
    int data_len = mg_read(conn, data, sizeof(data));
    if (XML_Parse(p, data, data_len, 0) == XML_STATUS_ERROR) {
        printf("xml -parse failed!---------\n");
    }

    if (XPR_UPS_SetString("/camera/video/encoder/0/codec", streamchannels[0].videoCodecType, strlen(streamchannels[0].videoCodecType)) != 0)
        printf("set 0 codec error!\n");
    else
        printf("set 0 codec:%s ok!\n", streamchannels[0].videoCodecType);

    if (XPR_UPS_SetString("/camera/video/encoder/0/resolution", streamchannels[0].videoResolution, strlen(streamchannels[0].videoResolution)) != 0)
        printf("set 0 resolution error!\n");
    else
        printf("set 0 resolution:%s ok!\n", streamchannels[0].videoResolution);

    if (XPR_UPS_SetInteger("/camera/video/encoder/0/fps", streamchannels[0].maxFrameRate) != 0)
        printf("set 0 fps error!\n");
    else
        printf("set 0 fps:%d ok!\n", streamchannels[0].maxFrameRate);

    if (strcmp(streamchannels[0].videoCodecType, "MJPEG") == 0) {
	    if (XPR_UPS_SetInteger("/camera/video/encoder/0/mjpeg/quality", streamchannels[0].fixedQuality) != 0)
			printf("set 0 mjpeg quality error!\n");
	    else
			printf("set 0 mjpeg quality:%d ok!\n", streamchannels[0].fixedQuality);
    } else if (strcmp(streamchannels[0].videoCodecType, "H.264") == 0) {
	    if(XPR_UPS_SetInteger("/camera/video/encoder/0/h264/quality", streamchannels[0].fixedQuality) != 0)
			printf("set 0 H.264 quality error!\n");
	    else
			printf("set 0 H.264 quality:%d ok!\n", streamchannels[0].fixedQuality);
    }

    if (XPR_UPS_SetInteger("/camera/video/encoder/0/frame_interval", streamchannels[0].keyFrameInterval) != 0)
        printf("set 0 frame interval error!\n");
    else
        printf("set 0 frame interval:%d ok!\n", streamchannels[0].keyFrameInterval);

    if (XPR_UPS_SetInteger("/camera/video/encoder/0/aisle_mode", streamchannels[0].hfr) != 0)
        printf("set 0 aisle mode error!\n");
    else
        printf("set 0 aisle mode:%d ok!\n", streamchannels[0].hfr);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/streaming/channels/101</requestURL>\n"
              "<statusCode>0</statusCode>\n"
              "<statusString>Reboot Required</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

// GET PSIA/Streaming/channels/102
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_Streaming_channels_102(struct mg_connection* conn,
                                    const struct mg_request_info* request_info)
{
    SecondaryStreamInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<StreamingChannel version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<Video>\n"
              "<videoCodecType>%s</videoCodecType>\n"
              "<hfrType>%d</hfrType>\n"
              "<videoResolution>%s</videoResolution>\n"
              "<constantBitRate>%d</constantBitRate>\n"
              "<fixedQuality>%d</fixedQuality>\n"
              "<maxFrameRate>%d</maxFrameRate>\n"
              //"<keyFrameInterval>1000</keyFrameInterval>\n"
              "</Video>\n"
              "<Audio>\n"
              "<audioInputChannelID>1</audioInputChannelID>\n"
              "<audioCompressionType>G.711alaw</audioCompressionType>\n"
              "</Audio>\n"
              "</StreamingChannel>", streamchannels[1].videoCodecType, streamchannels[1].hfr, streamchannels[1].videoResolution,
		streamchannels[1].constantBitRate, streamchannels[1].fixedQuality, streamchannels[1].maxFrameRate);
    return 1;
}

// PUT PSIA/Streaming/channels/102
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Streaming_channels_102(struct mg_connection* conn,
                                    const struct mg_request_info* request_info, XML_Parser p)
{
    XML_SetCharacterDataHandler(p, chardatahandler);
    XML_SetElementHandler(p, start, end);
    // 解析客户端传递的xml
    char data[4096];
    int data_len = mg_read(conn, data, sizeof(data));
    if (XML_Parse(p, data, data_len, 0) == XML_STATUS_ERROR) {
        printf("xml -parse failed!---------\n");
    }

    if (XPR_UPS_SetString("/camera/video/encoder/1/codec", streamchannels[1].videoCodecType, strlen(streamchannels[1].videoCodecType)) != 0)
        printf("set 1 codec error!\n");
    else
        printf("set 1 codec:%s ok!\n", streamchannels[1].videoCodecType);

    if (XPR_UPS_SetString("/camera/video/encoder/1/resolution", streamchannels[1].videoResolution, strlen(streamchannels[1].videoResolution)) != 0)
        printf("set 1 resolution error!\n");
    else
        printf("set 1 resolution:%s ok!\n", streamchannels[1].videoResolution);

    if (XPR_UPS_SetInteger("/camera/video/encoder/1/fps", streamchannels[1].maxFrameRate) != 0)
        printf("set 1 fps error!\n");
    else
        printf("set 1 fps:%d ok!\n", streamchannels[1].maxFrameRate);

    if (strcmp(streamchannels[1].videoCodecType, "MJPEG") == 0) {
	    if (XPR_UPS_SetInteger("/camera/video/encoder/1/mjpeg/quality", streamchannels[1].fixedQuality) != 0)
			printf("set 1 mjpeg quality error!\n");
	    else
			printf("set 1 mjpeg quality:%d ok!\n", streamchannels[1].fixedQuality);
    } else if(strcmp(streamchannels[1].videoCodecType, "H.264") == 0) {
	    if(XPR_UPS_SetInteger("/camera/video/encoder/1/h264/quality", streamchannels[1].fixedQuality) != 0)
			printf("set 1 h264 quality error!\n");
	    else
			printf("set 1 h264 quality:%d ok!\n", streamchannels[1].fixedQuality);
    }

    if (XPR_UPS_SetInteger("/camera/video/encoder/1/frame_interval", streamchannels[1].keyFrameInterval) != 0)
        printf("set 1 frame interval error!\n");
    else
        printf("set 1 frame interval:%d ok!\n", streamchannels[1].keyFrameInterval);

    if (XPR_UPS_SetInteger("/camera/video/encoder/1/aisle_mode", streamchannels[1].hfr) != 0)
        printf("set 1 aisle mode error!\n");
    else
        printf("set 1 aisle mode:%d ok!\n", streamchannels[1].hfr);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/streaming/channels/102</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>Reboot Required</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

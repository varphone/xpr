#include "SystemAudio.h"

struct AudioInfo {
    int encoder_codec;
    int encoder_input;
    int encoder_volume;
};

static struct AudioInfo audi;
static int audiodoor = 0;
int Depth;
char buffers[1024];

static int GetMotion(char* buffer, int lens)
{
    if (lens != 0) {
        if (audiodoor == 1) {
            audi.encoder_codec = atoi(buffers);
            audiodoor = 0;
        }
        if (audiodoor == 2) {
            audi.encoder_input = atoi(buffers);
            audiodoor = 0;
        }
        if (audiodoor == 3) {
            audi.encoder_volume = atoi(buffers);
            audiodoor = 0;
        }
    }
    return 0;
}


static void XMLCALL
chardatahandler(void* uerData, const char* target, int lens)
{
    memcpy(buffers, target, lens);
    buffers[lens] = '\0';
    GetMotion(buffers, lens);
}


static void XMLCALL
start(void* data, const char* el, const char** attr)
{
    if (strcmp(el, "AudioCodecType") == 0)
        audiodoor = 1;
    if (strcmp(el, "AudioInputType") == 0)
        audiodoor = 2;
    if (strcmp(el, "AudioVolume") == 0)
        audiodoor = 3;
    Depth++;
}

static void XMLCALL
end(void* data, const char* el)
{
    Depth--;
}

void SystemAudionInit(void)
{
    int ivalue = 0;
    if (XPR_UPS_GetInteger("/camera/audio/encoder/0/codec", &ivalue) != 0) {
        printf("get 0 audio encoder codec error!\n");
    } else {
        printf("0 audio encoder codec = %d\n", ivalue);
        audi.encoder_codec = ivalue; 
    }

    if (XPR_UPS_GetInteger("/camera/audio/encoder/0/input", &ivalue) != 0) {
        printf("get 0 audio encoder input error!\n");
    } else {
        printf("0 audio encoder input = %d\n", ivalue);
        audi.encoder_input = ivalue; 
    }

    if (XPR_UPS_GetInteger("/camera/audio/encoder/0/volume", &ivalue) != 0) {
        printf("get 0 audio encoder volume error!\n");
    } else {
        printf("0 audio encoder volume = %d\n", ivalue);
        audi.encoder_volume = ivalue; 
    }
}

// GET PSIA/System/Audio/Channels
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_System_Audio_Channels(struct mg_connection* conn,
                                   const struct mg_request_info* request_info)
{
    SystemAudionInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<AudioChannel version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<AudioCodecType>%d</AudioCodecType>\n"
              "<AudioInputType>%d</AudioInputType>\n"
              "<AudioVolume>%d</AudioVolume>\n"
              "</AudioChannel>", audi.encoder_codec, audi.encoder_input, audi.encoder_volume);
    return 1;//0让上一级返回消息，1消息直接返回ok;
}

//PUT PSIA/System/Audio/Channels
int PUT_PSIA_System_Audio_Channels(struct mg_connection* conn,
                                      const struct mg_request_info* request_info, XML_Parser p)
{
    XML_SetCharacterDataHandler(p, chardatahandler);
    XML_SetElementHandler(p, start, end);
    char data[4096];
    int data_len = mg_read(conn, data, sizeof(data));
    // 解析客户端传递的xml
    if (XML_Parse(p, data, data_len, 0) == XML_STATUS_ERROR) {
        printf("xml -parse failed!---------\n");
    }

    if (XPR_UPS_SetInteger("/camera/audio/encoder/0/codec", audi.encoder_codec) != 0)
        printf("set 0 audio encoder codec error!\n");
    else
        printf("set 0 audio encoder codec:%d ok!\n", audi.encoder_codec);

    if (XPR_UPS_SetInteger("/camera/audio/encoder/0/input", audi.encoder_input) != 0)
        printf("set 0 audio encoder input error!\n");
    else
        printf("set 0 audio encoder input:%d ok!\n", audi.encoder_input);

    if (XPR_UPS_SetInteger("/camera/audio/encoder/0/volume", audi.encoder_volume) != 0)
        printf("set 0 audio encoder volume error!\n");
    else
        printf("set 0 audio encoder volume:%d ok!\n", audi.encoder_volume);
    // 根据解析参数设置Set ...    
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/system/audio/channels</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}


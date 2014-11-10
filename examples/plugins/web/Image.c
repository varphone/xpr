/*#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>
#include <expat.h>

#include "mongoose.h"
*/
#include <fcntl.h>
#include "Image.h"
//#include "ambarella/a5s/amba_api.h"


#define VIDEOSVR_SOCK_PATH      "/tmp/videosvr.sock-0"
#define ABILITIES_XML_FILE      "/ambarella/abilities.xml"
#define CHANNEL_XML_FILE        "/ambarella/channel.xml"
#define GLOBAL_XML_FILE         "/ambarella/global.xml"
#define DEF_UN_RCVTMO           60000
#define DEF_UN_SNDTMO           1000

static ImageConfig imfg;
int Depth, Depth1;
char buffers[1024];


static int GetColor(char* buffer, int lens)
{
    if (lens != 0) {
        switch (imfg.color_door) {
        case 1:
            imfg.brightness = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 2:
            imfg.contrast = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 3:
            imfg.saturation = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 4:
            imfg.hue = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 5:
            imfg.sharpness = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 6:
            sprintf(imfg.irismode, "%s", buffers);
            imfg.color_door = 0;
            break;
        case 7:
            imfg.exporsetime = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 8: 
            imfg.exposurelevel = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 9: 
            imfg.exposuremintime = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 10: 
            imfg.exposuremaxtime = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 11:
            imfg.noise2dlevel = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 12:
            imfg.noise3dlevel = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 13:
            imfg.colorscale = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 14:
            imfg.whiteblancemode = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 15:
            imfg.mirrormode = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 16:
            imfg.antiflicker = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 17:
            imfg.blackwhitemode = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 18: 
            imfg.light = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 19: 
            imfg.lightmode = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 20: 
            imfg.demolevel = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 21:
            imfg.wdrmode = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 22: 
            imfg.flogmode = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 23: 
            imfg.metermode = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 24: 
            imfg.maxgain = atoi(buffers);
            imfg.color_door = 0;
            break;
        case 25: 
            sprintf(imfg.lumashow,"%s",buffers);
            imfg.color_door = 0;
            break;

        default:
            break;
        }
    }
    return 0;
}


static void XMLCALL
chardatahandler(void* uerData, const char* target, int lens)
{
    memcpy(buffers, target, lens);
    buffers[lens] = '\0';
    GetColor(buffers, lens);
}

static int noiseopen = 0, flipopen = 0, wdropen = 0, blcopen = 0, sceneopen = 0, mirrordoor = 0;
static int pattern, bayer_patterns;
static void XMLCALL
start(void* data, const char* el, const char** attr)
{
    if (strcmp(el, "BrightnessLevel") == 0)
        imfg.color_door = 1;
    if (strcmp(el, "ContrastLevel") == 0)
        imfg.color_door = 2;
    if (strcmp(el, "SaturationLevel") == 0)
        imfg.color_door = 3;
    if (strcmp(el, "hueLevel") == 0)
        imfg.color_door = 4;
    if (strcmp(el, "SharpnessLevel") == 0)
        imfg.color_door = 5;
    if (strcmp(el, "IrisMode") == 0)
        imfg.color_door = 6;
    if (strcmp(el, "ExporseTime") == 0)
        imfg.color_door = 7;
    if (strcmp(el, "ExposureLevel") == 0)
        imfg.color_door = 8;
    if (strcmp(el, "ExposureMinTimeLevel") == 0)
        imfg.color_door = 9;
    if (strcmp(el, "ExposureMaxTimeLevel") == 0)
        imfg.color_door = 10;
    if (strcmp(el, "Noise2DLevel") == 0)
        imfg.color_door = 11;
    if (strcmp(el, "Noise3DLevel") == 0)
        imfg.color_door = 12;
    if (strcmp(el, "GrayScaleMode") == 0)
        imfg.color_door = 13;
    if (strcmp(el, "WhiteBlanceStyle") == 0)
        imfg.color_door = 14;
    if (strcmp(el, "ImageMirrorMode") == 0)
        imfg.color_door = 15;
    if (strcmp(el, "PowerLineFrequencyMode") == 0)
        imfg.color_door = 16;
    if (strcmp(el, "IrcutFilterType") == 0)
        imfg.color_door = 17;
    if (strcmp(el, "Light") == 0)
        imfg.color_door = 18;
    if (strcmp(el, "LightMode") == 0)
        imfg.color_door = 19;
    if (strcmp(el, "DemoMode") == 0)
        imfg.color_door = 20;
    if (strcmp(el, "WDRMode") == 0)
        imfg.color_door = 21;
    if (strcmp(el, "FlogMode") == 0)
        imfg.color_door = 22;    
    if (strcmp(el, "MeterMode") == 0)
        imfg.color_door = 23;    
    if (strcmp(el, "GainLevel") == 0)
        imfg.color_door = 24;
    if (strcmp(el, "LumaShow") == 0)
        imfg.color_door = 25;
    
    Depth++;
}

static void XMLCALL
end(void* data, const char* el)
{
    Depth--;
}

void ImageConfigurationInit(void)
{
    int ivalue = 0;
    if (XPR_UPS_GetInteger("/camera/image/source/0/brightness", &ivalue) != 0) {
        printf("get 0 brightness error!\n");
    } else {
		printf("0 brightness = %d\n", ivalue);
        imfg.brightness = ivalue;
    }

    if(XPR_UPS_GetInteger("/camera/image/source/0/saturation", &ivalue) != 0) {
        printf("get 0 saturation error!\n");
    } else {
  		printf("0 saturation = %d\n", ivalue);
        imfg.saturation = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/contrast", &ivalue) != 0) {
        printf("get 0 contrast error!\n");
    } else {
		printf("0 contrast = %d\n", ivalue);
        imfg.contrast = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/sharpness", &ivalue) != 0) {
        printf("get 0 sharpness error!\n");
    } else {
  		printf("0 sharpness = %d\n", ivalue);
        imfg.sharpness = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/exposure/max_time", &ivalue) != 0) {
        printf("get 0 exposure max_time error!\n");
    } else {
  		printf("0 exposure max_time = %d\n", ivalue);
        imfg.exposuremaxtime = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/exposure/min_time", &ivalue) != 0) {
        printf("get 0 exposure min_time error!\n");
    } else {
		printf("0 exposure min_time = %d\n", ivalue);
        imfg.exposuremintime = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/exposure/time", &ivalue) != 0) {
        printf("get 0 exposure time error!\n");
    } else {
  		printf("0 exposure time = %d\n", ivalue);
        imfg.exporsetime = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/iris_mode", &ivalue) != 0) {
        printf("get 0 iris_mode error!\n");
    } else {
  		printf("0 iris_mode = %d\n", ivalue);
		if(ivalue == 0)
        	sprintf(imfg.irismode, "%s", "auto");
		else
	  		sprintf(imfg.irismode, "%s", "manual");
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/anti_flicker", &ivalue) != 0) {
        printf("get 0 anti_flicker error!\n");
    } else {
  		printf("0 anti_flicker = %d\n", ivalue);
        imfg.antiflicker = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/mirror_mode", &ivalue) != 0) {
        printf("get 0 mirror_mode error!\n");
    }else {
  		printf("0 mirror_mode = %d\n", ivalue);
        imfg.mirrormode = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/day_night_mode", &ivalue) != 0) {
        printf("get 0 day_night_mode error!\n");
    } else {
  		printf("0 day_night_mode = %d\n", ivalue);
        imfg.blackwhitemode = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/wdr_level", &ivalue) != 0) {
        printf("get 0 wdr_level error!\n");
    } else {
  		printf("0 wdr_level = %d\n", ivalue);
        imfg.wdrmode = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/white_blance_mode", &ivalue) != 0) {
        printf("get 0 white_blance_mode error!\n");
    } else {
  		printf("0 white_blance_mode = %d\n", ivalue);
        imfg.whiteblancemode = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/2d_noise_level", &ivalue) != 0) {
        printf("get 0 2d_noise_level error!\n");
    } else {
  		printf("0 2d_noise_level = %d\n", ivalue);
        imfg.noise2dlevel = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/3d_noise_level", &ivalue) != 0) {
        printf("get 0 3d_noise_level error!\n");
    } else {
  		printf("0 3d_noise_level = %d\n", ivalue);
        imfg.noise3dlevel = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/defog_mode", &ivalue) != 0) {
        printf("get 0 defog_mode error!\n");
    } else {
  		printf("0 defog_mode = %d\n", ivalue);
        imfg.flogmode = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/metering_mode", &ivalue) != 0) {
        printf("get 0 metering_mode error!\n");
    } else {
  		printf("0 metering_mode = %d\n", ivalue);
        imfg.metermode = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/image/source/0/grayscale", &ivalue) != 0) {
        printf("get 0 grayscale error!\n");
    } else {
  		printf("0 grayscale = %d\n", ivalue);
        imfg.colorscale = ivalue;
    }
}

// GET PSIA/Custom/Image/channels/1
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_Custom_Image_channels_1(struct mg_connection* conn,
        const struct mg_request_info* request_info)
{
    ImageConfigurationInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ImageChannel version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<Imaging>\n"
              "<Color>\n"
              "<BrightnessLevel>%d</BrightnessLevel>\n"
              "<ContrastLevel>%d</ContrastLevel>\n"
              "<SaturationLevel>%d</SaturationLevel>\n"
              "</Color>\n"
              "<Sharpness>\n"
              "<SharpnessLevel>%d</SharpnessLevel>\n"
              "</Sharpness>\n"
              "<NoiseReduceExt>\n"
              "<Noise2DLevel>%d</Noise2DLevel>\n"
              "<Noise3DLevel>%d</Noise3DLevel>\n"
              "</NoiseReduceExt>\n"
              "<Exposure>\n"
              "<IrisMode>%s</IrisMode>\n"
              "<ExporseTime>%d</ExporseTime>\n"
              "<ExposureLevel>%d</ExposureLevel>\n"
              "<ExposureMinTimeLevel>%d</ExposureMinTimeLevel>\n"
              "<ExposureMaxTimeLevel>%d</ExposureMaxTimeLevel>\n"
              "</Exposure>\n"
              "<GrayScale>\n"
              "<GrayScaleMode>%d</GrayScaleMode>\n"
              "</GrayScale>\n"              
              "<WhiteBlance>\n"
              "<WhiteBlanceStyle>%d</WhiteBlanceStyle>\n"
              "</WhiteBlance>\n"
              "<ImageMirror>\n"
              "<ImageMirrorMode>%d</ImageMirrorMode>\n"
              "</ImageMirror>\n"
              "<PowerLineFrequency>\n"
              "<PowerLineFrequencyMode>%d</PowerLineFrequencyMode>\n"
              "</PowerLineFrequency>\n"
              "<IrcutFilter>\n"
              "<IrcutFilterType>%d</IrcutFilterType>\n"
              "<Light>%d</Light>\n"
              "<LightMode>%d</LightMode>\n"
              "</IrcutFilter>\n"
              "<WDR>\n"
              "<Enabled>%d</Enabled>\n"
              "</WDR>\n"
              "<Flog>\n"
              "<Mode>%d</Mode>\n"
              "</Flog>\n" //透雾
              "<Meter>\n"
              "<Mode>%d</Mode>\n"
              "</Meter>\n" //测光模式
              "</Imaging>\n"
              "</ImageChannel>", imfg.brightness,imfg.contrast,imfg.saturation,imfg.sharpness,
				 imfg.noise2dlevel,imfg.noise3dlevel,
				 imfg.irismode,
				 imfg.exporsetime,imfg.exposurelevel,imfg.exposuremintime,imfg.exposuremaxtime,
				 imfg.colorscale,imfg.whiteblancemode,imfg.mirrormode,imfg.antiflicker,imfg.blackwhitemode,
				 imfg.light,imfg.lightmode,imfg.wdrmode,imfg.flogmode,imfg.metermode);
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/Brightness
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_Brightness(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/brightness", imfg.brightness) != 0)
        printf("set 0 brightness error!\n");
    else
        printf("set 0 brightness:%d ok!\n", imfg.brightness);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/Brightness</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/Contrast
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_Contrast(struct mg_connection* conn,
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
    // 根据解析参数设置Set ... 
    if (XPR_UPS_SetInteger("/camera/image/source/0/contrast", imfg.contrast) != 0)
        printf("set 0 contrast error!\n");
    else
        printf("set 0 contrast:%d ok!\n", imfg.contrast);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/Contrast</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/Saturation
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_Saturation(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/saturation", imfg.saturation) != 0)
        printf("set 0 saturation error!\n");
    else
        printf("set 0 saturation:%d ok!\n", imfg.saturation);


    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/Saturation</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/Sharpness
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_Sharpness(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/sharpness", imfg.sharpness) != 0)
        printf("set 0 sharpness error!\n");
    else
        printf("set 0 sharpness:%d ok!\n", imfg.sharpness);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/Sharpness</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/IrisMode
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_IrisMode(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    //如果光圈是开启参考下面的代码 auto,manual
    if (XPR_UPS_SetInteger("/camera/image/source/0/iris_mode", strcmp(imfg.irismode, "auto")?0:1) != 0)
        printf("set 0 irismode error!\n");
    else
        printf("set 0 irismode:%s ok!\n", imfg.irismode);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/IrisMode</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/ExporseTime
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_ExporseTime(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/exposure/time", imfg.exporsetime) != 0)
        printf("set 0 exporsetime error!\n");
    else
        printf("set 0 exporsetime:%d ok!\n", imfg.exporsetime);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/ExporseTime</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/ExposureLevel
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_ExposureLevel(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/exposure/time", imfg.exposurelevel) != 0)
        printf("set 0 exposurelevel error!\n");
    else
        printf("set 0 exposurelevel:%d ok!\n", imfg.exposurelevel);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/ExposureLevel</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/ExposureMinTime
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_ExposureMinTime(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/exposure/min_time", imfg.exposuremintime) != 0)
        printf("set 0 exposuremintime error!\n");
    else
        printf("set 0 exposuremintime:%d ok!\n", imfg.exposuremintime);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/ExposureMinTime</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/ExposureMaxTime
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_ExposureMaxTime(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/exposure/max_time", imfg.exposuremaxtime) != 0)
        printf("set 0 exposuremaxtime error!\n");
    else
        printf("set 0 exposuremaxtime:%d ok!\n", imfg.exposuremaxtime);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/ExposureMaxTime</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/Noise2D
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_Noise2D(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/2d_noise_level", imfg.noise2dlevel) != 0)
        printf("set 0 noise2dlevel error!\n");
    else
        printf("set 0 noise2dlevel:%d ok!\n", imfg.noise2dlevel);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/Noise2D</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/Noise3D
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_Noise3D(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/3d_noise_level", imfg.noise3dlevel) != 0)
        printf("set 0 noise3dlevel error!\n");
    else
        printf("set 0 noise3dlevel:%d ok!\n", imfg.noise3dlevel);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/Noise3D</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/GrayScale
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_GrayScale(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/grayscale", imfg.colorscale) != 0)
        printf("set 0 colorscale error!\n");
    else
        printf("set 0 colorscale:%d ok!\n", imfg.colorscale);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/GrayScale</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/WhiteBlance
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_WhiteBlance(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/white_blance_mode", imfg.whiteblancemode) != 0)
        printf("set 0 whiteblancemode error!\n");
    else
        printf("set 0 whiteblancemode:%d ok!\n", imfg.whiteblancemode);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/WhiteBlance</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/ImageMirror
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_ImageMirror(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/mirror_mode", imfg.mirrormode) != 0)
        printf("set 0 mirrormode error!\n");
    else
        printf("set 0 mirrormode:%d ok!\n", imfg.mirrormode);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/ImageMirror</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/PowerLineFrequency
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_PowerLineFrequency(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/anti_flicker", imfg.antiflicker) != 0)
        printf("set 0 antiflicker error!\n");
    else
        printf("set 0 antiflicker:%d ok!\n", imfg.antiflicker);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/PowerLineFrequency</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/IrcutFilter
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_IrcutFilter(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/day_night_mode", imfg.blackwhitemode) != 0)
        printf("set 0 blackwhitemode error!\n");
    else
        printf("set 0 blackwhitemode:%d ok!\n", imfg.blackwhitemode);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/IrcutFilter</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/Light
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_Light(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/ext/light", imfg.light) != 0)
        printf("set 0 light error!\n");
    else
        printf("set 0 light:%d ok!\n", imfg.light);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/Light</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/LightMode
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_LightMode(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/ext/light_mode", imfg.lightmode) != 0)
        printf("set 0 lightmode error!\n");
    else
        printf("set 0 lightmode:%d ok!\n", imfg.lightmode);


    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/LightMode</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/DemoMode
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_DemoMode(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    printf("demolevel :%d\n",imfg.demolevel);


    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/DemoMode</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/WDRMode
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_WDRMode(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/wdr_level", imfg.wdrmode) != 0)
        printf("set 0 wdrmode error!\n");
    else
        printf("set 0 wdrmode:%d ok!\n", imfg.wdrmode);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/WDRMode</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/FlogMode
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_FlogMode(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/defog_mode", imfg.flogmode) != 0)
        printf("set 0 defog_mode error!\n");
    else
        printf("set 0 defog_mode:%d ok!\n", imfg.flogmode);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/FlogMode</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//PUT PSIA/Custom/Image/channels/1/MeterMode
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_Image_channels_1_MeterMode(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/image/source/0/metering_mode", imfg.metermode) != 0)
        printf("set 0 metermode error!\n");
    else
        printf("set 0 metermode:%d ok!\n", imfg.metermode);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/image/channels/1/MeterMode</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}


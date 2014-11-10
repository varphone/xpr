#ifndef _WEB_IMAGE_H_
#define _WEB_IMAGE_H_

#include "Config.h"

typedef struct ImageConfig {
    int  color_door;
    int  brightness;
    int  contrast;
    int  saturation;
    int  hue;
    int  sharpness;
    int  exporsetime;
    int  noise2dlevel;
    int  noise3dlevel;
    int  colorscale;
    int  whiteblancemode;
    int  mirrormode;
    int  maxgain;
    int  exposuremaxtime;
    int  exposuremintime;
    int  exposurelevel;
    int  demolevel;
    int  antiflicker;
    int  blackwhitemode;
    int  light;
    int  lightmode;
    int  wdrmode;
    int  flogmode;
    int  metermode;
    char irismode[30];
    char lumashow[30];
} ImageConfig;

void ImageConfigurationInit(void);

int GET_PSIA_Custom_Image_channels_1(struct mg_connection* conn,
        const struct mg_request_info* request_info);

int PUT_PSIA_Custom_Image_channels_1_Brightness(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_Contrast(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_Saturation(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_Sharpness(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_IrisMode(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_ExporseTime(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_ExposureLevel(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_ExposureMinTime(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_ExposureMaxTime(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_Noise2D(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_Noise3D(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_GrayScale(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_WhiteBlance(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_ImageMirror(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_PowerLineFrequency(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_IrcutFilter(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_Light(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_LightMode(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_DemoMode(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_WDRMode(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_FlogMode(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_Image_channels_1_MeterMode(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);


#endif /* _WEB_IMAGE_H_ */

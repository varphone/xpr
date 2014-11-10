#ifndef _WEB_OSD_H_
#define _WEB_OSD_H_

#include "Config.h"

typedef struct OsdConfig {
    int osd_door;
    int osdenable;//osd enable
    int osdtimetype;//osdtimetype enable
    int osddatetype;//osddatetype enable
    int gisenable;//gis enable
    int speedenable;//speed enable
    int gyroenable;//gyro enable
    int aeinfoenable;//aeinfo enable
    char devicename[64];//devicename 
} OsdConfig;

void OsdInit(void);

int GET_PSIA_Custom_OSD_channels_1(struct mg_connection* conn,
        const struct mg_request_info* request_info);

int PUT_PSIA_Custom_OSD_channels_1(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_OSD_Gis_info(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_Custom_OSD_Ae_info(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

#endif /* _WEB_OSD_H_ */

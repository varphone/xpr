#ifndef _WEB_PTZ_H_
#define _WEB_PTZ_H_

#include "Config.h"

typedef struct PtzInfo {
    int address;
    int speed;
    int focusmode;
    int tptz_control;
    int tptz_zoom;
    int tptz_focus;
    int tptz_stop;
    int iris_enabled;
    char protocol[256];
} PtzInfo;

void PtzInit(void);

int GET_PSIA_Custom_PTZ_channels(struct mg_connection* conn,
        const struct mg_request_info* request_info);

int PUT_PSIA_Custom_PTZ_channels(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);


int PUT_PSIA_PTZ_Tptz_Control(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_PTZ_Tptz_Focus(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_PTZ_Tptz_Zoom(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_PTZ_Tptz_Stop(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int PUT_PSIA_PTZ_Iris_Enabled(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);
#endif /* _WEB_PTZ_H_ */

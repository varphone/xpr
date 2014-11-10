#include "Osd.h"

#define CHANNEL_XML_FILE        "/ambarella/channel.xml"

static OsdConfig ocfg;
int Depth;
char buffers[1024];

static int GetOsdInfo(char* buffers, int lens)
{
    if (lens != 0) {
        switch (ocfg.osd_door) {
        case 1:
            ocfg.osdenable = atoi(buffers);
            ocfg.osd_door = 0;
            break;
        case 2:
            ocfg.osdtimetype = atoi(buffers);
            ocfg.osd_door = 0;
            break;
        case 3:
            ocfg.osddatetype = atoi(buffers);
            ocfg.osd_door = 0;
            break;
        case 4:
            sprintf(ocfg.devicename, "%s", buffers);
            ocfg.osd_door = 0;
            break;
        case 5:
            ocfg.gisenable = atoi(buffers);
            ocfg.osd_door = 0;
            break;
        case 6:
            ocfg.speedenable = atoi(buffers);
            ocfg.osd_door = 0;
            break;
        case 7:
            ocfg.gyroenable = atoi(buffers);
            ocfg.osd_door = 0;
            break;
        case 8:
            ocfg.aeinfoenable = atoi(buffers);
            ocfg.osd_door = 0;
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
    GetOsdInfo(buffers, lens);
}


static void XMLCALL
start(void* data, const char* el, const char** attr)
{
    if (strcmp(el, "OsdEnable") == 0)
        ocfg.osd_door = 1;
    if (strcmp(el, "OsdTimeType") == 0)
        ocfg.osd_door = 2;
    if (strcmp(el, "OsdDateType") == 0)
        ocfg.osd_door = 3;
    if (strcmp(el, "DeviceName") == 0)
        ocfg.osd_door = 4;
    if (strcmp(el, "GisEnable") == 0)
        ocfg.osd_door = 5;
    if (strcmp(el, "GisSpeedEnable") == 0)
        ocfg.osd_door = 6;
    if (strcmp(el, "GisGyroEnable") == 0)
        ocfg.osd_door = 7;
    if (strcmp(el, "AEInfoEnable") == 0)
        ocfg.osd_door = 8;
    Depth++;
}

static void XMLCALL
end(void* data, const char* el)
{
    Depth--;
}

void OsdInit(void)
{
    char value[128] = {0}; 
    int size = sizeof(value);

    if (XPR_UPS_GetString("/system/information/name", value, &size) != 0) {
        printf("get ocfg devicename error!\n");
    } else {
        printf("ocfg devicename = %s\n", value);
        sprintf(ocfg.devicename, "%s", value); 
    }

    int ivalue = 0;
    if (XPR_UPS_GetInteger("/camera/osd/time_type", &ivalue) != 0)
        printf("get osdtimetype error!\n");
    else{
		printf("osdtimetype = %d\n", ivalue);
        ocfg.osdtimetype = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/osd/date_type", &ivalue) != 0)
        printf("get osddatetype error!\n");
    else{
		printf("osddatetype = %d\n", ivalue);
        ocfg.osddatetype = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/osd/enabled", &ivalue) != 0)
        printf("get osdenable error!\n");
    else{
		printf("osdenable = %d\n", ivalue);
        ocfg.osdenable = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/osd/ae_info_enabled", &ivalue) != 0)
        printf("get aeinfoenable error!\n");
    else{
		printf("aeinfoenable = %d\n", ivalue);
        ocfg.aeinfoenable = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/osd/gis_invalid_data_enabled", &ivalue) != 0)
        printf("get gisenable error!\n");
    else{
		printf("gisenable = %d\n", ivalue);
        ocfg.gisenable = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/osd/gis_speed_unit", &ivalue) != 0)
        printf("get speedenable error!\n");
    else{
		printf("speedenable = %d\n", ivalue);
        ocfg.speedenable = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/osd/gyro_enabled", &ivalue) != 0)
        printf("get gyroenable error!\n");
    else{
		printf("gyroenable = %d\n", ivalue);
        ocfg.gyroenable = ivalue;
    }
}

// GET /PSIA/Custom/OSD/channels/1  
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_Custom_OSD_channels_1(struct mg_connection* conn,
        const struct mg_request_info* request_info)
{
    OsdInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<OSD version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<OsdEnable>%d</OsdEnable>\n"
              "<OsdTimeType>%d</OsdTimeType>\n"
              "<OsdDateType>%d</OsdDateType>\n"
              "<DeviceName>%s</DeviceName>\n"
              "<GisEnable>%d</GisEnable>\n"
              "<GisSpeedEnable>%d</GisSpeedEnable>\n"
              "<GisGyroEnable>%d</GisGyroEnable>\n"
              "<AEInfoEnable>%d</AEInfoEnable>\n"
              "</OSD>", ocfg.osdenable, ocfg.osdtimetype, ocfg.osddatetype, ocfg.devicename, ocfg.gisenable, ocfg.speedenable, ocfg.gyroenable, ocfg.aeinfoenable);
    return 1;
}

// PUT PSIA/Custom/OSD/channels/1 //SET OSD 日期显示 格式
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_OSD_channels_1(struct mg_connection* conn,
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
    // 根据解析参数设置Set ...
    if (XPR_UPS_SetInteger("/camera/osd/time_type", ocfg.osdtimetype) != 0)
        printf("set osdtimetype error!\n");
    else
        printf("set osdtimetype:%d ok!\n", ocfg.osdtimetype);

    if (XPR_UPS_SetInteger("/camera/osd/date_type", ocfg.osddatetype) != 0)
        printf("set osddatetype error!\n");
    else
        printf("set osddatetype:%d ok!\n", ocfg.osddatetype);

    if (XPR_UPS_SetInteger("/camera/osd/enabled", ocfg.osdenable) != 0)
        printf("set osdenable error!\n");
    else
        printf("set osdenable:%d ok!\n", ocfg.osdenable);

    if(XPR_UPS_SetString("/system/information/name", ocfg.devicename, strlen(ocfg.devicename)) != 0)
        printf("set devicename error!\n");
    else
        printf("set devicename:%s ok!\n", ocfg.devicename);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/osd/channels/1</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

// PUT PSIA/Custom/OSD/Gis/info SET Gis osd显示
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_OSD_Gis_info(struct mg_connection* conn,
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
    if (XPR_UPS_SetInteger("/camera/osd/gis_invalid_data_enabled", ocfg.gisenable) != 0)
        printf("set 0 gis_invalid_data_enabled error!\n");
    else
        printf("set 0 gis_invalid_data_enabled:%d ok!\n", ocfg.gisenable);

    if (XPR_UPS_SetInteger("/camera/osd/gis_speed_unit", ocfg.speedenable) != 0)
        printf("set 0 speedenable error!\n");
    else
        printf("set 0 speedenable:%d ok!\n", ocfg.speedenable);

    if (XPR_UPS_SetInteger("/camera/osd/gyro_enabled", ocfg.gyroenable) != 0)
        printf("set 0 gyroenable error!\n");
    else
        printf("set 0 gyroenable:%d ok!\n", ocfg.gyroenable);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/osd/gis/info</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

// PUT PSIA/Custom/OSD/Ae/info SET Gis osd显示
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_OSD_Ae_info(struct mg_connection* conn,
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
    if (XPR_UPS_SetInteger("/camera/osd/ae_info_enabled", ocfg.aeinfoenable) != 0)
        printf("set aeinfoenable error!\n");
    else
        printf("set aeinfoenable:%d ok!\n", ocfg.aeinfoenable);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/osd/ae/info</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

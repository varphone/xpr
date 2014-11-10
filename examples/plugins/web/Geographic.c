#include "Geographic.h"

static int georgraphicdoor;
char buffers[1024];
int Depth;

typedef struct georgraphicInfo {
    int gps_enabled;
    int synctime_enabled;
    int work_mode;
    int gyro_enabled;
    int gyro_sensitive;
    int compass_enabled;
    int ldm_enabled;
} georgraphicInfo;

static georgraphicInfo ghif;

static int get_georgraphic(char* buffer, int lens)
{
    if (lens != 0) {
        switch (georgraphicdoor) {
            case 1: 
                ghif.gps_enabled = atoi(buffers);
                georgraphicdoor = 0;
                break;
            case 2: 
                ghif.synctime_enabled = atoi(buffers);
                georgraphicdoor = 0;
                break;
            case 3: 
                ghif.work_mode = atoi(buffers);
                georgraphicdoor = 0;
                break;
            case 4: 
                ghif.compass_enabled = atoi(buffers);
                georgraphicdoor = 0;
                break;
            case 5: 
                ghif.gyro_enabled = atoi(buffers);
                georgraphicdoor = 0;
                break;
            case 6: 
                ghif.gyro_sensitive = atoi(buffers);
                georgraphicdoor = 0;
                break;
            case 7: 
                ghif.ldm_enabled = atoi(buffers);
                georgraphicdoor = 0;
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
    get_georgraphic(buffers, lens);
}

static void XMLCALL
start(void* data, const char* el, const char** attr)
{

    if (strcmp(el, "GpsServerEnabled") == 0)
        georgraphicdoor = 1;
    if (strcmp(el, "GpsSyncTime") == 0)
        georgraphicdoor = 2;
    if (strcmp(el, "GpsWorkMode") == 0)
        georgraphicdoor = 3;
    if (strcmp(el, "CompassEnabled") == 0)
        georgraphicdoor = 4;
    if (strcmp(el, "GyroEnabled") == 0)
        georgraphicdoor = 5;
    if (strcmp(el, "GyroSensitive") == 0)
        georgraphicdoor = 6;
    if (strcmp(el, "LdmEnabled") == 0)
        georgraphicdoor = 7;
    Depth++;
}

static void XMLCALL
end(void* data, const char* el)
{
    Depth--;
}

void GeographicInit(void)
{
    int ivalue = 0;
    if (XPR_UPS_GetInteger("/camera/gis/enabled", &ivalue) != 0) {
        printf("get ghif gps_enabled error!\n");
    } else {
		printf("ghif gps_enabled enabled = %d\n", ivalue);
        ghif.gps_enabled = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/gis/synctime_enabled", &ivalue) != 0) {
        printf("get ghif synctime_enabled error!\n");
    } else {
		printf("ghif synctime_enabled enabled = %d\n", ivalue);
        ghif.synctime_enabled = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/gis/work_mode", &ivalue) != 0) {
        printf("get ghif work_mode error!\n");
    } else {
		printf("ghif work_mode enabled = %d\n", ivalue);
        ghif.work_mode = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/gis/gyro_enabled", &ivalue) != 0) {
        printf("get ghif gyro_enabled error!\n");
    } else {
		printf("ghif gyro_enabled enabled = %d\n", ivalue);
        ghif.gyro_enabled = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/gis/gyro_sensitive", &ivalue) != 0) {
        printf("get ghif gyro_sensitive error!\n");
    } else {
		printf("ghif gyro_sensitive enabled = %d\n", ivalue);
        ghif.gyro_sensitive = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/gis/compass_enabled", &ivalue) != 0) {
        printf("get ghif compass_enabled error!\n");
    } else {
		printf("ghif compass_enabled enabled = %d\n", ivalue);
        ghif.compass_enabled = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/gis/ldm_enabled", &ivalue) != 0) {
        printf("get ghif ldm_enabled error!\n");
    } else {
		printf("ghif ldm_enabled enabled = %d\n", ivalue);
        ghif.ldm_enabled = ivalue;
    }
}

int GET_PSIA_GPSSystem_information(struct mg_connection* conn,
        const struct mg_request_info* request_info)
{
    GeographicInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<GPSServer version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<GpsServerEnabled>%d</GpsServerEnabled>\n"
              "<GpsSyncTime>%d</GpsSyncTime>\n"
              "<GpsWorkMode>%d</GpsWorkMode>\n"
              "</GPSServer>", ghif.gps_enabled, ghif.synctime_enabled, ghif.work_mode);
    return 1;
}

int PUT_PSIA_GPSSystem_information(struct mg_connection* conn,
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

    if (XPR_UPS_SetInteger("/camera/gis/enabled", ghif.gps_enabled) != 0)
        printf("set ghif gps_enabled error!\n");
    else
        printf("set ghif gps_enabled:%d ok!\n", ghif.gps_enabled);

    if (XPR_UPS_SetInteger("/camera/gis/synctime_enabled", ghif.synctime_enabled) != 0)
        printf("set ghif synctime_enabled error!\n");
    else
        printf("set ghif synctime_enabled:%d ok!\n", ghif.synctime_enabled);

    if (XPR_UPS_SetInteger("/camera/gis/work_mode", ghif.work_mode) != 0)
        printf("set ghif work_mode error!\n");
    else
        printf("set ghif work_mode:%d ok!\n", ghif.work_mode);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/geographicinformationsystems/GPSBeiDou</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

int GET_PSIA_Compass_information(struct mg_connection* conn,
        const struct mg_request_info* request_info)
{
    GeographicInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<Compass version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<CompassEnabled>%d</CompassEnabled>\n"
              "</Compass>", ghif.compass_enabled);
    return 1;
}

int PUT_PSIA_Compass_information(struct mg_connection* conn,
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

    if (XPR_UPS_SetInteger("/camera/gis/compass_enabled", ghif.compass_enabled) != 0)
        printf("set ghif compass enabled error!\n");
    else
        printf("set ghif compass enabled:%d ok!\n", ghif.compass_enabled);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/geographicinformationsystems/GPSBeiDou</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

int GET_PSIA_Gyro_information(struct mg_connection* conn,
        const struct mg_request_info* request_info)
{
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<Gyro version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<GyroEnabled>%d</GyroEnabled>\n"
              "<GyroSensitive>%d</GyroSensitive>\n"
              "</Gyro>", ghif.gyro_enabled, ghif.gyro_sensitive);
    return 1;
}

int PUT_PSIA_Gyro_information(struct mg_connection* conn,
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

    if (XPR_UPS_SetInteger("/camera/gis/gyro_enabled", ghif.gyro_enabled) != 0)
        printf("set ghif gyro enabled error!\n");
    else
        printf("set ghif gyro enabled:%d ok!\n", ghif.gyro_enabled);

    if (XPR_UPS_SetInteger("/camera/gis/gyro_sensitive", ghif.gyro_sensitive) != 0)
        printf("set ghif gyro sensitive error!\n");
    else
        printf("set ghif gyro sensitive:%d ok!\n", ghif.gyro_sensitive);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/gyro/information</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

int PUT_PSIA_LaserRanging_information(struct mg_connection* conn,
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

    if (XPR_UPS_SetInteger("/camera/gis/ldm_enabled", ghif.ldm_enabled) != 0)
        printf("set ghif ldm_enabled error!\n");
    else
        printf("set ghif ldm_enabled:%d ok!\n", ghif.ldm_enabled);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/geographicinformationsystems/GPSBeiDou</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

int GET_PSIA_LaserRanging_information(struct mg_connection* conn,
        const struct mg_request_info* request_info)
{
    GeographicInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<LaserRanging version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<LdmEnabled>%d</LdmEnabled>\n"
              "</LaserRanging>", ghif.ldm_enabled);
    return 1;
}

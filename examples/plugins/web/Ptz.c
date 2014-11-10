#include "Ptz.h"


static PtzInfo pzif;

static int ptzdoor;
char ptzbuffer[1024];
int Depth;

static int get_ptzparams(char* buffers, int lens)
{
    if (lens != 0) {
        switch (ptzdoor) {
        case 1:
            sprintf(pzif.protocol, "%s", buffers); 
            ptzdoor = 0;
            break;
        case 2:
            pzif.address = atoi(buffers); 
            ptzdoor = 0;
            break;
        case 3:
            pzif.focusmode = atoi(buffers); 
            ptzdoor = 0;
            break;
        case 4:
            pzif.speed = atoi(buffers); 
            ptzdoor = 0;
            break;
        case 5:
            pzif.tptz_control = atoi(buffers); 
            ptzdoor = 0;
            break;
        case 6:
            pzif.tptz_focus = atoi(buffers); 
            ptzdoor = 0;
            break;
        case 7:
            pzif.tptz_zoom = atoi(buffers); 
            ptzdoor = 0;
            break;
        case 8:
            pzif.tptz_stop = atoi(buffers); 
            ptzdoor = 0;
            break;
        case 9:
            pzif.iris_enabled = atoi(buffers); 
            ptzdoor = 0;
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
    memcpy(ptzbuffer, target, lens);
    ptzbuffer[lens] = '\0';
    get_ptzparams(ptzbuffer, lens);
}

static void XMLCALL
start(void* data, const char* el, const char** attr)
{

    if (strcmp(el, "ControlProtocol") == 0)
        ptzdoor = 1;
    if (strcmp(el, "ControlAddress") == 0)
        ptzdoor = 2;
    if (strcmp(el, "ControlFocusMode") == 0)
        ptzdoor = 3;
    if (strcmp(el, "ControlSpeed") == 0)
        ptzdoor = 4;
    if (strcmp(el, "TptzControl") == 0)
        ptzdoor = 5;
    if (strcmp(el, "TptzFocus") == 0)
        ptzdoor = 6;
    if (strcmp(el, "TptzZoom") == 0)
        ptzdoor = 7;
    if (strcmp(el, "TptzStop") == 0)
        ptzdoor = 8;
    if (strcmp(el, "IrisEnabled") == 0)
        ptzdoor = 9;
    Depth++;
}

static void XMLCALL
end(void* data, const char* el)
{
    Depth--;
}



void PtzInit(void)
{
    char value[128] = {0}; 
    int size = sizeof(value);

    if (XPR_UPS_GetString("/camera/ptz/protocol", value, &size) != 0) {
        printf("get ptz protocol error!\n");
    } else {
        printf("ptz protocol = %s\n", value);
        sprintf(pzif.protocol, "%s", value); 
    }

    int ivalue = 0;
    if (XPR_UPS_GetInteger("/camera/ptz/address", &ivalue) != 0)
        printf("get ptz address error!\n");
    else {
		printf("ptz address = %d\n", ivalue);
        pzif.address = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/ptz/speed", &ivalue) != 0)
        printf("get ptz speed error!\n");
    else {
  		printf("ptz speed = %d\n", ivalue);
        pzif.speed = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/ptz/tptz/focus_mode", &ivalue) != 0)
        printf("get ptz focusmode error!\n");
    else {
		printf("ptz focusmode = %d\n", ivalue);
        pzif.focusmode = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/ptz/tptz/control", &ivalue) != 0)
        printf("get ptz tptz_control error!\n");
    else {
  		printf("ptz tptz_control = %d\n", ivalue);
        pzif.tptz_control = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/ptz/tptz/zoom", &ivalue) != 0)
        printf("get ptz tptz_zoom error!\n");
    else {
  		printf("ptz tptz_zoom = %d\n", ivalue);
        pzif.tptz_zoom = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/ptz/tptz/focus", &ivalue) != 0)
        printf("get pzif tptz_focus error!\n");
    else {
  		printf("pzif tptz_focus = %d\n", ivalue);
        pzif.tptz_focus = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/ptz/tptz/stop", &ivalue) != 0)
        printf("get pzif tptz_stop error!\n");
    else {
  		printf("pzif tptz_stop = %d\n", ivalue);
        pzif.tptz_stop = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/ptz/iris_enabled", &ivalue) != 0)
        printf("get pzif iris_enabled error!\n");
    else {
  		printf("pzif iris_enabled = %d\n", ivalue);
        pzif.iris_enabled = ivalue;
    }
}

// GET PSIA/Custom/PTZ/channels
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_Custom_PTZ_channels(struct mg_connection* conn,
        const struct mg_request_info* request_info)
{
    PtzInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<PTZChannel version=\"1.0\" xmlns=\"http://www.hikvision.com/ver10/XMLSchema\">\n"
              "<id>1</id>\n"
              "<ControlFocusMode>%d</ControlFocusMode>\n"
              "<ControlProtocol>%s</ControlProtocol>\n"
              "<ControlAddress>%d</ControlAddress>\n"
              "<ControlSpeed>%d</ControlSpeed>\n"
              "</PTZChannel>", pzif.focusmode, pzif.protocol, pzif.address, pzif.speed);
    return 1;
}


#if 1
// PUT PSIA/Custom/PTZ/channels
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_Custom_PTZ_channels(struct mg_connection* conn,
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

    if (XPR_UPS_SetString("/camera/ptz/protocol", pzif.protocol, strlen(pzif.protocol)) != 0)
        printf("set pzif protocol error!\n");
    else
        printf("set pzif protocol:%s ok!\n", pzif.protocol);

    if (XPR_UPS_SetInteger("/camera/ptz/address", pzif.address) != 0)
        printf("set pzif address error!\n");
    else
        printf("set pzif address:%d ok!\n", pzif.address);

    if (XPR_UPS_SetInteger("/camera/ptz/speed", pzif.speed) != 0)
        printf("set pzif speed error!\n");
    else
        printf("set pzif speed:%d ok!\n", pzif.speed);

    if (XPR_UPS_SetInteger("/camera/ptz/tptz/focus_mode", pzif.focusmode) != 0)
        printf("set pzif focusmode error!\n");
    else
        printf("set pzif focusmode:%d ok!\n", pzif.focusmode);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/ptz/channels</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//control
int PUT_PSIA_PTZ_Tptz_Control(struct mg_connection* conn,
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

    if (XPR_UPS_SetInteger("/camera/ptz/tptz/control", pzif.tptz_control) != 0)
        printf("set pzif tptz control error!\n");
    else
        printf("set pzif tptz control:%d ok!\n", pzif.tptz_control);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/ptz/tptz/control</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

int PUT_PSIA_PTZ_Tptz_Focus(struct mg_connection* conn,
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

    if (XPR_UPS_SetInteger("/camera/ptz/tptz/focus", pzif.tptz_focus) != 0)
        printf("set pzif tptz focus error!\n");
    else
        printf("set pzif tptz focus:%d ok!\n", pzif.tptz_focus);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/ptz/tptz/focus</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

int PUT_PSIA_PTZ_Tptz_Zoom(struct mg_connection* conn,
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
   
    if (XPR_UPS_SetInteger("/camera/ptz/tptz/zoom", pzif.tptz_zoom) != 0)
        printf("set pzif tptz zoom error!\n");
    else
        printf("set pzif tptz zoom:%d ok!\n", pzif.tptz_zoom);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/system/video/inputs/channels/1/focus</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

int PUT_PSIA_PTZ_Tptz_Stop(struct mg_connection* conn,
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

    if (XPR_UPS_SetInteger("/camera/ptz/tptz/stop", pzif.tptz_stop) != 0)
        printf("set pzif tptz stop error!\n");
    else
        printf("set pzif tptz stop:%d ok!\n", pzif.tptz_stop);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/system/video/inputs/channels/1/focus</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

int PUT_PSIA_PTZ_Iris_Enabled(struct mg_connection* conn,
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

    if (XPR_UPS_SetInteger("/camera/ptz/iris_enabled", pzif.iris_enabled) != 0)
        printf("set pzif iris_enabled error!\n");
    else
        printf("set pzif iris_enabled:%d ok!\n", pzif.iris_enabled);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/system/video/inputs/channels/1/iris</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}
#endif

#include "Event.h"



struct EventInfo {
    int motion_enabled;
    int motion_sensitive;
    int motion_threshold;
    int cover_enabled;
    int cover_sensitive;
    char motion_scope[128];
};

static struct EventInfo eifo;
static int eventdoor = 0;
int Depth;
char buffers[1024];
static int MSIZE = 0;

static int GetMotion(char* buffer, int lens)
{
    if (lens != 0) {
        if (eventdoor == 1) {
            eifo.motion_enabled = atoi(buffers);
            eventdoor = 0;
        }
        if (eventdoor == 2) {
            sprintf(eifo.motion_scope, "%s", buffer);
            MSIZE++;
            eventdoor = 0;
        }
        if (eventdoor == 3) {
            eifo.motion_sensitive = atoi(buffers);
            eventdoor = 0;
        }
        if (eventdoor == 4) {
            eifo.motion_threshold = atoi(buffers);
            eventdoor = 0;
        }
        if (eventdoor == 5) {
            eifo.cover_enabled = atoi(buffers);
            eventdoor = 0;
        }
        if (eventdoor == 6) {
            eifo.cover_sensitive = atoi(buffers);
            eventdoor = 0;
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
    if (strcmp(el, "MontionEabled") == 0)
        eventdoor = 1;
    if (strcmp(el, "MontionScope") == 0)
        eventdoor = 2;
    if (strcmp(el, "MontionSensitive") == 0)
        eventdoor = 3;
    if (strcmp(el, "MontionThreshold") == 0)
        eventdoor = 4;
    if (strcmp(el, "CoverEnabled") == 0)
        eventdoor = 5;
    if (strcmp(el, "CoverSensitive") == 0)
        eventdoor = 6;

    Depth++;
}

static void XMLCALL
end(void* data, const char* el)
{
    Depth--;
}


void EventInit(void)
{
    char value[128] = {0}; 
    int size = sizeof(value);

    if (XPR_UPS_GetString("/camera/event/motion_detection/scope", value, &size) != 0) {
        printf("get motion detection scope error!\n");
    } else {
        printf("motion detection scope = %s\n", value);
        sprintf(eifo.motion_scope, "%s", value); 
    }

    int ivalue = 0;
    if (XPR_UPS_GetInteger("/camera/event/motion_detection/enabled", &ivalue) != 0) {
        printf("get motion detection enabled error!\n");
    } else {
		printf("motion detection enabled = %d\n", ivalue);
        eifo.motion_enabled = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/event/motion_detection/sensitive", &ivalue) != 0) {
        printf("get motion detection sensitive error!\n");
    } else {
  		printf("motion detection sensitive = %d\n", ivalue);
        eifo.motion_sensitive = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/event/motion_detection/threshold", &ivalue) != 0) {
        printf("get motion detection threshold error!\n");
    } else {
		printf("motion detection threshold = %d\n", ivalue);
        eifo.motion_threshold = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/event/cover_detection/enabled", &ivalue) != 0) {
        printf("get cover detection enabled error!\n");
    } else {
  		printf("cover detection enabled = %d\n", ivalue);
        eifo.cover_enabled = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/event/cover_detection/sensitive", &ivalue) != 0) {
        printf("get cover detection sensitive error!\n");
    } else {
  		printf("cover detection sensitive = %d\n", ivalue);
        eifo.cover_sensitive = ivalue;
    }
}
// GET PSIA/MotionDetection/information
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_MotionDetection_information(struct mg_connection* conn,
                                      const struct mg_request_info* request_info)
{
    EventInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<MotionDetection version=\"1.0\" xmlns=\"urn:selfextension:psiaext-ver10-xsd\">\n"
              "<MontionEabled>%d</MontionEabled>\n"
              "<MontionSensitive>%d</MontionSensitive>\n"
              "<MontionScope>%s</MontionScope>\n"
              "</MotionDetection>", eifo.motion_enabled, eifo.motion_sensitive, eifo.motion_scope);
    return 1;
}


//PUT PSIA/MotionDetection/information
int PUT_PSIA_MotionDetection_information(struct mg_connection* conn,
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

    if (XPR_UPS_SetString("/camera/event/motion_detection/scope", eifo.motion_scope, strlen(eifo.motion_scope)) != 0)
        printf("set motion scope error!\n");
    else
        printf("set motion scope:%s ok!\n", eifo.motion_scope);

    if (XPR_UPS_SetInteger("/camera/event/motion_detection/enabled", eifo.motion_enabled) != 0)
        printf("set motion enabled error!\n");
    else
        printf("set motion enabled:%d ok!\n", eifo.motion_enabled);

    if (XPR_UPS_SetInteger("/camera/event/motion_detection/sensitive", eifo.motion_sensitive) != 0)
        printf("set motion sensitive error!\n");
    else
        printf("set motion sensitive:%d ok!\n", eifo.motion_sensitive);

    if (XPR_UPS_SetInteger("/camera/event/motion_detection/threshold", eifo.motion_threshold) != 0)
        printf("set motion threshold error!\n");
    else
        printf("set motion threshold:%d ok!\n", eifo.motion_threshold);

    // 根据解析参数设置Set ...
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/motiondetection/information</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}



// GET PSIA/CoverDetection/information
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_CoverDetection_information(struct mg_connection* conn,
        const struct mg_request_info* request_info)
{
    EventInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<CoverServer version=\"1.0\" xmlns=\"http://www.std-cgi.com/ver10/XMLSchema\">\n"
              "<CoverEnabled>%d</CoverEnabled>\n"
              "<CoverSensitive>%d</CoverSensitive>\n"
              "</CoverServer>", eifo.cover_enabled, eifo.cover_sensitive);
    return 1;
}             

// PUT PSIA/CoverDetection/information
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_CoverDetection_information(struct mg_connection* conn,
        const struct mg_request_info* request_info,XML_Parser p)
{
    XML_SetCharacterDataHandler(p, chardatahandler);
    XML_SetElementHandler(p, start, end);
    char data[4096];
    int data_len = mg_read(conn, data, sizeof(data));
    // 解析客户端传递的xml
    if (XML_Parse(p, data, data_len, 0) == XML_STATUS_ERROR) {
        printf("xml parse failed!\n");
    }

    if (XPR_UPS_SetInteger("/camera/event/cover_detection/enabled", eifo.cover_enabled) != 0)
        printf("set cover detection enabled error!\n");
    else
        printf("set cover detection enabled:%d ok!\n", eifo.cover_enabled);

    if (XPR_UPS_SetInteger("/camera/event/cover_detection/sensitive", eifo.cover_sensitive) != 0)
        printf("set cover detection sensitive error!\n");
    else
        printf("set cover detection sensitive:%d ok!\n", eifo.cover_sensitive);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/custom/SelfExt/TamperDetection/channels/BlockAlarm</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

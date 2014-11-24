#include "SystemTime.h"
//#include <sys/time.h>
//#include <time.h>

int Depth;

struct timeconfig {
    int time_door;
    int time_type;
    char time_buffes[64];
    char zone_buffes[64];
    char ntp_addr[128];
    int ntpport;
};

static struct timeconfig tmcf;

static void XMLCALL
chardatahandler(void* uerData, const char* target, int lens)
{
    if (tmcf.time_door == 1) {
        if (lens != 0) {
            tmcf.time_type = atoi(target);;
            tmcf.time_door = 0;
        }
    }
    if (tmcf.time_door == 2) {
        if (lens != 0) {
            memcpy(tmcf.time_buffes, target, lens);
            tmcf.time_buffes[lens] = '\0';
            tmcf.time_door = 0;
        }
    }
    if (tmcf.time_door == 3) {
        if (lens != 0) {
            memcpy(tmcf.zone_buffes, target, lens);
            tmcf.zone_buffes[lens] = '\0';
            tmcf.time_door = 0;
        }
    }
    if (tmcf.time_door == 4) {
        if (lens != 0) {
            memcpy(tmcf.ntp_addr, target, lens);
            tmcf.ntp_addr[lens] = '\0';
            tmcf.time_door = 0;
        }
    }
    if (tmcf.time_door == 5) {
        if (lens != 0) {
            tmcf.ntpport = atoi(target);
            tmcf.time_door = 0;
        }
    }
}

static void XMLCALL
start(void* data, const char* el, const char** attr)
{
    int i;
    if (strcmp(el, "timeMode") == 0)
        tmcf.time_door = 1;
    if (strcmp(el, "localTime") == 0)
        tmcf.time_door = 2;
    if (strcmp(el, "timeZone") == 0)
        tmcf.time_door = 3;
    if (strcmp(el, "ntpAddress") == 0)
        tmcf.time_door = 4;
    if (strcmp(el, "ntpPortNo") == 0)
        tmcf.time_door = 5;
    Depth++;
}

static void XMLCALL
end(void* data, const char* el)
{
    Depth--;
}

void SystemTimeInit(void)
{
    char value[128] = {0}; 
    int size = sizeof(value); 
    printf("value 0:%s\n",value);
    if (XPR_UPS_GetString("/system/time/time_zone", value, &size) != 0) {
        printf("get system time zone error!\n");
    } else {
        printf("system time zone = %s\n", value);
        sprintf(tmcf.zone_buffes, "%s", value); 
    }
    printf("value 1111 size:%d\n",size);
    memset(value,0,sizeof(value));
    size = sizeof(value); 
    printf("value 2 size:%d\n",size);
    //char value_t[128] = {0}; 
    //int size_n = sizeof(value_t); 
    if (XPR_UPS_GetString("/system/time/date_time", value, &size) != 0) {
        printf("get system date time error!\n");
    } else {
        printf("system date time = %s\n", value);
        sprintf(tmcf.time_buffes, "%s", value); 
    }
    char value_s[128] = {0}; 
    int size_s = sizeof(value_s); 
    if (XPR_UPS_GetString("/system/network/ntp/server_address", value_s, &size_s) != 0) {
        printf("get system ntp address error!\n");
    } else {
        printf("system ntp address = %s\n", value_s);
        sprintf(tmcf.ntp_addr, "%s", value_s); 
    }

    int ivalue = 0;
    if (XPR_UPS_GetInteger("/system/network/ntp/enabled", &ivalue) != 0) {
        printf("get ntp enabled error!\n");
    } else {
		printf("ntp enabled = %d\n", ivalue);
        tmcf.time_type = ivalue;
    }
    int tvalue = 0;
    if (XPR_UPS_GetInteger("/system/network/ntp/server_port", &tvalue) != 0) {
        printf("get ntp port error!\n");
    } else {
		printf("ntp port = %d\n", tvalue);
        tmcf.ntpport = tvalue;
    }
}

// GET PSIA/System/time
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_System_time(struct mg_connection* conn,
                         const struct mg_request_info* request_info)
{
    SystemTimeInit();
    char settime[50];
    static struct tm* t;
    static time_t tts;
    time(&tts);
    t = localtime(&tts);
    sprintf(settime, "%4d-%02d-%02dT%02d:%02d:%02d+08:00\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<Time version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<timeMode>%d</timeMode>\n"
              "<localTime>%s</localTime>\n"
              "<timeZone>%s</timeZone>\n"
              "<ntpAddress>%s</ntpAddress>\n"
              "<ntpPortNo>%d</ntpPortNo>\n"
              "</Time>", tmcf.time_type, tmcf.time_buffes, tmcf.zone_buffes, tmcf.ntp_addr, tmcf.ntpport);
    return 1;
}

static int SetTime()
{
    char time_set[40];
    char* result = NULL;
    char* tm1 = NULL;
    char* tm2 = NULL;
    char* tm3 = NULL;
    char* tm4 = NULL;
    char* tm5 = NULL;
    char* tm6 = NULL;
    result = strtok(tmcf.time_buffes, "-");
    tm1 = result;
    result = strtok(NULL, "-");
    tm2 = result;
    result = strtok(NULL, "-");
    tm3 = result;
    result = strtok(result, "T");
    tm4 = result;
    result = strtok(result, ":");
    tm5 = result;
    result = strtok(result, ":");
    tm6 = result;
    sprintf(time_set, "date -s %s%s%s%s%s.%s", tm1, tm2, tm3, tm4, tm5, tm6);
    system(time_set);
}

static int SetZone()
{
    char* zone = NULL;
    zone = strstr(tmcf.zone_buffes, "+");
    if (zone) {
        zone = strtok(zone, ":");
    } else {
        zone = strstr(tmcf.zone_buffes, "-");
        if (zone) {
            zone = strtok(zone, ":");
        }
    }
    char cmdline[1024];
    snprintf(cmdline, sizeof(cmdline), "echo UTC\"%s\":00 > /ambarella/TZ", zone);
    system(cmdline);
}

static int ntpdoors = 0;

// PUT PSIA/System/time
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_System_time(struct mg_connection* conn,
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
    if (XPR_UPS_SetString("/system/network/ntp/server_address", tmcf.ntp_addr, strlen(tmcf.ntp_addr)) != 0)
        printf("set ntp server address error!\n");
    else
        printf("set ntp server address:%s ok!\n", tmcf.ntp_addr);

    if (XPR_UPS_SetString("/system/time/date_time", tmcf.time_buffes, strlen(tmcf.time_buffes)) != 0)
        printf("set time date error!\n");
    else
        printf("set time date:%s ok!\n", tmcf.time_buffes);

    if (XPR_UPS_SetString("/system/time/time_zone", tmcf.zone_buffes, strlen(tmcf.zone_buffes)) != 0)
        printf("set time zone error!\n");
    else
        printf("set time zone:%s ok!\n", tmcf.zone_buffes);

    if (XPR_UPS_SetInteger("/system/network/ntp/enabled", tmcf.time_type) != 0)
        printf("set 0 ntp enabled error!\n");
    else
        printf("set 0 ntp enabled:%d ok!\n", tmcf.time_type);

    if (XPR_UPS_SetInteger("/system/network/ntp/server_port", tmcf.ntpport) != 0)
        printf("set 0 ntp port error!\n");
    else
        printf("set 0 ntp port:%d ok!\n", tmcf.ntpport);

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<Time version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/system/time</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</Time>");
    return 1;
}
#if 0
// GET PSIA/System/time/ntpServers/1
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_System_time_ntpServers_1(struct mg_connection* conn,
                                      const struct mg_request_info* request_info)
{
    static NetWorkConfigTable NetWorkConfig;
    if (dbc_get_network_config(dbc, &NetWorkConfig) < 0) {
        //MF_log(0, MF_LL_ERROR, "dbc_get_network_config failed!\n");
        return -1;
    }
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<NTPServer version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<id>1</id>\n"
              "<addressingFormatType>hostname</addressingFormatType>\n"
              "<hostName>%s</hostName>\n"
              "<portNo>%d</portNo>\n"
              /*"<Extensions>\n"
              "<synchronizeInterval>1440</synchronizeInterval>\n"
              "</Extensions>\n"*/
              "</NTPServer>",NetWorkConfig.NtpServer,NetWorkConfig.NtpPort);
    return 1;
}

// PUT PSIA/System/time/ntpServers/1
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_System_time_ntpServers_1(struct mg_connection* conn,
                                      const struct mg_request_info* request_info, XML_Parser p)
{
    static NetWorkConfigTable NetWorkConfig;
    if (dbc_get_network_config(dbc, &NetWorkConfig) < 0) {
        //MF_log(0, MF_LL_ERROR, "dbc_get_network_config failed!\n");
        return -1;
    }
    XML_SetCharacterDataHandler(p, chardatahandler);
    XML_SetElementHandler(p, start, end);
    // 解析客户端传递的xml
    char data[4096];
    int data_len = mg_read(conn, data, sizeof(data));
    if (XML_Parse(p, data, data_len, 0) == XML_STATUS_ERROR) {
        printf("xml -parse failed!---------\n");
    }
    // 根据解析参数设置Set ...
    if (strcmp(tmcf.time_type, "NTP") == 0) {
        //set ntp addr
        NetWorkConfig.NtpPort = tmcf.ntpport;
        sprintf(NetWorkConfig.NtpServer, "%s", tmcf.ntp_addr);
        if (dbc_set_network_config(dbc, &NetWorkConfig) < 0) {
            return -1;
        }
        char xmls[1024];
        sprintf(xmls, "/usr/bin/ntptest -n 3 -t 500 %s", tmcf.ntp_addr);
        system(xmls);
        memset(xmls, 0, sizeof(char) * 1024);
        sprintf(xmls, "xmlc -f /ambarella/network.xml -s "" -r //network/ -K ntp/@enable -V 1");
        system(xmls);
        memset(xmls, 0, sizeof(char) * 1024);
        sprintf(xmls, "xmlc -f /ambarella/network.xml -s "" -r //network/ -K ntp/server_port -V %d", tmcf.ntpport);
        system(xmls);
        memset(xmls, 0, sizeof(char) * 1024);
        sprintf(xmls, "xmlc -f /ambarella/network.xml -s "" -r //network/ -K ntp/server_addr -V %s", tmcf.ntp_addr);
        system(xmls);
        memset(xmls, 0, sizeof(char) * 1024);
        sprintf(xmls, "/usr/scripts/ntp_ctl.sh --restart & killall commsvr");
        system(xmls);
    }
    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<Time version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/system/time/ntpservers/1</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</Time>");
    return 1;
}
#endif

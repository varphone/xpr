#include "Serial.h"

static SerialConfig slcf[2];
int Depth;
char buffers[1024];
static int serial_door;

static int id = -1;

static int GetSerialConfig(char* buffers, int lens)
{
    if (lens != 0) {
        switch (serial_door) {
        case 1:
            if (strcmp(buffers, "RS232") == 0)
                id = 0;
            if (strcmp(buffers, "RS485") == 0)
                id = 1;
            serial_door = 0;
            break;
        case 2:
            slcf[id].baudrate = atoi(buffers);
            serial_door = 0;
            break;
        case 3:
            slcf[id].databits = atoi(buffers);
            serial_door = 0;
            break;
        case 4:
            slcf[id].stopbits = atoi(buffers);
            serial_door = 0;
            break;
        case 5:
            slcf[id].paritycheck = atoi(buffers);
            serial_door = 0;
            break;
        case 6:
            slcf[id].flowcontrol = atoi(buffers);
            serial_door = 0;
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
    GetSerialConfig(buffers, lens);
}


static void XMLCALL
start(void* data, const char* el, const char** attr)
{
    if (strcmp(el, "SerialPortType") == 0)
        serial_door = 1;
    if (strcmp(el, "BaudRate") == 0)
        serial_door = 2;
    if (strcmp(el, "DataBits") == 0)
        serial_door = 3;
    if (strcmp(el, "StopBits") == 0)
        serial_door = 4;
    if (strcmp(el, "ParityCheckType") == 0)
        serial_door = 5;
    if (strcmp(el, "FlowCtrl") == 0)
        serial_door = 6;
    Depth++;
}

static void XMLCALL
end(void* data, const char* el)
{
    Depth--;
}

void SystemSerialInit(void)
{
    int ivalue = 0;
    if (XPR_UPS_GetInteger("/camera/rs485/baud_rate", &ivalue) != 0) {
        printf("get rs485 baud_rate error!\n");
    } else {
		printf("rs485 baud_rate = %d\n", ivalue);
        slcf[1].baudrate = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/rs485/data_bits", &ivalue) != 0) {
        printf("get rs485 data_bits error!\n");
    } else {
  		printf("rs485 data_bits = %d\n", ivalue);
        slcf[1].databits = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/rs485/stop_bits", &ivalue) != 0) {
        printf("get rs485 stop_bits error!\n");
    } else {
		printf("rs485 stop_bits = %d\n", ivalue);
        slcf[1].stopbits = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/rs485/parity_check", &ivalue) != 0) {
        printf("get rs485 parity_check error!\n");
    } else {
  		printf("rs485 parity_check = %d\n", ivalue);
        slcf[1].paritycheck = ivalue;
    }

    if (XPR_UPS_GetInteger("/camera/rs485/flow_control", &ivalue) != 0) {
        printf("get rs485 flow_control error!\n");
    } else {
  		printf("rs485 flow_control = %d\n", ivalue);
        slcf[1].flowcontrol = ivalue;
    }
}

// GET PSIA/System/Serial/ports/3
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_System_Serial_ports_3(struct mg_connection* conn,
                                   const struct mg_request_info* request_info)
{
    SystemSerialInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<SerialPort version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<id>3</id>\n"
              "<enabled>true</enabled>\n"
              "<serialPortType>RS232</serialPortType>\n"
              "<duplexMode>half</duplexMode>\n"
              "<baudRate>%d</baudRate>\n"
              "<dataBits>%d</dataBits>\n"
              "<parityType>%d</parityType>\n"
              "<stopBits>%d</stopBits>\n"
              "<flowCtrl>%d</flowCtrl>\n"
              "<workMode>console</workMode>\n"
              "</SerialPort>", slcf[0].baudrate, slcf[0].databits, slcf[0].stopbits, slcf[0].paritycheck, slcf[0].flowcontrol);
    return 1;
}


//#endif



// GET PSIA/System/Serial/ports/1
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_System_Serial_ports_1(struct mg_connection* conn,
                                   const struct mg_request_info* request_info)
{
    SystemSerialInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<SerialPort version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<id>1</id>\n"
              "<SerialPortType>RS485</SerialPortType>\n"
              "<BaudRate>%d</BaudRate>\n"
              "<DataBits>%d</DataBits>\n"
              "<StopBits>%d</StopBits>\n"
              "<ParityCheckType>%d</ParityCheckType>\n"
              "<FlowCtrl>%d</FlowCtrl>\n"
              "</SerialPort>", slcf[1].baudrate, slcf[1].databits, slcf[1].stopbits, slcf[1].paritycheck, slcf[1].flowcontrol);
    return 1;
}

// PUT PSIA/System/Serial/ports/1
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_System_Serial_ports_1(struct mg_connection* conn,
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
    if (XPR_UPS_SetInteger("/camera/rs485/baud_rate", slcf[1].baudrate) != 0)
        printf("set rs485 baudrate error!\n");
    else
        printf("set rs485 baudrate:%d ok!\n", slcf[1].baudrate);

    if (XPR_UPS_SetInteger("/camera/rs485/data_bits", slcf[1].databits) != 0)
        printf("set rs485 databits error!\n");
    else
        printf("set rs485 databits:%d ok!\n", slcf[1].databits);

    if (XPR_UPS_SetInteger("/camera/rs485/stop_bits", slcf[1].stopbits) != 0)
        printf("set rs485 stopbits error!\n");
    else
        printf("set rs485 stopbits:%d ok!\n", slcf[1].stopbits);

    if (XPR_UPS_SetInteger("/camera/rs485/parity_check", slcf[1].paritycheck) != 0)
        printf("set rs485 paritycheck error!\n");
    else
        printf("set rs485 paritycheck:%d ok!\n", slcf[1].paritycheck);

    if (XPR_UPS_SetInteger("/camera/rs485/flow_control", slcf[1].flowcontrol) != 0)
        printf("set rs485 flowcontrol error!\n");
    else
        printf("set rs485 flowcontrol:%d ok!\n", slcf[1].flowcontrol);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/system/serial/ports/1</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

//#endif


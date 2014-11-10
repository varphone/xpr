#ifndef _WEB_SERIAL_H_
#define _WEB_SERIAL_H_

#include "Config.h"

typedef struct SerialConfig {
    int baudrate;
    int databits;
    int stopbits;
    int paritycheck;
    int flowcontrol;
} SerialConfig;

void SystemSerialInit(void);

int GET_PSIA_System_Serial_ports(struct mg_connection* conn,
                                 const struct mg_request_info* request_info);

int GET_PSIA_System_Serial_ports_3(struct mg_connection* conn,
                                   const struct mg_request_info* request_info);

int GET_PSIA_System_Serial_ports_1(struct mg_connection* conn,
                                   const struct mg_request_info* request_info);

int PUT_PSIA_System_Serial_ports_1(struct mg_connection* conn,
                                   const struct mg_request_info* request_info, XML_Parser p);


#endif /* _WEB_SERIAL_H_ */

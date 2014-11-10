#ifndef _WEB_TIME_H_
#define _WEB_TIME_H_

#include "Config.h"

void SystemTimeInit(void);

int GET_PSIA_System_time(struct mg_connection* conn,
                         const struct mg_request_info* request_info);

int PUT_PSIA_System_time(struct mg_connection* conn,
                         const struct mg_request_info* request_info, XML_Parser p);

int GET_PSIA_System_time_ntpServers_1(struct mg_connection* conn,
                                      const struct mg_request_info* request_info);

int PUT_PSIA_System_time_ntpServers_1(struct mg_connection* conn,
                                      const struct mg_request_info* request_info, XML_Parser p);

#endif /* _WEB_TIME_H_ */

#ifndef _WEB_EVENT_H_
#define _WEB_EVENT_H_

#include "Config.h"

void EventInit(void);

int GET_PSIA_MotionDetection_information(struct mg_connection* conn,
                                      const struct mg_request_info* request_info);

int PUT_PSIA_MotionDetection_information(struct mg_connection* conn,
                                      const struct mg_request_info* request_info, XML_Parser p);


int GET_PSIA_CoverDetection_information(struct mg_connection* conn,
        const struct mg_request_info* request_info);

int PUT_PSIA_CoverDetection_information(struct mg_connection* conn,
                                      const struct mg_request_info* request_info, XML_Parser p);
#endif /* _WEB_EVENT_H_ */

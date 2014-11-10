#ifndef _WEB_GEOGRAPHIC_H_
#define _WEB_GEOGRAPHIC_H_

#include "Config.h"

void GeographicInit(void);

int GET_PSIA_GPSSystem_information(struct mg_connection* conn,
        const struct mg_request_info* request_info);

int PUT_PSIA_GPSSystem_information(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int GET_PSIA_Compass_information(struct mg_connection* conn,
        const struct mg_request_info* request_info);

int PUT_PSIA_Compass_information(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int GET_PSIA_Gyro_information(struct mg_connection* conn,
        const struct mg_request_info* request_info);

int PUT_PSIA_Gyro_information(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);

int GET_PSIA_LaserRanging_information(struct mg_connection* conn,
        const struct mg_request_info* request_info);

int PUT_PSIA_LaserRanging_information(struct mg_connection* conn,
        const struct mg_request_info* request_info, XML_Parser p);
#endif /* _WEB_GEOGRAPHIC_H_ */

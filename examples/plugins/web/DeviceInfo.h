#ifndef _WEB_DEVICE_INFO_H_
#define _WEB_DEVICE_INFO_H_

#include "Config.h"

typedef struct DeviceInfo {
    char deviceName[64];
    char hardware[64]; //deviceID
    char location[64];
    char serialNumber[64];
    char manufacture[64]; //systemContact
    char firmwareVersion[64];
    char model[64];
    char macAddress[64];
} DeviceInfos;

void DeviceInfoInit(void);

int GET_PSIA_System_deviceInfo(struct mg_connection* conn,
                               const struct mg_request_info* request_info);

int PUT_PSIA_System_deviceInfo(struct mg_connection* conn,
                               const struct mg_request_info* request_info, XML_Parser p);


#endif /* _WEB_DEVICE_INFO_H_ */

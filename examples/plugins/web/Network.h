#ifndef _WEB_NETWORK_H_
#define _WEB_NETWORK_H_

#include "Config.h"

typedef struct NetworkInfo {
    int  id;
    int  mtu;
    int  door;
    int  status;
    int  httpport;
    int  httpsport;
    int  rtspport;
    int  httpenabled;
    int  httpsenabled;
    int  rtspenabled;
    char type[32];
    char ipv4[32];
    char netmask[32];
    char gateway[32];
    char primarydns[32];
    char secondarydns[32];
    char maskaddress[32];
} NetworkInfo;

void NetWorkInit(void);

int GET_PSIA_System_Network_interfaces(struct mg_connection* conn,
                                       const struct mg_request_info* request_info);

int PUT_PSIA_System_Network_interfaces(struct mg_connection* conn,
                                       const struct mg_request_info* request_info, XML_Parser p);

int GET_PSIA_Custom_SelfExt_PPPoE(struct mg_connection* conn,
                                  const struct mg_request_info* request_info);

int GET_PSIA_Custom_SelfExt_DDNS_capabilities(struct mg_connection* conn,
        const struct mg_request_info* request_info);

int GET_PSIA_Custom_SelfExt_DDNS_1(struct mg_connection* conn,
                                   const struct mg_request_info* request_info);

int GET_PSIA_System_Network_interfaces_1_ipFilter(struct mg_connection* conn,
        const struct mg_request_info* request_info);

#endif /* _WEB_NETWORK_H_ */

#include "Network.h"

int Depth;
char buffers[1024];
static NetworkInfo network;

void NetWorkInit(void)
{
    char value[128] = {0}; 
	int size = sizeof(value);

    if (XPR_UPS_GetString("/system/network/eth0/ipv4/address", value, &size) != 0) {
        printf("get net address error!\n");
    } else {
        printf("address = %s\n", value);
        sprintf(network.ipv4, "%s", value); //ipv4
    }

    size = sizeof(value);
    if (XPR_UPS_GetString("/system/network/eth0/ipv4/netmask", value, &size) != 0) {
        printf("get net netmask error!\n");
    } else {
        printf("netmask = %s\n", value);
        sprintf(network.netmask, "%s", value); //netmask
    }

    size = sizeof(value);
    if (XPR_UPS_GetString("/system/network/eth0/ipv4/gateway", value, &size) != 0) {
        printf("get net gateway error!\n");
    } else {
        printf("gateway = %s\n", value);
        sprintf(network.gateway, "%s", value); //gateway
    }

    size = sizeof(value);
    if (XPR_UPS_GetString("/system/network/eth0/ipv4/dns1", value, &size) != 0) {
        printf("get net dns1 error!\n");
    } else {
        printf("dns1 = %s\n", value);
        sprintf(network.primarydns, "%s", value); //dns1
    }

    size = sizeof(value);
    if (XPR_UPS_GetString("/system/network/eth0/mac", value, &size) != 0) {
        printf("get net mac error!\n");
    } else {
        printf("mac = %s\n", value);
        sprintf(network.maskaddress, "%s", value); //mac
    }

    int dhcp =0;
    if (XPR_UPS_GetBoolean("/system/network/eth0/ipv4/dhcp", &dhcp) != 0) {
        printf("get net dhcp error!\n");
    } else {
        printf("dhcp = %d\n", dhcp);
        if (dhcp)
            sprintf(network.type, "dynamic");
        else
            sprintf(network.type, "static");
    }

    int ivalue = 0;
    if (XPR_UPS_GetInteger("/system/network/http_port", &ivalue) != 0)
        printf("get http_port error!\n");
    else
        network.httpport = ivalue;

    if(XPR_UPS_GetInteger("/system/network/https_port", &ivalue) != 0)
        printf("get https_port error!\n");
    else
        network.httpsport = ivalue;

    if (XPR_UPS_GetInteger("/system/network/rtsp_port", &ivalue) != 0)
        printf("get rtsp_port error!\n");
    else
        network.rtspport = ivalue;

    if (XPR_UPS_GetInteger("/system/network/http_enabled", &ivalue) != 0)
        printf("get http_enabled error!\n");
    else
        network.httpenabled = ivalue;

    if (XPR_UPS_GetInteger("/system/network/https_enabled", &ivalue) != 0)
        printf("get https_enabled error!\n");
    else
        network.httpsenabled = ivalue;

    if (XPR_UPS_GetInteger("/system/network/rtsp_enabled", &ivalue) != 0)
        printf("get rtsp_enabled error!\n");
    else
        network.rtspenabled = ivalue;
}

static int GetNetwork(char* buffers, int lens)
{
    if (network.door == 1) {
        if (lens != 0) {
            sprintf(network.ipv4, "%s", buffers);
            network.door = 0;
        }
    }
    else if (network.door == 2) {
        if (lens != 0) {
            sprintf(network.type, "%s", buffers);
            network.door = 0;
        }
    }
    else if (network.door == 3) {
        if (lens != 0) {
            sprintf(network.netmask, "%s", buffers);
            network.door = 0;
        }
    }
    else if (network.door == 4) {
        if (lens != 0) {
            sprintf(network.gateway, "%s", buffers);
            network.door = 0;
        }
    }
    else if (network.door == 5) {
        if (lens != 0) {
            sprintf(network.primarydns, "%s", buffers);
            network.door = 0;
        }
    }
    else if (network.door == 6) {
        if (lens != 0) {
            sprintf(network.secondarydns, "%s", buffers);
            network.door = 0;
        }
    }
    else if (network.door == 7) {
        if (lens != 0) {
            sprintf(network.maskaddress, "%s", buffers);
            network.door = 0;
        }
    }
    else if (network.door == 8) {
        if (lens != 0) {
            network.mtu = atoi(buffers);
            network.door = 0;
        }
    }
    else if (network.door == 9) {
        if (lens != 0) {
            network.httpport = atoi(buffers);
            network.door = 0;
        }
    }
    else if (network.door == 10) {
        if (lens != 0) {
            network.httpsport = atoi(buffers);
            network.door = 0;
        }
    }
    else if (network.door == 11) {
        if (lens != 0) {
            network.rtspport = atoi(buffers);
            network.door = 0;
        }
    }
}

static void XMLCALL chardatahandler(void* uerData, const char* target, int lens)
{
    memcpy(buffers, target, lens);
    buffers[lens] = '\0';
    GetNetwork(buffers, lens);
}

static void XMLCALL start(void* data, const char* el, const char** attr)
{
    if (strcmp(el, "IPAddress") == 0) {
        network.status = 4;
    }
    if ((strcmp(el, "ipAddress") == 0) && (network.status == 4))  {
        network.door = 1;
        network.status = 0;
    }
    if (strcmp(el, "addressingType") == 0) {
        network.door = 2;
    }
    if (strcmp(el, "subnetMask") == 0)
        network.door = 3;
    if (strcmp(el, "DefaultGateway") == 0) {
        network.door = 4;
    }
    if (strcmp(el, "PrimaryDNS") == 0) { 
        network.door = 5;
    }
    if (strcmp(el, "SecondaryDNS") == 0) {
        network.status = 3;
    }
    if ((strcmp(el, "ipAddress") == 0) && (network.status == 3)) {
        network.door = 6;
        network.status = 0;
    }
    if (strcmp(el, "MACAddress") == 0)
        network.door = 7;
    if (strcmp(el, "MTU") == 0)
        network.door = 8;
    if (strcmp(el, "httpPort") == 0)
        network.door = 9;
    if (strcmp(el, "httpsPort") == 0)
        network.door = 10;
    if (strcmp(el, "rtspPort") == 0)
        network.door = 11;

    Depth++;
}

static void XMLCALL end(void* data, const char* el)
{
    Depth--;
}


// GET PSIA/System/Network/interfaces
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_System_Network_interfaces(struct mg_connection* conn,
                                       const struct mg_request_info* request_info)
{
    NetWorkInit();
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<NetworkInterfaceList version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<NetworkInterface version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<id>1</id>\n"
              "<IPAddress version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<ipVersion>v4</ipVersion>\n"
              "<addressingType>%s</addressingType>\n"
              "<ipAddress>%s</ipAddress>\n"
              "<subnetMask>%s</subnetMask>\n"
              "<DefaultGateway>\n"
              "<ipAddress>%s</ipAddress>\n"
              "</DefaultGateway>\n"
              "<PrimaryDNS>\n"
              "<ipAddress>%s</ipAddress>\n"
              "</PrimaryDNS>\n"
              /*"<SecondaryDNS>\n"
              "<ipAddress>0.0.0.0</ipAddress>\n"
              "</SecondaryDNS>\n"*/
              "<MACAddress>\n"
              "<ipAddress>%s</ipAddress>\n"
              "</MACAddress>\n"
              "<Http>\n"
              "<port>%d</port>\n"
              "</Http>\n"
              "<Rtsp>\n"
              "<port>%d</port>\n"
              "</Rtsp>\n"
              "<Https>\n"
              "<port>%d</port>\n"
              "</Https>\n"
              "</IPAddress>\n"
              /*"<Discovery version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<UPnP>\n"
              "<enabled>true</enabled>\n"
              "</UPnP>\n"
              "<Zeroconf>\n"
              "<enabled>true</enabled>\n"
              "</Zeroconf>\n"
              "</Discovery>\n"
              "<Extensions version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<Link version=\"1.0\" xmlns=\"http://www.std-cgi.com/ver10/XMLSchema\">\n"
              //"<MACAddress>%s</MACAddress>\n"
              "<autoNegotiation>true</autoNegotiation>\n"
              "<speed>10</speed>\n"
              "<duplex>Full</duplex>\n"
              "<MTU>1500</MTU>\n"
              "</Link>\n"
              "</Extensions>\n"*/
              "</NetworkInterface>\n"
              "</NetworkInterfaceList>", network.type, network.ipv4, network.netmask, network.gateway, network.primarydns, network.maskaddress, network.httpport, network.rtspport, network.httpsport);
    return 1;
}

// PUT PSIA/System/Network/interfaces/1
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_System_Network_interfaces(struct mg_connection* conn,
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
    // 根据解析参数设置Set
    if (XPR_UPS_SetString("/system/network/eth0/ipv4/address", network.ipv4, strlen(network.ipv4)) != 0)
        printf("set net address error!\n");
    else
        printf("set net address ok!\n");

    if (XPR_UPS_SetString("/system/network/eth0/ipv4/netmask", network.netmask, strlen(network.netmask)) != 0)
        printf("set net netmask error!\n");
    else
        printf("set net netmask ok!\n");
    
    if (XPR_UPS_SetString("/system/network/eth0/ipv4/gateway", network.gateway, strlen(network.gateway)) != 0)
        printf("set net netmask error!\n");
    else
        printf("set net netmask ok!\n");

    int dhcp = 1;
    if (strcmp(network.type, "static") == 0) {

} else {
  /*      dhcp = 0;
    else 
        dhcp = 1;
	*/
    if(XPR_UPS_SetBoolean("/system/network/eth0/ipv4/dhcp", dhcp) != 0)
        printf("set net dhcp error!\n");
    else
        printf("set dhcp ok!\n");
}

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/system/network/interfaces/1</requestURL>\n"
              "<statusCode>7</statusCode>\n"
              "<statusString>Reboot Required</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

// PUT PSIA/System/Network/ports
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_System_Network_ports(struct mg_connection* conn,
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
    // 根据解析参数设置

    if (XPR_UPS_SetInteger("/system/network/http_port", network.httpport) != 0)
        printf("set net http port error!\n");
    else
        printf("set net http port ok!\n");

    if (XPR_UPS_SetInteger("/system/network/https_port", network.httpsport) != 0)
        printf("set net https port error!\n");
    else
        printf("set net https port ok!\n");
    
    if (XPR_UPS_SetInteger("/system/network/rtsp_port", network.rtspport) != 0)
        printf("set net rtsp port error!\n");
    else
        printf("set net rtsp port ok!\n");

    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/system/network/interfaces/1</requestURL>\n"
              "<statusCode>7</statusCode>\n"
              "<statusString>Reboot Required</statusString>\n"
              "</ResponseStatus>");
    return 1;
}


// GET PSIA/Custom/SelfExt/PPPoE  目前未用到，保留此接口
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_Custom_SelfExt_PPPoE(struct mg_connection* conn,
                                  const struct mg_request_info* request_info)
{
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<PPPoEList version=\"1.0\" xmlns=\"http://www.hikvision.com/ver10/XMLSchema\">\n"
              "<PPPoE version=\"1.0\" xmlns=\"http://www.hikvision.com/ver10/XMLSchema\">\n"
              "<id>1</id>\n"
              "<enabled>false</enabled>\n"
              "<userName></userName>\n"
              "<password></password>\n"
              "</PPPoE>\n"
              "</PPPoEList>");
    return 1;
}

// GET PSIA/Custom/SelfExt/DDNS/capabilities
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_Custom_SelfExt_DDNS_capabilities(struct mg_connection* conn,
        const struct mg_request_info* request_info)
{
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<DDNSList version=\"1.0\" xmlns=\"http://www.std-cgi.com/ver10/XMLSchema\">\n"
              "<DDNS version=\"1.0\" xmlns=\"http://www.std-cgi.com/ver10/XMLSchema\">\n"
              "<id>1</id>\n"
              "<enabled opt=\"true,false\">false</enabled>\n"
              "<provider opt=\"IPServer,DynDNS,HiDDNS\">HiDDNS</provider>\n"
              "<serverAddress>\n"
              "<addressingFormatType opt=\"ipaddress,hostname\">ipaddress</addressingFormatType>\n"
              "<ipAddress></ipAddress>\n"
              "</serverAddress>\n"
              "<portNo min=\"0\" max=\"65535\">0</portNo>\n"
              "<deviceDomainName></deviceDomainName>\n"
              "<userName></userName>\n"
              "</DDNS>\n"
              "</DDNSList>");
    return 1;
}

// GET PSIA/Custom/SelfExt/DDNS/1
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_Custom_SelfExt_DDNS_1(struct mg_connection* conn,
                                   const struct mg_request_info* request_info)
{
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<DDNS version=\"1.0\" xmlns=\"http://www.std-cgi.com/ver10/XMLSchema\">\n"
              "<id>1</id>\n"
              "<enabled>false</enabled>\n"
              "<provider>HiDDNS</provider>\n"
              "<serverAddress>\n"
              "<addressingFormatType>hostname</addressingFormatType>\n"
              "<hostName>www.hiddns.com</hostName>\n"
              "</serverAddress>\n"
              "<portNo>0</portNo>\n"
              "<deviceDomainName></deviceDomainName>\n"
              "<userName></userName>\n"
              "</DDNS>");
    return 1;
}

// GET PSIA/System/Network/interfaces/1/ipFilter
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_System_Network_interfaces_1_ipFilter(struct mg_connection* conn,
        const struct mg_request_info* request_info)
{
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<IPFilter version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<enabled>false</enabled>\n"
              "<permissionType>deny</permissionType>\n"
              "<IPFilterAddressList> </IPFilterAddressList>\n"
              "</IPFilter>");
    return 1;
}

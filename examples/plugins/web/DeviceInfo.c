#include "DeviceInfo.h"

int door = 0, Depth = 0;
char system_name[100];
static DeviceInfos deviceInfo;

static void XMLCALL
chardatahandler(void* uerData, const char* target, int lens)
{
    if (door) {
        if (lens != 0) {
            memcpy(system_name, target, lens);
            system_name[lens] = '\0';
            door = 0;
        }
    }
}

static void XMLCALL
start(void* data, const char* el, const char** attr)
{
    if (strcmp(el, "deviceName") == 0)
        door = 1;
    Depth++;
}

static void XMLCALL
end(void* data, const char* el)
{
    Depth--;
}

void DeviceInfoInit(void)
{
    char value[128] = {0}; 
	int size = sizeof(value);

    if (XPR_UPS_GetString("/system/information/name", value, &size) != 0) {
        printf("get info name error!\n");
    } else {
        printf("name = %s\n", value);
        sprintf(deviceInfo.deviceName, "%s", value); //摄像机名称
    }
    size = sizeof(value);
    if (XPR_UPS_GetString("/system/information/location", value, &size) != 0) {
        printf("get info location error!\n");
    } else {
        printf("location = %s\n", value);
		sprintf(deviceInfo.location, "%s", value);
    }

    size = sizeof(value);
    if (XPR_UPS_GetString("/system/information/model", value, &size) != 0) {
        printf("get info model error!\n");
    } else {
        printf("model = %s\n", value);
		sprintf(deviceInfo.model, "%s", value);
    }

    size = sizeof(value);
    if (XPR_UPS_GetString("/system/information/manufacturer", value, &size) != 0) {
        printf("get info manufacturer error!\n");
    } else {
        printf("manufacturer = %s\n", value);
		sprintf(deviceInfo.manufacture, "%s", value);
    }

    size = sizeof(value);
    if (XPR_UPS_GetString("/system/information/hardware", value, &size) != 0) {
        printf("get info hardware error!\n");
    } else {
        printf("hardware = %s\n", value);
		sprintf(deviceInfo.hardware, "%s", value);
    }

    size = sizeof(value);
    if (XPR_UPS_GetString("/system/information/serial_number", value, &size) != 0) {
        printf("get info serialNumber error!\n");
    } else {
        printf("serialNumber = %s\n", value);
		sprintf(deviceInfo.serialNumber, "%s", value);
    }

    size = sizeof(value);
    if (XPR_UPS_GetString("/system/network/eth0/mac", value, &size) != 0) {
        printf("get info macAddress error!\n");
    } else {
        printf("macAddress = %s\n", value);
		sprintf(deviceInfo.macAddress, "%s", value);
    }

    size = sizeof(value);
    if (XPR_UPS_GetString("/system/information/firmware", value, &size) != 0) {
        printf("get info firmwareVersion error!\n");
    } else {
        printf("firmwareVersion = %s\n", value);
		sprintf(deviceInfo.firmwareVersion, "%s", value);
    }
}

// GET PSIA/System/deviceInfo
////////////////////////////////////////////////////////////////////////////////
int GET_PSIA_System_deviceInfo(struct mg_connection* conn,
                               const struct mg_request_info* request_info)
{	
    DeviceInfoInit();
    printf("get system device info\n");
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<DeviceInfo version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<deviceName>%s</deviceName>\n" //CameraBase.Name
              "<deviceID>%s</deviceID>\n" //硬件编号
              "<deviceDescription>IPCamera</deviceDescription>\n"//IPCamera 枪机 IPDome 球机
              "<deviceLocation>%s</deviceLocation>\n" //CameraBase.Location
              "<systemContact>%s</systemContact>\n" //55 China
              "<model>%s</model>\n" //设备型号
              "<serialNumber>%s</serialNumber>\n" //CameraBase.SerialNumber
              "<macAddress>%s</macAddress>\n" //NetWorkConfig.HwAddress
              "<firmwareVersion>%s</firmwareVersion>\n" //软件版本
              "</DeviceInfo>", deviceInfo.deviceName, deviceInfo.hardware, deviceInfo.location, deviceInfo.manufacture, deviceInfo.model, deviceInfo.serialNumber, 				       deviceInfo.macAddress, deviceInfo.firmwareVersion);
    return 1;
}

// PUT PSIA/System/deviceInfo
////////////////////////////////////////////////////////////////////////////////
int PUT_PSIA_System_deviceInfo(struct mg_connection* conn,
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
    printf("system_name :%s\n",system_name);

    if(XPR_UPS_SetString("/system/information/name", system_name, strlen(system_name)) != 0)
        printf("set info name error!\n");
    else
        printf("set info name ok!\n");
    // 根据解析参数设置Set ...
 /*   CameraBaseTable CameraBase;
    if (dbc_get_camera_base(dbc, &CameraBase) < 0) {
        //        MF_log(0, MF_LL_ERROR, "dbc_get_camera_base failed!\n");
    }
    sprintf(CameraBase.Name, "%s", system_name);
    sprintf(deviceInfo.deviceName, "%s", system_name);
    dbc_set_camera_base(dbc, &CameraBase);
    char xmls[100];
    sprintf(xmls, "xmlc -f /ambarella/ssdp.xml -s "" -r //system/basic/ -K name -V %s", system_name);
    system(xmls);
    xmls[100] = 0;
    sprintf(xmls, "xmlc -f /ambarella/system.xml -s " " -r //system/basic/ -K name -V %s", system_name);
    system(xmls);
    system("killall keeper");*/
    //设置完成后返回消息给客户端
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/xml\r\n\r\n");
    mg_printf(conn,
              "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<ResponseStatus version=\"1.0\" xmlns=\"urn:psialliance-org\">\n"
              "<requestURL>/psia/system/deviceinfo</requestURL>\n"
              "<statusCode>1</statusCode>\n"
              "<statusString>OK</statusString>\n"
              "</ResponseStatus>");
    return 1;
}

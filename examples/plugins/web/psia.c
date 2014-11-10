/*#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>
#include <expat.h>

#include "mongoose.h"
*/

#include "Config.h"
#include "DeviceInfo.h"
#include "Network.h"
#include "SystemTime.h"
#include "Stream.h"
#include "Image.h"
#include "Osd.h"
#include "Serial.h"
#include "Ptz.h"
#include "Geographic.h"
#include "Event.h"
#include "Ptz.h"
#include "SystemAudio.h"


int handle_psia_request(struct mg_connection* conn,
                        const struct mg_request_info* request_info, XML_Parser p)
{
    //XML_Parser p = XML_ParserCreate(NULL);
    printf("uri: %s\n", request_info->uri);
    if (strcmp(request_info->request_method, "GET") == 0) {

        if (strcmp(request_info->uri, "/PSIA/System/deviceInfo") == 0)
            return GET_PSIA_System_deviceInfo(conn, request_info);
        if (strcmp(request_info->uri, "/PSIA/System/time") == 0)
            return GET_PSIA_System_time(conn, request_info);


        if (strcmp(request_info->uri, "/PSIA/System/Network/interfaces") == 0)
            return GET_PSIA_System_Network_interfaces(conn, request_info);
        if (strcmp(request_info->uri, "/PSIA/System/Network/interfaces/1") == 0)
            return GET_PSIA_System_Network_interfaces(conn, request_info);
        //if (strcmp(request_info->uri, "/PSIA/System/Network/ports") == 0)
            //return GET_PSIA_System_Network_ports(conn, request_info);

        if (strcmp(request_info->uri, "/PSIA/Streaming/channels/101") == 0)
            return GET_PSIA_Streaming_channels_101(conn, request_info);
        if (strcmp(request_info->uri, "/PSIA/Streaming/channels/102") == 0)
            return GET_PSIA_Streaming_channels_102(conn, request_info);

        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1") == 0)
            return GET_PSIA_Custom_Image_channels_1(conn, request_info);

        if (strcmp(request_info->uri, "/PSIA/Custom/OSD/channels/1") == 0) 
            return GET_PSIA_Custom_OSD_channels_1(conn, request_info);

        if (strcmp(request_info->uri, "/PSIA/System/Serial/ports/1") == 0) 
            return GET_PSIA_System_Serial_ports_1(conn, request_info);

        if (strcmp(request_info->uri, "/PSIA/Custom/PTZ/channels") == 0) 
            return GET_PSIA_Custom_PTZ_channels(conn, request_info);

        if (strcmp(request_info->uri, "/PSIA/GPSSystem/information") == 0) 
            return GET_PSIA_GPSSystem_information(conn, request_info);
        if (strcmp(request_info->uri, "/PSIA/Compass/information") == 0) 
            return GET_PSIA_Compass_information(conn, request_info);
        if (strcmp(request_info->uri, "/PSIA/Gyro/information") == 0) 
            return GET_PSIA_Gyro_information(conn, request_info);
        if (strcmp(request_info->uri, "/PSIA/LaserRanging/information") == 0) 
            return GET_PSIA_LaserRanging_information(conn, request_info);

        if (strcmp(request_info->uri, "/PSIA/MotionDetection/information") == 0) 
            return GET_PSIA_MotionDetection_information(conn, request_info);
        if (strcmp(request_info->uri, "/PSIA/CoverDetection/information") == 0) 
            return GET_PSIA_CoverDetection_information(conn, request_info);

        if (strcmp(request_info->uri, "/PSIA/System/Audio/Channels") == 0) 
            return GET_PSIA_System_Audio_Channels(conn, request_info);



    } else if (strcmp(request_info->request_method, "POST") == 0) {

    } else if (strcmp(request_info->request_method, "DELETE") == 0) {

    } else if (strcmp(request_info->request_method, "PUT") == 0) {
        //SET SYSTEM DEVICE TIME NTP
        if (strcmp(request_info->uri, "/PSIA/System/deviceInfo") == 0)
            return PUT_PSIA_System_deviceInfo(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/System/time") == 0)
            return PUT_PSIA_System_time(conn, request_info,p);
        //SET NETWORK
        if (strcmp(request_info->uri, "/PSIA/System/Network/interfaces/1") == 0)
            return PUT_PSIA_System_Network_interfaces(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/System/Network/ports") == 0)
            return PUT_PSIA_System_Network_ports(conn, request_info, p);

        if (strcmp(request_info->uri, "/PSIA/Streaming/channels/101") == 0)
            return PUT_PSIA_Streaming_channels_101(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Streaming/channels/102") == 0)
            return PUT_PSIA_Streaming_channels_102(conn, request_info, p);

        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/Brightness") == 0)
            return PUT_PSIA_Custom_Image_channels_1_Brightness(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/Contrast") == 0)
            return PUT_PSIA_Custom_Image_channels_1_Contrast(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/Saturation") == 0)
            return PUT_PSIA_Custom_Image_channels_1_Saturation(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/Sharpness") == 0)
            return PUT_PSIA_Custom_Image_channels_1_Sharpness(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/IrisMode") == 0)
            return PUT_PSIA_Custom_Image_channels_1_IrisMode(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/ExporseTime") == 0)
            return PUT_PSIA_Custom_Image_channels_1_ExporseTime(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/ExposureLevel") == 0)
            return PUT_PSIA_Custom_Image_channels_1_ExposureLevel(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/ExposureMinTime") == 0)
            return PUT_PSIA_Custom_Image_channels_1_ExposureMinTime(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/ExposureMaxTime") == 0)
            return PUT_PSIA_Custom_Image_channels_1_ExposureMaxTime(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/Noise2D") == 0)
            return PUT_PSIA_Custom_Image_channels_1_Noise2D(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/Noise3D") == 0)
            return PUT_PSIA_Custom_Image_channels_1_Noise3D(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/GrayScale") == 0)
            return PUT_PSIA_Custom_Image_channels_1_GrayScale(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/WhiteBlance") == 0)
            return PUT_PSIA_Custom_Image_channels_1_WhiteBlance(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/ImageMirror") == 0)
            return PUT_PSIA_Custom_Image_channels_1_ImageMirror(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/PowerLineFrequency") == 0)
            return PUT_PSIA_Custom_Image_channels_1_PowerLineFrequency(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/IrcutFilter") == 0)
            return PUT_PSIA_Custom_Image_channels_1_IrcutFilter(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/Light") == 0)
            return PUT_PSIA_Custom_Image_channels_1_Light(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/LightMode") == 0)
            return PUT_PSIA_Custom_Image_channels_1_LightMode(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/DemoMode") == 0)
            return PUT_PSIA_Custom_Image_channels_1_DemoMode(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/WDRMode") == 0)
            return PUT_PSIA_Custom_Image_channels_1_WDRMode(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/FlogMode") == 0)
            return PUT_PSIA_Custom_Image_channels_1_FlogMode(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/Image/channels/1/MeterMode") == 0)
            return PUT_PSIA_Custom_Image_channels_1_MeterMode(conn, request_info, p);

        if (strcmp(request_info->uri, "/PSIA/Custom/OSD/channels/1") == 0)
            return PUT_PSIA_Custom_OSD_channels_1(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/OSD/Gis/info") == 0)
            return PUT_PSIA_Custom_OSD_Gis_info(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Custom/OSD/Ae/info") == 0)
            return PUT_PSIA_Custom_OSD_Ae_info(conn, request_info, p);

        if (strcmp(request_info->uri, "/PSIA/System/Serial/ports/1") == 0) 
            return PUT_PSIA_System_Serial_ports_1(conn, request_info, p);
       
        if (strcmp(request_info->uri, "/PSIA/Custom/PTZ/channels") == 0) 
            return PUT_PSIA_Custom_PTZ_channels(conn, request_info, p);

        if (strcmp(request_info->uri, "/PSIA/GPSSystem/information") == 0) 
            return PUT_PSIA_GPSSystem_information(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Compass/information") == 0) 
            return PUT_PSIA_Compass_information(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/Gyro/information") == 0) 
            return PUT_PSIA_Gyro_information(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/LaserRanging/information") == 0) 
            return PUT_PSIA_LaserRanging_information(conn, request_info, p);

        if (strcmp(request_info->uri, "/PSIA/MotionDetection/information") == 0) 
            return PUT_PSIA_MotionDetection_information(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/CoverDetection/information") == 0) 
            return PUT_PSIA_CoverDetection_information(conn, request_info, p);

        if (strcmp(request_info->uri, "/PSIA/PTZ/Tptz/Control") == 0) 
            return PUT_PSIA_PTZ_Tptz_Control(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/PTZ/Tptz/Zoom") == 0) 
            return PUT_PSIA_PTZ_Tptz_Zoom(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/PTZ/Tptz/Focus") == 0) 
            return PUT_PSIA_PTZ_Tptz_Focus(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/PTZ/Tptz/Stop") == 0) 
            return PUT_PSIA_PTZ_Tptz_Stop(conn, request_info, p);
        if (strcmp(request_info->uri, "/PSIA/PTZ/Iris/Enabled") == 0) 
            return PUT_PSIA_PTZ_Iris_Enabled(conn, request_info, p);

        if (strcmp(request_info->uri, "/PSIA/System/Audio/Channels") == 0) 
            return PUT_PSIA_System_Audio_Channels(conn, request_info, p);
    }
    return 0;
}


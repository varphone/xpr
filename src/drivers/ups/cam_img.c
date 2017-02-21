#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_utils.h>


//static int camera_common_get(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, void* buffer, int* size)
//{
//	const char *s = NULL;
//	int result = 0, len=0;
//    //printf("camera_image_get....%s %s\n", key, buffer);

//	switch(ent->type) {
//		case XPR_UPS_ENTRY_TYPE_BOOLEAN:
//			*(int*)buffer = XPR_JSON_BooleanValue(json);
//			break;
//		case XPR_UPS_ENTRY_TYPE_BLOB:
//			// not support yet...
//			break;
//		case XPR_UPS_ENTRY_TYPE_INT:
//			*(int*)buffer = XPR_JSON_IntegerValue(json);
//			break;
//		case XPR_UPS_ENTRY_TYPE_INT64:
//			*(int64_t*)buffer = XPR_JSON_Integer64Value(json);
//			break;
//		case XPR_UPS_ENTRY_TYPE_REAL:
//			*(double*)buffer = XPR_JSON_RealValue(json);
//			break;
//		case XPR_UPS_ENTRY_TYPE_STRING:
//			s = XPR_JSON_StringValue(json);
//			len = strlen(s);
//			if(len >= *size) {
//				result =  -1;
//				break;
//			}
//			strcpy_s(buffer, *size, s);
//			*size = len;
//			break;
//		default:
//			return -1;
//	}
//	return result;
//}

//static int camera_common_set(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, const void* data, int size)
//{
//	int result = -1;
//	int ipv4[4];

//    //printf("network_set_ipv4....%s %s\n", key, data);

//	switch(ent->type) {
//		case XPR_UPS_ENTRY_TYPE_BOOLEAN:
////			result = XPR_JSON_BooleanSet(js, *(int*)buffer);
//			break;
//		case XPR_UPS_ENTRY_TYPE_BLOB:
//			// not support yet...
//			break;
//		case XPR_UPS_ENTRY_TYPE_INT:
//			result = XPR_JSON_IntegerSet(json, *(int*)data);
//			break;
//		case XPR_UPS_ENTRY_TYPE_INT64:
//			result = XPR_JSON_Integer64Set(json, *(int64_t*)data);
//			break;
//		case XPR_UPS_ENTRY_TYPE_REAL:
//			result = XPR_JSON_RealSet(json, *(double*)data);
//			break;
//		case XPR_UPS_ENTRY_TYPE_STRING:
//            //printf("js_json=%s data=%s\n",  XPR_JSON_DumpString(json), data);
//			result = XPR_JSON_StringSet(json, (char *)data);
//			break;
//		default:
//			return -1;
//	}
	
//	// we need to save it to file....
//	return result ==0 ? 0 : -1;
//}

static int camera_common_get(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, void* buffer, int* size)
{
    return XPR_UPS_ReadData(ent, json, key, buffer, size);
}

static int camera_common_set(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, const void* data, int size)
{
    if(-1 == XPR_UPS_WriteData(ent, json, key, data, size))
        return XPR_ERR_UPS_WRITE;

    return XPR_ERR_OK;
}
/*
	/system/network/eth0/ipv4/address
*/
static const char* xpr_ups_driver_cam_names[] = { "camera", 0};
static const char* xpr_ups_driver_cam_descs[] = { "camera", 0};


static const char* xpr_ups_driver_cam_video_names[] = { "video", 0};
static const char* xpr_ups_driver_cam_video_descs[] = { "video", 0};

static const char* xpr_ups_driver_cam_video_encoder_names[] = { "encoder", 0};
static const char* xpr_ups_driver_cam_video_encoder_descs[] = { "encoder", 0};

static const char* xpr_ups_driver_cam_video_encoder_main_names[] = { "0", 0};
static const char* xpr_ups_driver_cam_video_encoder_main_descs[] = { "0", 0};

static const char* xpr_ups_driver_cam_video_encoder_secondary_names[] = { "1", 0};
static const char* xpr_ups_driver_cam_video_encoder_secondary_descs[] = { "1", 0};

static const char* xpr_ups_driver_cam_video_encoder_main_strings_names[] = { "codec", "resolution", 0};
static const char* xpr_ups_driver_cam_video_encoder_main_strings_descs[] = { "codec", "resolution", 0};

static const char* xpr_ups_driver_cam_video_encoder_secondary_strings_names[] = { "codec", "resolution", 0};
static const char* xpr_ups_driver_cam_video_encoder_secondary_strings_descs[] = { "codec", "resolution", 0};

static const char* xpr_ups_driver_cam_video_encoder_main_ints_names[] = { "fps", "bitrate", "frame_interval", "gov_length", "aisle_mode", 0};
static const char* xpr_ups_driver_cam_video_encoder_main_ints_descs[] = { "fps", "bitrate", "frame_interval", "gov_length", "aisle_mode", 0};

static const char* xpr_ups_driver_cam_video_encoder_secondary_ints_names[] = { "fps", "bitrate", "frame_interval", "gov_length", "aisle_mode", 0};
static const char* xpr_ups_driver_cam_video_encoder_secondary_ints_descs[] = { "fps", "bitrate", "frame_interval", "gov_length", "aisle_mode", 0};

static const char* xpr_ups_driver_cam_video_encoder_main_h264_names[] = { "h264", 0};
static const char* xpr_ups_driver_cam_video_encoder_main_h264_descs[] = { "h264", 0};

static const char* xpr_ups_driver_cam_video_encoder_secondary_h264_names[] = { "h264", 0};
static const char* xpr_ups_driver_cam_video_encoder_secondary_h264_descs[] = { "h264", 0};

static const char* xpr_ups_driver_cam_video_encoder_main_mjpeg_names[] = { "mjpeg", 0};
static const char* xpr_ups_driver_cam_video_encoder_main_mjpeg_descs[] = { "mjpeg", 0};

static const char* xpr_ups_driver_cam_video_encoder_secondary_mjpeg_names[] = { "mjpeg", 0};
static const char* xpr_ups_driver_cam_video_encoder_secondary_mjpeg_descs[] = { "mjpeg", 0};

static const char* xpr_ups_driver_cam_video_encoder_main_h264_ints_names[] = { "quality", 0};
static const char* xpr_ups_driver_cam_video_encoder_main_h264_ints_descs[] = { "quality", 0};

static const char* xpr_ups_driver_cam_video_encoder_secondary_h264_ints_names[] = { "quality", 0};
static const char* xpr_ups_driver_cam_video_encoder_secondary_h264_ints_descs[] = { "quality", 0};

static const char* xpr_ups_driver_cam_video_encoder_main_mjpeg_ints_names[] = { "quality", 0};
static const char* xpr_ups_driver_cam_video_encoder_main_mjpeg_ints_descs[] = { "quality", 0};

static const char* xpr_ups_driver_cam_video_encoder_secondary_mjpeg_ints_names[] = { "quality", 0};
static const char* xpr_ups_driver_cam_video_encoder_secondary_mjpeg_ints_descs[] = { "quality", 0};


static const char* xpr_ups_driver_cam_video_decoder_names[] = { "decoder", 0};
static const char* xpr_ups_driver_cam_video_decoder_descs[] = { "decoder", 0};

static const char* xpr_ups_driver_cam_video_decoder_main_names[] = { "0", 0};
static const char* xpr_ups_driver_cam_video_decoder_main_descs[] = { "0", 0};

static const char* xpr_ups_driver_cam_video_decoder_secondary_names[] = { "1", 0};
static const char* xpr_ups_driver_cam_video_decoder_secondary_descs[] = { "1", 0};

static const char* xpr_ups_driver_cam_video_decoder_main_strings_names[] = { "codec", 0};
static const char* xpr_ups_driver_cam_video_decoder_main_strings_descs[] = { "codec", 0};

static const char* xpr_ups_driver_cam_video_decoder_secondary_strings_names[] = { "codec", 0};
static const char* xpr_ups_driver_cam_video_decoder_secondary_strings_descs[] = { "codec", 0};


static const char* xpr_ups_driver_cam_audio_names[] = { "audio", 0};
static const char* xpr_ups_driver_cam_audio_descs[] = { "audio", 0};

static const char* xpr_ups_driver_cam_audio_encoder_names[] = { "encoder", 0};
static const char* xpr_ups_driver_cam_audio_encoder_descs[] = { "encoder", 0};

static const char* xpr_ups_driver_cam_audio_encoder_main_names[] = { "0", 0};
static const char* xpr_ups_driver_cam_audio_encoder_main_descs[] = { "0", 0};

static const char* xpr_ups_driver_cam_audio_encoder_secondary_names[] = { "1", 0};
static const char* xpr_ups_driver_cam_audio_encoder_secondary_descs[] = { "1", 0};

static const char* xpr_ups_driver_cam_audio_encoder_main_ints_names[] = { "codec", "input", "volume", 0};
static const char* xpr_ups_driver_cam_audio_encoder_main_ints_descs[] = { "codec", "input", "volume", 0};

static const char* xpr_ups_driver_cam_audio_encoder_secondary_ints_names[] = { "codec", "input", "volume", 0};
static const char* xpr_ups_driver_cam_audio_encoder_secondary_ints_descs[] = { "codec", "input", "volume", 0};


static const char* xpr_ups_driver_cam_audio_decoder_names[] = { "decoder", 0};
static const char* xpr_ups_driver_cam_audio_decoder_descs[] = { "decoder", 0};

static const char* xpr_ups_driver_cam_audio_decoder_main_names[] = { "0", 0};
static const char* xpr_ups_driver_cam_audio_decoder_main_descs[] = { "0", 0};

static const char* xpr_ups_driver_cam_audio_decoder_secondary_names[] = { "1", 0};
static const char* xpr_ups_driver_cam_audio_decoder_secondary_descs[] = { "1", 0};

static const char* xpr_ups_driver_cam_audio_decoder_main_strings_names[] = { "codec", 0};
static const char* xpr_ups_driver_cam_audio_decoder_main_strings_descs[] = { "codec", 0};

static const char* xpr_ups_driver_cam_audio_decoder_secondary_strings_names[] = { "codec", 0};
static const char* xpr_ups_driver_cam_audio_decoder_secondary_strings_descs[] = { "codec", 0};


static const char* xpr_ups_driver_cam_img_names[] = { "image", 0};
static const char* xpr_ups_driver_cam_img_descs[] = { "image", 0};

static const char* xpr_ups_driver_cam_img_source_names[] = { "source", 0};
static const char* xpr_ups_driver_cam_img_source_descs[] = { "source", 0};

static const char* xpr_ups_driver_cam_img_source_main_names[] = { "0", 0};
static const char* xpr_ups_driver_cam_img_source_main_descs[] = { "0", 0};

static const char* xpr_ups_driver_cam_img_source_secondary_names[] = { "1", 0};
static const char* xpr_ups_driver_cam_img_source_secondary_descs[] = { "1", 0};

static const char* xpr_ups_driver_cam_img_source_main_ints_names[] = { "brightness", "saturation", "contrast", "sharpness", "iris_mode", "anti_flicker", "day_night_mode", "mirror_mode", "wdr_level", "white_blance_mode", "2d_noise_level", "3d_noise_level", "defog_mode", "metering_mode", "grayscale", 0};
static const char* xpr_ups_driver_cam_img_source_main_ints_descs[] = { "brightness", "saturation", "contrast", "sharpness", "iris_mode", "anti_flicker", "day_night_mode", "mirror_mode", "wdr_level", "white_blance_mode", "2d_noise_level", "3d_noise_level", "defog_mode", "metering_mode", "grayscale", 0};

static const char* xpr_ups_driver_cam_img_source_secondary_ints_names[] = { "brightness", "saturation", "contrast", "sharpness", "iris_mode", "anti_flicker", "day_night_mode", "mirror_mode", "wdr_level", "white_blance_mode", "2d_noise_level", "3d_noise_level", "defog_mode", "metering_mode", "grayscale", 0};
static const char* xpr_ups_driver_cam_img_source_secondary_ints_descs[] = { "brightness", "saturation", "contrast", "sharpness", "iris_mode", "anti_flicker", "day_night_mode", "mirror_mode", "wdr_level", "white_blance_mode", "2d_noise_level", "3d_noise_level", "defog_mode", "metering_mode", "grayscale", 0};

static const char* xpr_ups_driver_cam_img_source_main_exposure_names[] = { "exposure", 0};
static const char* xpr_ups_driver_cam_img_source_main_exposure_descs[] = { "exposure", 0};

static const char* xpr_ups_driver_cam_img_source_secondary_exposure_names[] = { "exposure", 0};
static const char* xpr_ups_driver_cam_img_source_secondary_exposure_descs[] = { "exposure", 0};

static const char* xpr_ups_driver_cam_img_source_main_exposure_ints_names[] = { "time", "max_time", "min_time", "max_gain", 0};
static const char* xpr_ups_driver_cam_img_source_main_exposure_ints_descs[] = { "time", "max_time", "min_time", "max_gain", 0};

static const char* xpr_ups_driver_cam_img_source_secondary_exposure_ints_names[] = { "time", "max_time", "min_time", "max_gain", 0};
static const char* xpr_ups_driver_cam_img_source_secondary_exposure_ints_descs[] = { "time", "max_time", "min_time", "max_gain", 0};


static const char* xpr_ups_driver_cam_osd_names[] = { "osd", 0};
static const char* xpr_ups_driver_cam_osd_descs[] = { "osd", 0};

static const char* xpr_ups_driver_cam_osd_ints_names[] = { "time_type", "date_type", "enabled", "ae_info_enabled", "gis_invalid_data_enabled", "gis_speed_unit", "gyro_enabled", 0};
static const char* xpr_ups_driver_cam_osd_ints_descs[] = { "time_type", "date_type", "enabled", "ae_info_enabled", "gis_invalid_data_enabled", "gis_speed_unit", "gyro_enabled", 0};

static const char* xpr_ups_driver_cam_gis_names[] = { "gis", 0};
static const char* xpr_ups_driver_cam_gis_descs[] = { "gis", 0};

static const char* xpr_ups_driver_cam_gis_ints_names[] = { "enabled", "synctime_enabled", "work_mode", "gyro_enabled", "gyro_sensitive", "compass_enabled", "ldm_enabled", 0};
static const char* xpr_ups_driver_cam_gis_ints_descs[] = { "enabled", "synctime_enabled", "work_mode", "gyro_enabled", "gyro_sensitive", "compass_enabled", "ldm_enabled", 0};

static const char* xpr_ups_driver_cam_event_names[] = { "event", 0};
static const char* xpr_ups_driver_cam_event_descs[] = { "event", 0};

static const char* xpr_ups_driver_cam_event_motion_detection_names[] = { "motion_detection", 0};
static const char* xpr_ups_driver_cam_event_motion_detection_descs[] = { "motion_detection", 0};

static const char* xpr_ups_driver_cam_event_motion_detection_strings_names[] = { "scope", 0};
static const char* xpr_ups_driver_cam_event_motion_detection_strings_descs[] = { "scope", 0};

static const char* xpr_ups_driver_cam_event_motion_detection_ints_names[] = { "enabled", "sensitive", "threshold", 0};
static const char* xpr_ups_driver_cam_event_motion_detection_ints_descs[] = { "enabled", "sensitive", "threshold", 0};

static const char* xpr_ups_driver_cam_event_cover_detection_names[] = { "cover_detection", 0};
static const char* xpr_ups_driver_cam_event_cover_detection_descs[] = { "cover_detection", 0};

static const char* xpr_ups_driver_cam_event_cover_detection_ints_names[] = { "enabled", "sensitive", 0};
static const char* xpr_ups_driver_cam_event_cover_detection_ints_descs[] = { "enabled", "sensitive", 0};


static const char* xpr_ups_driver_cam_ptz_names[] = { "ptz", 0};
static const char* xpr_ups_driver_cam_ptz_descs[] = { "ptz", 0};

static const char* xpr_ups_driver_cam_ptz_strings_names[] = { "protocol", 0};
static const char* xpr_ups_driver_cam_ptz_strings_descs[] = { "protocol", 0};

static const char* xpr_ups_driver_cam_ptz_ints_names[] = { "address", "speed", "iris_enabled", 0};
static const char* xpr_ups_driver_cam_ptz_ints_descs[] = { "address", "speed", "iris_enabled", 0};

static const char* xpr_ups_driver_cam_ptz_tptz_names[] = { "tptz", 0};
static const char* xpr_ups_driver_cam_ptz_tptz_descs[] = { "tptz", 0};

static const char* xpr_ups_driver_cam_ptz_eptz_names[] = { "eptz", 0};
static const char* xpr_ups_driver_cam_ptz_eptz_descs[] = { "eptz", 0};

static const char* xpr_ups_driver_cam_ptz_tptz_ints_names[] = { "control", "zoom", "stop", "focus_mode", "focus", 0};
static const char* xpr_ups_driver_cam_ptz_tptz_ints_descs[] = { "control", "zoom", "stop", "focus_mode", "focus", 0};

static const char* xpr_ups_driver_cam_ptz_eptz_ints_names[] = { "control", "zoom", "stop", 0};
static const char* xpr_ups_driver_cam_ptz_eptz_ints_descs[] = { "control", "zoom", "stop", 0};


static const char* xpr_ups_driver_cam_rs232_names[] = { "rs232", 0};
static const char* xpr_ups_driver_cam_rs232_descs[] = { "rs232", 0};

static const char* xpr_ups_driver_cam_rs485_names[] = { "rs485", 0};
static const char* xpr_ups_driver_cam_rs485_descs[] = { "rs485", 0};

static const char* xpr_ups_driver_cam_rs232_ints_names[] = { "baud_rate", "data_bits", "stop_bits", "parity_check", "flow_control", 0};
static const char* xpr_ups_driver_cam_rs232_ints_descs[] = { "baud_rate", "data_bits", "stop_bits", "parity_check", "flow_control", 0};

static const char* xpr_ups_driver_cam_rs485_ints_names[] = { "baud_rate", "data_bits", "stop_bits", "parity_check", "flow_control", 0};
static const char* xpr_ups_driver_cam_rs485_ints_descs[] = { "baud_rate", "data_bits", "stop_bits", "parity_check", "flow_control", 0};


XPR_UPS_Entry xpr_ups_driver_camera_image[] = {
    {
        xpr_ups_driver_cam_names,
        xpr_ups_driver_cam_descs,
        "ups/dir",
        "/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_names,
        xpr_ups_driver_cam_video_descs,
        "ups/dir",
        "/camera/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_names,
        xpr_ups_driver_cam_video_encoder_descs,
        "ups/dir",
        "/camera/video/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_main_names,
        xpr_ups_driver_cam_video_encoder_main_descs,
        "ups/dir",
        "/camera/video/encoder/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_secondary_names,
        xpr_ups_driver_cam_video_encoder_secondary_descs,
        "ups/dir",
        "/camera/video/encoder/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_main_ints_names,
        xpr_ups_driver_cam_video_encoder_main_ints_descs,
        "ups/dir",
        "/camera/video/encoder/0/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_secondary_ints_names,
        xpr_ups_driver_cam_video_encoder_secondary_ints_descs,
        "ups/dir",
        "/camera/video/encoder/1/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_main_h264_names,
        xpr_ups_driver_cam_video_encoder_main_h264_descs,
        "ups/dir",
        "/camera/video/encoder/0/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_main_mjpeg_names,
        xpr_ups_driver_cam_video_encoder_main_mjpeg_descs,
        "ups/dir",
        "/camera/video/encoder/0/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_secondary_h264_names,
        xpr_ups_driver_cam_video_encoder_secondary_h264_descs,
        "ups/dir",
        "/camera/video/encoder/1/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_secondary_mjpeg_names,
        xpr_ups_driver_cam_video_encoder_secondary_mjpeg_descs,
        "ups/dir",
        "/camera/video/encoder/1/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_main_h264_ints_names,
        xpr_ups_driver_cam_video_encoder_main_h264_ints_descs,
        "ups/dir",
        "/camera/video/encoder/0/h264/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_main_mjpeg_ints_names,
        xpr_ups_driver_cam_video_encoder_main_mjpeg_ints_descs,
        "ups/dir",
        "/camera/video/encoder/0/mjpeg/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_secondary_h264_ints_names,
        xpr_ups_driver_cam_video_encoder_secondary_h264_ints_descs,
        "ups/dir",
        "/camera/video/encoder/1/h264/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_secondary_mjpeg_ints_names,
        xpr_ups_driver_cam_video_encoder_secondary_mjpeg_ints_descs,
        "ups/dir",
        "/camera/video/encoder/1/mjpeg/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_main_strings_names,
        xpr_ups_driver_cam_video_encoder_main_strings_descs,
        "ups/dir",
        "/camera/video/encoder/0/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_encoder_secondary_strings_names,
        xpr_ups_driver_cam_video_encoder_secondary_strings_descs,
        "ups/dir",
        "/camera/video/encoder/1/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_decoder_names,
        xpr_ups_driver_cam_video_decoder_descs,
        "ups/dir",
        "/camera/video/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_decoder_main_names,
        xpr_ups_driver_cam_video_decoder_main_descs,
        "ups/dir",
        "/camera/video/decoder/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_decoder_secondary_names,
        xpr_ups_driver_cam_video_decoder_secondary_descs,
        "ups/dir",
        "/camera/video/decoder/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_decoder_main_strings_names,
        xpr_ups_driver_cam_video_decoder_main_strings_descs,
        "ups/dir",
        "/camera/video/decoder/0/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_video_decoder_secondary_strings_names,
        xpr_ups_driver_cam_video_decoder_secondary_strings_descs,
        "ups/dir",
        "/camera/video/decoder/1/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_audio_names,
        xpr_ups_driver_cam_audio_descs,
        "ups/dir",
        "/camera/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_audio_encoder_names,
        xpr_ups_driver_cam_audio_encoder_descs,
        "ups/dir",
        "/camera/audio/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_audio_decoder_names,
        xpr_ups_driver_cam_audio_decoder_descs,
        "ups/dir",
        "/camera/audio/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_audio_encoder_main_names,
        xpr_ups_driver_cam_audio_encoder_main_descs,
        "ups/dir",
        "/camera/audio/encoder/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_audio_encoder_secondary_names,
        xpr_ups_driver_cam_audio_encoder_secondary_descs,
        "ups/dir",
        "/camera/audio/encoder/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_audio_encoder_main_ints_names,
        xpr_ups_driver_cam_audio_encoder_main_ints_descs,
        "ups/dir",
        "/camera/audio/encoder/0/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_audio_encoder_secondary_ints_names,
        xpr_ups_driver_cam_audio_encoder_secondary_ints_descs,
        "ups/dir",
        "/camera/audio/encoder/1/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_audio_decoder_main_names,
        xpr_ups_driver_cam_audio_decoder_main_descs,
        "ups/dir",
        "/camera/audio/decoder/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_audio_decoder_secondary_names,
        xpr_ups_driver_cam_audio_decoder_secondary_descs,
        "ups/dir",
        "/camera/audio/decoder/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_audio_decoder_main_strings_names,
        xpr_ups_driver_cam_audio_decoder_main_strings_descs,
        "ups/dir",
        "/camera/audio/decoder/0/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_audio_decoder_secondary_strings_names,
        xpr_ups_driver_cam_audio_decoder_secondary_strings_descs,
        "ups/dir",
        "/camera/audio/decoder/1/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_img_names,
        xpr_ups_driver_cam_img_descs,
        "ups/dir",
        "/camera/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_img_source_names,
        xpr_ups_driver_cam_img_source_descs,
        "ups/dir",
        "/camera/image/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_img_source_main_names,
        xpr_ups_driver_cam_img_source_main_descs,
        "ups/dir",
        "/camera/image/source/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_img_source_secondary_names,
        xpr_ups_driver_cam_img_source_secondary_descs,
        "ups/dir",
        "/camera/image/source/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_img_source_main_ints_names,
        xpr_ups_driver_cam_img_source_main_ints_descs,
        "ups/dir",
        "/camera/image/source/0/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_img_source_secondary_ints_names,
        xpr_ups_driver_cam_img_source_secondary_ints_descs,
        "ups/dir",
        "/camera/image/source/1/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_img_source_main_exposure_names,
        xpr_ups_driver_cam_img_source_main_exposure_descs,
        "ups/dir",
        "/camera/image/source/0/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_img_source_secondary_exposure_names,
        xpr_ups_driver_cam_img_source_secondary_exposure_descs,
        "ups/dir",
        "/camera/image/source/1/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_img_source_main_exposure_ints_names,
        xpr_ups_driver_cam_img_source_main_exposure_ints_descs,
        "ups/dir",
        "/camera/image/source/0/exposure/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_img_source_secondary_exposure_ints_names,
        xpr_ups_driver_cam_img_source_secondary_exposure_ints_descs,
        "ups/dir",
        "/camera/image/source/1/exposure/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_osd_names,
        xpr_ups_driver_cam_osd_descs,
        "ups/dir",
        "/camera/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_osd_ints_names,
        xpr_ups_driver_cam_osd_ints_descs,
        "ups/dir",
        "/camera/osd/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_gis_names,
        xpr_ups_driver_cam_gis_descs,
        "ups/dir",
        "/camera/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_gis_ints_names,
        xpr_ups_driver_cam_gis_ints_descs,
        "ups/dir",
        "/camera/gis/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_event_names,
        xpr_ups_driver_cam_event_descs,
        "ups/dir",
        "/camera/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_event_motion_detection_names,
        xpr_ups_driver_cam_event_motion_detection_descs,
        "ups/dir",
        "/camera/event/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_event_cover_detection_names,
        xpr_ups_driver_cam_event_cover_detection_descs,
        "ups/dir",
        "/camera/event/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_event_motion_detection_ints_names,
        xpr_ups_driver_cam_event_motion_detection_ints_descs,
        "ups/dir",
        "/camera/event/motion_detection/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_event_motion_detection_strings_names,
        xpr_ups_driver_cam_event_motion_detection_strings_names,
        "ups/dir",
        "/camera/event/motion_detection/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_event_cover_detection_ints_names,
        xpr_ups_driver_cam_event_cover_detection_ints_descs,
        "ups/dir",
        "/camera/event/cover_detection/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_ptz_names,
        xpr_ups_driver_cam_ptz_descs,
        "ups/dir",
        "/camera/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_ptz_strings_names,
        xpr_ups_driver_cam_ptz_strings_descs,
        "ups/dir",
        "/camera/ptz/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_ptz_ints_names,
        xpr_ups_driver_cam_ptz_ints_descs,
        "ups/dir",
        "/camera/ptz/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_ptz_eptz_names,
        xpr_ups_driver_cam_ptz_eptz_descs,
        "ups/dir",
        "/camera/ptz/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_ptz_tptz_names,
        xpr_ups_driver_cam_ptz_tptz_descs,
        "ups/dir",
        "/camera/ptz/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_ptz_eptz_ints_names,
        xpr_ups_driver_cam_ptz_eptz_ints_descs,
        "ups/dir",
        "/camera/ptz/eptz/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_ptz_tptz_ints_names,
        xpr_ups_driver_cam_ptz_tptz_ints_descs,
        "ups/dir",
        "/camera/ptz/tptz/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_rs232_names,
        xpr_ups_driver_cam_rs232_descs,
        "ups/dir",
        "/camera/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_rs485_names,
        xpr_ups_driver_cam_rs485_descs,
        "ups/dir",
        "/camera/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_rs232_ints_names,
        xpr_ups_driver_cam_rs232_ints_descs,
        "ups/dir",
        "/camera/rs232/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_cam_rs485_ints_names,
        xpr_ups_driver_cam_rs485_ints_descs,
        "ups/dir",
        "/camera/rs485/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_common_get, camera_common_set,
        0, 0, 0
    }
};

const int xpr_ups_driver_camera_image_count =  _countof(xpr_ups_driver_camera_image);

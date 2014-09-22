#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_utils.h>


static int camera_image_get(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, void* buffer, int* size)
{
	const char *s = NULL;
	int result = 0, len=0;	

	switch(ent->type) {
		case XPR_UPS_ENTRY_TYPE_BOOLEAN:
			*(int*)buffer = XPR_JSON_BooleanValue(json);
			break;
		case XPR_UPS_ENTRY_TYPE_BLOB:
			// not support yet...
			break;
		case XPR_UPS_ENTRY_TYPE_INT:
			*(int*)buffer = XPR_JSON_IntegerValue(json);	
			break;
		case XPR_UPS_ENTRY_TYPE_INT64:
			*(int64_t*)buffer = XPR_JSON_Integer64Value(json);
			break;
		case XPR_UPS_ENTRY_TYPE_REAL:
			*(double*)buffer = XPR_JSON_RealValue(json);
			break;
		case XPR_UPS_ENTRY_TYPE_STRING:
			s = XPR_JSON_StringValue(json);
			len = strlen(s);
			if(len >= *size) {
				result =  -1;
				break;
			}	
			strcpy_s(buffer, *size, s);
			*size = len;
			break;
		default:
			return -1;
	}
	return result;
}

static int camera_image_set(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, const void* data, int size)
{
	int result = -1;
	int ipv4[4];

	switch(ent->type) {
		case XPR_UPS_ENTRY_TYPE_BOOLEAN:
//			result = XPR_JSON_BooleanSet(js, *(int*)buffer);
			break;
		case XPR_UPS_ENTRY_TYPE_BLOB:
			// not support yet...
			break;
		case XPR_UPS_ENTRY_TYPE_INT:
			result = XPR_JSON_IntegerSet(json, *(int*)data);	
			break;
		case XPR_UPS_ENTRY_TYPE_INT64:
			result = XPR_JSON_Integer64Set(json, *(int64_t*)data);
			break;
		case XPR_UPS_ENTRY_TYPE_REAL:
			result = XPR_JSON_RealSet(json, *(double*)data);
			break;
		case XPR_UPS_ENTRY_TYPE_STRING:
			result = XPR_JSON_StringSet(json, (char *)data);
			break;
		default:
			return -1;
	}
	
	// we need to save it to file....
	return result ==0 ? 0 : -1;
}
/*
	/system/network/eth0/ipv4/address
*/
static const char* xpr_ups_driver_cam_names[] = { "camera", 0};
static const char* xpr_ups_driver_cam_descs[] = { "camera", 0};

static const char* xpr_ups_driver_cam_img_names[] = { "image", 0};
static const char* xpr_ups_driver_cam_img_descs[] = { "image", 0};

static const char* xpr_ups_driver_cam_img_strings_names[] = { "focus", "exposure", 0};
static const char* xpr_ups_driver_cam_img_strings_descs[] = { "focus", "exposure", 0};

static const char* xpr_ups_driver_cam_img_ints_names[] = { "brightness", "saturation", "contrast", "sharpness", "gain", "shutter", 0};
static const char* xpr_ups_driver_cam_img_ints_descs[] = { "brightness", "saturation", "contrast", "sharpness", "gain", "shutter", 0};

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
        xpr_ups_driver_cam_img_names,
        xpr_ups_driver_cam_img_descs,
        "ups/dir",
        "/camera/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {	
        xpr_ups_driver_cam_img_ints_names,
        xpr_ups_driver_cam_img_ints_descs,
        "ups/entry",
        "/camera/image/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, camera_image_get, camera_image_set,
        0, 0, 0
    },
    {	
        xpr_ups_driver_cam_img_strings_names,
        xpr_ups_driver_cam_img_strings_descs,
        "ups/entry",
        "/camera/image/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, camera_image_get, camera_image_set,
        0, 0, 0
    },
};

const int xpr_ups_driver_camera_image_count =  _countof(xpr_ups_driver_camera_image);

#include <xpr/xpr_ups.h>

static const char* xpr_ups_driver_root_names[] = { "/"};
static const char* xpr_ups_driver_root_descs[] = { "root"};

XPR_UPS_Entry xpr_ups_driver_root = {
    xpr_ups_driver_root_names,
    xpr_ups_driver_root_descs,
    "ups/root",
    "",
    XPR_UPS_ENTRY_TYPE_DIR,
    0, 0, 0, 0,
    0, 0, 0
};


#include <xpr/xpr_ups.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-conversion"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
#elif defined(_MSC_VER)
// FIXME:
#endif

XPR_UPS_Entry __xpr_ups_driver_fakeroot[] = {
    XPR_UPS_ENTRY_DIR4("camera", "Camera settings", "ups/dir", "/"),
    XPR_UPS_ENTRY_DIR4("dsp", "Digital Signal Processor", "ups/dir", "/"),
    XPR_UPS_ENTRY_DIR4("system", "System settings", "ups/dir", "/"),
    XPR_UPS_ENTRY_END(),
};

const int __xpr_ups_driver_fakeroot_count =  _countof(__xpr_ups_driver_fakeroot);

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
// FIXME:
#endif

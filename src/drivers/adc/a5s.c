#ifdef BOARD_MAJOR_A5S
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <xpr/xpr_adc.h>

static int gADCFd = -1;

XPR_API int XPR_ADC_Init(void)
{
    if (gADCFd > 0)
        return 1;
    gADCFd = open("/proc/ambarella/adc", O_RDONLY);
    return gADCFd > 0 ? 0 : -1;
}

XPR_API void XPR_ADC_Fini(void)
{
    if (gADCFd > 0) {
        close(gADCFd);
        gADCFd = -1;
    }
}

XPR_API int XPR_ADC_GetChannelCount(void)
{
    return 4;
}

XPR_API int XPR_ADC_GetValue(int channel)
{
    if (channel < 0 || channel > 3)
        return 0;
    char tmp[256];
    lseek(gADCFd, SEEK_SET, 0);
    if (read(gADCFd, tmp, sizeof(tmp)) < 0)
        return 0;
    char key[64];
    snprintf(key, sizeof(key), "adc%d = ", channel);
    char* p = strstr(tmp, key);
    if (!p)
        return 0;
    return strtol(p + strlen(key), NULL, 0);
}

#endif

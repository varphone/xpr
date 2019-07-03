#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <gpc/gpc.h>

static int g_xpr_gpio_dev_fd = -1;

XPR_API int XPR_GPIO_Init(void)
{
    g_xpr_gpio_dev_fd = open("/dev/gpc", O_RDWR);
    if (g_xpr_gpio_dev_fd <= 0)
        return XPR_ERR_ERROR;
    return XPR_ERR_SUCCESS;  
}

XPR_API void XPR_GPIO_Fini(void)
{
    close(g_xpr_gpio_dev_fd);
    g_xpr_gpio_dev_fd = -1;
}

XPR_API int XPR_GPIO_Get(int port, int* value)
{
    gpio_info_t gi = { port, 0 };
    if (ioctl(g_xpr_gpio_dev_fd, GPC_IOC_GET, &gi) < 0)
        return XPR_ERR_ERROR;
    if (value)
        *value = gi.level;
    return XPR_ERR_SUCCESS;
}

XPR_API int XPR_GPIO_Set(int por, int value)
{
    gpio_info_t gi = { port, value };
    if (ioctl(g_xpr_gpio_dev_fd, GPC_IOC_SET, &gi) < 0)
        return XPR_ERR_ERROR;
    return XPR_ERR_SUCCESS;
}

// FIXME：
XPR_API int XPR_GPIO_GetMode(int port, int* mode)
{
    return XPR_ERR_ERROR;
}

XPR_API int XPR_GPIO_SetMode(int por, int mode)
{
    gpio_config_t gc = { port, mode };
    if (ioctl(g_xpr_gpio_dev_fd, GPC_IOC_CONFIG, &gc) < 0)
        return XPR_ERR_ERROR;
    return XPR_ERR_SUCCESS;
}


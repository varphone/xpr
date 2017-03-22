#include <xpr/xpr_errno.h>
#include <xpr/xpr_gpio.h>
#include <hi3518c/mpi_sys.h>

static int get_physical_address(int port)
{
    int major = XPR_GPIO_PORT_MAJOR(port);
    int minor = XPR_GPIO_PORT_MINOR(port);
    int address = XPR_GPIOX_BASE[major] + XPR_GPIO_OFFSET[minor];
    return address;
}

int XPR_GPIO_Init(void)
{
    return XPR_ERR_SUCCESS;
}

int XPR_GPIO_Fini(void)
{
    return XPR_ERR_SUCCESS;
}

int XPR_GPIO_Get(int port, int* value)
{
    int address = get_physical_address(port);
    if (HI_MPI_SYS_GetReg(address, value) == HI_FAILURE)
        return XPR_ERR_ERROR;

    return XPR_ERR_SUCCESS;
}

int XPR_GPIO_Set(int port, int value)
{
    int address = get_physical_address(port);
    if (HI_MPI_SYS_SetReg(address, value) == HI_FAILURE)
        return XPR_ERR_ERROR;

    return XPR_ERR_ERROR;
}

int XPR_GPIO_GetMode(int port, int* mode)
{
    return XPR_ERR_SUCCESS;
}

int XPR_GPIO_SetMode(int port, int mode)
{
    return XPR_ERR_SUCCESS;
}

int XPR_GPIO_Enable(int port, int value)
{
    int major = XPR_GPIO_PORT_MAJOR(port);
    int minor = XPR_GPIO_PORT_MINOR(port);
    int address = 0;
    if(major == 5) {
        if(minor == 2)
            address = GPIO5_2_ENABLE;
        else if(minor == 3)
            address = GPIO5_3_ENABLE;
    } else if(major == 0) {
        if(minor == 6)
            address = GPIO0_6_ENABLE;
    }

    if (HI_MPI_SYS_SetReg(address, value) == HI_FAILURE)
        return XPR_ERR_ERROR;

    return XPR_ERR_SUCCESS;
}


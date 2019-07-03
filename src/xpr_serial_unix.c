#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <xpr/xpr_drm.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_json.h>
#include <xpr/xpr_serial.h>
#include <xpr/xpr_utils.h>


/// @brief 打开串口库
/// @param [in] portAddr 端口地址：/dev/ttyS0
/// @retval -1  失败
/// @retval 0   文件描述符
/// @sa close_port()
XPR_API int XPR_OpenPort(const char* portAddr)
{
    int fd = -1;
    fd = open(portAddr, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        printf("Can't Open Serial Port !");
        return -1;
    }
    if (fcntl(fd, F_SETFL, 0) <
        0) { /*恢复串口的状态为阻塞状态，用于等待串口数据的读入*/
        printf("fcntl failed !\n");
        return (-1);
    }
    else {
        printf("fcntl = %d !\n", fcntl(fd, F_SETFL, 0));
    }
    /*测试打开的文件描述符是否应用一个终端设备，以进一步确认串口是否正确打开*/
    if (!isatty(STDIN_FILENO)) {
        printf("Standard input isn't a terminal device !\n");
        return -1;
    }
    else {
        printf("It's a serial terminal device!\n");
    }
    return fd;
}

/// @brief 关闭串口库
/// @param [in] fd           文件描述符
/// @retval -1  失败
/// @retval 0   成功
/// @sa open_port()
XPR_API int XPR_ClosePort(int fd)
{
    int ret = -1;
    ret = close(fd);
    return ret;
}

/// @brief 配置串口信息
/// @param [in] fd           文件描述符
/// @param [in] baudRate     波特率
/// @param [in] dataBit      数据位
/// @param [in] parity       校验
/// @param [in] stopBit      停止位
/// @retval -1  失败
/// @retval 0   成功
XPR_API int XPR_SetPort(int fd, int baudRate, int dataBit, char parity,
                        int stopBit)
{
    int ret = 0;
    struct termios oldtio, newtio;
    ret = tcgetattr(fd, &oldtio); /*保存原先串口配置*/
    if (ret) {
        printf("Can't get old terminal description !");
        return -1;
    }
    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag |= CLOCAL | CREAD; /*设置本地连接和接收使用*/
    /*设置输入输出波特率*/
    switch (baudRate) {
    case 2400:
        cfsetispeed(&newtio, B2400);
        cfsetospeed(&newtio, B2400);
        break;
    case 4800:
        cfsetispeed(&newtio, B4800);
        cfsetospeed(&newtio, B4800);
        break;
    case 9600:
        cfsetispeed(&newtio, B9600);
        cfsetospeed(&newtio, B9600);
        break;
    case 19200:
        cfsetispeed(&newtio, B19200);
        cfsetospeed(&newtio, B19200);
        break;
    case 38400:
        cfsetispeed(&newtio, B38400);
        cfsetospeed(&newtio, B38400);
        break;
    case 57600:
        cfsetispeed(&newtio, B57600);
        cfsetospeed(&newtio, B57600);
        break;
    case 115200:
        cfsetispeed(&newtio, B115200);
        cfsetospeed(&newtio, B115200);
        break;
    case 460800:
        cfsetispeed(&newtio, B460800);
        cfsetospeed(&newtio, B460800);
        break;
    default:
        printf("Don't exist baudRate %d !\n", baudRate);
        return -1;
    }
    /*设置数据位*/
    newtio.c_cflag &= (~CSIZE);
    switch (dataBit) {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    default:
        printf("Don't exist dataBit %d !\n", dataBit);
        return -1;
    }
    /*设置校验位*/
    switch (parity) {
    case 'N': /*无校验*/
        newtio.c_cflag &= (~PARENB);
        break;
    case 'O': /*奇校验*/
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E': /*偶校验*/
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= (~PARODD);
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    default:
        printf("Don't exist cParity %c !\n", parity);
        return -1;
    }
    /*设置停止位*/
    switch (stopBit) {
    case 1:
        newtio.c_cflag &= (~CSTOPB);
        break;
    case 2:
        newtio.c_cflag |= CSTOPB;
        break;
    default:
        printf("Don't exist stopBit %d !\n", stopBit);
        return -1;
    }
    newtio.c_cc[VTIME] =
        200; /*设置等待时间,要阻塞的话一定要配置时间，不然和非阻塞没区别*/
    newtio.c_cc[VMIN] = 0; /*设置最小字符*/
    tcflush(fd, TCIFLUSH); /*刷新输入队列(TCIOFLUSH为刷新输入输出队列)*/
    ret =
        tcsetattr(fd, TCSANOW,
                  &newtio); /*激活新的设置使之生效,参数TCSANOW表示更改立即发生*/
    if (ret) {
        printf("Set new terminal description error !");
        return -1;
    }
    printf("set_port success !\n");
    return 0;
}

/// @brief 读取串口数据
/// @param [in] fd           文件描述符
/// @param [in] buf          数据缓存
/// @param [in] length       数据长度
/// @retval <=0 失败
/// @retval >0  成功
XPR_API int XPR_ReadPort(int fd, void* buf, int length)
{
    int ret = 0;
    if (!length) {
        printf("Read byte number error !\n");
        return -1;
    }
    ret = read(fd, buf, length);
    return ret;
}

/// @brief 串口写入数据
/// @param [in] fd           文件描述符
/// @param [in] buf          数据缓存
/// @param [in] length       数据长度
/// @retval -1  失败
/// @retval 0   成功
XPR_API int XPR_WritePort(int fd, void* buf, int length)
{
    int ret = 0;
    if (!length) {
        printf("Write byte number error !\n");
        return -1;
    }
    ret = write(fd, buf, length);
    return 0;
}

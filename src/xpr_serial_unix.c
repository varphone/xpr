#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_serial.h>
#include <xpr/xpr_utils.h>

XPR_API int XPR_SerialOpen(const char* portAddr)
{
    int fd = -1;
    fd = open(portAddr, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        DBG(DBG_L2, "XPR_Serial: open(%s) failed, errno: %d", portAddr, errno);
        return -1;
    }
    // 恢复串口的状态为阻塞状态，用于等待串口数据的读入
    if (fcntl(fd, F_SETFL, 0) < 0) {
        DBG(DBG_L2, "XPR_Serial: fcntl(%d,F_SETFL,0) failed, errno: %d", fd,
            errno);
        return -1;
    }

    return fd;
}

XPR_API int XPR_SerialClose(int fd)
{
    int ret = -1;
    ret = close(fd);
    return ret;
}

XPR_API int XPR_SerialSetup(int fd, int baudRate, int dataBit, char parity,
                            int stopBit)
{
    int ret = 0;
    struct termios oldtio, newtio;
    // 保存原先串口配置
    ret = tcgetattr(fd, &oldtio);
    if (ret) {
        DBG(DBG_L2, "XPR_Serial: tcgetattr(%d) failed, errno: %d", fd, errno);
        return -1;
    }
    bzero(&newtio, sizeof(newtio));
    // 设置本地连接和接收使用
    newtio.c_cflag |= CLOCAL | CREAD;
    // 设置输入输出波特率
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
        DBG(DBG_L2, "XPR_Serial: unsupported baudRate: %d", baudRate);
        return -1;
    }
    // 设置数据位
    newtio.c_cflag &= (~CSIZE);
    switch (dataBit) {
    case 7:
        newtio.c_cflag |= CS7;
        break;
    case 8:
        newtio.c_cflag |= CS8;
        break;
    default:
        DBG(DBG_L2, "XPR_Serial: unsupported dataBit: %d", dataBit);
        return -1;
    }
    // 设置校验位
    switch (parity) {
    case 'N': // 无校验
        newtio.c_cflag &= (~PARENB);
        break;
    case 'O': // 奇校验
        newtio.c_cflag |= PARENB;
        newtio.c_cflag |= PARODD;
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    case 'E': // 偶校验
        newtio.c_cflag |= PARENB;
        newtio.c_cflag &= (~PARODD);
        newtio.c_iflag |= (INPCK | ISTRIP);
        break;
    default:
        DBG(DBG_L2, "XPR_Serial: unsupported parityBit %c", parity);
        return -1;
    }
    // 设置停止位
    switch (stopBit) {
    case 1:
        newtio.c_cflag &= (~CSTOPB);
        break;
    case 2:
        newtio.c_cflag |= CSTOPB;
        break;
    default:
        DBG(DBG_L2, "XPR_Serial: unsupported stopBit: %d", stopBit);
        return -1;
    }
    // 设置等待时间，要阻塞的话一定要配置时间，不然和非阻塞没区别
    newtio.c_cc[VTIME] = 200;
    // 设置最小字符
    newtio.c_cc[VMIN] = 0;
    // 刷新输入队列(TCIOFLUSH为刷新输入输出队列)
    tcflush(fd, TCIFLUSH);
    // 激活新的设置使之生效，参数TCSANOW表示更改立即发生
    ret = tcsetattr(fd, TCSANOW, &newtio);
    if (ret) {
        DBG(DBG_L2, "XPR_Serial: tcsetattr(%d) failed, errno: %d", fd, errno);
        return -1;
    }
    return 0;
}

XPR_API int XPR_SerialRead(int fd, void* buf, int length)
{
    int ret = 0;
    if (!length) {
        return -1;
    }
    ret = read(fd, buf, length);
    if (ret != length)
        DBG(DBG_L2, "XPR_Serial: read(%d,%p,%d)=%d failed, errno: %d", fd, buf,
            length, ret, errno);
    return ret;
}

XPR_API int XPR_SerialWrite(int fd, void* buf, int length)
{
    int ret = 0;
    if (!length) {
        return -1;
    }
    ret = write(fd, buf, length);
    if (ret != length)
        DBG(DBG_L2, "XPR_Serial: write(%d,%p,%d)=%d failed, errno: %d", fd, buf,
            length, ret, errno);
    return 0;
}

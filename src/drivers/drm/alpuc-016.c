#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <rl/rl.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_drm.h>

static int gDevFd = -1;
static char* gDevFile = "/dev/i2c-0";
static int gDevSlaveAddress = 0x7a;
const int gSerialLength = 8;

static int gSeq = 0;
static void dump(uint8_t* buffer, int size, const char* mark)
{
    printf("[%-2d] %s", gSeq++, mark);
    for (int i=0; i<size; i++) {
        printf("%02x ", buffer[i]);
    }
    printf("\n");
}

static int i2cRead(uint8_t* buffer, int size)
{
    if (gDevFd < 0)
        return -1;
    int result = read(gDevFd, buffer, size);
    if (result != size) {
        printf("i2cRead() = %d, errno = %d\n", result, errno);
        return -1;
    }
    //dump(buffer, size, "R: ");
    return result;
}

static int i2cRead8a(uint8_t regAddr, uint8_t* buffer, int size)
{
    struct i2c_msg msgs[2];
    msgs[0].addr = gDevSlaveAddress >> 1;
    msgs[0].flags = 0;
    msgs[0].buf = &regAddr;
    msgs[0].len = 1;
    msgs[1].addr = gDevSlaveAddress >> 1;
    msgs[1].flags = I2C_M_RD;
    msgs[1].buf = buffer;
    msgs[1].len = size;

    struct i2c_rdwr_ioctl_data ioData;
    ioData.msgs = msgs;
    ioData.nmsgs = 2;

    if (ioctl(gDevFd, I2C_RDWR, &ioData) < 0) {
        printf("ioctl(I2C_RDWR) failure, errno = %d\n", errno);
        return -1;
    }

    return size;
}

static int i2cWrite(const uint8_t* data, int count)
{
    if (gDevFd < 0)
        return -1;
    int result = write(gDevFd, data, count);
    if (result != count) {
        printf("i2cWrite() = %d, errno = %d\n", result, errno);
        return -1;
    }
    //dump(data, count, "W: ");
    return result;
}

static int i2cWrite8a(uint8_t regAddr, const uint8_t* data, int count)
{
    uint8_t tmp[256];
    tmp[0] = regAddr;
    memcpy(&tmp[1], data, count);

    struct i2c_msg msgs[1];
    msgs[0].addr = gDevSlaveAddress >> 1;
    msgs[0].flags = 0;
    msgs[0].buf = tmp;
    msgs[0].len = count+1;

    struct i2c_rdwr_ioctl_data ioData;
    ioData.msgs = msgs;
    ioData.nmsgs = 1;

    if (ioctl(gDevFd, I2C_RDWR, &ioData) < 0) {
        printf("ioctl(I2C_RDWR) failure, errno = %d\n", errno);
        return -1;
    }

    return count;
}

// ALPUC-016
//==============================================================================
//_alpum_process fuction has been declared in the library file.
//You should call this function by extern to ecrypt your system.
extern unsigned char alpuc_process(unsigned char*, unsigned char*);

//These Variables below has been declared in the library file.
//You should call this function by extern. These Variables are for ckecking the data from ecryption mode.

//These fuction below are necessary for ecryption mode.
//Please declare these fuction as below in your ALPU Check function or main source.

unsigned char _alpu_rand(void);
void _alpu_delay_ms(unsigned char i);

unsigned char _i2c_write(unsigned char slaveAddr,
                         unsigned char regAddr, unsigned char* data, int length)
{
#if 0
    unsigned char tmp[256];
    tmp[0] = regAddr;
    memcpy(&tmp[1], data, length);
    return i2cWrite(tmp, length + 1) > 0 ? 0 : 1;
#else
    return i2cWrite8a(regAddr, data, length) > 0 ? 0 : 1;
#endif
}

unsigned char _i2c_read(unsigned char slaveAddr,
                        unsigned char regAddr, unsigned char* data, int length)
{
#if 0
    if (i2cWrite(&regAddr, 1) < 0)
        return 1;
    return i2cRead(data, length) > 0 ? 0 : 1;
#else
    return i2cRead8a(regAddr, data, length) > 0 ? 0 : 1;
#endif
}

#define Delay_ms(x) usleep((x) * 1000)
void _alpu_delay_ms(unsigned char i)
{
    Delay_ms(i);
}

// Modify this fuction using RTC. But you should not change the function name.
unsigned char _alpu_rand(void)   
{
    static unsigned long seed; // 2byte, must be a static variable
    seed = seed + rand(); // rand(); <------------------ add time value
    seed =  seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

struct IDCodes {
    uint8_t code1[8];
    uint8_t code2[8];
    uint8_t code3[8];
    uint8_t code4[8];
};

static int alpucReadIDCodes(struct IDCodes* ids)
{
    int errorCode = 0;
    memset(ids, 0, sizeof(*ids));
    errorCode |= _i2c_read(gDevSlaveAddress, 0x73, ids->code1, 8);
    errorCode |= _i2c_read(gDevSlaveAddress, 0x74, ids->code2, 8);
    errorCode |= _i2c_read(gDevSlaveAddress, 0x75, ids->code3, 8);
    errorCode |= _i2c_read(gDevSlaveAddress, 0x76, ids->code4, 8);
#if 0
    printf("ID Code1 : ");
    for (int i = 0; i < 8; i++) printf("0x%02x ", ids->code1[i]); printf("\n");
    printf("ID Code2 : ");
    for (int i = 0; i < 8; i++) printf("0x%02x ", ids->code2[i]); printf("\n");
    printf("ID Code3 : ");
    for (int i = 0; i < 8; i++) printf("0x%02x ", ids->code3[i]); printf("\n");
    printf("ID Code4 : ");
    for (int i = 0; i < 8; i++) printf("0x%02x ", ids->code4[i]); printf("\n");
#endif
    return errorCode ? -1 : 0;
}

static int alpucVerify(void)
{
    unsigned char error_code;
    unsigned char dx_data[8];  // 计算的数据， 加密 正确的话应该和tx_data相等
    unsigned char tx_data[8];  // 随机数 或 您的系统的数据
    for (int i = 0; i < 8; i++) {
        tx_data[i] = _alpu_rand();
        dx_data[i] = 0;
    }
    error_code = alpuc_process(tx_data, dx_data);
#if 0
    if (error_code) {
        printf("Alpu-M Encryption Test Fail!!!\n");
    }
    else {
        printf("Alpu-M Encryption Test Success!!!\n");
    }
    printf("Error_code : %d\n", error_code);
    printf("========================ALPU-M IC Encryption========================\n");
    printf(" Tx Data : ");
    for (int i = 0; i < 8; i++) printf("0x%02x ", tx_data[i]); printf("\n");
    printf(" Dx Data : ");
    for (int i = 0; i < 8; i++) printf("0x%02x ", dx_data[i]); printf("\n");
    printf("====================================================================\n");
#endif
    return error_code ? -1 : 0;
}
//==============================================================================

// FIXME:
int XPR_DRM_Config(int request, const void* data, int size)
{
    return XPR_ERR_SUCCESS;
}

int XPR_DRM_Init(void)
{
    if (gDevFd > 0)
        return 1;
    gDevFd = open(gDevFile, O_RDWR);
    if (gDevFd < 0)
        return -1;
    // Mode
    if (ioctl(gDevFd, I2C_TENBIT, 0) < 0) {
        XPR_DRM_Fini();
        return -1;
    }
    // Slave address
    if (ioctl(gDevFd, I2C_SLAVE, gDevSlaveAddress >> 1) < 0) {
        XPR_DRM_Fini();
        return -1;
    }
    return 0;
}

void XPR_DRM_Fini(void)
{
    if (gDevFd > 0) {
        close(gDevFd);
        gDevFd = -1;
    }
}

int XPR_DRM_Verify(void)
{
    return alpucVerify();
}

int XPR_DRM_GetSerial(uint8_t* buffer, int* length)
{
    if (!buffer || !length || (length && *length <= XPR_DRM_SERIAL_SIZE)) {
        errno = EINVAL;
        return -1;
    }
    struct IDCodes ids;
    if (alpucReadIDCodes(&ids) < 0) {
        errno = EIO;
        return -1;
    }
    memcpy(buffer, ids.code4, XPR_DRM_SERIAL_SIZE);
    memcpy(buffer, ids.code3+2, XPR_DRM_SERIAL_SIZE-2);
    if (*length > XPR_DRM_SERIAL_SIZE)
        buffer[XPR_DRM_SERIAL_SIZE] = 0;
    *length = XPR_DRM_SERIAL_SIZE;
    return 0;
}

const char* XPR_DRM_GetSerialString(void)
{
    static char serialString[256];
    char tmp[256];
    int sl = sizeof(tmp);
    if (XPR_DRM_GetSerial(tmp, &sl) < 0)
        return -1;
    for (int i=0; i<sl; i++)
        snprintf(serialString+(i*2), sizeof(tmp) - (i*2), "%02hhX", tmp[i]);
    return serialString;
}

const char* XPR_DRM_GetUuidString(void)
{
    return XPR_DRM_UUID_STRING_NULL;
}

#include "aes_private_keys.i"
#define AES_KEY_PART_LEN 8

/**
 * Pick the aes key position 2,4,3,7 in seeds
 * @warning done change the position number
 * @param [out] key 保存获取的key
 * @param [in] seeds 输入的随机数用于取aes key.
 */
static void pickupAESKey(unsigned char* key, unsigned char* seeds)
{
    memcpy(key + AES_KEY_PART_LEN*0, aes_private_key_p0[seeds[2] & 0x0F], AES_KEY_PART_LEN);
    memcpy(key + AES_KEY_PART_LEN*1, aes_private_key_p1[seeds[4] & 0x0F], AES_KEY_PART_LEN);
    memcpy(key + AES_KEY_PART_LEN*2, aes_private_key_p2[seeds[3] & 0x0F], AES_KEY_PART_LEN);
    memcpy(key + AES_KEY_PART_LEN*3, aes_private_key_p3[seeds[7] & 0x0F], AES_KEY_PART_LEN);
}

static int setSerial(const uint8_t* data, int length)
{
    int fd = open("/proc/camera/serial", O_RDWR);
    if (fd <= 0)
        return -1;
    ssize_t size = write(fd, data, length);
    close(fd);
    return (size == length) ? 0 : -1;
}

int XPR_DRM_InstallSerial(const uint8_t* data, int length)
{
    uint8_t random[8] = {0};
    uint8_t buffer[40] = {0};
    uint8_t plaintext[16] = {0};
    uint8_t cipertext[16] = {0};
    uint8_t key[32] = {0};
    for(;;) {
        // Generate key random matrix
        if (rl_read_random(random, sizeof(random)) < 0)
            break;
        pickupAESKey(key, random);
        struct aes_ctx tfm = {0};   
        if (rl_aes_set_key(&tfm, key, sizeof(key)) < 0) 
            break;
        // Set serial code
        memcpy(plaintext, data, length);
        // Pad random data
        memcpy(plaintext+XPR_DRM_SERIAL_SIZE, random, sizeof(random));
        rl_aes_encrypt(&tfm, cipertext, plaintext, sizeof(plaintext));
        memcpy(buffer, random, sizeof(random));
        memcpy(buffer+sizeof(random), cipertext, sizeof(cipertext));
        rl_md5(plaintext, sizeof(plaintext), cipertext);
        memcpy(buffer+sizeof(random)+sizeof(cipertext), cipertext, sizeof(cipertext));
        // buffer 为 40 个字节的数据
        // 前 8 个字节为随机随
        // 中间 16 个字节为 aes256(硬件ID+随机数)
        // 后 16 个字节为 md5(硬件ID+随机数)
        //dump(data,    8, "Data  : ");
        //dump(buffer, 40, "Buffer: ");
        if (setSerial(buffer, sizeof(buffer)) < 0)
            break;
        return 0;
    }
    return -1;
}


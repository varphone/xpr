/* 
 * @Author lwx, lwx_me@163.com
 * @Date 2014-06-17 10:58:42
 *
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <rl/rl.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_drm.h>

// FIXME:
int XPR_DRM_Config(int request, const void* data, int size)
{
    return XPR_ERR_SUCCESS;
}

// FIXME:
int XPR_DRM_Init(void)
{
    return XPR_ERR_SUCCESS;
}

// FIXME:
void XPR_DRM_Fini(void)
{
}

// FIXME:
int XPR_DRM_Verify(void)
{
    return XPR_ERR_SUCCESS;
}


int XPR_DRM_GetSerial(uint8_t* buffer, int* length)
{
    int err = -1;
    int l = 0;
    long long int v = 0;
    char tmp[128] = {0};
    if (!buffer || !length || (length && *length <= XPR_DRM_SERIAL_SIZE)) {
        errno = EINVAL;
        return -1;
    }
    
    err = rl_mtd_get_sn(tmp, sizeof(tmp));
    if(err !=0 ) {
        errno = EINVAL;
        return -1;
    }
    l = strlen(tmp);
    if (l > 12)
        v = strtoll(tmp+(l-12), 0, 0);
    else
        v = strtoll(tmp, 0, 0);
    memcpy(buffer, &v, XPR_DRM_SERIAL_SIZE);
    if (*length > XPR_DRM_SERIAL_SIZE)
        buffer[XPR_DRM_SERIAL_SIZE] = 0;
    *length = XPR_DRM_SERIAL_SIZE;
    return err;
}

// FIXME:
const char* XPR_DRM_GetSerialString(void)
{
    static char serialString[256];
    char tmp[256];
    int sl = sizeof(tmp);
    if (XPR_DRM_GetSerial((uint8_t*)tmp, &sl) < 0)
        return NULL;
    for (int i=0; i<sl; i++)
        snprintf(serialString+(i*2), sizeof(tmp) - (i*2), "%02hhX", tmp[i]);
    return serialString;
}

// FIXME:
const char* XPR_DRM_GetUuidString(void)
{
    return 0;
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


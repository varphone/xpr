#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_drm.h>
#include <hi3518c/hi_unf_ecs.h>
#include <hi3518c/mpi_sys.h>
#include <hi3518c/hi_i2c.h>

static char* gDevFile = "/dev/hi_i2c";
static int gDevSlaveAddress = 0x0a;

HI_S32 Setconfiginfo(HI_HANDLE chnHandle)
{
    HI_U32 i;
    HI_UNF_CIPHER_CTRL_S cipherCtrl;
    HI_U32 cm[8] = {0};

    unsigned char key[16] = {0x3c, 0xdd, 0xa4, 0x11, 0xd9, 0xd4, 0x85, 0xac, 0xbc, 0x93, 0xd6, 0xee, 0x83, 0xb3, 0x2c, 0x89};
    cm[0] = (key[0] << 24)  | (key[1] << 16) | (key[2] << 8) | (key[3] );
    cm[1] = (key[4] << 24)  | (key[5] << 16) | (key[6] << 8) | (key[7] );
    cm[2] = (key[8] << 24)  | (key[9] << 16) | (key[10] << 8) | (key[11]);
    cm[3] = (key[12] << 24) | (key[13] << 16)| (key[14] << 8) | (key[15]);

    cipherCtrl.bKeyByCA = HI_FALSE;
    cipherCtrl.enAlg = HI_UNF_CIPHER_ALG_AES;
    cipherCtrl.enWorkMode = HI_UNF_CIPHER_WORK_MODE_ECB;
    cipherCtrl.enBitWidth = HI_UNF_CIPHER_BIT_WIDTH_8BIT;
    cipherCtrl.enKeyLen = HI_UNF_CIPHER_KEY_AES_128BIT;

    for (i = 0; i < 8; i++)
        cipherCtrl.u32Key[i] = cm[i];

    for (i = 0; i < 4; i++)
        cipherCtrl.u32IV[i] = cm[i];

    return HI_UNF_CIPHER_ConfigHandle(chnHandle, &cipherCtrl);
}

static int cipherAesEncrypt(void* inData, int inSize, void* outData, int outSize)
{
	return -1;
}

static int cipherAesDecrypt(void* inData, int inSize, void* outData, int outsize)
{
	HI_U32 result;
    HI_U32 u32ByteLength = 512;
    HI_U32 u32SrcPhyAddr;
    HI_U32 u32DestPhyAddr;
    HI_U8* u32SrcAddrVir;
    HI_U8* u32DestAddrVir;
    HI_HANDLE chnidDec;

	if(inSize > u32ByteLength)
		return -1;

	result = HI_UNF_CIPHER_Open();
	if ( result != HI_SUCCESS)
	    return  -1;

	result = HI_UNF_CIPHER_CreateHandle(&chnidDec);
	if ( result != HI_SUCCESS) {
	    HI_UNF_CIPHER_Close();
		return -1;
	}	

	/*************************************************************/
	result = HI_MPI_SYS_MmzAlloc(&u32SrcPhyAddr, (HI_VOID **)&u32SrcAddrVir, "cipherSource", NULL, u32ByteLength);
	if ( result != HI_SUCCESS) {
		HI_UNF_CIPHER_DestroyHandle(chnidDec);
		HI_UNF_CIPHER_Close();
	    return -1;
	}		

	result = HI_MPI_SYS_MmzAlloc(&u32DestPhyAddr, (HI_VOID **)&u32DestAddrVir, "cipherDest" ,NULL, u32ByteLength);
	if ( result != HI_SUCCESS) {
	    HI_MPI_SYS_MmzFree(u32SrcPhyAddr, u32SrcAddrVir);
		HI_UNF_CIPHER_DestroyHandle(chnidDec);
		HI_UNF_CIPHER_Close();
	    return -1;
	}

	for(;;) {
		/*************************************************************/
		memset(u32SrcAddrVir, 'O', u32ByteLength);
		memset(u32DestAddrVir, 'R', u32ByteLength);
		memcpy(u32SrcAddrVir, inData, inSize);

		result = Setconfiginfo(chnidDec);
		if(result != HI_SUCCESS) {
			result = -1;
			break;
		}	

		result = HI_UNF_CIPHER_Decrypt(chnidDec, u32SrcPhyAddr, u32DestPhyAddr, u32ByteLength);
		if ( result != HI_SUCCESS) {
			result = -1;
			break;
		}	
		memcpy(outData, u32DestAddrVir, outsize);
		result = 0;
		break;
	}

    HI_MPI_SYS_MmzFree(u32SrcPhyAddr, u32SrcAddrVir);
	HI_MPI_SYS_MmzFree(u32DestPhyAddr, u32DestAddrVir);
    HI_UNF_CIPHER_DestroyHandle(chnidDec);
    HI_UNF_CIPHER_Close();
    return  result;
}

static int i2cReadData(uint8_t regAddr, uint8_t* buffer, int len)
{
	int fd, reg_addr;
	I2C_DATA_S i2c_data;

	if(!buffer || len >256) {
		printf("buffer is NULL or len is bigger then 256\n");
		return -1;
	}

	fd = open(gDevFile, O_RDWR);
	if(fd <0) {
		perror("open /dev/hi_i2c:");
		return -1;
	}

	i2c_data.dev_addr = 0xa0;
    i2c_data.addr_byte_num = 1; 
    i2c_data.data_byte_num = 1;
	for(reg_addr =0; reg_addr < len; reg_addr++) {
        i2c_data.reg_addr = reg_addr; 
        ioctl(fd, CMD_I2C_READ, &i2c_data);
        buffer[reg_addr] =  i2c_data.data & 0xff;		
	}
	close(fd);
	return 0;
}


// FIXME:
int XPR_DRM_Config(int request, const void* data, int size)
{
    return XPR_ERR_ERROR;
}

// FIXME:
int XPR_DRM_Init(void)
{
    return XPR_ERR_ERROR;
}

// FIXME:
int  XPR_DRM_Fini(void)
{
	return XPR_ERR_ERROR;
}

// FIXME:
int XPR_DRM_Verify(void)
{
    return XPR_ERR_ERROR;
}

// FIXME:
int XPR_DRM_GetSerial(uint8_t* buffer, int* length)
{
    return XPR_ERR_ERROR;
}

// FIXME:
const char* XPR_DRM_GetSerialString(void)
{
    return XPR_DRM_SERIAL_STRING_NULL;
}

// FIXME:
const char* XPR_DRM_GetUuidString(void)
{
    return XPR_DRM_UUID_STRING_NULL;
}

// FIXME:
int XPR_DRM_InstallSerial(const uint8_t* data, int length)
{
    return XPR_ERR_ERROR;
}

int XPR_DRM_GetRawData(uint8_t* data, int *size)
{
	uint8_t buffer[256];
	int len;
	
	if(!data || !size)
		return XPR_ERR_NULL_PTR;

	len = sizeof(buffer);
	if(*size < len)
		return XPR_ERR_NOBUF;

	if(i2cReadData(0, buffer, sizeof(buffer))!=0)
		return XPR_ERR_ERROR;

	printf("i2c read ok...\n");

	if(cipherAesDecrypt(buffer, len, data, len)!=0)
		return XPR_ERR_ERROR;

	*size = len;
	return XPR_ERR_SUCCESS;
}

#ifndef XPR_MCDEC_H
#define XPR_MCDEC_H

#include <stdint.h>
#include "xpr_avframe.h"
#include "xpr_streamblock.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// PORT �Ľ���
//
// ��������в������ǻ��� PORT ��ʵ��, PORT ����˼�Ƕ˿�, �ڱ����д�����Ŀ�ꡢͨ���ĺ���
// PORT �� 2 ���ֹ��ɣ�
//     major   ��Ŀ��, �������� ������ ��
//     minor   ��Ŀ��, ����ָ�� ������ �����ض�Ŀ��
//
// ���磺
//     PORT = 0x00010000 = ��һ�� ������ ��
//     PORT = 0x00010001 = ��һ�� ������ ���е�һ���˿�
//
// ÿ���ֶ��� 3 ��ֵ�������⺬���:
//     0    ��ʾ�˲��ֱ�����
//     M    ��ʾ����Ŀ��
//     M-1  ��ʾ����Ŀ���е���һ
// * (M) ��ÿ���ֵ����ֵ, ���� major �� 16 ��λ, �����ֵ�� 0xFFFF
//
//==============================================================================

///
/// �ϳ� ������ �˿ں�
///
#define XPR_MCDEC_PORT(major, minor)     (((major)<<16)|(minor))

///
/// ��ȡ ������ �˿ں�����Ŀ��
///
#define XPR_MCDEC_PORT_MAJOR(port)       (((port)>>16) & 0xffff)

///
/// ��ȡ ������ �˿ں��д�Ŀ��
///
#define XPR_MCDEC_PORT_MINOR(port)       ((port) & 0xffff)

#define XPR_MCDEC_PORT_MAJOR_ALL         0xffff
#define XPR_MCDEC_PORT_MAJOR_ANY         0xfffe
#define XPR_MCDEC_PORT_MAJOR_DEF         0x0001
#define XPR_MCDEC_PORT_MAJOR_NUL         0x0000
#define XPR_MCDEC_PORT_MAJOR_MIN         0x0001
#define XPR_MCDEC_PORT_MAJOR_MAX         0xfffd

///
/// ͨ�����ݽ�������
///
#define XPR_MCDEC_PORT_MAJOR_GEN         0x0001

#define XPR_MCDEC_PORT_MINOR_ALL         0xffff
#define XPR_MCDEC_PORT_MINOR_ANY         0xfffe
#define XPR_MCDEC_PORT_MINOR_DEF         0x0001
#define XPR_MCDEC_PORT_MINOR_NUL         0x0000
#define XPR_MCDEC_PORT_MINOR_MIN         0x0001
#define XPR_MCDEC_PORT_MINOR_MAX         0xfffd

///
/// ����˿ڣ�BMP ��ʽ������
///
#define XPR_MCDEC_SPEC_BMPENC_PORT      (0x00000001)

///
/// ����˿ڣ�JPG ��ʽ������
///
#define XPR_MCDEC_SPEC_JPGENC_PORT      (0x00000002)

///
/// ����˿ڣ�PNG ��ʽ������
///
#define XPR_MCDEC_SPEC_PNGENC_PORT      (0x00000003)

///
/// �����˿ڿ����ɵ����ص���������
///
#define XPR_MCDEC_MAX_HANDLERS          8

///
/// ��/��Ƶԭʼ֡���ݻص�����
///
/// @param [in] opaque		�û���������
/// @param [in] port		�����ص��Ķ˿�
/// @param [in] avf			��/��Ƶԭʼ֡����
/// return Ŀǰ���Է���ֵ
typedef int (*XPR_MCDEC_AVF_FXN)(void* opaque, int port, const XPR_AVFrame* avf);

///
/// ��/��Ƶ����֡���ݻص�����
///
/// @param [in] opaque		�û���������
/// @param [in] port		�����ص��Ķ˿�
/// @param [in] stb			��/��Ƶ����֡����
/// return Ŀǰ���Է���ֵ
typedef int (*XPR_MCDEC_STB_FXN)(void* opaque, int port, const XPR_StreamBlock* stb);

///
/// Print no output.
///
#define XPR_MCDEC_LOG_LEVEL_QUIET    -8

///
/// Something went really wrong and we will crash now.
///
#define XPR_MCDEC_LOG_LEVEL_PANIC     0

///
/// Something went wrong and recovery is not possible.
/// For example, no header was found for a format which depends
/// on headers or an illegal combination of parameters is used.
///
#define XPR_MCDEC_LOG_LEVEL_FATAL     8

///
/// Something went wrong and cannot losslessly be recovered.
/// However, not all future data is affected.
///
#define XPR_MCDEC_LOG_LEVEL_ERROR    16

///
/// Something somehow does not look correct. This may or may not
/// lead to problems. An example would be the use of '-vstrict -2'.
///
#define XPR_MCDEC_LOG_LEVEL_WARNING  24

///
/// Standard information.
///
#define XPR_MCDEC_LOG_LEVEL_INFO     32

///
/// Detailed information.
///
#define XPR_MCDEC_LOG_LEVEL_VERBOSE  40

///
/// Stuff which is only useful for libav* developers.
///
#define XPR_MCDEC_LOG_LEVEL_DEBUG    48

typedef enum XPR_MCDEC_CFG {
	XPR_MCDEC_CFG_C4WGA,
	XPR_MCDEC_CFG_LOG_LEVEL,		///< ��־��ӡ�ȼ�, ��������: int, ֵ: �� XPR_MCDEC_LOG_LEVEL_xxx 
    XPR_MCDEC_CFG_MAX,
} XPR_MCDEC_CFG;

typedef enum XPR_MCDEC_FLAG {
	XPR_MCDEC_FLAG_BLKED = 0x00000001,
	XPR_MCDEC_FLAG_CLOSE = 0x00000002,
	XPR_MCDEC_FLAG_PNDNG = 0x00000004,
	XPR_MCDEC_FLAG_SYNRQ = 0x00000008,
} XPR_MCDEC_FLAG;

typedef enum XPR_MCDEC_PARAM {
	XPR_MCDEC_PARAM_NULL,
	XPR_MCDEC_PARAM_BLOCK,			///< data: int, value: 0 = unblock, 1 = block
	XPR_MCDEC_PARAM_DEINTERLANCE,	///< data: int, value: 0 = off, 1 = odd, 2 = even
	XPR_MCDEC_PARAM_SKIP_FRAME,		///< data: int, value: 0 = none, 1 = B, 2 = P, 3 = BP, 4 = I, 5 = BI, 6 = PI, 7 = BPI
	XPR_MCDEC_PARAM_FORCE_FOURCC,	///< data: uint32_t, value: FourCC
    XPR_MCDEC_PARAM_IMAGE_WIDTH,
    XPR_MCDEC_PARAM_IMAGE_HEIGHT,
    XPR_MCDEC_PARAM_IMAGE_QUALITY,
	XPR_MCDEC_PARAM_MAX,
} XPR_MCDEC_PARAM;

///
/// ����������ͨ��������
///
/// @param [in] cfg         ����ѡ�����, �μ� [#XPR_MCDEC_CFG]
/// @param [in] data        ��������
/// @param [in] size        �������ݳ���, NULL ��β���ִ������ݿ���Ϊ -1
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
int XPR_MCDEC_Config(XPR_MCDEC_CFG cfg, const void* data, int size);

///
/// ��ʼ��������ͨ��������
///
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
int XPR_MCDEC_Init(void);

///
/// �ͷ�������ͨ��������
///
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
int XPR_MCDEC_Fini(void);

///
/// ������ϴ���뻺����
///
/// @param [in] port        Ҫ�����Ķ˿�
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
int XPR_MCDEC_Flush(int port);

///
/// ����ָ���˿ڲ��ͷ����������һ����Դ
///
/// @param [in] port        Ҫ�����Ķ˿�
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
int XPR_MCDEC_Reset(int port);

/// ��ָ���˿�����һ֡ԭʼ��/��Ƶ֡����
///
/// @param [in] port        Ҫ�����Ķ˿�
/// @param [in] avf         ��/��Ƶԭʼ֡����
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
int XPR_MCDEC_PushAVFrame(int port, const XPR_AVFrame* avf);

///
/// ��ָ���˿�����һ֡ԭʼ��/��Ƶ��������
///
/// @param [in] port        Ҫ�����Ķ˿�
/// @param [in] stb         ��/��Ƶ��������
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
int XPR_MCDEC_PushStreamBlock(int port, const XPR_StreamBlock* stb);

///
/// ��ָ���˿�����û��������װ������
///
/// @param [in] port        Ҫ�����Ķ˿�
/// @param [in] data        Ҫ���������, ���ʽ��Ҫʹ�� XPR_MCDEC_SetParam() ����Э��
/// @param [in] length      Ҫ��������ݵ��ֽ���
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
int XPR_MCDEC_PushData(int port, const uint8_t* data, int length);

///
/// �ַ�����������
/// @param [in] port        Ҫ�����Ķ˿�
/// @param [in] avf         ��/��Ƶԭʼ֡����
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
void XPR_MCDEC_DeliverAVFrame(int port, const XPR_AVFrame* avf);

///
/// �ַ�����/ת��������
/// @param [in] port        Ҫ�����Ķ˿�
/// @param [in] stb         ��/��Ƶ��������
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
void XPR_MCDEC_DeliverStreamBlock(int port, const XPR_StreamBlock* stb);

///
/// ��ָ���˿���Ӵ�����/��Ƶԭʼ֡���ݵĻص�����
///
/// @param [in] port        Ҫ�����Ķ˿�
/// @param [in] handler     �ص�����
/// @param [in] opaque      �ù�������
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
/// @note �˽ӿڶ˿ڿ���Ϊ�������������������е��ض�Ŀ��
///       ���˿�Ϊ �������� ʱ, ���������е�����Ŀ�궼����ô˻ص�����
int XPR_MCDEC_AddAVFrameHandler(int port, XPR_MCDEC_AVF_FXN handler, void* opaque);

int XPR_MCDEC_DelAVFrameHandler(int port, XPR_MCDEC_AVF_FXN handler, void* opaque);

///
/// ��ָ���˿���Ӵ�����/��Ƶ�������ݵĻص�����
///
/// @param [in] port        Ҫ�����Ķ˿�
/// @param [in] handler     �ص�����
/// @param [in] opaque      �ù�������
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
/// @note �˽ӿڶ˿ڿ���Ϊ�������������������е��ض�Ŀ��
///       ���˿�Ϊ �������� ʱ, ���������е�����Ŀ�궼����ô˻ص�����
int XPR_MCDEC_AddStreamBlockHandler(int port, XPR_MCDEC_STB_FXN handler, void* opaque);

int XPR_MCDEC_DelStreamBlockHandler(int port, XPR_MCDEC_STB_FXN handler, void* opaque);

///
/// ����ָ���˿ڵĲ���
/// @param [in] port        Ҫ�����Ķ˿�
/// @param [in] param       Ҫ���õĲ������, �μ� [#XPR_MCDEC_Param]
/// @param [in] data        Ҫ���õĲ���ֵ/����
/// @param [in] size        Ҫ���õĲ���ֵ/�����ֽ���
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
int XPR_MCDEC_SetParam(int port, XPR_MCDEC_PARAM param, const void* data, int size);

///
/// ��ȡָ���˿ڵĲ���
/// @param [in] port        Ҫ�����Ķ˿�
/// @param [in] param       Ҫ��ȡ�Ĳ������, �μ� [#XPR_MCDEC_Param]
/// @param [in] data        Ҫ��ȡ�Ĳ���ֵ/���ݽ��ջ�����
/// @param [in,out] size    Ҫ��ȡ�Ĳ���ֵ/���ݽ��ջ������ֽ���, ������ʵ�����ݳ���
/// @retval XPR_ERR_OK      �ɹ�
/// @retval XPR_ERR_ERROR   ʧ��
int XPR_MCDEC_GetParam(int port, XPR_MCDEC_PARAM param, void* data, int* size);

#ifdef __cplusplus
}
#endif

#endif // XPR_MCDEC_H


#ifndef XPR_RTSP_IDD_H
#define XPR_RTSP_IDD_H

#include <stdint.h>

/// @defgroup xpr_rtsp_idd RTSP �������ݷ�����
/// @brief ͨ��RTSP �������ݷ�����
/// @{
///

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_RTSP_IDD_MAX_BUFFER_SIZE    8196

#ifndef XPR_RTSP_IDD_TYPE_DEFINED
#define XPR_RTSP_IDD_TYPE_DEFINED
struct XPR_RTSP_IDD;
typedef struct XPR_RTSP_IDD XPR_RTSP_IDD;
#endif // XPR_RTSP_IDD_TYPE_DEFINED

#ifndef XPR_RTSP_IDD_DATA_TYPE_TYPE_DEFINED
#define XPR_RTSP_IDD_DATA_TYPE_TYPE_DEFINED
///
/// RTSP �ѷ�����������
///
typedef enum XPR_RTSP_IDD_DataType {
    XPR_RTSP_IDD_DATA_TYPE_BIN,     ///< ����������, ����: rtp, rtcp ..
    XPR_RTSP_IDD_DATA_TYPE_HDR,     ///< RTSP �������Ӧͷ
    XPR_RTSP_IDD_DATA_TYPE_SDP,     ///< SDP ����
} XPR_RTSP_IDD_DataType;
#endif //XPR_RTSP_IDD_DATA_TYPE_TYPE_DEFINED

///
/// RTSP �������ݷ������ѷ������ݴ�����
///
/// @param [in] opaque      �û���������
/// @param [in] channel     ��������ͨ��
/// @param [in] dataType    ��������
/// @param [in] data        �ѷ�������
/// @param [in] length      �ѷ��������ֽ���
/// @retval XPR_ERR_OK      ����ɹ�
/// @retval XPR_ERR_ERROR   ����ʧ��
typedef int (*XPR_RTSP_IDD_DataHandler)(void* opaque, int channel, int dataType,
                                        uint8_t* data, int length);

#ifndef XPR_RTSP_IDD_PARAM_TYPE_TYPE_DEFINED
#define XPR_RTSP_IDD_PARAM_TYPE_TYPE_DEFINED
///
/// RTSP �������ݷ�������������
///
typedef enum XPR_RTSP_IDD_ParamType {
    XPR_RTSP_IDD_PARAM_DATA_HANDLER,    ///< ���ݴ���ص�����
    XPR_RTSP_IDD_PARAM_OPAQUE,          ///< �û���������
} XPR_RTSP_IDD_ParamType;
#endif // XPR_RTSP_IDD_PARAM_TYPE_TYPE_DEFINED

///
/// ����һ�� RTSP �������ݷ�����
///
/// @retval NULL    ����ʧ��
/// @retval Other   �Ѵ����õ����ݷ��������
XPR_RTSP_IDD* XPR_RTSP_IDD_New(void);

///
/// ����һ���Ѵ����� RTSP �������ݷ�����
///
int XPR_RTSP_IDD_Destroy(XPR_RTSP_IDD* idd);

int XPR_RTSP_IDD_PushData(XPR_RTSP_IDD* idd, const uint8_t* data, int length);

int XPR_RTSP_IDD_SetParam(XPR_RTSP_IDD* idd, int param, const void* data, int length);

int XPR_RTSP_IDD_GetParam(XPR_RTSP_IDD* idd, int param, void* buffer, int* size);

#ifdef __cplusplus
}
#endif

/// @}
///

#endif // XPR_RTSP_IDD_H

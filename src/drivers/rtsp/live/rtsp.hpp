#ifndef XPR_RTSP_DRIVER_LIVE_RTSP_HPP
#define XPR_RTSP_DRIVER_LIVE_RTSP_HPP

#include <string>
#include <vector>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_utils.h>

#define XPR_RTSP_MAX_CALLBACKS      4
//#define XPR_RTSP_MAX_CLIENTS        256
//#define XPR_RTSP_MAX_STREAMS        256
#define XPR_RTSP_MAX_WORKERS        8
#define XPR_RTSP_ACT_WORKERS        4
#define XPR_RTSP_TMO_US             30000000
#define XPR_RTSP_LVI_US             30000000

#define H264_FLAG_ADD_AUD           0x00000001
#define H264_FLAG_SINGLE_FRAME      0x00000002
#define H264_FLAG_START_CODE        0x00000004

/// 回调函数集
typedef struct Callback {
    XPR_RTSP_DCB            dcb;            ///< 数据回调
    void*                   dcb_opaque;     ///< 数据回调关联数据
    XPR_RTSP_EVCB           evcb;           ///< 事件回调
    void*                   evcb_opaque;    ///< 事件回调关联数据
#ifdef HAVE_XPR_RTSP_XD_STREAM_API
    XD_StreamDataCallback   dcb_xd;         ///< 数据回调 (XD)
    void*                   dcb_opaque_xd;  ///< 数据回调关联数据 (XD)
    XD_StreamEventCallback  evcb_xd;        ///< 事件回调 (XD)
    void*                   evcb_opaque_xd; ///< 事件回调关联数据 (XD)
#endif
} Callback;

/// 端口上下文
typedef struct PortContext {
    int                 usr_flags;      ///< 用户设定标志
    int                 act_flags;      ///< 当前生效标志
    int64_t             lats;           ///< 最后活动时间
    int64_t             lvts;           ///< 最后心跳时间
    uint64_t            total_frames;   ///< 传输帧数统计
    char                url[1024];      ///< 目标地址
    char                username[64];   ///< 用于认证的用户名称
    char                password[64];   ///< 用于认证的用户密码
    Callback            cbs[XPR_RTSP_MAX_CALLBACKS];
    void*               rtsp_priv;
    int                 out_fourcc;     ///< RTSP 客户端输出数据格式
#ifdef HAVE_XPR_RTSP_HKMI
    HIK_MEDIAINFO       out_pes_hkmi;   ///< 海康 PES 头信息
#endif

    XPR_RTSP_TRSPEC     trspec;         ///< RTSP 客户端传输方式

    uint32_t h264_flags;

#ifdef HAVE_XPR_RTSP_PES
    XPR_PES*            out_pes;
    int                 out_pes_port;
    XPR_StreamBlock*    out_pes_stb;
#endif

} PortContext;

typedef struct XPR_RTSP {
    int exit_loop;
    PortContext client_ports[XPR_RTSP_PORT_MINOR_MAX + 2];
    PortContext stream_ports[XPR_RTSP_PORT_MINOR_MAX + 2];
    XPR_Thread* threads[XPR_RTSP_MAX_WORKERS];
} XPR_RTSP;

/// 获取可用的客户端端口索引
int XPR_RTSP_GetAvailClientPortIndex(void);

/// 获取可用的服务流端口索引
int XPR_RTSP_GetAvailStreamPortIndex(void);

/// 更新端口的最后活动时间
void XPR_RTSP_UpdatePortLATS(int port);

/// 更新端口的最后心跳时间
void XPR_RTSP_UpdatePortLVTS(int port);

namespace xpr
{

namespace rtsp
{

// 前置声明
class Port;
class PortManager;

namespace PortFlags
{
const uint32_t PORT_FLAG_NULL = 0x00000000;
const uint32_t PORT_FLAG_OPEN = 0x00000001;
const uint32_t PORT_FLAG_CLOSE = 0x00000002;
const uint32_t PORT_FLAG_START = 0x00000004;
const uint32_t PORT_FLAG_STOP = 0x00000008;
const uint32_t PORT_FLAG_FLUSHING = 0x00000010;
const uint32_t PORT_FLAG_PENDING = 0x00000020;
const uint32_t PORT_FLAG_PLAYING = 0x00000040;
};

enum StateTransition {
    // Normal flows
    XPR_RTSP_STATE_NULL_OPEN,
    XPR_RTSP_STATE_OPEN_START,
    XPR_RTSP_STATE_START_PLAYING,
    XPR_RTSP_STATE_PLAYING_STOP,
    XPR_RTSP_STATE_STOP_CLOSE,
    XPR_RTSP_STATE_CLOSE_NULL,
    // Abnormal flows
    XPR_RTSP_STATE_OPEN_CLOSE,
    XPR_RTSP_STATE_STOP_START,
    XPR_RTSP_STATE_PLAYING_TIMEOUT,
    XPR_RTSP_STATE_START_STOP,
    XPR_RTSP_STATE_START_TIMEOUT,
};

enum TaskId {
    XPR_RTSP_TASK_NULL,
    XPR_RTSP_TASK_OPEN,
    XPR_RTSP_TASK_CLOSE,
    XPR_RTSP_TASK_START,
    XPR_RTSP_TASK_STOP,
};

class Port
{
public:
    Port(int id, Port* parent = NULL)
        : mId(id)
        , mParent(parent)
        , mActiveFlags(PortFlags::PORT_FLAG_NULL)
        , mUserFlags(PortFlags::PORT_FLAG_NULL)
        , mUrl()
        , mUsername()
        , mPassword()
    {
        // Nothing TODO
    }

    virtual ~Port(void)
    {
    }

    virtual int id(void) const
    {
        return mId;
    }

    virtual Port* parent(void) const
    {
        return mParent;
    }

    virtual uint32_t& activeFlags()
    {
        return mActiveFlags;
    }

    virtual uint32_t& activeFlags(uint32_t replaced)
    {
        mActiveFlags = replaced;
        return mActiveFlags;
    }

    virtual uint32_t& activeFlags(uint32_t added, uint32_t removed)
    {
        mActiveFlags |= added;
        mActiveFlags &= ~removed;
        return mActiveFlags;
    }

    virtual uint32_t& userFlags()
    {
        return mUserFlags;
    }

    virtual uint32_t& userFlags(uint32_t replaced)
    {
        mUserFlags = replaced;
        return mUserFlags;
    }

    virtual uint32_t& userFlags(uint32_t added, uint32_t removed)
    {
        mUserFlags |= added;
        mUserFlags &= ~removed;
        return mUserFlags;
    }

    virtual std::string& url()
    {
        return mUrl;
    }

    virtual std::string& url(const char* url)
    {
        mUrl = url;
        return mUrl;
    }

    virtual int isPortValid(int port)
    {
        return XPR_FALSE;
    }

    virtual int open(int port, const char* url)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    virtual int close(int port)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    virtual int start(int port)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    virtual int stop(int port)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    virtual int addDataCallback(int port, XPR_RTSP_DCB cb, void* opaque)
    {
        Port* p = getPort(port);
        if (p) {
            for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
                Callback* ncb = &p->mCallbacks[i];
                if (ncb->dcb)
                    continue;
                ncb->dcb = cb;
                ncb->dcb_opaque = opaque;
                return XPR_ERR_OK;
            }
        }
        return XPR_ERR_GEN_UNEXIST;
    }

    virtual int delDataCallback(int port, XPR_RTSP_DCB cb, void* opaque)
    {
        Port* p = getPort(port);
        if (p) {
            for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
                Callback* ncb = &p->mCallbacks[i];
                if (ncb->dcb == cb && ncb->dcb_opaque == opaque) {
                    ncb->dcb = 0;
                    ncb->dcb_opaque = 0;
                    return XPR_ERR_OK;
                }
            }
        }
        return XPR_ERR_GEN_UNEXIST;
    }

    virtual int addEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque)
    {
        Port* pc = getPort(port);
        if (pc) {
            for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
                Callback* ncb = &pc->mCallbacks[i];
                if (ncb->evcb)
                    continue;
                ncb->evcb = cb;
                ncb->evcb_opaque = opaque;
                return XPR_ERR_OK;
            }
        }
        return XPR_ERR_GEN_UNEXIST;
    }

    virtual int delEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque)
    {
        Port* p = getPort(port);
        if (p) {
            for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
                Callback* ncb = &p->mCallbacks[i];
                if (ncb->evcb == cb && ncb->evcb_opaque == opaque) {
                    ncb->evcb = NULL;
                    ncb->evcb_opaque = NULL;
                    return XPR_ERR_OK;
                }
            }
        }
        return XPR_ERR_GEN_UNEXIST;
    }

    virtual int setAuth(int port, const char* username, const char* password,
                        int pwdIsMD5)
    {
        mUsername = username;
        mPassword = password;
        mPwdIsMD5 = pwdIsMD5;
        return XPR_ERR_OK;
    }

    virtual int setOutputFormat(int port, uint32_t fourcc)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    virtual int setTrSpec(int port, XPR_RTSP_TRSPEC trspec)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    virtual int pushData(int port, XPR_StreamBlock* stb)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    virtual int postData(int port, XPR_StreamBlock* stb)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    virtual int postEvent(int port, const XPR_RTSP_EVD* evd)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    virtual int getParam(int port, XPR_RTSP_PARAM param, void* buffer, int* size)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    virtual int setParam(int port, XPR_RTSP_PARAM param, const void* data,
                         int length)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    /// @brief 获取端口上下文
    /// @param [in] port        端口句柄
    /// return 成功返回端口的指针，失败返回 NULL
    virtual Port* getPort(int port)
    {
        return NULL;
    }

    /// @brief 获取端口上下文
    /// @param [in] port        端口句柄
    /// return 成功返回端口的指针，失败返回 NULL
    virtual Port* getMajorPort(int port)
    {
        return NULL;
    }

    /// @brief 获取端口上下文
    /// @param [in] port        端口句柄
    /// return 成功返回端口的指针，失败返回 NULL
    virtual Port* getMinorPort(int port) const
    {
        return NULL;
    }

    /// @brief 执行任务
    /// @param [in] port        端口句柄
    /// @param [in] task        任务标识
    virtual int runTask(int port, TaskId task)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    /// 处理状态变更事件
    /// @param [in] port        端口句柄
    /// @param [in] transition  状态变更
    virtual int stateChanged(int port, StateTransition transition)
    {
        return XPR_ERR_GEN_NOT_SUPPORT;
    }

    /// 获取可用的流端口编号
    /// @param [in] majorPort       主端口
    static int getAvailStreamId(int majorPort);

    /// 获取可用的流轨道端口编号
    /// @param [in] streamPort      流端口
    static int getAvailStreamTrackId(int streamPort);

protected:
    // 清理所有回调
    void clearCallbacks(void);

protected:
    int             mId;
    Port*           mParent;
    uint32_t        mActiveFlags;
    uint32_t        mUserFlags;
    int64_t         mLastActiveTS;
    int64_t         mLastLivenessTS;
    uint32_t        mOutputFourCC;
    XPR_RTSP_TRSPEC mTrSpec;
    uint64_t        mTotalFrames;
    std::string     mUrl;
    std::string     mUsername;
    std::string     mPassword;
    int             mPwdIsMD5;
    Callback        mCallbacks[XPR_RTSP_MAX_CALLBACKS];
};

// 端口管理器
class PortManager : public Port
{
public:
    PortManager(void);
    virtual ~PortManager(void);

    // Port interfaces
    virtual int isPortValid(int port);
    virtual int open(int port, const char* url);
    virtual int close(int port);
    virtual int start(int port);
    virtual int stop(int port);

    virtual int pushData(int port, XPR_StreamBlock* stb);

    virtual Port* getPort(int port);
    virtual Port* getMajorPort(int port);
    virtual Port* getMinorPort(int port);

private:
#if defined(HAVE_XPR_RTSP_CLIENT)
    int setupConnectionManager(const char* url);
    int clearConnectionManager(void);
#endif
#if defined(HAVE_XPR_RTSP_SERVER)
    int setupServer(const char* url);
    int clearServer(void);
#endif

private:
    Port*       mMajorPorts[XPR_RTSP_PORT_MAJOR_ALL + 1];
};

// Convert string to bool
//  - true = "1", "true", "TRUE", "y", "yes"
//  - false = "0", "false", "FALSE", "n", "no"
bool str2bool(const char* s);

} // namespace xpr::rtsp

} // namespace xpr


#endif // XPR_RTSP_DRIVER_LIVE_RTSP_HPP

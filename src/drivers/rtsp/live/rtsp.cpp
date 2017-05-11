#if defined(HAVE_XPR_RTSP_DRIVER_LIVE) && !defined(HAVE_XPR_RTSP_CLIENT1)

#if defined(_MSC_VER)
#  if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "BasicUsageEnvironment-mt-s-vc140d.lib")
#pragma comment(lib, "groupsock-mt-s-vc140d.lib")
#pragma comment(lib, "liveMedia-mt-s-vc140d.lib")
#pragma comment(lib, "UsageEnvironment-mt-s-vc140d.lib")
#  else
#pragma comment(lib, "BasicUsageEnvironment-mt-s-vc140.lib")
#pragma comment(lib, "groupsock-mt-s-vc140.lib")
#pragma comment(lib, "liveMedia-mt-s-vc140.lib")
#pragma comment(lib, "UsageEnvironment-mt-s-vc140.lib")
#  endif // defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "ws2_32.lib")
#endif // defined(_MSC_VER)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_streamblock.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_pes.h>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_sys.h>
#include <xpr/xpr_url.h>
#include <live/GroupsockHelper.hh>

#include "rtsp.hpp"

// 端口主端口槽位
static xpr::rtsp::PortManager*  rtsp = NULL;

#if defined(HAVE_XPR_RTSP_CLIENT)
#include "rtsp_client.hpp"
#endif // defined(HAVE_XPR_RTSP_CLIENT)

#if defined(HAVE_XPR_RTSP_SERVER)
#include "rtsp_server.hpp"
#endif // defined(HAVE_XPR_RTSP_SERVER)

// Internal interfaces
//============================================================================
int xpr::rtsp::Port::getAvailStreamId(int majorPort)
{
    return 0;
}

int xpr::rtsp::Port::getAvailStreamTrackId(int streamPort)
{
    return 0;
}

// PortManager
//============================================================================
xpr::rtsp::PortManager::PortManager(void)
    : Port(0, NULL)
{
    memset(mMajorPorts, 0, sizeof(mMajorPorts));
}

xpr::rtsp::PortManager::~PortManager(void)
{
}

int xpr::rtsp::PortManager::isPortValid(int port)
{
    int major = XPR_RTSP_PORT_MAJOR(port);
    int minor = XPR_RTSP_PORT_MINOR(port);
    if (major == XPR_RTSP_PORT_MAJOR_CLI ||
        major == XPR_RTSP_PORT_MAJOR_SVR) {
        if (minor == XPR_RTSP_PORT_MINOR_NUL ||
            minor == XPR_RTSP_PORT_MINOR_ALL ||
            minor == XPR_RTSP_PORT_MINOR_ANY) {
            return XPR_TRUE;
        }
        xpr::rtsp::Port* p = getMajorPort(port);
        return p ? p->isPortValid(port) : XPR_FALSE;
    }
    return XPR_FALSE;
}

int xpr::rtsp::PortManager::open(int port, const char* url)
{
    if (isPortValid(port) == XPR_FALSE || !url) {
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    }
    int major = XPR_RTSP_PORT_MAJOR(port);
    int minor = XPR_RTSP_PORT_MINOR(port);
#if defined(HAVE_XPR_RTSP_CLIENT)
    if (major == XPR_RTSP_PORT_MAJOR_SVR &&
        minor == XPR_RTSP_PORT_MINOR_NUL)
        return setupClient(url);
#endif
#if defined(HAVE_XPR_RTSP_SERVER)
    if (major == XPR_RTSP_PORT_MAJOR_SVR &&
        minor == XPR_RTSP_PORT_MINOR_NUL)
        return setupServer(url);
#endif // defined(HAVE_XPR_RTSP_SERVER)
    Port* p = getMajorPort(port);
    return p ? p->open(port, url) : XPR_ERR_GEN_SYS_NOTREADY;
}

int xpr::rtsp::PortManager::close(int port)
{
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    int major = XPR_RTSP_PORT_MAJOR(port);
    int minor = XPR_RTSP_PORT_MINOR(port);
#if defined(HAVE_XPR_RTSP_CLIENT)
    if (major == XPR_RTSP_PORT_MAJOR_SVR &&
        minor == XPR_RTSP_PORT_MINOR_NUL)
        return clearClient(url);
#endif
#if defined(HAVE_XPR_RTSP_SERVER)
    if (major == XPR_RTSP_PORT_MAJOR_SVR &&
        minor == XPR_RTSP_PORT_MINOR_NUL)
        return clearServer();
#endif // defined(HAVE_XPR_RTSP_SERVER)
    Port* p = getMajorPort(port);
    return p ? p->close(port) : XPR_ERR_GEN_SYS_NOTREADY;
}

int xpr::rtsp::PortManager::start(int port)
{
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    Port* p = getMajorPort(port);
    return p ? p->start(port) : XPR_ERR_GEN_SYS_NOTREADY;
}

int xpr::rtsp::PortManager::stop(int port)
{
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    Port* p = getMajorPort(port);
    return p ? p->stop(port) : XPR_ERR_GEN_SYS_NOTREADY;
}

int xpr::rtsp::PortManager::pushData(int port, XPR_StreamBlock* stb)
{
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    Port* p = getMajorPort(port);
    return p ? p->pushData(port, stb) : XPR_ERR_GEN_SYS_NOTREADY;
}

xpr::rtsp::Port* xpr::rtsp::PortManager::getPort(int port)
{
    int major = XPR_RTSP_PORT_MAJOR(port);
    int minor = XPR_RTSP_PORT_MINOR(port);
    if (minor == 0)
        return mMajorPorts[major];
    return mMajorPorts[major]->getPort(port);
}

xpr::rtsp::Port* xpr::rtsp::PortManager::getMajorPort(int port)
{
    int major = XPR_RTSP_PORT_MAJOR(port);
    return mMajorPorts[major];
}

xpr::rtsp::Port* xpr::rtsp::PortManager::getMinorPort(int port)
{
    xpr::rtsp::Port* mp = getMajorPort(port);
    return mp ? mp->getPort(port) : NULL;
}

/*
int xpr::rtsp::PortManager::init(void)
{
#if defined(HAVE_XPR_RTSP_CLIENT)
    mMajorPorts[XPR_RTSP_PORT_MAJOR_CLI] = new xpr::rtsp::ClientManager();
#endif // HAVE_XPR_RTSP_CLIENT
#if defined(HAVE_XPR_RTSP_SERVER)
    mMajorPorts[XPR_RTSP_PORT_MAJOR_SVR] = new xpr::rtsp::Server(XPR_RTSP_PORT_MAJOR_SVR, this);
#endif // HAVE_XPR_RTSP_SERVER
    return XPR_ERR_OK;
}

int xpr::rtsp::PortManager::fini(void)
{
#if defined(HAVE_XPR_RTSP_CLIENT)
    delete mMajorPorts[XPR_RTSP_PORT_MAJOR_CLI];
    mMajorPorts[XPR_RTSP_PORT_MAJOR_CLI] = NULL;
#endif // HAVE_XPR_RTSP_CLIENT
#if defined(HAVE_XPR_RTSP_SERVER)
    delete mMajorPorts[XPR_RTSP_PORT_MAJOR_SVR];
    mMajorPorts[XPR_RTSP_PORT_MAJOR_SVR] = NULL;
#endif // HAVE_XPR_RTSP_SERVER
    return XPR_ERR_OK;
}
*/

int xpr::rtsp::PortManager::setupServer(const char* url)
{
    // 创建服务器
    xpr::rtsp::Server* server = new xpr::rtsp::Server(XPR_RTSP_PORT_MAJOR_SVR,
                                                      this);
    if (server == NULL)
        return XPR_ERR_GEN_NOMEM;
    int err = server->open(XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 0, 0), url);
    if (err == XPR_ERR_OK)
        mMajorPorts[XPR_RTSP_PORT_MAJOR_SVR] = server;
    return XPR_ERR_OK;
}

int xpr::rtsp::PortManager::clearServer(void)
{
    xpr::rtsp::Server* server = (xpr::rtsp::Server*)
                                mMajorPorts[XPR_RTSP_PORT_MAJOR_SVR];
    if (server) {
        server->close(XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 0, 0));
        delete server;
        mMajorPorts[XPR_RTSP_PORT_MAJOR_SVR] = NULL;
    }
    return XPR_ERR_OK;
}

// Public interfaces
//============================================================================
XPR_API int XPR_RTSP_Init(void)
{
    char* env = NULL;
    if (rtsp)
        return XPR_ERR_GEN_EXIST;
    // Set live555 ReceivingInterfaceAddr & SendingInterfaceAddr from ENV
    env = getenv("XPR_RTSP_RX_IF_ADDR");
    if (env && *env) {
        ReceivingInterfaceAddr = our_inet_addr(env);
    }
    env = getenv("XPR_RTSP_TX_IF_ADDR");
    if (env && *env) {
        SendingInterfaceAddr = our_inet_addr(env);
    }
    //
    rtsp = new xpr::rtsp::PortManager();
    if (rtsp == NULL)
        return XPR_ERR_GEN_NOMEM;
    //return rtsp->init();
    return XPR_ERR_OK;
    /*
        memset(rtsp_ports, 0, sizeof(rtsp_ports));
    #if defined(HAVE_XPR_RTSP_CLIENT)
        rtsp_ports[XPR_RTSP_PORT_MAJOR_CLI] = new xpr::rtsp::ClientManager();
    #endif // HAVE_XPR_RTSP_CLIENT
    #if defined(HAVE_XPR_RTSP_SERVER)
        rtsp_ports[XPR_RTSP_PORT_MAJOR_SVR] = new xpr::rtsp::ServerManager();
    #endif // HAVE_XPR_RTSP_SERVER
        return 0;
    */
}

XPR_API int XPR_RTSP_Fini(void)
{
    if (rtsp == NULL)
        return XPR_ERR_GEN_UNEXIST;
    //int retval = rtsp->fini();
    //if (retval != XPR_ERR_OK)
    //  return retval;
    delete rtsp;
    rtsp = NULL;
    return XPR_ERR_OK;
    /*
    #if defined(HAVE_XPR_RTSP_CLIENT)
        delete rtsp_ports[XPR_RTSP_PORT_MAJOR_CLI];
        rtsp_ports[XPR_RTSP_PORT_MAJOR_CLI] = NULL;
    #endif // HAVE_XPR_RTSP_CLIENT
    #if defined(HAVE_XPR_RTSP_SERVER)
        delete rtsp_ports[XPR_RTSP_PORT_MAJOR_SVR];
        rtsp_ports[XPR_RTSP_PORT_MAJOR_SVR] = NULL;
    #endif // HAVE_XPR_RTSP_SERVER
        return 0;
    */
}

XPR_API int XPR_RTSP_IsPortValid(int port)
{
    return rtsp->isPortValid(port);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? (p->isPortValid(port) ? XPR_TRUE : XPR_FALSE) : XPR_FALSE;
}

XPR_API int XPR_RTSP_Open(int port, const char* url)
{
    return rtsp->open(port, url);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->open(port, url) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_Close(int port)
{
    return rtsp->close(port);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->close(port) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_Start(int port)
{
    return rtsp->start(port);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->start(port) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_Stop(int port)
{
    return rtsp->stop(port);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->stop(port) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_AddDataCallback(int port, XPR_RTSP_DCB cb, void* opaque)
{
    return rtsp->addDataCallback(port, cb, opaque);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->addDataCallback(port, cb, opaque) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_DelDataCallback(int port, XPR_RTSP_DCB cb, void* opaque)
{
    return rtsp->delDataCallback(port, cb, opaque);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->delDataCallback(port, cb, opaque) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_AddEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque)
{
    return rtsp->addEventCallback(port, cb, opaque);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->addEventCallback(port, cb, opaque) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_DelEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque)
{
    return rtsp->delEventCallback(port, cb, opaque);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->delEventCallback(port, cb, opaque) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_SetAuth(int port, const char* username,
                             const char* password, int pwdIsMD5)
{
    return rtsp->setAuth(port, username, password, pwdIsMD5);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->setAuth(port, username, password, pwdIsMD5) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_SetOutputFormat(int port, int fourcc)
{
    return rtsp->setOutputFormat(port, fourcc);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->setOutputFormat(port, fourcc) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_PushData(int port, XPR_StreamBlock* stb)
{
    return rtsp->pushData(port, stb);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->postData(port, stb) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_PostData(int port, XPR_StreamBlock* stb)
{
    return rtsp->postData(port, stb);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->postData(port, stb) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_PostEvent(int port, const XPR_RTSP_EVD* evd)
{
    return rtsp->postEvent(port, evd);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->postEvent(port, evd) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_SetTrSpec(int port, XPR_RTSP_TRSPEC trspec)
{
    return rtsp->setTrSpec(port, trspec);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->setTrSpec(port, trspec) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_GetParam(int port, XPR_RTSP_PARAM param, void* buffer,
                              int* size)
{
    return rtsp->getParam(port, param, buffer, size);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->getParam(port, param, buffer, size) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_SetParam(int port, XPR_RTSP_PARAM param, const void* data,
                              int length)
{
    return rtsp->setParam(port, param, data, length);
    //xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
    //return p ? p->setParam(port, param, data, length) : XPR_ERR_GEN_NOT_SUPPORT;
}

#endif // defined(HAVE_XPR_RTSP_DRIVER_LIVE)

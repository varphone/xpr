#if defined(XPR_RTSP_DRIVER_LIVE)

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
#  endif
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

#include "rtsp.hpp"

// 端口主端口槽位
static xpr::rtsp::Port*		rtsp_ports[XPR_RTSP_PORT_MAJOR_ALL+1];

#if defined(XPR_RTSP_HAVE_CLIENT)
#include "rtsp_client.hpp"
static xpr::rtsp::ClientManager* rtsp_climgr = NULL;
#endif // defined(XPR_RTSP_HAVE_CLIENT)

#if defined(XPR_RTSP_HAVE_SERVER)
#include "rtsp_server.hpp"
static xpr::rtsp::ServerManager* rtsp_svrmgr = NULL;
#endif // defined(XPR_RTSP_HAVE_SERVER)

// Internal interfaces
//============================================================================
xpr::rtsp::Port* xpr::rtsp::Port::getPort(int port)
{
	int major = XPR_RTSP_PORT_MAJOR(port);
	if (major == XPR_RTSP_PORT_MAJOR_CLI ||
		major == XPR_RTSP_PORT_MAJOR_SVR)
		return rtsp_ports[major];
	return NULL;
}

int xpr::rtsp::Port::getAvailStreamId(int majorPort)
{
	return 0;
}

int xpr::rtsp::Port::getAvailStreamTrackId(int streamPort)
{
	return 0;
}

// Public interfaces
//============================================================================
XPR_API int XPR_RTSP_Init(void)
{
	memset(rtsp_ports, 0, sizeof(rtsp_ports));
#if defined(XPR_RTSP_HAVE_CLIENT)
	rtsp_ports[XPR_RTSP_PORT_MAJOR_CLI] = new xpr::rtsp::ClientManager();
#endif
#if defined(XPR_RTSP_HAVE_SERVER)
	rtsp_ports[XPR_RTSP_PORT_MAJOR_SVR] = new xpr::rtsp::ServerManager();
#endif
    return 0;
}

XPR_API int XPR_RTSP_Fini(void)
{
#if defined(XPR_RTSP_HAVE_CLIENT)
	delete rtsp_ports[XPR_RTSP_PORT_MAJOR_CLI];
	rtsp_ports[XPR_RTSP_PORT_MAJOR_CLI] = NULL;
#endif
#if defined(XPR_RTSP_HAVE_SERVER)
	delete rtsp_ports[XPR_RTSP_PORT_MAJOR_SVR];
	rtsp_ports[XPR_RTSP_PORT_MAJOR_SVR] = NULL;
#endif
    return 0;
}

XPR_API int XPR_RTSP_IsPortValid(int port)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? (p->isPortValid(port) ? XPR_TRUE : XPR_FALSE) : XPR_FALSE;
}

XPR_API int XPR_RTSP_Open(int port, const char* url)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->open(port, url) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_Close(int port)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->close(port) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_Start(int port)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->start(port) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_Stop(int port)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->stop(port) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_AddDataCallback(int port, XPR_RTSP_DCB cb, void* opaque)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->addDataCallback(port, cb, opaque) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_DelDataCallback(int port, XPR_RTSP_DCB cb, void* opaque)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->delDataCallback(port, cb, opaque) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_AddEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->addEventCallback(port, cb, opaque) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_DelEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->delEventCallback(port, cb, opaque) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_SetAuth(int port, const char* username, const char* password, int pwdIsMD5)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->setAuth(port, username, password, pwdIsMD5) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_SetOutputFormat(int port, int fourcc)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->setOutputFormat(port, fourcc) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_PostData(int port, XPR_StreamBlock* stb)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->postData(port, stb) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_PostEvent(int port, const XPR_RTSP_EVD* evd)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->postEvent(port, evd) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_SetTrSpec(int port, XPR_RTSP_TRSPEC trspec)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->setTrSpec(port, trspec) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_GetParam(int port, XPR_RTSP_PARAM param, void* buffer, int* size)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->getParam(port, param, buffer, size) : XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_RTSP_SetParam(int port, XPR_RTSP_PARAM param, const void* data, int length)
{
	xpr::rtsp::Port* p = xpr::rtsp::Port::getPort(port);
	return p ? p->setParam(port, param, data, length) : XPR_ERR_GEN_NOT_SUPPORT;
}

#endif // defined(XPR_RTSP_DRIVER_LIVE)

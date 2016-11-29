#if defined(XPR_RTSP_DRIVER_LIVE)

#include "rtsp_server.hpp"
#include <live/GroupsockHelper.hh>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_url.h>
#include <map>

#if 0
H264VideoFramedSource::H264VideoFramedSource(UsageEnvironment& env)
	: FramedSource(env)
{
}

H264VideoFramedSource::~H264VideoFramedSource(void)
{
	envir().taskScheduler().unscheduleDelayedTask(mCurrentTask);
}

void H264VideoFramedSource::doGetNextFrame()
{
	// 根据 fps，计算等待时间  
	double delay = 1000.0 / (25 * 2);  // ms  
	int to_delay = delay * 1000;  // us  

	mCurrentTask = envir().taskScheduler().scheduleDelayedTask(to_delay, getNextFrame, this);
}

unsigned int H264VideoFramedSource::maxFrameSize() const
{
	return XPR_RTSP_STREAM_MAX_FRAME_SIZE;
}

void H264VideoFramedSource::fetchFrame()
{
	::gettimeofday(&fPresentationTime, 0);

	/*
	fFrameSize = 0;

	int len = 0;
	unsigned char buffer[BUFFER_SIZE] = { 0 };
	while ((len = read(m_hFifo, buffer, BUFFER_SIZE))>0)
	{
		memcpy(m_pFrameBuffer + fFrameSize, buffer, len);
		fFrameSize += len;
	}
	//printf("[MEDIA SERVER] GetFrameData len = [%d],fMaxSize = [%d]\n",fFrameSize,fMaxSize);  

	// fill frame data  
	memcpy(fTo, m_pFrameBuffer, fFrameSize);

	if (fFrameSize > fMaxSize)
	{
		fNumTruncatedBytes = fFrameSize - fMaxSize;
		fFrameSize = fMaxSize;
	}
	else
	{
		fNumTruncatedBytes = 0;
	}
	*/

	afterGetting(this);
}

void H264VideoFramedSource::getNextFrame(void * ptr)
{
	if (ptr) {
		((H264VideoFramedSource*)ptr)->fetchFrame();
	}
}
#endif

// H264VideoServerMediaSession
//============================================================================
xpr::rtsp::H264VideoServerMediaSession::H264VideoServerMediaSession(UsageEnvironment & env, FramedSource* source)
	: OnDemandServerMediaSubsession(env, True)
{
}

xpr::rtsp::H264VideoServerMediaSession::~H264VideoServerMediaSession(void)
{
}

char const * xpr::rtsp::H264VideoServerMediaSession::getAuxSDPLine(RTPSink * rtpSink, FramedSource * inputSource)
{
	return nullptr;
}

FramedSource * xpr::rtsp::H264VideoServerMediaSession::createNewStreamSource(unsigned clientSessionId, unsigned & estBitrate)
{
	return nullptr;
}

RTPSink * xpr::rtsp::H264VideoServerMediaSession::createNewRTPSink(Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource * inputSource)
{
	return nullptr;
}

void xpr::rtsp::H264VideoServerMediaSession::chkForAuxSDPLine1()
{
}

xpr::rtsp::H264VideoServerMediaSession* xpr::rtsp::H264VideoServerMediaSession::createNew(UsageEnvironment& env, FramedSource* source)
{
	return new H264VideoServerMediaSession(env, source);
}

void xpr::rtsp::H264VideoServerMediaSession::afterPlayingDummy(void * ptr)
{
}

void xpr::rtsp::H264VideoServerMediaSession::chkForAuxSDPLine(void * ptr)
{
}

// Server
//============================================================================
xpr::rtsp::Server::Server(void)
	: mBindAddress("0.0.0.0")
	, mBindPort(554)
	, mMaxStreams(16)
	, mMaxStreamTracks(4)
	, mMaxWorkers(1)
	, mWorkers()
{
	DBG(DBG_L4, "xpr::rtsp::Server::Server(void)");
	memset(mWorkers, 0, sizeof(mWorkers));
}

xpr::rtsp::Server::~Server(void)
{
	DBG(DBG_L4, "xpr::rtsp::Server::~Server(void)");
}

int xpr::rtsp::Server::start(void)
{
	setupWorkers();
	return XPR_ERR_OK;
}

int xpr::rtsp::Server::stop(void)
{
	clearWorkers();
	return XPR_ERR_OK;
}

const char * xpr::rtsp::Server::getBindAddress(void) const
{
	return mBindAddress.c_str();
}

void xpr::rtsp::Server::setBindAddress(const char * bindAddress)
{
	mBindAddress = bindAddress;
}

uint16_t xpr::rtsp::Server::getBindPort(void) const
{
	return mBindPort;
}

void xpr::rtsp::Server::setBindPort(uint16_t bindPort)
{
	mBindPort = bindPort;
}

size_t xpr::rtsp::Server::getMaxStreams(void) const
{
	return mMaxStreams;
}

void xpr::rtsp::Server::setMaxStreams(size_t maxStreams)
{
	mMaxStreams = maxStreams;
}

size_t xpr::rtsp::Server::getMaxStreamTracks(void) const
{
	return mMaxStreamTracks;
}

void xpr::rtsp::Server::setMaxStreamTracks(size_t maxStreamTracks)
{
	mMaxStreamTracks = maxStreamTracks;
}

size_t xpr::rtsp::Server::getMaxWorkers(void) const
{
	return mMaxWorkers;
}

void xpr::rtsp::Server::setMaxWorkers(size_t maxWorkers)
{
	mMaxWorkers = maxWorkers;
}

bool xpr::rtsp::Server::isValidStreamId(int streamId)
{
	return false;
}

bool xpr::rtsp::Server::isValidStreamTrackId(int streamTrackId)
{
	return false;
}

void xpr::rtsp::Server::setupWorkers(void)
{
	for (size_t i = 0; i < mMaxWorkers; i++) {
		mWorkers[i] = new Worker(i, this);
	}
	for (size_t i = 0; i < mMaxWorkers; i++) {
		if (mWorkers[i]) {
			mWorkers[i]->start();
		}
	}
}

void xpr::rtsp::Server::clearWorkers(void)
{
	for (size_t i = 0; i < mMaxWorkers; i++) {
		if (mWorkers[i]) {
			mWorkers[i]->stop();
		}
	}
	for (size_t i = 0; i < mMaxWorkers; i++) {
		if (mWorkers[i]) {
			delete mWorkers[i];
			mWorkers[i] = NULL;
		}
	}
}

// Worker
//============================================================================
xpr::rtsp::Worker::Worker(int id, Server* server)
	: mId(id)
	, mServer(server)
	, mScheduler(NULL)
	, mEnv(NULL)
	, mThread(NULL)
	, mExitLoop(false)
{
	DBG(DBG_L4, "xpr::rtsp::Worker::Worker(int id, Server* server)");
}

xpr::rtsp::Worker::~Worker(void)
{
	DBG(DBG_L4, "xpr::rtsp::Worker::~Worker(void)");
	terminate();
	stop();
}

void xpr::rtsp::Worker::run(void)
{
	DBG(DBG_L4, "worker [%d @ %p] running ...", id(), this);
	while (!mExitLoop) {
		((BasicTaskScheduler*)mScheduler)->SingleStep(10);
	}
	DBG(DBG_L4, "worker [%d @ %p] exited.", id(), this); 
}

int xpr::rtsp::Worker::start(void)
{
	if (mScheduler != NULL ||
		mEnv != NULL ||
		mThread != NULL)
		return XPR_ERR_GEN_BUSY;

	mScheduler = BasicTaskScheduler::createNew();
	mEnv = BasicUsageEnvironment::createNew(*mScheduler);
	mThread = XPR_ThreadCreate(xpr::rtsp::Worker::thread, 1024 * 1024 * 4, this);

	return XPR_ERR_OK;
}

int xpr::rtsp::Worker::stop(void)
{
	if (mEnv != NULL) {
		mEnv->reclaim();
		mEnv = NULL;
	}
	if (mScheduler != NULL) {
		delete mScheduler;
		mScheduler = NULL;
	}
	if (mExitLoop != true)
		mExitLoop = true;
	if (mThread != NULL) {
		XPR_ThreadJoin(mThread);
		XPR_ThreadDestroy(mThread);
		mThread = NULL;
	}
	return XPR_ERR_OK;
}

int xpr::rtsp::Worker::terminate(void)
{
	mExitLoop = true;
	return XPR_ERR_OK;
}

int xpr::rtsp::Worker::id(void) const
{
	return mId;
}

xpr::rtsp::Server& xpr::rtsp::Worker::server(void)
{
	return *mServer;
}

TaskScheduler& xpr::rtsp::Worker::scheduler(void)
{
	return *mScheduler;
}

UsageEnvironment& xpr::rtsp::Worker::env(void)
{
	return *mEnv;
}

void* xpr::rtsp::Worker::thread(void * opaque, XPR_Thread * thread)
{
	if (opaque)
		((xpr::rtsp::Worker*)opaque)->run();
	return NULL;
}

// ServerManager
//============================================================================

xpr::rtsp::ServerManager::ServerManager(void)
	: mServer(NULL)
{
	DBG(DBG_L4, "xpr::rtsp::ServerManager::ServerManager(void)");
}

xpr::rtsp::ServerManager::~ServerManager(void)
{
	DBG(DBG_L4, "xpr::rtsp::ServerManager::~ServerManager(void)");
	if (mServer) {
		delete mServer;
		mServer = NULL;
	}
}

bool xpr::rtsp::ServerManager::isPortValid(int port)
{
	int streamId = XPR_RTSP_PORT_STREAM(port);
	int trackId = XPR_RTSP_PORT_TRACK(port);
	if (trackId == XPR_RTSP_PORT_TRACK_NUL ||
		trackId == XPR_RTSP_PORT_TRACK_ALL ||
		trackId == XPR_RTSP_PORT_TRACK_ANY)
		return mServer->isValidStreamId(streamId);
	return mServer->isValidStreamId(streamId) && mServer->isValidStreamTrackId(trackId);
}

int xpr::rtsp::ServerManager::open(int port, const char * url)
{
	if (port == XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 0, 0)) {
		if (mServer)
			return XPR_ERR_GEN_EXIST;
		return setupServer(url);
	}
	if (mServer == NULL)
		return XPR_ERR_GEN_SYS_NOTREADY;
	return setupStream(port, url);
}

int xpr::rtsp::ServerManager::close(int port)
{
	if (port == XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 0, 0)) {
		if (mServer == NULL)
			return XPR_ERR_GEN_UNEXIST;
		return clearServer();
	}
	if (mServer == NULL)
		return XPR_ERR_GEN_SYS_NOTREADY;
	return clearStream(port);
}

int xpr::rtsp::ServerManager::start(int port)
{
	if (port == XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 0, 0)) {
		if (mServer == NULL)
			return XPR_ERR_GEN_SYS_NOTREADY;
		return mServer->start();
	}
	return 0;
}

int xpr::rtsp::ServerManager::stop(int port)
{
	if (port == XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 0, 0)) {
		if (mServer == NULL)
			return XPR_ERR_GEN_SYS_NOTREADY;
		return mServer->stop();
	}
	return 0;
}

int xpr::rtsp::ServerManager::setupServer(const char * url)
{
	// 创建服务器
	mServer = new Server();
	if (mServer == NULL)
		return XPR_ERR_GEN_NOMEM;
	// 解析地址参数
	XPR_Url* u = XPR_UrlParse(url, -1);
	if (u == NULL)
		return XPR_ERR_GEN_ILLEGAL_PARAM;
	const char* host = XPR_UrlGetHost(u);
	uint16_t port = XPR_UrlGetPort(u);
	// 设定绑定地址及端口
	if (host)
		mServer->setBindAddress(host);
	if (port)
		mServer->setBindPort(port);
	// 使用 Query 参数来配置服务器
	configServer(XPR_UrlGetQuery(u));
	//
	return XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 0, 0);
}

int xpr::rtsp::ServerManager::clearServer(void)
{
	if (mServer) {
		delete mServer;
		mServer = NULL;
	}
	return XPR_ERR_OK;
}

void xpr::rtsp::ServerManager::configServer(const char * query)
{
	if (query) {
		DBG(DBG_L3, "Server configuration query: %s", query);
		xpr_foreach_s(query, -1, "&", xpr::rtsp::handleServerConfig, this);
	}
}

void xpr::rtsp::ServerManager::configServer(const char * key, const char * value)
{
	DBG(DBG_L3, "Server configuration: %s = %s", key, value);
	if (strcmp(key, "maxStreams") == 0)
		mServer->setMaxStreams(strtol(value, NULL, 10));
	else if (strcmp(key, "maxStreamTracks") == 0)
		mServer->setMaxStreamTracks(strtol(value, NULL, 10));
	else if (strcmp(key, "maxWorkers") == 0)
		mServer->setMaxWorkers(strtol(value, NULL, 10));
	else {
		DBG(DBG_L3, "Server configuration: %s unsupported.", key);
	}
}

int xpr::rtsp::ServerManager::setupStream(int port, const char * url)
{
	return 0;
}

int xpr::rtsp::ServerManager::clearStream(int port)
{
	return 0;
}

void xpr::rtsp::handleServerConfig(void * opaque, char * seg)
{
	if (opaque && seg) {
		char* key = NULL;
		char* value = NULL;
		if (xpr_split_to_kv(seg, &key, &value) == XPR_ERR_OK)
			((xpr::rtsp::ServerManager*)opaque)->configServer(key, value);
	}
}

#endif // defined(XPR_RTSP_DRIVER_LIVE)
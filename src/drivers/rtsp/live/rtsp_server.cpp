#if defined(HAVE_XPR_RTSP_DRIVER_LIVE)

#include "rtsp_server.hpp"
#include <live/GroupsockHelper.hh>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_url.h>
#include <map>

// H264VideoFramedSource
//============================================================================
xpr::rtsp::H264VideoFramedSource::H264VideoFramedSource(UsageEnvironment& env)
	: FramedSource(env)
	, mBuffer(new uint8_t[XPR_RTSP_H264_VIDEO_BUFFER_SIZE])
	, mBufferSize(XPR_RTSP_H264_VIDEO_BUFFER_SIZE)
	, mFreeList(NULL)
	, mDataList(NULL)
{
	DBG(DBG_L4, "xpr::rtsp::H264VideoFramedSource::H264VideoFramedSource(%p) = %p", &env, this);
	mFreeList = XPR_FifoCreate(sizeof(XPR_StreamBlock*), 10);
	mDataList = XPR_FifoCreate(sizeof(XPR_StreamBlock*), 10);
}

xpr::rtsp::H264VideoFramedSource::~H264VideoFramedSource(void)
{
	DBG(DBG_L4, "xpr::rtsp::H264VideoFramedSource::~H264VideoFramedSource() = %p", this);
	envir().taskScheduler().unscheduleDelayedTask(mCurrentTask);
}

void xpr::rtsp::H264VideoFramedSource::doGetNextFrame()
{
	DBG(DBG_L4, "void xpr::rtsp::H264VideoFramedSource::doGetNextFrame() = %p", this);
	// ���� fps������ȴ�ʱ��  
	//double delay = 1000.0 / (25 * 2);  // ms  
	//int to_delay = delay * 1000;  // us  

	printf("void xpr::rtsp::H264VideoFramedSource::doGetNextFrame()\n");

	mCurrentTask = envir().taskScheduler().scheduleDelayedTask(10000, getNextFrame, this);
}

unsigned int xpr::rtsp::H264VideoFramedSource::maxFrameSize() const
{
	return XPR_RTSP_H264_MAX_FRAME_SIZE;
}

void xpr::rtsp::H264VideoFramedSource::fetchFrame()
{
	/*
	::gettimeofday(&fPresentationTime, 0);

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

	printf("fetch frame\n");

	XPR_StreamBlock* ntb = (XPR_StreamBlock*)XPR_FifoGetAsAtomic(mDataList);
	if (ntb) {
		printf("fMaxSize %d, fTo %p\n", fMaxSize, fTo);
		fFrameSize = min(XPR_StreamBlockLength(ntb), fMaxSize);
		fNumTruncatedBytes = XPR_StreamBlockLength(ntb) - fMaxSize;
		memcpy(fTo, XPR_StreamBlockData(ntb), fFrameSize);
		fPresentationTime.tv_sec = XPR_StreamBlockPTS(ntb) / 1000000;
		fPresentationTime.tv_usec = XPR_StreamBlockPTS(ntb) % 1000000;
		afterGetting(this);
	}
	else {
		fFrameSize = 0;
		fNumTruncatedBytes = 0;
		doGetNextFrame();
	}
}

int xpr::rtsp::H264VideoFramedSource::putFrame(const XPR_StreamBlock * stb)
{
	// Pop from free list
	XPR_StreamBlock* ntb = (XPR_StreamBlock*)XPR_FifoGetAsAtomic(mFreeList);
	if (ntb == NULL) {
		ntb = XPR_StreamBlockAlloc(0);
		ntb->buffer = mBuffer + mBufferOffset;
		ntb->bufferSize = mBufferSize - mBufferOffset;
		ntb->data = ntb->buffer;
	}
	if (XPR_StreamBlockSize(ntb) < XPR_StreamBlockLength(stb)) {
		ntb->buffer = mBuffer;
		ntb->bufferSize = mBufferSize;
		ntb->data = ntb->buffer;
		mBufferOffset = 0;
	}
	//
	XPR_StreamBlockCopyHeader(stb, ntb);
	XPR_StreamBlockCopyData(stb, ntb);
	// Put to data list
	int err = XPR_FifoPutAsAtomic(mDataList, (uintptr_t)ntb);
	if (err < 0) {
		XPR_StreamBlockFree(ntb);
	}
	//printf("push data err %x\n", err);
	return err;
}

void xpr::rtsp::H264VideoFramedSource::getNextFrame(void * ptr)
{
	if (ptr) {
		printf("getNextFrame\n");
		((H264VideoFramedSource*)ptr)->fetchFrame();
	}
}

//Boolean xpr::rtsp::H264VideoFramedSource::isH264VideoStreamFramer() const
//{
//	return True;
//}

// H264VideoServerMediaSubsession
//============================================================================
xpr::rtsp::H264VideoServerMediaSubsession::H264VideoServerMediaSubsession(UsageEnvironment & env, FramedSource* source)
	: OnDemandServerMediaSubsession(env, True)
	, mSource(source)
	, mSink(NULL)
	, mSDPDone(0)
	, mSDPLine(NULL)
{
	DBG(DBG_L4, "xpr::rtsp::H264VideoServerMediaSubsession::H264VideoServerMediaSubsession(%p, %p) = %p", &env, source, this);
}

xpr::rtsp::H264VideoServerMediaSubsession::~H264VideoServerMediaSubsession(void)
{
	DBG(DBG_L4, "xpr::rtsp::H264VideoServerMediaSubsession::~H264VideoServerMediaSubsession() = %p", this);
	if (mSDPLine) {
		free(mSDPLine);
		mSDPLine = NULL;
	}
	mSource = NULL;
	mSink = NULL;
	mSDPDone = false;
}

char const * xpr::rtsp::H264VideoServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource)
{
	if (mSDPLine) {
		return mSDPLine;
	}

	printf("getAuxSDPLine(%p, %p)\n", rtpSink, inputSource);

	mSink = rtpSink;
	//FramedSource* fs = new H264VideoFramedSource(envir());
	mSink->startPlaying(*inputSource, 0, 0);
	printf("AAAAAAAAAAAAAA-1101\n");
	chkForAuxSDPLine(this);
	printf("AAAAAAAAAAAAAA-1121\n");
	mSDPDone = 0;
	envir().taskScheduler().doEventLoop(&mSDPDone);
	printf("AAAAAAAAAAAAAA-1131\n");
#if defined(_MSC_VER)
	mSDPLine = _strdup(mSink->auxSDPLine());
#else
	mSDPLine = strdup(mSink->auxSDPLine());
#endif // defined(_MSC_VER)
	//mSink->stopPlaying();
	return mSDPLine;
}

FramedSource* xpr::rtsp::H264VideoServerMediaSubsession::createNewStreamSource(
		unsigned clientSessionId, unsigned & estBitrate)
{
	//mSource->
	return H264VideoStreamFramer::createNew(envir(), mSource, True);
	//return H264VideoStreamFramer::createNew(envir(), new H264VideoFramedSource(envir()), True);
	//return new H264VideoFramedSource(envir());
	//return H264VideoStreamFramer::createNew(envir(), new WW_H264VideoSource(envir()));
	//return NULL;// new H264VideoFramedSource::createNew()
}

RTPSink * xpr::rtsp::H264VideoServerMediaSubsession::createNewRTPSink(
		Groupsock * rtpGroupsock,
		unsigned char rtpPayloadTypeIfDynamic,
		FramedSource * inputSource)
{
	return H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic);
}

void xpr::rtsp::H264VideoServerMediaSubsession::chkForAuxSDPLine1()
{
	printf("mSink %p\n", mSink);
	if (mSink && mSink->auxSDPLine())
	{
		printf("aux line: %s\n", mSink->auxSDPLine());
		mSDPDone = 0xff;
	}
	else
	{
		nextTask() = envir().taskScheduler().scheduleDelayedTask(20000, chkForAuxSDPLine, this);
	}
}

void xpr::rtsp::H264VideoServerMediaSubsession::sdpDone(bool done)
{
	mSDPDone = done ? 0xFF : 0x00;
}

xpr::rtsp::H264VideoServerMediaSubsession* xpr::rtsp::H264VideoServerMediaSubsession::createNew(UsageEnvironment& env, FramedSource* source)
{
	return new H264VideoServerMediaSubsession(env, source);
}

void xpr::rtsp::H264VideoServerMediaSubsession::afterPlayingDummy(void* ptr)
{
	H264VideoServerMediaSubsession* self = (H264VideoServerMediaSubsession*)ptr;
	if (self)
		self->mSDPDone = 0xff;
}

void xpr::rtsp::H264VideoServerMediaSubsession::chkForAuxSDPLine(void* ptr)
{
	H264VideoServerMediaSubsession* self = (H264VideoServerMediaSubsession *)ptr;
	if (self)
		self->chkForAuxSDPLine1();
}

// Server
//============================================================================
xpr::rtsp::Server::Server(int id, Port* parent)
	: Port(id, parent)
	, mBindAddress("0.0.0.0")
	, mBindPort(554)
	, mMaxStreams(16)
	, mMaxStreamTracks(4)
	, mMaxWorkers(1)
	, mStreams()
	, mWorkers()
	, mAuthDB(NULL)
	, mRTSPServer(NULL)
{
	DBG(DBG_L4, "xpr::rtsp::Server::Server(%d, %p) = %p", id, parent, this);
	memset(mStreams, 0, sizeof(mStreams));
	memset(mWorkers, 0, sizeof(mWorkers));
}

xpr::rtsp::Server::~Server(void)
{
	DBG(DBG_L4, "xpr::rtsp::Server::~Server(id, %p) = %p", id(), parent(), this);
}

int xpr::rtsp::Server::isPortValid(int port)
{
	uint32_t major = XPR_RTSP_PORT_MAJOR(port);
	uint32_t streamId = XPR_RTSP_PORT_STREAM(port);
	uint32_t trackId = XPR_RTSP_PORT_TRACK(port);
	if (streamId == XPR_RTSP_PORT_TRACK_ALL ||
		streamId == XPR_RTSP_PORT_TRACK_ANY ||
		streamId == XPR_RTSP_PORT_TRACK_NUL)
		return XPR_TRUE;
	if (trackId == XPR_RTSP_PORT_TRACK_ALL ||
		trackId == XPR_RTSP_PORT_TRACK_ANY ||
		trackId == XPR_RTSP_PORT_TRACK_NUL)
		return XPR_TRUE;
	if (streamId <= mMaxStreams)
		return XPR_TRUE;
	if (trackId <= mMaxStreamTracks)
		return XPR_TRUE;
	return XPR_FALSE;
}

int xpr::rtsp::Server::open(int port, const char * url)
{
	if (isPortValid(port) == XPR_FALSE || url == NULL) {
		return XPR_ERR_GEN_ILLEGAL_PARAM;
	}
	// Open server if XPR_RTSP_PORT_MINOR_NUL
	if (XPR_RTSP_PORT_MINOR(port) == XPR_RTSP_PORT_MINOR_NUL)
		return openServer(url);
	return openStream(port, url);
}

int xpr::rtsp::Server::close(int port)
{
	if (isPortValid(port) == XPR_FALSE)
		return XPR_ERR_GEN_ILLEGAL_PARAM;
	// Close server if XPR_RTSP_PORT_MINOR_NUL
	if (XPR_RTSP_PORT_MINOR(port) == XPR_RTSP_PORT_MINOR_NUL)
		return closeServer();
	return closeStream(port);
}

int xpr::rtsp::Server::start(int port)
{
	if (isPortValid(port) == XPR_FALSE)
		return XPR_ERR_GEN_ILLEGAL_PARAM;
	// Start server if XPR_RTSP_PORT_MINOR_NUL
	if (XPR_RTSP_PORT_MINOR(port) == XPR_RTSP_PORT_MINOR_NUL)
		return startServer();
	return startStream(port);
}

int xpr::rtsp::Server::stop(int port)
{
	if (isPortValid(port) == XPR_FALSE)
		return XPR_ERR_GEN_ILLEGAL_PARAM;
	// Stop server if XPR_RTSP_PORT_MINOR_NUL
	if (XPR_RTSP_PORT_MINOR(port) == XPR_RTSP_PORT_MINOR_NUL)
		return stopServer();
	return stopStream(port);
}

int xpr::rtsp::Server::pushData(int port, XPR_StreamBlock * stb)
{
	if (isPortValid(port) == XPR_FALSE)
		return XPR_ERR_GEN_ILLEGAL_PARAM;
	int minor = XPR_RTSP_PORT_MINOR(port);
	// Not support push data if minor ==
	//   XPR_RTSP_PORT_MINOR_NUL ||
	//   XPR_RTSP_PORT_MINOR_ALL ||
	//   XPR_RTSP_PORT_MINOR_ANY
	if (minor == XPR_RTSP_PORT_MINOR_NUL ||
		minor == XPR_RTSP_PORT_MINOR_ALL ||
		minor == XPR_RTSP_PORT_MINOR_ANY)
		return XPR_ERR_GEN_NOT_SUPPORT;
	return mStreams[minor]->pushData(port, stb);
}

RTSPServer * xpr::rtsp::Server::rtspServer(void)
{
	return mRTSPServer;
}

xpr::rtsp::Stream ** xpr::rtsp::Server::streams(void)
{
	return mStreams;
}

xpr::rtsp::Worker ** xpr::rtsp::Server::workers(void)
{
	return mWorkers;
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
	mMaxStreams = min(maxStreams, XPR_RTSP_PORT_STREAM_MAX);
}

size_t xpr::rtsp::Server::getMaxStreamTracks(void) const
{
	return mMaxStreamTracks;
}

void xpr::rtsp::Server::setMaxStreamTracks(size_t maxStreamTracks)
{
	mMaxStreamTracks = min(maxStreamTracks, XPR_RTSP_PORT_TRACK_MAX);
}

size_t xpr::rtsp::Server::getMaxWorkers(void) const
{
	return mMaxWorkers;
}

void xpr::rtsp::Server::setMaxWorkers(size_t maxWorkers)
{
	mMaxWorkers = min(maxWorkers, XPR_RTSP_MAX_WORKERS);
}

bool xpr::rtsp::Server::isValidStreamId(int streamId)
{
	return false;
}

bool xpr::rtsp::Server::isValidStreamTrackId(int streamTrackId)
{
	return false;
}

/// �򿪷�����
/// @retval XPR_ERR_GEN_BUSY	��������������
/// @retval XPR_ERR_OK			�������򿪳ɹ� 
int xpr::rtsp::Server::openServer(const char * url)
{
	if (activeFlags() != PortFlags::PORT_FLAG_NULL ||
		activeFlags() & PortFlags::PORT_FLAG_CLOSE)
		return XPR_ERR_GEN_BUSY;
	if (activeFlags() & PortFlags::PORT_FLAG_OPEN)
		return XPR_ERR_OK;
	setupServer(url);
	// Stream must be setup first
	setupStreams();
	// Worker must be setup after stream
	setupWorkers();
	//
	setupRTSPServer();
	//
	activeFlags(PortFlags::PORT_FLAG_OPEN);
	return XPR_ERR_OK;
}

int xpr::rtsp::Server::closeServer(void)
{
	if (activeFlags() & PortFlags::PORT_FLAG_CLOSE)
		return XPR_ERR_OK;
	// Must be stop first
	if (!(activeFlags() & PortFlags::PORT_FLAG_STOP))
		return XPR_ERR_GEN_BUSY;
	clearRTSPServer();
	// Stream must be clear first
	clearStreams();
	// Worker must be clear after stream
	clearWorkers();
	// Server must be clear last
	clearServer();
	activeFlags(PortFlags::PORT_FLAG_CLOSE, 0);
	return XPR_ERR_OK;
}

int xpr::rtsp::Server::startServer(void)
{
	if (!(activeFlags() & PortFlags::PORT_FLAG_OPEN))
		return XPR_ERR_GEN_SYS_NOTREADY;
	for (size_t i = 0; i < mMaxWorkers; i++) {
		if (mWorkers[i]) {
			mWorkers[i]->start();
		}
	}
	activeFlags(PortFlags::PORT_FLAG_START, 0);
	return 0;
}

int xpr::rtsp::Server::stopServer(void)
{
	if (!(activeFlags() & PortFlags::PORT_FLAG_START))
		return XPR_ERR_GEN_SYS_NOTREADY;
	for (size_t i = 0; i < mMaxWorkers; i++) {
		if (mWorkers[i]) {
			mWorkers[i]->stop();
		}
	}
	activeFlags(PortFlags::PORT_FLAG_STOP, 0);
	return 0;
}

int xpr::rtsp::Server::openStream(int port, const char * url)
{
	int streamId = XPR_RTSP_PORT_STREAM(port);
	Stream* stream = mStreams[streamId];
	if (stream)
		return stream->open(port, url);
	return XPR_ERR_GEN_UNEXIST;
}

int xpr::rtsp::Server::closeStream(int port)
{
	int streamId = XPR_RTSP_PORT_STREAM(port);
	Stream* stream = mStreams[streamId];
	if (stream)
		return stream->close(port);
	return XPR_ERR_GEN_UNEXIST;
}

int xpr::rtsp::Server::startStream(int port)
{
	int streamId = XPR_RTSP_PORT_STREAM(port);
	Stream* stream = mStreams[streamId];
	if (stream)
		return stream->start(port);
	return XPR_ERR_GEN_UNEXIST;
}

int xpr::rtsp::Server::stopStream(int port)
{
	int streamId = XPR_RTSP_PORT_STREAM(port);
	Stream* stream = mStreams[streamId];
	if (stream)
		return stream->stop(port);
	return XPR_ERR_GEN_UNEXIST;
}

int xpr::rtsp::Server::setupServer(const char * url)
{
	// ������ַ����
	XPR_Url* u = XPR_UrlParse(url, -1);
	if (u == NULL)
		return XPR_ERR_GEN_ILLEGAL_PARAM;
	//
	mUrl = url;
	//
	const char* host = XPR_UrlGetHost(u);
	uint16_t port = XPR_UrlGetPort(u);
	// �趨�󶨵�ַ���˿�
	if (host)
		setBindAddress(host);
	if (port)
		setBindPort(port);
	// ʹ�� Query ���������÷�����
	configServer(XPR_UrlGetQuery(u));
	//
	XPR_UrlDestroy(u);
	//
	return XPR_ERR_OK;
}

int xpr::rtsp::Server::clearServer(void)
{
	mUrl.clear();
	mUsername.clear();
	mPassword.clear();
	mBindAddress.clear();
	return XPR_ERR_OK;
}

void xpr::rtsp::Server::configServer(const char * query)
{
	if (query) {
		DBG(DBG_L3, "Server configuration query: %s", query);
		xpr_foreach_s(query, -1, "&", xpr::rtsp::Server::handleServerConfig, this);
	}
}

void xpr::rtsp::Server::configServer(const char * key, const char * value)
{
	DBG(DBG_L3, "Server configuration: %s = %s", key, value);
	if (strcmp(key, "maxStreams") == 0)
		setMaxStreams(strtol(value, NULL, 10));
	else if (strcmp(key, "maxStreamTracks") == 0)
		setMaxStreamTracks(strtol(value, NULL, 10));
	else if (strcmp(key, "maxWorkers") == 0)
		setMaxWorkers(strtol(value, NULL, 10));
	else {
		DBG(DBG_L3, "Server configuration: %s unsupported.", key);
	}
}

void xpr::rtsp::Server::setupStreams(void)
{
	for (size_t i = XPR_RTSP_PORT_STREAM_MIN; i <= mMaxStreams; i++) {
		mStreams[i] = new Stream(i, this);
	}
}

void xpr::rtsp::Server::clearStreams(void)
{
	for (size_t i = XPR_RTSP_PORT_STREAM_MIN; i <= mMaxStreams; i++) {
		if (mStreams[i]) {
			delete mStreams[i];
			mStreams[i] = NULL;
		}
	}
}

void xpr::rtsp::Server::configStream(int port, const char * query)
{
}

void xpr::rtsp::Server::configStream(int port, const char * key, const char * value)
{
}

void xpr::rtsp::Server::setupWorkers(void)
{
	for (size_t i = 0; i < mMaxWorkers; i++) {
		mWorkers[i] = new Worker(i, this);
	}
}

void xpr::rtsp::Server::clearWorkers(void)
{
	for (size_t i = 0; i < mMaxWorkers; i++) {
		if (mWorkers[i]) {
			delete mWorkers[i];
			mWorkers[i] = NULL;
		}
	}
}

void xpr::rtsp::Server::setupRTSPServer(void)
{
	mAuthDB = new UserAuthenticationDatabase();
#if defined(DEBUG) || defined(_DEBUG)
	mAuthDB->addUserRecord("test", "test");
#endif
	mRTSPServer = RTSPServer::createNew(mWorkers[0]->env(), mBindPort, mAuthDB);
}

void xpr::rtsp::Server::clearRTSPServer(void)
{
	if (mRTSPServer) {
		for (size_t i = XPR_RTSP_PORT_STREAM_MIN; i <= mMaxStreams; i++) {
			if (mStreams[i] && mStreams[i]->sms())
				mRTSPServer->removeServerMediaSession(mStreams[i]->sms());
		}
		Medium::close(mRTSPServer);
		mRTSPServer = NULL;
	}
	if (mAuthDB) {
		delete mAuthDB;
		mAuthDB = NULL;
	}
}

void xpr::rtsp::Server::handleServerConfig(void * opaque, char * seg)
{
	if (opaque && seg) {
		char* key = NULL;
		char* value = NULL;
		if (xpr_split_to_kv(seg, &key, &value) == XPR_ERR_OK)
			((xpr::rtsp::Server*)opaque)->configServer(key, value);
	}
}

// Stream
//============================================================================
xpr::rtsp::Stream::Stream(int id, Port* parent)
	: Port(id, parent)
	, mSMS(NULL)
{
	DBG(DBG_L4, "xpr::rtsp::Stream::Stream(%d, %p) = %p", id, parent, this);
	memset(mFramedSources, 0, sizeof(mFramedSources));
}

xpr::rtsp::Stream::~Stream(void)
{
	DBG(DBG_L4, "xpr::rtsp::Stream::~Stream(%d, %p) = %p", id(), parent(), this);
}

int xpr::rtsp::Stream::open(int port, const char* url)
{
	// ������ַ����
	XPR_Url* u = XPR_UrlParse(url, -1);
	if (u == NULL)
		return XPR_ERR_GEN_ILLEGAL_PARAM;
	//
	mUrl = url;
	//
	Server* server = (Server*)parent();
	const char* path = XPR_UrlGetPath(u);
	if (!path) {
		XPR_UrlDestroy(u);
		return XPR_ERR_GEN_ILLEGAL_PARAM;
	}
	// ������ʼ '/'
	if (*path == '/')
		path++;

	DBG(DBG_L4, "stream name: %s", path);
	// �����������Ự
	mSMS = ServerMediaSession::createNew(server->workers()[0]->env(), path, NULL, path);
	// ʹ�� Query ����������
	configStream(XPR_UrlGetQuery(u));
	//
	XPR_UrlDestroy(u);
	//
	return XPR_ERR_OK;
}

int xpr::rtsp::Stream::close(int port)
{
	return 0;
}

int xpr::rtsp::Stream::start(int port)
{
	Server* server = (Server*)parent();
	if (mSMS == NULL || server->rtspServer() == NULL)
		return XPR_ERR_GEN_SYS_NOTREADY;
	server->rtspServer()->addServerMediaSession(mSMS);
	DBG(DBG_L4, "stream url: %s", server->rtspServer()->rtspURL(mSMS));
	return XPR_ERR_OK;
}

int xpr::rtsp::Stream::stop(int port)
{
	return 0;
}

int xpr::rtsp::Stream::pushData(int port, XPR_StreamBlock* stb)
{
	//ServerMediaSubsessionIterator smsitr(*mSMS);
	//ServerMediaSubsession* smss = NULL;
	//while ((smss = smsitr.next())) {
	//	if (smss->trackNumber() == stb->track) {
	//		smss->getStreamSource(NULL);
	//	}
	//}
	if (mSourceType == "stb") {
		H264VideoFramedSource* src = (H264VideoFramedSource*)mFramedSources[stb->track];
		if (src) {
			return src->putFrame(stb);
		}
	}
	return XPR_ERR_GEN_UNEXIST;
}

ServerMediaSession * xpr::rtsp::Stream::sms(void) const
{
	return mSMS;
}

void xpr::rtsp::Stream::configStream(const char* query)
{
	if (query) {
		DBG(DBG_L3, "stream configuration: %s", query);
		xpr_foreach_s(query, -1, "&", xpr::rtsp::Stream::handleStreamConfig, this);
	}
}

void xpr::rtsp::Stream::configStream(const char * key, const char * value)
{
	DBG(DBG_L3, "stream configuration: %s = %s", key, value);
	Server* server = (Server*)parent();
	if (strcmp(key, "sourceType") == 0) {
		mSourceType = value;
	}
	else if (strcmp(key, "track") == 0) {
		mSourceType = "stb";
		mTrackId = value;
	}
	else if (strcmp(key, "mime") == 0) {
		if (strcmp(value, "video/H264") == 0) {
			if (mSourceType == "stb") {
				UsageEnvironment& env = server->workers()[0]->env();
				int trackId = strtol(mTrackId.c_str(), NULL, 10);
				mFramedSources[trackId] = new H264VideoFramedSource(env);
				mSMS->addSubsession(H264VideoServerMediaSubsession::createNew(env, mFramedSources[trackId]));
			}
		}
	}
	else {
		DBG(DBG_L3, "Server configuration: %s unsupported.", key);
	}
}

void xpr::rtsp::Stream::handleStreamConfig(void* opaque, char* seg)
{
	if (opaque && seg) {
		char* key = NULL;
		char* value = NULL;
		if (xpr_split_to_kv(seg, &key, &value) == XPR_ERR_OK)
			((xpr::rtsp::Stream*)opaque)->configStream(key, value);
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
	DBG(DBG_L4, "xpr::rtsp::Worker::Worker(%d, %p) = %p", id, server, this);
	mScheduler = BasicTaskScheduler::createNew();
	mEnv = BasicUsageEnvironment::createNew(*mScheduler);
}

xpr::rtsp::Worker::~Worker(void)
{
	DBG(DBG_L4, "xpr::rtsp::Worker::~Worker(%d, %p) = %p", mId, mServer, this);
	stop();
	//
	if (mEnv != NULL) {
		mEnv->reclaim();
		mEnv = NULL;
	}
	if (mScheduler != NULL) {
		delete mScheduler;
		mScheduler = NULL;
	}
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
	if (mScheduler == NULL ||
		mEnv == NULL)
		return XPR_ERR_GEN_SYS_NOTREADY;

	if (mThread != NULL)
		return XPR_ERR_GEN_BUSY;

	mThread = XPR_ThreadCreate(xpr::rtsp::Worker::thread, 1024 * 1024 * 4, this);

	return XPR_ERR_OK;
}

int xpr::rtsp::Worker::stop(void)
{
	if (mThread == NULL)
		return XPR_ERR_GEN_SYS_NOTREADY;

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

#endif // defined(HAVE_XPR_RTSP_DRIVER_LIVE)

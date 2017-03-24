#if defined(HAVE_XPR_RTSP_DRIVER_LIVE)

#include "rtsp_server.hpp"
#include <live/GroupsockHelper.hh>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_url.h>
#include <algorithm>
#include <map>

#undef min // avoid from cstdlib

// H264VideoFramedSource
//============================================================================
xpr::rtsp::H264VideoFramedSource::H264VideoFramedSource(UsageEnvironment& env,
                                                        xpr::rtsp::Stream* stream)
    : FramedSource(env)
    , mStream(stream)
    , mLastPTS(0)
{
    DBG(DBG_L4, "xpr::rtsp::H264VideoFramedSource::H264VideoFramedSource(%p) = %p",
        &env, this);
}

xpr::rtsp::H264VideoFramedSource::H264VideoFramedSource(
    const H264VideoFramedSource& rhs)
    : FramedSource(rhs.envir())
    , mStream(rhs.stream())
    , mLastPTS(rhs.mLastPTS)
{
}

xpr::rtsp::H264VideoFramedSource::~H264VideoFramedSource(void)
{
    DBG(DBG_L4, "xpr::rtsp::H264VideoFramedSource::~H264VideoFramedSource() = %p",
        this);
    envir().taskScheduler().unscheduleDelayedTask(mCurrentTask);
}

void xpr::rtsp::H264VideoFramedSource::doGetNextFrame()
{
    fetchFrame();
}

unsigned int xpr::rtsp::H264VideoFramedSource::maxFrameSize() const
{
    return XPR_RTSP_H264_MAX_FRAME_SIZE;
}

void xpr::rtsp::H264VideoFramedSource::fetchFrame()
{
    // Check video queue, if empty, delay for next.
    if (!mStream->hasVideoFrame()) {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(5000, getNextFrame,
                                                                 this);
        return;
    }
    // Fetch one block from video queue.
    XPR_StreamBlock* ntb = mStream->getVideoFrame();
    if (ntb) {
        fFrameSize = XPR_StreamBlockLength(ntb);
        if (fFrameSize > fMaxSize) {
            fNumTruncatedBytes = fFrameSize - fMaxSize;
            fFrameSize = fMaxSize;
        }
        memcpy(fTo, XPR_StreamBlockData(ntb), fFrameSize);
#if 0
        // Setup PTS with ntb->pts
        if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
            gettimeofday(&fPresentationTime, NULL);
        }
        else {
            int64_t usecs = XPR_StreamBlockPTS(ntb) - mLastPTS;
            fPresentationTime.tv_sec += usecs / 1000000;
            fPresentationTime.tv_usec += usecs % 1000000;
            mLastPTS = XPR_StreamBlockPTS(ntb);
        }
#else
        // Auto filled by H264VideoSteamFramer
        fPresentationTime.tv_sec = 0;
        fPresentationTime.tv_usec = 0;
#endif
        mStream->releaseVideoFrame(ntb);
        if (fNumTruncatedBytes > 0)
            fprintf(stderr, "XPR_RTSP: WARN: %u bytes truncated.\n", fNumTruncatedBytes);
    }
    else {
        // Should never run here
        fprintf(stderr, "XPR_RTSP: BUG: %s:%d\n", __FILE__, __LINE__);
        // Clear
        fFrameSize = 0;
        fNumTruncatedBytes = 0;
    }
    afterGetting(this);
}

xpr::rtsp::Stream* xpr::rtsp::H264VideoFramedSource::stream(void) const
{
    return mStream;
}

void xpr::rtsp::H264VideoFramedSource::getNextFrame(void* ptr)
{
    if (ptr) {
        ((H264VideoFramedSource*)ptr)->fetchFrame();
    }
}

Boolean xpr::rtsp::H264VideoFramedSource::isH264VideoStreamFramer() const
{
    return True;
}

// H264VideoServerMediaSubsession
//============================================================================
xpr::rtsp::H264VideoServerMediaSubsession::H264VideoServerMediaSubsession(
    UsageEnvironment& env, FramedSource* source, xpr::rtsp::Stream* stream)
    : OnDemandServerMediaSubsession(env, True)
    , mSource(source)
    , mSink(NULL)
    , mStream(stream)
    , mDoneFlag(0)
    , mAuxSDPLine(NULL)
{
    DBG(DBG_L4,
        "xpr::rtsp::H264VideoServerMediaSubsession::H264VideoServerMediaSubsession(%p, %p) = %p",
        &env, source, this);
}

xpr::rtsp::H264VideoServerMediaSubsession::~H264VideoServerMediaSubsession(void)
{
    DBG(DBG_L4,
        "xpr::rtsp::H264VideoServerMediaSubsession::~H264VideoServerMediaSubsession() = %p",
        this);
    if (mAuxSDPLine) {
        free(mAuxSDPLine);
        mAuxSDPLine = NULL;
    }
    mSource = NULL;
    mSink = NULL;
    mDoneFlag = 0;
}

static void afterPlayingDummy(void* clientData)
{
    xpr::rtsp::H264VideoServerMediaSubsession* subsess =
        (xpr::rtsp::H264VideoServerMediaSubsession*)clientData;
    subsess->afterPlayingDummy1();
}

void xpr::rtsp::H264VideoServerMediaSubsession::afterPlayingDummy1()
{
    // Unschedule any pending 'checking' task:
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    // Signal the event loop that we're done:
    setDoneFlag();
}

void xpr::rtsp::H264VideoServerMediaSubsession::setDoneFlag()
{
    mDoneFlag = ~0x00;
}

static void checkForAuxSDPLine(void* clientData)
{
    xpr::rtsp::H264VideoServerMediaSubsession* subsess =
        (xpr::rtsp::H264VideoServerMediaSubsession*)clientData;
    subsess->checkForAuxSDPLine1();
}

void xpr::rtsp::H264VideoServerMediaSubsession::checkForAuxSDPLine1()
{
    const char* dasl = NULL;
    nextTask() = NULL;
    if (mAuxSDPLine != NULL) {
        // Signal the event loop that we're done:
        setDoneFlag();
    }
    else if (mSink != NULL && (dasl = mSink->auxSDPLine()) != NULL) {
        mAuxSDPLine = strDup(dasl);
        mSink = NULL;
        DBG(DBG_L4, "H264VideoServerMediaSubsession: AuxSDPLine [%s]", mAuxSDPLine);
        // Signal the event loop that we're done:
        setDoneFlag();
    }
    else if (!mDoneFlag) {
        // try again after a brief delay:
        int uSecsToDelay = 10000; // 10 ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(uSecsToDelay,
                                                                 (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const* xpr::rtsp::H264VideoServerMediaSubsession::getAuxSDPLine(
    RTPSink* rtpSink, FramedSource* inputSource)
{
    if (mAuxSDPLine) {
        return mAuxSDPLine;
    }
    if (mSink == NULL) {
        // we're not already setting it up for another, concurrent stream
        // Note: For H264 video files, the 'config' information ("profile-level-id" and "sprop-parameter-sets") isn't known
        // until we start reading the file.  This means that "rtpSink"s "auxSDPLine()" will be NULL initially,
        // and we need to start reading data from our file until this changes.
        mSink = rtpSink;
        // Start reading the file:
        mSink->startPlaying(*inputSource, afterPlayingDummy, this);
        // Check whether the sink's 'auxSDPLine()' is ready:
        checkForAuxSDPLine(this);
    }
    envir().taskScheduler().doEventLoop(&mDoneFlag);
    return mAuxSDPLine;
}

FramedSource* xpr::rtsp::H264VideoServerMediaSubsession::createNewStreamSource(
    unsigned clientSessionId, unsigned& estBitrate)
{
    printf("XPR_RTSP: createNewStreamSource(%u, %u)\n", clientSessionId, estBitrate);
    estBitrate = 5000;
    H264VideoFramedSource* src = new H264VideoFramedSource(envir(), mStream);
    return H264VideoStreamFramer::createNew(envir(), src, False);
}

RTPSink* xpr::rtsp::H264VideoServerMediaSubsession::createNewRTPSink(
    Groupsock* rtpGroupsock,
    unsigned char rtpPayloadTypeIfDynamic,
    FramedSource* inputSource)
{
    OutPacketBuffer::maxSize = 320000;
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock,
                                       rtpPayloadTypeIfDynamic);
}

xpr::rtsp::H264VideoServerMediaSubsession*
xpr::rtsp::H264VideoServerMediaSubsession::createNew(
    UsageEnvironment& env, FramedSource* source, xpr::rtsp::Stream* stream)
{
    return new H264VideoServerMediaSubsession(env, source, stream);
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

int xpr::rtsp::Server::open(int port, const char* url)
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

int xpr::rtsp::Server::pushData(int port, XPR_StreamBlock* stb)
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

RTSPServer* xpr::rtsp::Server::rtspServer(void)
{
    return mRTSPServer;
}

xpr::rtsp::Stream** xpr::rtsp::Server::streams(void)
{
    return mStreams;
}

xpr::rtsp::Worker* xpr::rtsp::Server::worker(int index) const
{
    return mWorkers[index];
}

xpr::rtsp::Worker** xpr::rtsp::Server::workers(void)
{
    return mWorkers;
}

const char* xpr::rtsp::Server::getBindAddress(void) const
{
    return mBindAddress.c_str();
}

void xpr::rtsp::Server::setBindAddress(const char* bindAddress)
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
    mMaxStreams = MIN(maxStreams, XPR_RTSP_PORT_STREAM_MAX);
}

size_t xpr::rtsp::Server::getMaxStreamTracks(void) const
{
    return mMaxStreamTracks;
}

void xpr::rtsp::Server::setMaxStreamTracks(size_t maxStreamTracks)
{
    mMaxStreamTracks = MIN(maxStreamTracks, XPR_RTSP_PORT_TRACK_MAX);
}

size_t xpr::rtsp::Server::getMaxWorkers(void) const
{
    return mMaxWorkers;
}

void xpr::rtsp::Server::setMaxWorkers(size_t maxWorkers)
{
    mMaxWorkers = MIN(maxWorkers, XPR_RTSP_MAX_WORKERS);
}

bool xpr::rtsp::Server::isValidStreamId(int streamId)
{
    return false;
}

bool xpr::rtsp::Server::isValidStreamTrackId(int streamTrackId)
{
    return false;
}

/// 打开服务器
/// @retval XPR_ERR_GEN_BUSY    服务器正在运行
/// @retval XPR_ERR_OK          服务器打开成功
int xpr::rtsp::Server::openServer(const char* url)
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

int xpr::rtsp::Server::openStream(int port, const char* url)
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

int xpr::rtsp::Server::setupServer(const char* url)
{
    // 解析地址参数
    XPR_Url* u = XPR_UrlParse(url, -1);
    if (u == NULL)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    //
    mUrl = url;
    //
    const char* host = XPR_UrlGetHost(u);
    uint16_t port = XPR_UrlGetPort(u);
    // 设定绑定地址及端口
    if (host)
        setBindAddress(host);
    if (port)
        setBindPort(port);
    // 使用 Query 参数来配置服务器
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

void xpr::rtsp::Server::configServer(const char* query)
{
    if (query) {
        DBG(DBG_L3, "Server configuration query: %s", query);
        xpr_foreach_s(query, -1, "&", xpr::rtsp::Server::handleServerConfig, this);
    }
}

void xpr::rtsp::Server::configServer(const char* key, const char* value)
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

void xpr::rtsp::Server::configStream(int port, const char* query)
{
}

void xpr::rtsp::Server::configStream(int port, const char* key,
                                     const char* value)
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
    mRTSPServer = RTSPServer::createNew(mWorkers[0]->env(), mBindPort, NULL);
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

void xpr::rtsp::Server::handleServerConfig(void* opaque, char* seg)
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
    , mAudioQ(NULL)
    , mVideoQ(NULL)
    , mAQL(30)
    , mVQL(30)
{
    DBG(DBG_L4, "xpr::rtsp::Stream::Stream(%d, %p) = %p", id, parent, this);
    //memset(mFramedSources, 0, sizeof(mFramedSources));
}

xpr::rtsp::Stream::~Stream(void)
{
    DBG(DBG_L4, "xpr::rtsp::Stream::~Stream(%d, %p) = %p", id(), parent(), this);
}

int xpr::rtsp::Stream::open(int port, const char* url)
{
    // 解析地址参数
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
    // 跳过起始 '/'
    if (*path == '/')
        path++;
    DBG(DBG_L4, "stream name: %s", path);
    // 创建服务流会话
    mSMS = ServerMediaSession::createNew(server->worker(0)->env(), path, NULL,
                                         path);
    // 使用 Query 参数来配置
    configStream(XPR_UrlGetQuery(u));
    //
    XPR_UrlDestroy(u);
    // 创建音视频数据队列
    mAudioQ = XPR_FifoCreate(sizeof(uintptr_t), mAQL);
    mVideoQ = XPR_FifoCreate(sizeof(uintptr_t), mVQL);
    //
    return XPR_ERR_OK;
}

// FIXME:
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
    Server* server = (Server*)parent();
    if (mSMS == NULL || server->rtspServer() == NULL)
        return XPR_ERR_GEN_SYS_NOTREADY;
    server->rtspServer()->removeServerMediaSession(mSMS);
    mSMS = NULL;
    return XPR_ERR_OK;
}

int xpr::rtsp::Stream::pushData(int port, XPR_StreamBlock* stb)
{
    if (mSourceType == "stb") {
        switch (stb->codec) {
        case AV_FOURCC_AAC:
        case AV_FOURCC_PCMA:
        case AV_FOURCC_PCMU:
        case AV_FOURCC_G711A:
        case AV_FOURCC_G711U:
            return putAudioFrame(stb);
            break;
        case AV_FOURCC_H264:
        case AV_FOURCC_H265:
        case AV_FOURCC_JPEG:
            return putVideoFrame(stb);
            break;
        }
    }
    return XPR_ERR_GEN_UNEXIST;
}

ServerMediaSession* xpr::rtsp::Stream::sms(void) const
{
    return mSMS;
}

bool xpr::rtsp::Stream::hasAudioFrame(void) const
{
    return !XPR_FifoIsEmpty(mAudioQ);
}

bool xpr::rtsp::Stream::hasVideoFrame(void) const
{
    return !XPR_FifoIsEmpty(mVideoQ);
}

XPR_StreamBlock* xpr::rtsp::Stream::getAudioFrame(void) const
{
    XPR_StreamBlock* ntb = (XPR_StreamBlock*)XPR_FifoGetAsAtomic(mAudioQ);
    return ntb;
}

XPR_StreamBlock* xpr::rtsp::Stream::getVideoFrame(void) const
{
    XPR_StreamBlock* ntb = (XPR_StreamBlock*)XPR_FifoGetAsAtomic(mVideoQ);
    return ntb;
}

int xpr::rtsp::Stream::putAudioFrame(XPR_StreamBlock* stb)
{
    if (!stb || !mAudioQ)
        return XPR_ERR_GEN_NULL_PTR;
    XPR_StreamBlock* ntb = XPR_StreamBlockDuplicate(stb);
    int err = XPR_FifoPutAsAtomic(mAudioQ, (uintptr_t)ntb);
    if (XPR_IS_ERROR(err)) {
        XPR_StreamBlockFree(ntb);
    }
    return err;
}

int xpr::rtsp::Stream::putVideoFrame(XPR_StreamBlock* stb)
{
    if (!stb || !mVideoQ)
        return XPR_ERR_GEN_NULL_PTR;
    XPR_StreamBlock* ntb = XPR_StreamBlockDuplicate(stb);
    int err = XPR_FifoPutAsAtomic(mVideoQ, (uintptr_t)ntb);
    if (XPR_IS_ERROR(err)) {
        XPR_StreamBlockFree(ntb);
    }
    return err;
}

void xpr::rtsp::Stream::releaseAudioFrame(XPR_StreamBlock* stb)
{
    XPR_StreamBlockFree(stb);
}

void xpr::rtsp::Stream::releaseVideoFrame(XPR_StreamBlock* stb)
{
    XPR_StreamBlockFree(stb);
}

void xpr::rtsp::Stream::configStream(const char* query)
{
    if (query) {
        DBG(DBG_L3, "stream configuration: %s", query);
        xpr_foreach_s(query, -1, "&", xpr::rtsp::Stream::handleStreamConfig, this);
    }
}

void xpr::rtsp::Stream::configStream(const char* key, const char* value)
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
                //mFramedSources[trackId] = new H264VideoFramedSource(env);
                //H264VideoFramedSource* src = new H264VideoFramedSource(env, this);
                //printf("stream %p, src %p\n", this, src);
                mSMS->addSubsession(H264VideoServerMediaSubsession::createNew(env, NULL, this));
            }
        }
    }
    else if (strcmp(key, "aql") == 0) {
        mAQL = strtol(value, NULL, 10);
    }
    else if (strcmp(key, "vql") == 0) {
        mVQL = strtol(value, NULL, 10);
    }
    else if (strcmp(key, "asrc") == 0) {
        mAsrcId = strtol(value, NULL, 10);
    }
    else if (strcmp(key, "vsrc") == 0) {
        mVsrcId = strtol(value, NULL, 10);
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
        ((BasicTaskScheduler*)mScheduler)->SingleStep(0);
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
    mThread = XPR_ThreadCreate(xpr::rtsp::Worker::thread, 1024 * 1024 * 1, this);
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

void* xpr::rtsp::Worker::thread(void* opaque, XPR_Thread* thread)
{
    if (opaque)
        ((xpr::rtsp::Worker*)opaque)->run();
    return NULL;
}

#endif // defined(HAVE_XPR_RTSP_DRIVER_LIVE)

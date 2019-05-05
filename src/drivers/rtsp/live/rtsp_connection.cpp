#include "rtsp_connection.hpp"
#include "rtsp_connectionmanager.hpp"
#include "rtsp_worker.hpp"
#include <xpr/xpr_sys.h>
#include <xpr/xpr_meta.h>
#include <xpr/xpr_url.h>
#include <iostream>

namespace xpr
{

namespace rtsp
{

// Convert port id to port
inline int id_to_port(int id)
{
    return XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, id, 0);
}

// MyRTSPClient
//============================================================================
void MyRTSPClient::continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode,
                                         char* resultString)
{
    do {
        DBG(DBG_L4,
            "XPR_RTSP: MyRTSPClient(%p): continueAfterDESCRIBE(): code = %d, "
            "result = \"%s\"",
            rtspClient, resultCode, resultString);
        UsageEnvironment& env = rtspClient->envir();
        StreamClientState& scs = ((MyRTSPClient*)rtspClient)->mScs;
        if (resultCode != 0) {
            delete[] resultString;
            break;
        }
        char* const sdpDescription = resultString;
        scs.session = MediaSession::createNew(env, sdpDescription);
        delete[] sdpDescription;
        if (scs.session == NULL) {
            DBG(DBG_L4,
                "XPR_RTSP: MyRTSPClient(%p): MediaSession::createNew() failed!",
                rtspClient);
            break;
        }
        else if (!scs.session->hasSubsessions()) {
            DBG(DBG_L4,
                "XPR_RTSP: MyRTSPClient(%p): MediaSession::hasSubsessions() == "
                "false, aborted!",
                rtspClient);
            break;
        }
        scs.iter = new MediaSubsessionIterator(*scs.session);
        setupNextSubsession(rtspClient);
        // Update last active timestamp
        ((MyRTSPClient*)rtspClient)->updateLATS();
        return;
    } while (0);
    // An unrecoverable error occurred with this stream.
    streamError(rtspClient, resultCode);
}

void MyRTSPClient::setupNextSubsession(RTSPClient* rtspClient)
{
    StreamClientState& scs = ((MyRTSPClient*)rtspClient)->mScs;
    Boolean streamUsingTCP = ((MyRTSPClient*)rtspClient)->isStreamUsingTCP();
    scs.subsession = scs.iter->next();
    if (scs.subsession != NULL) {
#if 0
        // Skip not H264
        if (strcmp(scs.subsession->codecName(), "H264")) {
            std::cout << "Skip non H264 subsession!" << std::endl;
            setupNextSubsession(rtspClient);
            return;
        }
#endif
        if (!scs.subsession->initiate()) {
            setupNextSubsession(rtspClient);
        }
        else {
            if (scs.subsession->rtcpIsMuxed()) {
                // FIXME:
            }
            else {
                // FIXME:
            }
            rtspClient->sendSetupCommand(
                *scs.subsession, continueAfterSETUP, False, streamUsingTCP,
                False, ((MyRTSPClient*)rtspClient)->getAuthenticator());
        }
        scs.tracks++;
        // Update last active timestamp
        ((MyRTSPClient*)rtspClient)->updateLATS();
        return;
    }
    if (scs.session->absStartTime() != NULL) {
        rtspClient->sendPlayCommand(
            *scs.session, continueAfterPLAY, scs.session->absStartTime(),
            scs.session->absEndTime(), 1.0f,
            ((MyRTSPClient*)rtspClient)->getAuthenticator());
    }
    else {
        scs.duration =
            scs.session->playEndTime() - scs.session->playStartTime();
        rtspClient->sendPlayCommand(
            *scs.session, continueAfterPLAY, 0.0f, -1.0f, 1.0f,
            ((MyRTSPClient*)rtspClient)->getAuthenticator());
    }
    // Update last active timestamp
    ((MyRTSPClient*)rtspClient)->updateLATS();
}

void MyRTSPClient::continueAfterSETUP(RTSPClient* rtspClient, int resultCode,
                                      char* resultString)
{
    do {
        UsageEnvironment& env = rtspClient->envir();
        StreamClientState& scs = ((MyRTSPClient*)rtspClient)->mScs;
        if (resultCode != 0) {
            DBG(DBG_L4,
                "XPR_RTSP: MyRTSPClient(%p): continueAfterSETUP(): code = "
                "%d, result = \"%s\"",
                rtspClient, resultCode, resultString);
            break;
        }
        if (scs.subsession->rtcpIsMuxed()) {
            // FIXME:
        }
        else {
            // FIXME:
        }
        scs.subsession->sink = DummySink::createNew(
            env, (MyRTSPClient*)rtspClient, scs.subsession, scs.tracks);
        if (scs.subsession->sink == NULL) {
            break;
        }
        scs.subsession->miscPtr = rtspClient;
        scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
                                           subsessionAfterPlaying,
                                           scs.subsession);
        if (scs.subsession->rtcpInstance() != NULL) {
            scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler,
                                                          scs.subsession);
        }
    } while (0);
    delete[] resultString;
    // Set up the next subsession, if any:
    setupNextSubsession(rtspClient);
    // Update last active timestamp
    ((MyRTSPClient*)rtspClient)->updateLATS();
}

void MyRTSPClient::continueAfterPLAY(RTSPClient* rtspClient, int resultCode,
                                     char* resultString)
{
    Boolean success = False;
    do {
        UsageEnvironment& env = rtspClient->envir();
        StreamClientState& scs = ((MyRTSPClient*)rtspClient)->mScs;
        if (resultCode != 0) {
            DBG(DBG_L4,
                "XPR_RTSP: MyRTSPClient(%p): continueAfterPLAY(): code = %d, "
                "result = \"%s\"",
                rtspClient, resultCode, resultString);
            break;
        }
        if (scs.duration > 0) {
            unsigned const delaySlop = 2;
            scs.duration += delaySlop;
            unsigned uSecsToDelay = (unsigned)(scs.duration * 1000000);
            scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(
                uSecsToDelay, (TaskFunc*)streamTimerHandler, rtspClient);
        }
        if (scs.duration > 0) {
            // FIXME:
        }
        success = True;
    } while (0);
    delete[] resultString;
    if (!success) {
        // An unrecoverable error occurred with this stream.
        streamError(rtspClient, resultCode);
    }
    // Mark as playing
    ((MyRTSPClient*)rtspClient)->setPlaying(success);
    // Update last active timestamp
    ((MyRTSPClient*)rtspClient)->updateLATS();
}

void MyRTSPClient::continueAfterGET_PARAMETER(RTSPClient* rtspClient,
                                              int resultCode,
                                              char* resultString)
{
    (void)resultCode;
    if (rtspClient) {
        // printf("GET_PARAMETER %d, %s\n", resultCode, resultString);
        // Update last active timestamp
        ((MyRTSPClient*)rtspClient)->updateLATS();
    }
    delete[] resultString;
}

// Implementation of the other event handlers:
void MyRTSPClient::subsessionAfterPlaying(void* clientData)
{
    MediaSubsession* subsession = (MediaSubsession*)clientData;
    RTSPClient* rtspClient = (RTSPClient*)(subsession->miscPtr);
    // Begin by closing this subsession's stream:
    Medium::close(subsession->sink);
    subsession->sink = NULL;
    // Next, check whether *all* subsessions' streams have now been closed:
    MediaSession& session = subsession->parentSession();
    MediaSubsessionIterator iter(session);
    while ((subsession = iter.next()) != NULL) {
        if (subsession->sink != NULL)
            return; // this subsession is still active
    }
    // All subsessions' streams have now been closed, so shutdown the client:
    // shutdownStream(rtspClient);
    streamError(rtspClient, ENODATA);
}

void MyRTSPClient::subsessionByeHandler(void* clientData)
{
    MediaSubsession* subsession = (MediaSubsession*)clientData;
    // Now act as if the subsession had closed:
    subsessionAfterPlaying(subsession);
}

void MyRTSPClient::streamTimerHandler(void* clientData)
{
    MyRTSPClient* rtspClient = (MyRTSPClient*)clientData;
    StreamClientState& scs = rtspClient->mScs;
    scs.streamTimerTask = NULL;
    // Shut down the stream:
    streamError(rtspClient, ETIMEDOUT);
}

void MyRTSPClient::shutdownStream(RTSPClient* rtspClient, int exitCode)
{
    (void)exitCode;
    StreamClientState& scs = ((MyRTSPClient*)rtspClient)->mScs;
    Connection* conn = ((MyRTSPClient*)rtspClient)->getParent();
    // First, check whether any subsessions have still to be closed:
    if (scs.session != NULL) {
        Boolean someSubsessionsWereActive = False;
        MediaSubsessionIterator iter(*scs.session);
        MediaSubsession* subsession;
        while ((subsession = iter.next()) != NULL) {
            if (subsession->sink != NULL) {
                Medium::close(subsession->sink);
                subsession->sink = NULL;
                if (subsession->rtcpInstance() != NULL) {
                    subsession->rtcpInstance()->setByeHandler(NULL, NULL);
                }
                someSubsessionsWereActive = True;
            }
        }
        if (someSubsessionsWereActive) {
            // Send a RTSP "TEARDOWN" command, to tell the server to shutdown
            // the stream. Don't bother handling the response to the "TEARDOWN".
            rtspClient->sendTeardownCommand(
                *scs.session, NULL,
                ((MyRTSPClient*)rtspClient)->getAuthenticator());
        }
    }
    Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState"
    // structure to get reclaimed.
    conn->stateChanged(id_to_port(conn->id()), XPR_RTSP_STATE_STOP_CLOSE);
}

void MyRTSPClient::keepAlive(RTSPClient* rtspClient)
{
    if (!rtspClient)
        return;
    StreamClientState& scs = dynamic_cast<MyRTSPClient*>(rtspClient)->mScs;
    if (!scs.session)
        return;
    rtspClient->sendGetParameterCommand(
        *scs.session, continueAfterGET_PARAMETER, NULL,
        ((MyRTSPClient*)rtspClient)->getAuthenticator());
}

void MyRTSPClient::streamError(RTSPClient* rtspClient, int err)
{
    dynamic_cast<MyRTSPClient*>(rtspClient)->handleError(err);
}

// Implementation of "MyRTSPClient":
MyRTSPClient* MyRTSPClient::createNew(Connection* parent)
{
    Worker* worker = parent->worker();
    MyRTSPClient* c =
        new MyRTSPClient(worker->env(), parent->url().c_str(), parent);
    return c;
}

MyRTSPClient::MyRTSPClient(UsageEnvironment& env, const char* url,
                           Connection* parent)
    : RTSPClient(env, url, 0, NULL, 0, -1)
    , mParent(parent)
    , mAuth(nullptr)
    , mScs()
    , mIsPlaying(false)
    , mLastActiveTS(0)
{
    DBG(DBG_L5, "XPR_RTSP: MyRTSPClient::MyRTSPClient(%p,%s,%p) = %p", &env,
        url, parent, this);
}

MyRTSPClient::~MyRTSPClient()
{
    DBG(DBG_L5, "XPR_RTSP: MyRTSPClient::~MyRTSPClient(%p)", this);
    if (mAuth) {
        delete mAuth;
        mAuth = nullptr;
    }
}

Authenticator* MyRTSPClient::getAuthenticator(void)
{
    return mAuth;
}

void MyRTSPClient::setAuthenticator(const char* username, const char* password)
{
    if (mAuth)
        delete mAuth;
    mAuth = new Authenticator(username, password, false);
}

bool MyRTSPClient::isStreamUsingTCP() const
{
    return mStreamUsingTCP;
}

void MyRTSPClient::setStreamUsingTCP(bool streamUsingTCP)
{
    mStreamUsingTCP = streamUsingTCP;
}

Connection* MyRTSPClient::getParent(void)
{
    return mParent;
}

void MyRTSPClient::handleError(int err)
{
    if (err == ENODATA)
        setPlaying(false);
    else if (err == ETIMEDOUT)
        setTimeouted(true);
    else if (err == 404)
        setPlaying(false);
}

bool MyRTSPClient::isPlaying() const
{
    return mIsPlaying;
}

void MyRTSPClient::setPlaying(bool yes)
{
    StateTransition tr;
    if (!mIsPlaying && yes)
        tr = XPR_RTSP_STATE_START_PLAYING;
    else if (mIsPlaying && !yes)
        tr = XPR_RTSP_STATE_PLAYING_STOP;
    else if (!mIsPlaying && !yes)
        tr = XPR_RTSP_STATE_START_STOP;
    mIsPlaying = yes;
    if (mParent)
        mParent->stateChanged(id_to_port(mParent->id()), tr);
}

#define CS_TMO 65000000
#define RX_TMO 5000000

bool MyRTSPClient::isTimeouted() const
{
    int64_t tmo = mIsPlaying ? RX_TMO : CS_TMO;
    int64_t late = XPR_SYS_GetCTS() - mLastActiveTS;
    return late > tmo;
}

void MyRTSPClient::setTimeouted(bool yes)
{
    StateTransition tr = mIsPlaying ? XPR_RTSP_STATE_PLAYING_TIMEOUT
                                    : XPR_RTSP_STATE_START_TIMEOUT;
    if (yes && mParent)
        mParent->stateChanged(id_to_port(mParent->id()), tr);
}

void MyRTSPClient::updateLATS(void)
{
    mLastActiveTS = XPR_SYS_GetCTS();
}

// Implementation of "StreamClientState":
StreamClientState::StreamClientState()
    : iter(NULL)
    , session(NULL)
    , subsession(NULL)
    , streamTimerTask(NULL)
    , duration(0.0)
    , tracks(0)
{
}

StreamClientState::~StreamClientState()
{
    delete iter;
    if (session != NULL) {
        // We also need to delete "session", and unschedule "streamTimerTask"
        // (if set)
        UsageEnvironment& env = session->envir();
        env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
        Medium::close(session);
    }
}

// Implementation of "DummySink":

static inline bool isAudioFourcc(uint32_t fourcc)
{
    return fourcc == AV_FOURCC_AAC || fourcc == AV_FOURCC_G711A ||
           fourcc == AV_FOURCC_G711U || fourcc == AV_FOURCC_PCMA ||
           fourcc == AV_FOURCC_PCMU || fourcc == AV_FOURCC_S16B ||
           fourcc == AV_FOURCC_S16L;
}

static inline bool isVideoFourcc(uint32_t fourcc)
{
    return fourcc == AV_FOURCC_H264 || fourcc == AV_FOURCC_H265 ||
           fourcc == AV_FOURCC_HEVC || fourcc == AV_FOURCC_JPEG;
}

DummySink* DummySink::createNew(UsageEnvironment& env, MyRTSPClient* client,
                                MediaSubsession* subsession, int trackId)
{
    return new DummySink(env, client, subsession, trackId);
}

DummySink::DummySink(UsageEnvironment& env, MyRTSPClient* client,
                     MediaSubsession* subsession, int trackId)
    : MediaSink(env)
    , mClient(client)
    , mSubsession(subsession)
    , mBuffer(nullptr)
    , mMaxFrameSize(64512)
    , mFourcc(0)
    , mMeta(NULL)
    , mPTS(0)
    , mTrackId(trackId)
    , mStreamBlock()
{
    DBG(DBG_L5, "XPR_RTSP: DummySink::DummySink(%p,%p,%p,%d) = %p", &env,
        client, subsession, trackId, this);
    mBuffer = new uint8_t[mMaxFrameSize];
    memset(&mStreamBlock, 0, sizeof(mStreamBlock));
    const char* cn = subsession->codecName();
    DBG(DBG_L4, "XPR_RTSP: DummySink(%p): codecName = %s", this, cn);
    if (strcmp(cn, "AAC") == 0)
        mFourcc = AV_FOURCC_AAC;
    else if (strcmp(cn, "H264") == 0)
        mFourcc = AV_FOURCC_H264;
    else if (strcmp(cn, "JPEG") == 0)
        mFourcc = AV_FOURCC_JPEG;
    else if (strcmp(cn, "MPEG4-GENERIC") == 0) {
        const char* mode = subsession->attrVal_strToLower("mode");
        if (mode) {
            if (strncmp(mode, "aac", 3) == 0)
                mFourcc = AV_FOURCC_AAC;
        }
    }
    else if (strcmp(cn, "PCMA") == 0)
        mFourcc = AV_FOURCC_PCMA;
    else if (strcmp(cn, "PCMU") == 0)
        mFourcc = AV_FOURCC_PCMU;
    if (mFourcc == AV_FOURCC_AAC || mFourcc == AV_FOURCC_PCMA ||
        AV_FOURCC_PCMU) {
        XPR_AudioMeta* meta =
            reinterpret_cast<XPR_AudioMeta*>(malloc(sizeof(XPR_AudioMeta)));
        meta->numOfChannels = subsession->numChannels();
        meta->samplingFrequency = subsession->rtpTimestampFrequency();
        mMeta = meta;
    }
    mStreamBlock.codec = mFourcc;
    mStreamBlock.track = mTrackId;
    mStreamBlock.meta = mMeta;
    if (isAudioFourcc(mFourcc))
        mStreamBlock.flags |= XPR_STREAMBLOCK_FLAG_AUDIO_FRAME;
    if (isVideoFourcc(mFourcc))
        mStreamBlock.flags |= XPR_STREAMBLOCK_FLAG_VIDEO_FRAME;
}

DummySink::~DummySink()
{
    DBG(DBG_L5, "XPR_RTSP: DummySink::~DummySink(%p)", this);
    if (mBuffer) {
        delete mBuffer;
    }
    if (mMeta) {
        free(mMeta);
    }
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds)
{
    DummySink* sink = (DummySink*)clientData;
    sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime,
                            durationInMicroseconds);
}

static inline unsigned alignUpTo16K(unsigned val)
{
    return ((val >> 14) + 1) << 14;
}

void DummySink::afterGettingFrame(unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds)
{
    uint8_t* newBuffer = NULL;
    size_t newMaxFrameSize = 0;

    if (numTruncatedBytes) {
        DBG(DBG_L3, "XPR_RTSP: DummySink(%p): %d bytes truncated", this,
            numTruncatedBytes);
        // Increase the maximum frame size
        newMaxFrameSize = mMaxFrameSize + alignUpTo16K(numTruncatedBytes);
        DBG(DBG_L3, "XPR_RTSP: DummySink(%p): mMaxFrameSize increased to %d",
            this, newMaxFrameSize);
        newBuffer = new uint8_t[newMaxFrameSize];
    }

    int64_t pts = presentationTime.tv_sec;
    pts *= 1000000;
    pts += presentationTime.tv_usec;

    mPTS = pts;

    // Update last active timestamp
    mClient->updateLATS();

    // Fill stream block
    mStreamBlock.buffer = mBuffer;
    mStreamBlock.bufferSize = mMaxFrameSize;
    mStreamBlock.data = mBuffer + 4;
    mStreamBlock.dataSize = frameSize;
    mStreamBlock.dts = mStreamBlock.pts = pts;

    // If H264 data without start code,
    // Rewind the stb.data to head of the buffer.
    if (mFourcc == AV_FOURCC_H264) {
        uint8_t* p = mStreamBlock.data;
        if (p[0] != 0x00 && p[1] != 0x00 && p[2] != 0x00 && p[3] != 0x01) {
            mStreamBlock.data = mBuffer;
            mStreamBlock.dataSize += 4;
        }
    }

    // Push data to callbacks
    Connection* conn = mClient->getParent();
    conn->pushData(id_to_port(conn->id()), &mStreamBlock);

    // Replace the new buffer and size
    if (newBuffer && newMaxFrameSize) {
        delete mBuffer;
        mBuffer = newBuffer;
        mMaxFrameSize = newMaxFrameSize;
    }

    // Then continue, to request the next frame of data:
    continuePlaying();
}

Boolean DummySink::continuePlaying()
{
    if (fSource == NULL) {
        DBG(DBG_L2, "XPR_RTSP: DummySink(%p): Exit playing by fSource == NULL",
            this);
        return False; // sanity check (should not happen)
    }

    // Pre-Buffered start code for H264
    mBuffer[0] = 0x00;
    mBuffer[1] = 0x00;
    mBuffer[2] = 0x00;
    mBuffer[3] = 0x01;

    // Request the next frame of data from our input source.
    // "afterGettingFrame()" will get called later, when it arrives:
    fSource->getNextFrame(mBuffer + 4, mMaxFrameSize - 4, afterGettingFrame,
                          this, onSourceClosure, this);

    return True;
}

// Connection
//============================================================================
Connection::Connection(int id, Port* parent)
    : Port(id, parent), mError(0), mWorker(nullptr), mRtpOverTcp(false)
{
    DBG(DBG_L5, "XPR_RTSP: Connection::Connection(%d, %p) = %p", id, parent,
        this);
}

Connection::~Connection(void)
{
    DBG(DBG_L5, "XPR_RTSP: Connection::~Connection(%d, %p) = %p", id(),
        parent(), this);
}

int Connection::open(int port, const char* url)
{
    DBG(DBG_L4, "XPR_RTSP: Connection(%p): open(%08X, %s)", this, port, url);
    // If the port is opened, do nothing
    if (activeFlags() & PortFlags::PORT_FLAG_OPEN)
        return XPR_ERR_OK;
    // If the port closed, clear all flags
    if (activeFlags() & PortFlags::PORT_FLAG_CLOSE)
        activeFlags(PortFlags::PORT_FLAG_NULL);
    ConnectionManager* connMgr = (ConnectionManager*)parent();
    mWorker = connMgr->getPreferWorker(XPR_RTSP_PORT_MINOR(port));
    // 解析地址参数
    XPR_Url* u = XPR_UrlParse(url, -1);
    if (u == NULL)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    mUrl = url;
    const char* path = XPR_UrlGetPath(u);
    if (!path) {
        XPR_UrlDestroy(u);
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    }
    // 跳过起始 '/'
    if (*path == '/')
        path++;
    DBG(DBG_L4, "XPR_RTSP: Connection(%p): name: %s", this, path);
    // 使用 Query 参数来配置
    configConnection(XPR_UrlGetQuery(u));
    XPR_UrlDestroy(u);
    activeFlags(PortFlags::PORT_FLAG_OPEN);
    return XPR_ERR_OK;
}

int Connection::close(int port)
{
    DBG(DBG_L4, "XPR_RTSP: Connection(%p): close(%08X)", this, port);
    if (activeFlags() & PortFlags::PORT_FLAG_CLOSE)
        return XPR_ERR_OK;
    if (activeFlags() & PortFlags::PORT_FLAG_PENDING)
        return XPR_ERR_GEN_BUSY;
    // Must be stop first if started
    if ((activeFlags() & PortFlags::PORT_FLAG_START) &&
        (!(activeFlags() & PortFlags::PORT_FLAG_STOP)))
        return XPR_ERR_GEN_BUSY;
    clearCallbacks();
    activeFlags(PortFlags::PORT_FLAG_NULL);
    return 0;
}

int Connection::start(int port)
{
    DBG(DBG_L4, "XPR_RTSP: Connection(%p): start(%08X)", this, port);
    if (!(activeFlags() & PortFlags::PORT_FLAG_OPEN))
        return XPR_ERR_GEN_SYS_NOTREADY;
    if (activeFlags() & PortFlags::PORT_FLAG_PENDING)
        return XPR_ERR_GEN_BUSY;
    if (worker() == NULL)
        return XPR_ERR_GEN_SYS_NOTREADY;
    int ret = worker()->asyncTask(port, XPR_RTSP_TASK_START);
    if (ret == XPR_ERR_OK)
        activeFlags(PortFlags::PORT_FLAG_PENDING, 0);
    return ret;
}

int Connection::stop(int port)
{
    DBG(DBG_L4, "XPR_RTSP: Connection(%p): stop(%08X)", this, port);
    if (!(activeFlags() & PortFlags::PORT_FLAG_START))
        return XPR_ERR_GEN_SYS_NOTREADY;
    if (activeFlags() & PortFlags::PORT_FLAG_PENDING)
        return XPR_ERR_GEN_BUSY;
    if (worker() == NULL)
        return XPR_ERR_GEN_SYS_NOTREADY;
    int ret = worker()->asyncTask(port, XPR_RTSP_TASK_STOP);
    if (ret == XPR_ERR_OK)
        activeFlags(PortFlags::PORT_FLAG_PENDING, 0);
    return ret;
}

int Connection::pushData(int port, XPR_StreamBlock* stb)
{
    for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
        Callback* cb = &mCallbacks[i];
        if (cb->dcb == NULL)
            continue;
        cb->dcb(cb->dcb_opaque, port, stb);
    }
    return XPR_ERR_OK;
}

int Connection::postEvent(int port, const XPR_RTSP_EVD* evd)
{
    for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
        Callback* cb = &mCallbacks[i];
        if (cb->evcb == NULL)
            continue;
        cb->evcb(cb->evcb_opaque, port, evd);
    }
    return XPR_ERR_OK;
}

int Connection::runTask(int port, TaskId task)
{
    DBG(DBG_L4, "XPR_RTSP: Connection(%p): runTask(%08X, %d)", this, port,
        task);
    switch (task) {
    case XPR_RTSP_TASK_START:
        return startInTask(port);
    case XPR_RTSP_TASK_STOP:
        return stopInTask(port);
    default:
        return XPR_ERR_GEN_NOT_SUPPORT;
    }
}

int Connection::stateChanged(int port, StateTransition transition)
{
    DBG(DBG_L4, "XPR_RTSP: Connection(%p): stateChanged(%08X, %d)", this, port,
        transition);
    switch (transition) {
    case XPR_RTSP_STATE_START_PLAYING: {
        activeFlags(PortFlags::PORT_FLAG_START, PortFlags::PORT_FLAG_PENDING);
        XPR_RTSP_EVD evd = {XPR_RTSP_EVT_SRC_STARTED, NULL, 0};
        postEvent(port, &evd);
        break;
    }
    case XPR_RTSP_STATE_PLAYING_STOP: {
        XPR_RTSP_EVD evd = {XPR_RTSP_EVT_SRC_STOPPED, NULL, 0};
        postEvent(port, &evd);
        break;
    }
    case XPR_RTSP_STATE_START_STOP: {
        activeFlags(PortFlags::PORT_FLAG_START, PortFlags::PORT_FLAG_PENDING);
        XPR_RTSP_EVD evd = {XPR_RTSP_EVT_STOPPED, NULL, 0};
        postEvent(port, &evd);
        break;
    }
    case XPR_RTSP_STATE_STOP_CLOSE: {
        if ((activeFlags() & PortFlags::PORT_FLAG_START) &&
            (activeFlags() & PortFlags::PORT_FLAG_PENDING)) {
            activeFlags(PortFlags::PORT_FLAG_STOP,
                        PortFlags::PORT_FLAG_PENDING);
        };
        XPR_RTSP_EVD evd = {XPR_RTSP_EVT_STOPPED, NULL, 0};
        postEvent(port, &evd);
        break;
    }
    default:
        break;
    }
    return XPR_ERR_OK;
}

Worker* Connection::worker(void)
{
    return mWorker;
}

void Connection::configConnection(const char* query)
{
    if (query && query[0]) {
        DBG(DBG_L4, "XPR_RTSP: Connection(%p): configuration: \"%s\"", this, query);
        xpr_foreach_s(query, -1, "&", Connection::handleConnectionConfig, this);
    }
}

void Connection::configConnection(const char* key, const char* value)
{
    DBG(DBG_L4, "XPR_RTSP: Connection(%p): configuration: \"%s\"=\"%s\"", this,
        key, value);
    ConnectionManager* connMgr = (ConnectionManager*)parent();
    if (strcmp(key, "rtspOverTcp") == 0) {
        mRtpOverTcp = strtol(value, NULL, 10);
    }
    else {
        DBG(DBG_L4,
            "XPR_RTSP: Connection(%p): configuration: \"%s\" unsupported.",
            this, key);
    }
}

int Connection::startInTask(int port)
{
    DBG(DBG_L4, "XPR_RTSP: Connection(%p): startInTask(%08X)", this, port);
    mClient = MyRTSPClient::createNew(this);
    if (mClient) {
        mClient->setAuthenticator(mUsername.c_str(), mPassword.c_str());
        mClient->setStreamUsingTCP(mRtpOverTcp);
        // Prepare the timestamp for timeout detection
        mClient->updateLATS();
        // Send DESCRIBE command first
        (void)mClient->sendDescribeCommand(&MyRTSPClient::continueAfterDESCRIBE,
                                           mClient->getAuthenticator());
        activeFlags(PortFlags::PORT_FLAG_START, PortFlags::PORT_FLAG_PENDING);
        return XPR_ERR_OK;
    }
    return XPR_ERR_GEN_NOMEM;
}

int Connection::stopInTask(int port)
{
    DBG(DBG_L4, "XPR_RTSP: Connection(%p): stopInTask(%08X)", this, port);
    if (mClient == NULL)
        return XPR_ERR_GEN_SYS_NOTREADY;
    MyRTSPClient::shutdownStream(mClient);
    mClient = NULL;
    // The activeFlags changed in stateChanged()
    return XPR_ERR_OK;
}

void Connection::handleConnectionConfig(void* opaque, char* seg)
{
    if (opaque && seg) {
        char* key = NULL;
        char* value = NULL;
        if (xpr_split_to_kv(seg, &key, &value) == XPR_ERR_OK)
            ((Connection*)opaque)->configConnection(key, value);
    }
}

} // namespace rtsp
} // namespace xpr

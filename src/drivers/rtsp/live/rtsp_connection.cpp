#include "rtsp_connection.hpp"
#include "rtsp_connectionmanager.hpp"
#include "rtsp_framemerger.hpp"
#include "rtsp_worker.hpp"
#include <xpr/xpr_sys.h>
#include <xpr/xpr_meta.h>
#include <xpr/xpr_url.h>
#include <iostream>

namespace xpr
{

namespace rtsp
{

#define CS_TMO 65000000
#define KA_TMO 30000000
#define RX_TMO 5000000
#define H264_HISI_FRAME_TYPE 1

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
    MyRTSPClient* my = reinterpret_cast<MyRTSPClient*>(rtspClient);
    do {
        DBG(DBG_L4,
            "XPR_RTSP: MyRTSPClient(%p): continueAfterDESCRIBE(): code = %d, "
            "result = \"%s\"",
            rtspClient, resultCode, resultString);
        UsageEnvironment& env = rtspClient->envir();
        StreamClientState& scs = my->mScs;
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
        my->updateLATS();
        return;
    } while (0);
    // An unrecoverable error occurred with this stream.
    streamError(rtspClient, resultCode);
}

void MyRTSPClient::setupNextSubsession(RTSPClient* rtspClient)
{
    MyRTSPClient* my = reinterpret_cast<MyRTSPClient*>(rtspClient);
    StreamClientState& scs = my->mScs;
    Boolean streamUsingTCP = my->isStreamUsingTCP();
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
            rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP,
                                         False, streamUsingTCP, False,
                                         my->getAuthenticator());
        }
        scs.tracks++;
        // Update last active timestamp
        my->updateLATS();
        return;
    }
    if (scs.session->absStartTime() != NULL) {
        rtspClient->sendPlayCommand(
            *scs.session, continueAfterPLAY, scs.session->absStartTime(),
            scs.session->absEndTime(), 1.0f, my->getAuthenticator());
    }
    else {
        scs.duration =
            scs.session->playEndTime() - scs.session->playStartTime();
        rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY, 0.0f,
                                    -1.0f, 1.0f, my->getAuthenticator());
    }
    // Update last active timestamp
    my->updateLATS();
}

void MyRTSPClient::continueAfterSETUP(RTSPClient* rtspClient, int resultCode,
                                      char* resultString)
{
    MyRTSPClient* my = reinterpret_cast<MyRTSPClient*>(rtspClient);
    do {
        UsageEnvironment& env = rtspClient->envir();
        StreamClientState& scs = my->mScs;
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
        scs.subsession->sink =
            DummySink::createNew(env, my, scs.subsession, scs.tracks);
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
    my->updateLATS();
}

void MyRTSPClient::continueAfterPLAY(RTSPClient* rtspClient, int resultCode,
                                     char* resultString)
{
    MyRTSPClient* my = reinterpret_cast<MyRTSPClient*>(rtspClient);
    Boolean success = False;
    UsageEnvironment& env = rtspClient->envir();
    do {
        StreamClientState& scs = my->mScs;
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
    my->setPlaying(success);
    // Update last active timestamp
    my->updateLATS();
    // Setup keep alive timer task
    my->mKeepAliveTask = env.taskScheduler().scheduleDelayedTask(
        KA_TMO, (TaskFunc*)keepAlive, rtspClient);
    // Setup timeout check task
    my->mTimeoutCheckTask = env.taskScheduler().scheduleDelayedTask(
        my->mRxTimeout, (TaskFunc*)timeoutCheck, rtspClient);
}

void MyRTSPClient::continueAfterGET_PARAMETER(RTSPClient* rtspClient,
                                              int resultCode,
                                              char* resultString)
{
    UsageEnvironment& env = rtspClient->envir();
    MyRTSPClient* my = reinterpret_cast<MyRTSPClient*>(rtspClient);

    if (!my)
        return;

    if (resultCode != 0) {
        DBG(DBG_L4,
            "XPR_RTSP: MyRTSPClient(%p): continueAfterGET_PARAMETER(): code = %d, "
            "result = \"%s\"",
            rtspClient, resultCode, resultString);
        my->mKeepAliveTask = nullptr;
        timeoutCheckCancel(my);
        streamError(rtspClient, ETIMEDOUT);
    }
    else {
        if (my->isTimeouted()) {
            DBG(DBG_L3, "XPR_RTSP: MyRTSPClient(%p): timeouted by KA!");
            my->mKeepAliveTask = nullptr;
            timeoutCheckCancel(my);
            streamError(rtspClient, ETIMEDOUT);
        }
        else {
            // Setup keep alive timer task
            my->mKeepAliveTask = env.taskScheduler().scheduleDelayedTask(
                KA_TMO, (TaskFunc*)keepAlive, rtspClient);
        }
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
    keepAliveCancel(rtspClient);
    timeoutCheckCancel(rtspClient);
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
    MyRTSPClient* my = reinterpret_cast<MyRTSPClient*>(clientData);
    StreamClientState& scs = my->mScs;
    scs.streamTimerTask = NULL;
    // Shut down the stream:
    streamError(my, ETIMEDOUT);
}

void MyRTSPClient::shutdownStream(RTSPClient* rtspClient, int exitCode)
{
    (void)exitCode;
    MyRTSPClient* my = reinterpret_cast<MyRTSPClient*>(rtspClient);
    StreamClientState& scs = my->mScs;
    Connection* conn = my->getParent();
    // Cancel all delayed tasks first.
    keepAliveCancel(my);
    timeoutCheckCancel(my);
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
            rtspClient->sendTeardownCommand(*scs.session, NULL,
                                            my->getAuthenticator());
        }
    }
    Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState"
    // structure to get reclaimed.
    conn->stateChanged(id_to_port(conn->id()), XPR_RTSP_STATE_STOP_CLOSE);
}

void MyRTSPClient::keepAlive(RTSPClient* rtspClient)
{
    MyRTSPClient* my = reinterpret_cast<MyRTSPClient*>(rtspClient);
    if (!my)
        return;
    StreamClientState& scs = my->mScs;
    if (!scs.session)
        return;
    rtspClient->sendGetParameterCommand(
        *scs.session, continueAfterGET_PARAMETER, NULL, my->getAuthenticator());
}

void MyRTSPClient::keepAliveCancel(RTSPClient* rtspClient)
{
    MyRTSPClient* my = reinterpret_cast<MyRTSPClient*>(rtspClient);
    if (my->mKeepAliveTask) {
        my->envir().taskScheduler().unscheduleDelayedTask(
            my->mKeepAliveTask);
        my->mKeepAliveTask = nullptr;
    }
}

void MyRTSPClient::streamError(RTSPClient* rtspClient, int err)
{
    dynamic_cast<MyRTSPClient*>(rtspClient)->handleError(err);
}

void MyRTSPClient::timeoutCheck(RTSPClient* rtspClient)
{
    MyRTSPClient* my = reinterpret_cast<MyRTSPClient*>(rtspClient);
    if (!my)
        return;
    if (my->isTimeouted()) {
        DBG(DBG_L3, "XPR_RTSP: MyRTSPClient(%p): timeouted by TS!", my);
        my->mTimeoutCheckTask = nullptr;
        keepAliveCancel(my);
        streamError(my, ETIMEDOUT);
    }
    else {
        my->mTimeoutCheckTask = my->envir().taskScheduler().scheduleDelayedTask(
            my->mRxTimeout, (TaskFunc*)timeoutCheck, rtspClient);
    }
}

void MyRTSPClient::timeoutCheckCancel(RTSPClient* rtspClient)
{
    MyRTSPClient* my = reinterpret_cast<MyRTSPClient*>(rtspClient);
    if (my->mTimeoutCheckTask) {
        my->envir().taskScheduler().unscheduleDelayedTask(
            my->mTimeoutCheckTask);
        my->mTimeoutCheckTask = nullptr;
    }
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
    , mStreamUsingTCP(false)
    , mConTimeout(CS_TMO)
    , mRxTimeout(RX_TMO)
    , mUseFrameMerger(false)
    , mOutputFormats()
    , mAppendOriginPTS(0)
    , mIsPlaying(false)
    , mLastActiveTS(0)
    , mKeepAliveTask(nullptr)
    , mTimeoutCheckTask(nullptr)
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

void MyRTSPClient::setConnectTiemout(int conTimeout)
{
    mConTimeout = conTimeout;
}

void MyRTSPClient::setReceiveTimeout(int rxTimeout)
{
    mRxTimeout = rxTimeout;
}

void MyRTSPClient::setUseFrameMerger(bool useFrameMerger)
{
    mUseFrameMerger = useFrameMerger;
}

void MyRTSPClient::setOutputFormats(const std::string& outputFormats)
{
    mOutputFormats = outputFormats;
}

void MyRTSPClient::setAppendOriginPTS(int appendOriginPTS)
{
    mAppendOriginPTS = appendOriginPTS;
}

void MyRTSPClient::handleError(int err)
{
    if (err == ENODATA)
        setPlaying(false);
    else if (err == ETIMEDOUT)
        setTimeouted(true);
    else if (err == 404 || EINPROGRESS)
        setUnreached(true);
}

bool MyRTSPClient::isPlaying() const
{
    return mIsPlaying;
}

void MyRTSPClient::setPlaying(bool yes)
{
    StateTransition tr = XPR_RTSP_STATE_NULL_OPEN;
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

bool MyRTSPClient::isTimeouted() const
{
    int64_t tmo = mIsPlaying ? mRxTimeout : mConTimeout;
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

void MyRTSPClient::setUnreached(bool yes)
{
    StateTransition tr = XPR_RTSP_STATE_START_UNREACHED;
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

static inline bool contains(const std::string& str, const char* what)
{
    return str.find(what) != std::string::npos;
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
    , mFrameMerger(nullptr)
    , mOutputAdts(false)
    , mBuffer(nullptr)
    , mMaxFrameSize(64512)
    , mFourcc(0)
    , mMeta(NULL)
    , mPTS(0)
    , mOriginPTS(0)
    , mTrackId(trackId)
    , mStreamBlock()
{
    DBG(DBG_L5, "XPR_RTSP: DummySink::DummySink(%p,%p,%p,%d) = %p", &env,
        client, subsession, trackId, this);
    mBuffer = new uint8_t[mMaxFrameSize];
    memset(&mStreamBlock, 0, sizeof(mStreamBlock));
    const char* cn = mSubsession->codecName();
    DBG(DBG_L4, "XPR_RTSP: DummySink(%p): codecName = %s", this, cn);
    if (strcmp(cn, "AAC") == 0)
        mFourcc = AV_FOURCC_AAC;
    else if (strcmp(cn, "H264") == 0)
        mFourcc = AV_FOURCC_H264;
    else if (strcmp(cn, "JPEG") == 0)
        mFourcc = AV_FOURCC_JPEG;
    else if (strcmp(cn, "MPEG4-GENERIC") == 0) {
        const char* mode = mSubsession->attrVal_strToLower("mode");
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
        meta->numOfChannels = mSubsession->numChannels();
        meta->samplingFrequency = mSubsession->rtpTimestampFrequency();
        const char* cfg = mSubsession->attrVal_strToLower("config");
        if (cfg) {
            uint8_t cv[2];
            sscanf(cfg, "%02hhx%02hhx", &cv[0], &cv[1]);
            meta->channelConfig = (cv[1] & 0x78) >> 3;
            meta->profile = ((cv[0] & 0xF8) >> 3) - 1;
            meta->sampleRateIndex = ((cv[0] & 0x07) << 1) | ((cv[1] & 0x80) >> 7);
        }
        mMeta = meta;
    }
    mStreamBlock.codec = mFourcc;
    mStreamBlock.track = mTrackId;
    mStreamBlock.meta = mMeta;
    if (isAudioFourcc(mFourcc))
        mStreamBlock.flags |= XPR_STREAMBLOCK_FLAG_AUDIO_FRAME;
    if (isVideoFourcc(mFourcc))
        mStreamBlock.flags |= XPR_STREAMBLOCK_FLAG_VIDEO_FRAME;
    if (mClient->mUseFrameMerger &&
        (mFourcc == AV_FOURCC_H264 || mFourcc == AV_FOURCC_H265)) {
        DBG(DBG_L4, "XPR_RTSP: DummySink(%p): FrameMeger enabled!", this);
        mFrameMerger = new GenericFrameMerger();
        mFrameMerger->setMaxFrameSize(mMaxFrameSize);
        mFrameMerger->setMergedCallback(&DummySink::afterMergedFrame, this);
    }
    if (mFourcc == AV_FOURCC_AAC && contains(mClient->mOutputFormats, "adts")) {
        mOutputAdts = true;
    }
    mAppendOriginPTS = mClient->mAppendOriginPTS;
}

DummySink::~DummySink()
{
    DBG(DBG_L5, "XPR_RTSP: DummySink::~DummySink(%p)", this);
    if (mFrameMerger) {
        delete mFrameMerger;
        mFrameMerger = nullptr;
    }
    if (mBuffer) {
        delete [] mBuffer;
        mBuffer = nullptr;
    }
    if (mMeta) {
        free(mMeta);
        mMeta = nullptr;
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

void DummySink::afterMergedFrame(void* clientData, XPR_StreamBlock const& stb)
{
    DummySink* sink = reinterpret_cast<DummySink*>(clientData);
    sink->afterMergedFrame(stb);
}

#define FF_PROFILE_AAC_MAIN 0
#define FF_PROFILE_AAC_LOW 1
#define FF_PROFILE_AAC_SSR 2
#define FF_PROFILE_AAC_LTP 3
#define FF_PROFILE_AAC_HE 4
#define FF_PROFILE_AAC_HE_V2 28
#define FF_PROFILE_AAC_LD 22
#define FF_PROFILE_AAC_ELD 38

// Creates an ADTS header and stores in |hdr|
// Assumes |hdr| points to an array of length |kAdtsHeaderSize|
// Returns false if parameter values are for an unsupported configuration.
static bool generateAdtsHeader(int audio_profile, int sample_rate_index,
                               int private_stream, int channel_configuration,
                               int originality, int home,
                               int copyrighted_stream, int copyright_start,
                               int frame_length, int buffer_fullness,
                               int number_of_frames_minus_one, uint8_t* hdr)
{
    memset(reinterpret_cast<void*>(hdr), 0, 7);
    // Ref: http://wiki.multimedia.cx/index.php?title=ADTS
    // ADTS header structure is the following
    // AAAAAAAA  AAAABCCD  EEFFFFGH  HHIJKLMM  MMMMMMMM  MMMOOOOO  OOOOOOPP
    //
    // A    Syncword 0xFFF, all bits must be 1
    // B    MPEG Version: 0 for MPEG-4, 1 for MPEG-2
    // C    Layer: always 0
    // D    Protection absent: Set to 1 if no CRC and 0 if there is CRC.
    // E    Profile: the MPEG-4 Audio Object Type minus 1.
    // F    MPEG-4 Sampling Frequency Index (15 is forbidden)
    // G    Private stream:
    // H    MPEG-4 Channel Configuration
    // I    Originality
    // J    Home
    // K    Copyrighted Stream
    // L    Copyright_ start
    // M    Frame length. This must include the ADTS header length.
    // O    Buffer fullness
    // P    Number of AAC frames in ADTS frame minus 1.
    //      For maximum compatibility always use 1 AAC frame per ADTS frame.
    // Syncword
    hdr[0] = 0xFF;
    hdr[1] = 0xF0;
    // Layer is always 0. No further action required.
    // Protection absent (no CRC) is always 1.
    hdr[1] |= 1;
    switch (audio_profile) {
    case FF_PROFILE_AAC_MAIN:
        break;
    case FF_PROFILE_AAC_HE:
    case FF_PROFILE_AAC_HE_V2:
    case FF_PROFILE_AAC_LOW:
        hdr[2] |= (1 << 6);
        break;
    case FF_PROFILE_AAC_SSR:
        hdr[2] |= (2 << 6);
        break;
    case FF_PROFILE_AAC_LTP:
        hdr[2] |= (3 << 6);
        break;
    default:
        DBG(DBG_L3, "XPR_RTSP: unsupported audio profile: %d", audio_profile);
        return false;
    }
    hdr[2] |= ((sample_rate_index & 0xf) << 2);
    if (private_stream)
        hdr[2] |= (1 << 1);
    switch (channel_configuration) {
    case 1:
        // front-center
        hdr[3] |= (1 << 6);
        break;
    case 2:
        // front-left, front-right
        hdr[3] |= (2 << 6);
        break;
    case 3:
        // front-center, front-left, front-right
        hdr[3] |= (3 << 6);
        break;
    case 4:
        // front-center, front-left, front-right, back-center
        hdr[2] |= 1;
        break;
    case 5:
        // front-center, front-left, front-right, back-left, back-right
        hdr[2] |= 1;
        hdr[3] |= (1 << 6);
        break;
    case 6:
        // front-center, front-left, front-right, back-left, back-right,
        // LFE-channel
        hdr[2] |= 1;
        hdr[3] |= (2 << 6);
        break;
    case 8:
        // front-center, front-left, front-right, side-left, side-right,
        // back-left, back-right, LFE-channel
        hdr[2] |= 1;
        hdr[3] |= (3 << 6);
        break;
    default:
        DBG(DBG_L3, "XPR_RTSP: unsupported number of audio channels: %d",
            channel_configuration);
        return false;
    }
    if (originality)
        hdr[3] |= (1 << 5);
    if (home)
        hdr[3] |= (1 << 4);
    if (copyrighted_stream)
        hdr[3] |= (1 << 3);
    if (copyright_start)
        hdr[3] |= (1 << 2);
    // frame length
    hdr[3] |= (frame_length >> 11) & 0x03;
    hdr[4] = (frame_length >> 3) & 0xFF;
    hdr[5] |= (frame_length & 7) << 5;
    // buffer fullness
    hdr[5] |= (buffer_fullness >> 6) & 0x1F;
    hdr[6] |= (buffer_fullness & 0x3F) << 2;
    hdr[6] |= number_of_frames_minus_one & 0x3;
    return true;
}

static inline unsigned alignUpTo16K(unsigned val)
{
    return ((val >> 14) + 1) << 14;
}

#ifndef H264_HISI_FRAME_TYPE
// Detect H264 slice type from NALU (without startcode 00000001)
// FIXME: some bugs for bitstream compitable
//
static int H264_DetectSliceType(const uint8_t* data, size_t size)
{
    int i = 0;
    int n = 32;
    int first_mb_in_slice = 0;
    int first_mb_in_slice_zn = 0;
    int slice_type = 0;
    int slice_type_zn = 0;
    uint32_t b = 0;

    b |= size > 1 ? (uint32_t)data[0] << 24 : 0;
    b |= size > 2 ? (uint32_t)data[1] << 16 : 0;
    b |= size > 3 ? (uint32_t)data[2] << 8 : 0;
    b |= size > 4 ? (uint32_t)data[3] : 0;

    if ((b & 0xfffffff) == b && (b & 0x8000000))
        first_mb_in_slice_zn = 4;
    else if ((b & 0x1fffffff) == b && (b & 0x10000000))
        first_mb_in_slice_zn = 3;
    else if ((b & 0x3fffffff) == b && (b & 0x20000000))
        first_mb_in_slice_zn = 2;
    else if ((b & 0x7fffffff) == b && (b & 0x40000000))
        first_mb_in_slice_zn = 1;
    else if (b & 0x80000000)
        first_mb_in_slice_zn = 0;
    if (first_mb_in_slice_zn)
        first_mb_in_slice = (b >> (n - first_mb_in_slice_zn * 2 - 1)) - 1;

    if (first_mb_in_slice_zn == 0) {
        if ((b & 0x87ffffff) == b)
            slice_type_zn = 4;
        else if ((b & 0x8fffffff) == b)
            slice_type_zn = 3;
        else if ((b & 0x9fffffff) == b)
            slice_type_zn = 2;
        else if ((b & 0xbfffffff) == b)
            slice_type_zn = 1;
        else if (b & 0x40000000)
            slice_type_zn = 0;
    }
    else if (first_mb_in_slice_zn == 1) {
        if ((b & 0xe1ffffff) == b)
            slice_type_zn = 4;
        else if ((b & 0xe3ffffff) == b)
            slice_type_zn = 3;
        else if ((b & 0xe7ffffff) == b)
            slice_type_zn = 2;
        else if ((b & 0xefffffff) == b)
            slice_type_zn = 1;
        else if (b & 0x10000000)
            slice_type_zn = 0;
    }
    else if (first_mb_in_slice_zn == 2) {
        if ((b & 0xf87fffff) == b)
            slice_type_zn = 4;
        else if ((b & 0xf8ffffff) == b)
            slice_type_zn = 3;
        else if ((b & 0xf9ffffff) == b)
            slice_type_zn = 2;
        else if ((b & 0xfbffffff) == b)
            slice_type_zn = 1;
        else if (b & 0x4000000)
            slice_type_zn = 0;
    }
    else if (first_mb_in_slice_zn == 3) {
        if ((b & 0xfe1fffff) == b)
            slice_type_zn = 4;
        else if ((b & 0xfe3fffff) == b)
            slice_type_zn = 3;
        else if ((b & 0xfe7fffff) == b)
            slice_type_zn = 2;
        else if ((b & 0xfeffffff) == b)
            slice_type_zn = 1;
        else if (b & 0x1000000)
            slice_type_zn = 0;
    }
    else if (first_mb_in_slice_zn == 4) {
        if ((b & 0xff87ffff) == b)
            slice_type_zn = 4;
        else if ((b & 0xff8fffff) == b)
            slice_type_zn = 3;
        else if ((b & 0xff9fffff) == b)
            slice_type_zn = 2;
        else if ((b & 0xffbfffff) == b)
            slice_type_zn = 1;
        else if (b & 0x400000)
            slice_type_zn = 0;
    }

    if (slice_type_zn) {
        b &= (0xffffffff >> (first_mb_in_slice_zn * 2 + 1));
        slice_type =
            (b >> (n - first_mb_in_slice_zn * 2 - 1 - slice_type_zn * 2 - 1)) -
            1;
    }

    slice_type &= 0xf;

    return slice_type;
}

static int H264_SliceTypeToFrameType(int slice_type)
{
    int frame_type = XPR_STREAMBLOCK_FLAG_TYPE_I;
    switch (slice_type & 0xf) {
    case 0:
    case 3:
    case 5:
    case 8:
        frame_type = XPR_STREAMBLOCK_FLAG_TYPE_P;
        break;
    case 1:
    case 6:
        frame_type = XPR_STREAMBLOCK_FLAG_TYPE_B;
        break;
    case 2:
    case 4:
    case 7:
    case 9:
        frame_type = XPR_STREAMBLOCK_FLAG_TYPE_I;
        break;
    }
    return frame_type;
}
#else
// Detect H264 frame type from NALU (without startcode 00000001)
static int H264_DetectFrameTypeForHisi(const uint8_t* data, size_t size)
{
    int nalType = data[0] & 0x1f;
    switch (nalType) {
    case 1:
        return XPR_STREAMBLOCK_FLAG_TYPE_P;
    case 5:
        return XPR_STREAMBLOCK_FLAG_TYPE_I;
    default:
        break;
    }
    return 0;
}
#endif

static int64_t getOriginPTS(XPR_StreamBlock* stb)
{
    if (stb->dataSize < H264_OPTS_FRM_LEN)
        return 0;
    int n = stb->dataSize - H264_OPTS_FRM_LEN;
    if (n < 0)
        return 0;
    const uint8_t sei[] = {0x00, 0x00, 0x00, 0x01, 0x06, 0x05,
                           12,   'O',  'P',  'T',  'S'};
    int a = memcmp(stb->data + n, sei, H264_OPTS_HDR_LEN);
    if (a == 0) {
        stb->dataSize = n;
        int64_t pts = 0;
        memcpy(&pts, stb->data + n + 11, sizeof(pts));
        return pts;
    }
    return 0;
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

    int64_t dts = static_cast<int64_t>(
        mSubsession->getNormalPlayTime(presentationTime) * 1000);
    dts *= 1000;

    int64_t pts = presentationTime.tv_sec;
    pts *= 1000000;
    pts += presentationTime.tv_usec;

    // Update last active timestamp
    mClient->updateLATS();

    size_t offset = 0;
    switch (mFourcc) {
    case AV_FOURCC_H264:
    case AV_FOURCC_H265:
        offset = 4;
        break;
    case AV_FOURCC_AAC:
        offset = mOutputAdts ? 7 : 0;
        break;
    default:
        break;
    }

    if (mAppendOriginPTS == 1)
        pts = mPTS;

    // Fill stream block
    mStreamBlock.buffer = mBuffer;
    mStreamBlock.bufferSize = mMaxFrameSize;
    mStreamBlock.data = mBuffer + offset;
    mStreamBlock.dataSize = frameSize;
    mStreamBlock.dts = dts;
    mStreamBlock.pts = pts;
    mPTS = pts;

    // If H264 data without start code,
    // Rewind the stb.data to head of the buffer.
    if (mFourcc == AV_FOURCC_H264) {
        uint8_t* p = mStreamBlock.data;
        if (p[0] != 0x00 || p[1] != 0x00 || p[2] != 0x00 || p[3] != 0x01) {
            mStreamBlock.data = mBuffer;
            mStreamBlock.dataSize += 4;
        }
        // Clear the frame type flags.
        mStreamBlock.flags &=
            ~(XPR_STREAMBLOCK_FLAG_TYPE_I | XPR_STREAMBLOCK_FLAG_TYPE_P |
              XPR_STREAMBLOCK_FLAG_TYPE_B);
#if 0
        DBG(DBG_L2,
            "XPR_RTSP: DummySink(%p): hex dump: [%02X %02X %02X %02X %02X %02X "
            "%02X %02X]",
            this, mStreamBlock.data[0], mStreamBlock.data[1],
            mStreamBlock.data[2], mStreamBlock.data[3], mStreamBlock.data[4],
            mStreamBlock.data[5], mStreamBlock.data[6], mStreamBlock.data[7]);
#endif
        if (mAppendOriginPTS == 1) {
            int64_t opts = getOriginPTS(&mStreamBlock);
            if (opts != 0) {
                mPTS = pts = opts;
                goto next;
            }
        }
#ifndef H264_HISI_FRAME_TYPE
        // Detect frame type (P/I) from NALU
        int slt = H264_DetectSliceType(mStreamBlock.data + 4,
                                       mStreamBlock.dataSize - 4);
        // Set new frame type flag for current.
        mStreamBlock.flags |= H264_SliceTypeToFrameType(slt);
#else
        mStreamBlock.flags |= H264_DetectFrameTypeForHisi(
            mStreamBlock.data + 4, mStreamBlock.dataSize - 4);
#endif
    }
    if (mFourcc == AV_FOURCC_AAC && mOutputAdts) {
        XPR_AudioMeta* meta = reinterpret_cast<XPR_AudioMeta*>(mMeta);
        if (meta) {
            mStreamBlock.data -= 7;
            mStreamBlock.dataSize += 7;
            generateAdtsHeader(
                meta->profile, meta->sampleRateIndex, 0, meta->channelConfig, 0,
                0, 0, 0, mStreamBlock.dataSize, 0x7ff, 0, mStreamBlock.data);
#if 0
            uint8_t* p = mStreamBlock.data;
            DBG(DBG_L4,
                "XPR_RTSP: DummySink(%p): hex dump: [%02X %02X %02X %02X %02X "
                "%02X %02X]",
                this, p[0], p[1], p[2], p[3], p[4], p[5], p[6]);
#endif
        }
    }

    if (mFrameMerger) {
        mFrameMerger->merge(mStreamBlock);
    }
    else {
        // Push data to callbacks
        Connection* conn = mClient->getParent();
        conn->pushData(id_to_port(conn->id()), &mStreamBlock);
    }

next:

    // Replace the new buffer and size
    if (newBuffer && newMaxFrameSize) {
        delete mBuffer;
        mBuffer = newBuffer;
        mMaxFrameSize = newMaxFrameSize;
        mFrameMerger->setMaxFrameSize(mMaxFrameSize);
    }

    // Then continue, to request the next frame of data:
    continuePlaying();
}

void DummySink::afterMergedFrame(XPR_StreamBlock const& stb)
{
    // Push data to callbacks
    Connection* conn = mClient->getParent();
    conn->pushData(id_to_port(conn->id()), const_cast<XPR_StreamBlock*>(&stb));
}

Boolean DummySink::continuePlaying()
{
    if (fSource == NULL) {
        DBG(DBG_L2, "XPR_RTSP: DummySink(%p): Exit playing by fSource == NULL",
            this);
        return False; // sanity check (should not happen)
    }

    size_t offset = 0;

    // Pre-Buffered start code for H264,H265
    if (mFourcc == AV_FOURCC_H264 || mFourcc == AV_FOURCC_H265) {
        mBuffer[0] = 0x00;
        mBuffer[1] = 0x00;
        mBuffer[2] = 0x00;
        mBuffer[3] = 0x01;
        offset = 4;
    }

    // Fill extra header for AAC
    if (mFourcc == AV_FOURCC_AAC && mOutputAdts) {
        offset = 7;
    }

    // Request the next frame of data from our input source.
    // "afterGettingFrame()" will get called later, when it arrives:
    fSource->getNextFrame(mBuffer + offset, mMaxFrameSize - offset,
                          afterGettingFrame, this, onSourceClosure, this);

    return True;
}

// Connection
//============================================================================
Connection::Connection(int id, Port* parent)
    : Port(id, parent)
    , mWorker(nullptr)
    , mRestartTask(nullptr)
    , mRtpOverTcp(false)
    , mConTimeout(CS_TMO)
    , mRxTimeout(RX_TMO)
    , mUseFrameMerger(false)
    , mAutoRestart(false)
    , mRestartDelay(5000000)
    , mOutputFormats()
    , mAppendOriginPTS(0)
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
    const char* path = XPR_UrlGetPath(u);
    if (!path) {
        XPR_UrlDestroy(u);
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    }
    // 跳过起始 '/'
    if (*path == '/')
        path++;
    DBG(DBG_L4, "XPR_RTSP: Connection(%p): name: %s", this, path);
    // 分离出原始地址及用于配置连接的扩展查询，其格式为：
    // rtsp://host[:port]/path[?query][??ext-query]
    std::string str(url);
    std::string extQuery;
    size_t pos = str.rfind("??");
    if (pos == std::string::npos) {
        mUrl = str;
    }
    else {
        mUrl = str.substr(0, pos);
        extQuery = str.substr(pos+2);
    }
    // 使用 extQuery 参数来配置
    configConnection(extQuery.c_str());
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
    // Disable auto restart to prevent dead-loop.
    mAutoRestart = false;
    // Cancel delayed restart task if exists.
    if (mRestartTask) {
        worker()->scheduler().unscheduleDelayedTask(mRestartTask);
        mRestartTask = nullptr;
    }
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
    case XPR_RTSP_TASK_RESTART:
        return restartInTask(port);
    default:
        return XPR_ERR_GEN_NOT_SUPPORT;
    }
}

int Connection::stateChanged(int port, StateTransition transition)
{
    DBG(DBG_L4, "XPR_RTSP: Connection(%p): stateChanged(%08X, %d)", this, port,
        transition);
    bool needRestart = false;
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
        needRestart = mAutoRestart;
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
    case XPR_RTSP_STATE_PLAYING_TIMEOUT: {
        XPR_RTSP_EVD evd = {XPR_RTSP_EVT_TIMEOUT, NULL, 0};
        postEvent(port, &evd);
        needRestart = mAutoRestart;
        break;
    }
    case XPR_RTSP_STATE_START_UNREACHED: {
        XPR_RTSP_EVD evd = {XPR_RTSP_EVT_UNREACHABLE, NULL, 0};
        postEvent(port, &evd);
        needRestart = mAutoRestart;
        break;
    }
    default:
        break;
    }
    if (needRestart)
        return restart(port);
    return XPR_ERR_OK;
}

struct RestartTaskData
{
    Connection* conn;
    int port;

    RestartTaskData(Connection* conn_, int port_) : conn(conn_), port(port_)
    {
    }
};

int Connection::restart(int port)
{
    RestartTaskData* d = new RestartTaskData(this, port);
    mRestartTask = worker()->scheduler().scheduleDelayedTask(
                mRestartDelay, (TaskFunc*)afterRestart, d);
    if (mRestartTask != nullptr) {
        activeFlags(PortFlags::PORT_FLAG_RESTART, 0);
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
    if (strcmp(key, "rtpOverTcp") == 0) {
        mRtpOverTcp = str2bool(value);
    }
    else if (strcmp(key, "conTimeout") == 0) {
        mConTimeout = strtol(value, NULL, 10);
    }
    else if (strcmp(key, "rxTimeout") == 0) {
        mRxTimeout = strtol(value, NULL, 10);
    }
    else if (strcmp(key, "useFrameMerger") == 0) {
        mUseFrameMerger = str2bool(value);
    }
    else if (strcmp(key, "autoRestart") == 0) {
        mAutoRestart = str2bool(value);
    }
    else if (strcmp(key, "restartDelay") == 0) {
        mRestartDelay = strtol(value, NULL, 10);
    }
    else if (strcmp(key, "outputFormats") == 0) {
        mOutputFormats.assign(value);
    }
    else if (strcmp(key, "appendOriginPTS") == 0) {
        mAppendOriginPTS = strtol(value, NULL, 10);
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
        mClient->setConnectTiemout(mConTimeout);
        mClient->setReceiveTimeout(mRxTimeout);
        mClient->setUseFrameMerger(mUseFrameMerger);
        mClient->setOutputFormats(mOutputFormats);
        mClient->setAppendOriginPTS(mAppendOriginPTS);
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

int Connection::restartInTask(int port)
{
DBG(DBG_L4, "XPR_RTSP: Connection(%p): restartInTask(%08X)", this, port);
    int err;
    activeFlags(0, PortFlags::PORT_FLAG_PENDING);
    err = stopInTask(port);
    if (err == XPR_ERR_OK)
        err = startInTask(port);
    activeFlags(0, PortFlags::PORT_FLAG_RESTART);
    return err;
}

void Connection::afterRestart(void* opaque)
{
    RestartTaskData* d = reinterpret_cast<RestartTaskData*>(opaque);
    d->conn->restartInTask(d->port);
    delete d;
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

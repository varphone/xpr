#if defined(HAVE_XPR_RTSP_DRIVER_LIVE)

#include "rtsp_server.hpp"
#include "rtsp_worker.hpp"
#include <algorithm>
#include <live/GroupsockHelper.hh>
#include <map>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_url.h>

#undef min // avoid from cstdlib

class MyRTSPServer : public RTSPServer {
public:
    static RTSPServer*
    createNew(xpr::rtsp::Server* server, UsageEnvironment& env,
              Port ourPort = 554,
              UserAuthenticationDatabase* authDatabase = NULL,
              unsigned reclamationSeconds = 65)
    {
        int ourSocketIPv4 = setUpOurSocket(env, ourPort, AF_INET);
        int ourSocketIPv6 = setUpOurSocket(env, ourPort, AF_INET6);
        if (ourSocketIPv4 < 0 && ourSocketIPv6 < 0)
            return NULL;

        return new MyRTSPServer(server, env, ourSocketIPv4, ourSocketIPv6,
                                ourPort, authDatabase, reclamationSeconds);
    }

protected:
    MyRTSPServer(xpr::rtsp::Server* server, UsageEnvironment& env,
                 int ourSocketIPv4, int ourSocketIPv6, Port ourPort,
                 UserAuthenticationDatabase* authDatabase,
                 unsigned reclamationSeconds)
        : RTSPServer(env, ourSocketIPv4, ourSocketIPv6, ourPort, authDatabase,
                     reclamationSeconds)
        , mServer(server)
    {
    }

    virtual ~MyRTSPServer()
    {
    }

    virtual Boolean
    specialClientUserAccessCheck(int clientSocket,
                                 struct sockaddr_storage const& clientAddr,
                                 char const* urlSuffix, char const* username)
    {
        DBG(DBG_L5,
            "XPR_RTSP: Server(%p): specialClientUserAccessCheck(%d, %s, %s)",
            this, clientSocket, urlSuffix, username);
        XPR_RTSP_EVD_UAC evdUAC = {clientSocket, &clientAddr, urlSuffix,
                                   username};
        XPR_RTSP_EVD evd = {XPR_RTSP_EVT_UAC, &evdUAC, sizeof(evdUAC)};
        int port = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_SVR, 0, 0);
        return mServer->postEvent(port, &evd) ? XPR_ERR_OK : True;
    }

private:
    xpr::rtsp::Server* mServer;
};

namespace xpr
{

namespace rtsp
{

// ADTSAudioFramedSource
//============================================================================
static unsigned const samplingFrequencyTable[16] = {
    96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
    16000, 12000, 11025, 8000,  7350,  0,     0,     0};

ADTSAudioFramedSource::ADTSAudioFramedSource(UsageEnvironment& env,
                                             Stream* stream, u_int8_t profile,
                                             u_int8_t samplingFrequencyIndex,
                                             u_int8_t channelConfiguration)
    : FramedSource(env), mStream(stream), mLastPTS(0)
{
    DBG(DBG_L5,
        "XPR_RTSP: ADTSAudioFramedSource::ADTSAudioFramedSource(%p) = %p", &env,
        this);

    mSamplingFrequency = samplingFrequencyTable[samplingFrequencyIndex];
    mNumChannels = channelConfiguration == 0 ? 2 : channelConfiguration;
    // uSecsPerFrame = samples-per-frame / samples-per-second
    mUSecsPerFrame = (1024 * 1000000) / mSamplingFrequency;

    // Construct the 'AudioSpecificConfig', and from it, the corresponding ASCII
    // string:
    unsigned char audioSpecificConfig[2];
    u_int8_t const audioObjectType = profile + 1;
    audioSpecificConfig[0] =
        (audioObjectType << 3) | (samplingFrequencyIndex >> 1);
    audioSpecificConfig[1] =
        (samplingFrequencyIndex << 7) | (channelConfiguration << 3);
    sprintf(mConfigStr, "%02X%02x", audioSpecificConfig[0],
            audioSpecificConfig[1]);
}

ADTSAudioFramedSource::ADTSAudioFramedSource(const ADTSAudioFramedSource& rhs)
    : FramedSource(rhs.envir())
    , mStream(rhs.stream())
    , mLastPTS(rhs.mLastPTS)
    , mSamplingFrequency(rhs.mSamplingFrequency)
    , mNumChannels(rhs.mNumChannels)
    , mUSecsPerFrame(rhs.mUSecsPerFrame)
{
    memcpy(mConfigStr, rhs.mConfigStr, sizeof(mConfigStr));
}

ADTSAudioFramedSource::~ADTSAudioFramedSource(void)
{
    DBG(DBG_L5,
        "XPR_RTSP: ADTSAudioFramedSource::~ADTSAudioFramedSource() = %p", this);
    envir().taskScheduler().unscheduleDelayedTask(mCurrentTask);
}

void ADTSAudioFramedSource::doGetNextFrame()
{
    fetchFrame();
}

unsigned int ADTSAudioFramedSource::maxFrameSize() const
{
    return XPR_RTSP_ADTS_MAX_FRAME_SIZE;
}

void ADTSAudioFramedSource::fetchFrame()
{
    // Check audio queue, if empty, delay for next.
    if (!mStream->hasAudioFrame()) {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            5000, getNextFrame, this);
        return;
    }

    // Fetch one block from audio queue.
    XPR_StreamBlock* ntb = mStream->getAudioFrame();
    if (ntb) {
#if 0
        fFrameSize = XPR_StreamBlockLength(ntb);
        if (fFrameSize > fMaxSize) {
            fNumTruncatedBytes = fFrameSize - fMaxSize;
            fFrameSize = fMaxSize;
        }
#endif

        unsigned char* cur = XPR_StreamBlockData(ntb);
        unsigned char* headers = cur;

        // Extract important fields from the headers:
        Boolean protection_absent = headers[1] & 0x01;

        u_int16_t frame_length = ((headers[3] & 0x03) << 11) |
                                 (headers[4] << 3) | ((headers[5] & 0xE0) >> 5);
        if (0) {
            u_int16_t syncword = (headers[0] << 4) | (headers[1] >> 4);
            DBG(DBG_L3,
                "XPR_RTSP: ADTSAudioFramedSource(%p): Read frame: syncword "
                "0x%x, protection_absent %d, frame_length %d",
                this, syncword, protection_absent, frame_length);
            if (syncword != 0xFFF) {
                DBG(DBG_L2,
                    "XPR_RTSP: ADTSAudioFramedSource(%p): Bad syncword!", this);
            }
        }

        unsigned numBytesToRead = frame_length > 7 ? frame_length - 7 : 0;
        int pos = 7;

        // If there's a 'crc_check' field, skip it:
        if (!protection_absent) {
            pos += 2;
            numBytesToRead = numBytesToRead > 2 ? numBytesToRead - 2 : 0;
        }

        // Next, read the raw frame data into the buffer provided:
        if (numBytesToRead > fMaxSize) {
            fNumTruncatedBytes = numBytesToRead - fMaxSize;
            numBytesToRead = fMaxSize;
        }

        memcpy(fTo, cur + pos, numBytesToRead);

        int numBytesRead = numBytesToRead;
        fFrameSize = numBytesRead;
        fNumTruncatedBytes += numBytesToRead - numBytesRead;

#if 1
        // Set the 'presentation time':
        if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
            // This is the first frame, so use the current time:
            gettimeofday(&fPresentationTime, NULL);
            fDurationInMicroseconds = 20000;
        }
        else {
            // Increment by the play time of the previous frame:
            int64_t usecs = XPR_StreamBlockPTS(ntb) - mLastPTS;
            if (usecs) {
                usecs += fPresentationTime.tv_usec;
                fPresentationTime.tv_sec += usecs / 1000000;
                fPresentationTime.tv_usec = usecs % 1000000;
            }
        }

        mLastPTS = XPR_StreamBlockPTS(ntb);
#else
        // Auto filled by AudioRTPSink
        fPresentationTime.tv_sec = 0;
        fPresentationTime.tv_usec = 0;
#endif
        mStream->releaseAudioFrame(ntb);
        if (fNumTruncatedBytes > 0) {
            DBG(DBG_L2,
                "XPR_RTSP: ADTSAudioFramedSource(%p): %u bytes truncated.",
                this, fNumTruncatedBytes);
        }
    }
    else {
        // Should never run here
        DBG(DBG_L1, "XPR_RTSP: ADTSAudioFramedSource(%p): BUG: %s:%d", this,
            __FILE__, __LINE__);
        // Clear
        fFrameSize = 0;
        fNumTruncatedBytes = 0;
    }
    afterGetting(this);
}

Stream* ADTSAudioFramedSource::stream(void) const
{
    return mStream;
}

void ADTSAudioFramedSource::getNextFrame(void* ptr)
{
    if (ptr) {
        ((ADTSAudioFramedSource*)ptr)->fetchFrame();
    }
}

// ADTSAudioServerMediaSubsession
//============================================================================
ADTSAudioServerMediaSubsession::ADTSAudioServerMediaSubsession(
    UsageEnvironment& env, FramedSource* source, Stream* stream,
    u_int8_t profile, u_int8_t samplingFrequencyIndex,
    u_int8_t channelConfiguration)
    : OnDemandServerMediaSubsession(env, True)
    , mSource(source)
    , mSink(NULL)
    , mStream(stream)
    , mProfile(profile)
    , mSamplingFrequencyIndex(samplingFrequencyIndex)
    , mChannelConfiguration(channelConfiguration)
    , mDoneFlag(0)
    , mAuxSDPLine(NULL)
{
    DBG(DBG_L5,
        "XPR_RTSP: ADTSAudioServerMediaSubsession::"
        "ADTSAudioServerMediaSubsession(%p, %p) = %p",
        &env, source, this);
}

ADTSAudioServerMediaSubsession::~ADTSAudioServerMediaSubsession(void)
{
    DBG(DBG_L5,
        "XPR_RTSP: ADTSAudioServerMediaSubsession::~"
        "ADTSAudioServerMediaSubsession() = %p", this);
    if (mAuxSDPLine) {
        free(mAuxSDPLine);
        mAuxSDPLine = NULL;
    }
    mSource = NULL;
    mSink = NULL;
    mDoneFlag = 0;
}

void ADTSAudioServerMediaSubsession::afterPlayingDummy(void* clientData)
{
    ADTSAudioServerMediaSubsession* subsess =
        (ADTSAudioServerMediaSubsession*)clientData;
    subsess->afterPlayingDummy1();
}

void ADTSAudioServerMediaSubsession::afterPlayingDummy1()
{
    // Unschedule any pending 'checking' task:
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    // Signal the event loop that we're done:
    setDoneFlag();
}

void ADTSAudioServerMediaSubsession::setDoneFlag()
{
    mDoneFlag = ~0x00;
}

void ADTSAudioServerMediaSubsession::checkForAuxSDPLine(void* clientData)
{
    ADTSAudioServerMediaSubsession* subsess =
        (ADTSAudioServerMediaSubsession*)clientData;
    subsess->checkForAuxSDPLine1();
}

void ADTSAudioServerMediaSubsession::checkForAuxSDPLine1()
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
        DBG(DBG_L4,
            "XPR_RTSP: ADTSAudioServerMediaSubsession(%p): AuxSDPLine [%s]",
            this, mAuxSDPLine);
        // Signal the event loop that we're done:
        setDoneFlag();
    }
    else if (!mDoneFlag) {
        // try again after a brief delay:
        int uSecsToDelay = 10000; // 10 ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            uSecsToDelay, (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const*
ADTSAudioServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink,
                                              FramedSource* inputSource)
{
    if (mAuxSDPLine) {
        return mAuxSDPLine;
    }
    if (mSink == NULL) {
        // we're not already setting it up for another, concurrent stream
        // Note: For H264 video files, the 'config' information
        // ("profile-level-id" and "sprop-parameter-sets") isn't known until we
        // start reading the file.  This means that "rtpSink"s "auxSDPLine()"
        // will be NULL initially, and we need to start reading data from our
        // file until this changes.
        mSink = rtpSink;
        // Start reading the file:
        mSink->startPlaying(*inputSource, afterPlayingDummy, this);
        // Check whether the sink's 'auxSDPLine()' is ready:
        checkForAuxSDPLine(this);
    }
    envir().taskScheduler().doEventLoop(&mDoneFlag);
    return mAuxSDPLine;
}

FramedSource*
ADTSAudioServerMediaSubsession::createNewStreamSource(unsigned clientSessionId,
                                                      unsigned& estBitrate)
{
    estBitrate = 96;
    return new ADTSAudioFramedSource(envir(), mStream, mProfile,
                                     mSamplingFrequencyIndex,
                                     mChannelConfiguration);
}

RTPSink* ADTSAudioServerMediaSubsession::createNewRTPSink(
    Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
    FramedSource* inputSource)
{
    OutPacketBuffer::maxSize = XPR_RTSP_ADTS_MAX_FRAME_SIZE;
    ADTSAudioFramedSource* adtsSource = (ADTSAudioFramedSource*)inputSource;
    return MPEG4GenericRTPSink::createNew(
        envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
        adtsSource->samplingFrequency(), "audio", "AAC-hbr",
        adtsSource->configStr(), adtsSource->numChannels());
}

ADTSAudioServerMediaSubsession* ADTSAudioServerMediaSubsession::createNew(
    UsageEnvironment& env, FramedSource* source, Stream* stream,
    u_int8_t profile, u_int8_t samplingFrequencyIndex,
    u_int8_t channelConfiguration)
{
    return new ADTSAudioServerMediaSubsession(env, source, stream, profile,
                                              samplingFrequencyIndex,
                                              channelConfiguration);
}

// H264VideoFramedSource
//============================================================================
H264VideoFramedSource::H264VideoFramedSource(UsageEnvironment& env,
                                             Stream* stream)
    : FramedSource(env), mStream(stream), mLastPTS(0)
{
    DBG(DBG_L5, "XPR_RTSP: H264VideoFramedSource::H264VideoFramedSource(%p) = %p", &env,
        this);
}

H264VideoFramedSource::H264VideoFramedSource(const H264VideoFramedSource& rhs)
    : FramedSource(rhs.envir()), mStream(rhs.stream()), mLastPTS(rhs.mLastPTS)
{
}

H264VideoFramedSource::~H264VideoFramedSource(void)
{
    DBG(DBG_L5, "XPR_RTSP: H264VideoFramedSource::~H264VideoFramedSource() = %p", this);
    envir().taskScheduler().unscheduleDelayedTask(mCurrentTask);
}

void H264VideoFramedSource::doGetNextFrame()
{
    fetchFrame();
}

unsigned int H264VideoFramedSource::maxFrameSize() const
{
    return XPR_RTSP_H264_MAX_FRAME_SIZE;
}

void H264VideoFramedSource::fetchFrame()
{
    // Check video queue, if empty, delay for next.
    if (!mStream->hasVideoFrame()) {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            5000, getNextFrame, this);
        return;
    }
    // Fetch one block from video queue.
    XPR_StreamBlock* ntb = mStream->getVideoFrame();
    if (ntb) {
        fFrameSize = XPR_StreamBlockLength(ntb);
        if (mStream->appendOriginPTS() == 1)
            fFrameSize += H264_OPTS_FRM_LEN;
        if (fFrameSize > fMaxSize) {
            fNumTruncatedBytes = fFrameSize - fMaxSize;
            fFrameSize = fMaxSize;
        }
        if (mStream->appendOriginPTS() == 1) {
            uint32_t n = fFrameSize - H264_OPTS_FRM_LEN;
            int64_t usecs = XPR_StreamBlockPTS(ntb);
            const uint8_t sei[H264_OPTS_HDR_LEN] = {
                0x00, 0x00, 0x00, 0x01, 0x06, 0x05, 12, 'O', 'P', 'T', 'S'};
            memcpy(fTo, sei, sizeof(sei));
            memcpy(fTo + H264_OPTS_HDR_LEN, &usecs, sizeof(int64_t));
            memcpy(fTo + H264_OPTS_FRM_LEN, XPR_StreamBlockData(ntb), n);
        }
        else {
            memcpy(fTo, XPR_StreamBlockData(ntb), fFrameSize);
        }
#if 1
        if (mStream->discreteInput() == 1) {
            // Force PTS to ntb->pts
            int64_t usecs = XPR_StreamBlockPTS(ntb);
            fPresentationTime.tv_sec += usecs / 1000000;
            fPresentationTime.tv_usec = usecs % 1000000;
        }
        else {
            // Setup PTS with ntb->pts
            if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
                gettimeofday(&fPresentationTime, NULL);
                fDurationInMicroseconds = 10000;
            }
            else {
                int64_t usecs = XPR_StreamBlockPTS(ntb) - mLastPTS;
                if (usecs) {
                    usecs += fPresentationTime.tv_usec;
                    fPresentationTime.tv_sec += usecs / 1000000;
                    fPresentationTime.tv_usec = usecs % 1000000;
                }
            }
        }
        mLastPTS = XPR_StreamBlockPTS(ntb);
#else
        // Auto filled by H264VideoSteamFramer
        fPresentationTime.tv_sec = 0;
        fPresentationTime.tv_usec = 0;
#endif
        mStream->releaseVideoFrame(ntb);
        if (fNumTruncatedBytes > 0) {
            DBG(DBG_L2,
                "XPR_RTSP: H264VideoFramedSource(%p): %u bytes truncated.",
                this, fNumTruncatedBytes);
        }
    }
    else {
        // Should never run here
        DBG(DBG_L1, "XPR_RTSP: H264VideoFramedSource(%p): BUG: %s:%d\n", this,
            __FILE__, __LINE__);
        // Clear
        fFrameSize = 0;
        fNumTruncatedBytes = 0;
    }
    afterGetting(this);
}

Stream* H264VideoFramedSource::stream(void) const
{
    return mStream;
}

void H264VideoFramedSource::getNextFrame(void* ptr)
{
    if (ptr) {
        ((H264VideoFramedSource*)ptr)->fetchFrame();
    }
}

Boolean H264VideoFramedSource::isH264VideoStreamFramer() const
{
    return True;
}

// H264VideoServerMediaSubsession
//============================================================================
H264VideoServerMediaSubsession::H264VideoServerMediaSubsession(
    UsageEnvironment& env, FramedSource* source, Stream* stream)
    : OnDemandServerMediaSubsession(env, True)
    , mSource(source)
    , mSink(NULL)
    , mStream(stream)
    , mDoneFlag(0)
    , mAuxSDPLine(NULL)
{
    DBG(DBG_L5,
        "XPR_RTSP: "
        "H264VideoServerMediaSubsession::H264VideoServerMediaSubsession(%p, "
        "%p) = %p",
        &env, source, this);
}

H264VideoServerMediaSubsession::~H264VideoServerMediaSubsession(void)
{
    DBG(DBG_L5,
        "XPR_RTSP: "
        "H264VideoServerMediaSubsession::~H264VideoServerMediaSubsession() = "
        "%p",
        this);
    if (mAuxSDPLine) {
        free(mAuxSDPLine);
        mAuxSDPLine = NULL;
    }
    mSource = NULL;
    mSink = NULL;
    mDoneFlag = 0;
}

void H264VideoServerMediaSubsession::afterPlayingDummy(void* clientData)
{
    H264VideoServerMediaSubsession* subsess =
        (H264VideoServerMediaSubsession*)clientData;
    subsess->afterPlayingDummy1();
}

void H264VideoServerMediaSubsession::afterPlayingDummy1()
{
    // Unschedule any pending 'checking' task:
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    // Signal the event loop that we're done:
    setDoneFlag();
}

void H264VideoServerMediaSubsession::setDoneFlag()
{
    mDoneFlag = ~0x00;
}

void H264VideoServerMediaSubsession::checkForAuxSDPLine(void* clientData)
{
    H264VideoServerMediaSubsession* subsess =
        (H264VideoServerMediaSubsession*)clientData;
    subsess->checkForAuxSDPLine1();
}

void H264VideoServerMediaSubsession::checkForAuxSDPLine1()
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
        DBG(DBG_L4,
            "XPR_RTSP: H264VideoServerMediaSubsession(%p): AuxSDPLine [%s]",
            this, mAuxSDPLine);
        // Signal the event loop that we're done:
        setDoneFlag();
    }
    else if (!mDoneFlag) {
        // try again after a brief delay:
        int uSecsToDelay = 10000; // 10 ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            uSecsToDelay, (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const*
H264VideoServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink,
                                              FramedSource* inputSource)
{
    if (mAuxSDPLine) {
        return mAuxSDPLine;
    }
    if (mSink == NULL) {
        // we're not already setting it up for another, concurrent stream
        // Note: For H264 video files, the 'config' information
        // ("profile-level-id" and "sprop-parameter-sets") isn't known until we
        // start reading the file.  This means that "rtpSink"s "auxSDPLine()"
        // will be NULL initially, and we need to start reading data from our
        // file until this changes.
        mSink = rtpSink;
        // Start reading the file:
        mSink->startPlaying(*inputSource, afterPlayingDummy, this);
        // Check whether the sink's 'auxSDPLine()' is ready:
        checkForAuxSDPLine(this);
    }
    envir().taskScheduler().doEventLoop(&mDoneFlag);
    return mAuxSDPLine;
}

FramedSource*
H264VideoServerMediaSubsession::createNewStreamSource(unsigned clientSessionId,
                                                      unsigned& estBitrate)
{
    estBitrate = 5000;
    H264VideoFramedSource* src = new H264VideoFramedSource(envir(), mStream);
    if (mStream->discreteInput()) {
        DBG(DBG_L4,
            "XPR_RTSP: H264VideoServerMediaSubsession(%p): Use Discrete Input!",
            this);
        return H264VideoStreamDiscreteFramer::createNew(envir(), src, False);
    }
    return H264VideoStreamFramer::createNew(envir(), src, False);
}

RTPSink* H264VideoServerMediaSubsession::createNewRTPSink(
    Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
    FramedSource* inputSource)
{
    OutPacketBuffer::maxSize = 320000;
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock,
                                       rtpPayloadTypeIfDynamic);
}

H264VideoServerMediaSubsession*
H264VideoServerMediaSubsession::createNew(UsageEnvironment& env,
                                          FramedSource* source, Stream* stream)
{
    return new H264VideoServerMediaSubsession(env, source, stream);
}

// H265VideoFramedSource
//============================================================================
H265VideoFramedSource::H265VideoFramedSource(UsageEnvironment& env,
                                             Stream* stream)
    : FramedSource(env), mStream(stream), mLastPTS(0)
{
    DBG(DBG_L5, "XPR_RTSP: H265VideoFramedSource::H265VideoFramedSource(%p) = %p", &env,
        this);
}

H265VideoFramedSource::H265VideoFramedSource(const H265VideoFramedSource& rhs)
    : FramedSource(rhs.envir()), mStream(rhs.stream()), mLastPTS(rhs.mLastPTS)
{
}

H265VideoFramedSource::~H265VideoFramedSource(void)
{
    DBG(DBG_L5, "XPR_RTSP: H265VideoFramedSource::~H265VideoFramedSource() = %p", this);
    envir().taskScheduler().unscheduleDelayedTask(mCurrentTask);
}

void H265VideoFramedSource::doGetNextFrame()
{
    fetchFrame();
}

unsigned int H265VideoFramedSource::maxFrameSize() const
{
    return XPR_RTSP_H264_MAX_FRAME_SIZE;
}

void H265VideoFramedSource::fetchFrame()
{
    // Check video queue, if empty, delay for next.
    if (!mStream->hasVideoFrame()) {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            5000, getNextFrame, this);
        return;
    }
    // Fetch one block from video queue.
    XPR_StreamBlock* ntb = mStream->getVideoFrame();
    if (ntb) {
        fFrameSize = XPR_StreamBlockLength(ntb);
        if (mStream->appendOriginPTS() == 1)
            fFrameSize += H264_OPTS_FRM_LEN;
        if (fFrameSize > fMaxSize) {
            fNumTruncatedBytes = fFrameSize - fMaxSize;
            fFrameSize = fMaxSize;
        }
        if (mStream->appendOriginPTS() == 1) {
            uint32_t n = fFrameSize - H264_OPTS_FRM_LEN;
            int64_t usecs = XPR_StreamBlockPTS(ntb);
            const uint8_t sei[H264_OPTS_HDR_LEN] = {
                0x00, 0x00, 0x00, 0x01, 0x06, 0x05, 12, 'O', 'P', 'T', 'S'};
            memcpy(fTo, sei, sizeof(sei));
            memcpy(fTo + H264_OPTS_HDR_LEN, &usecs, sizeof(int64_t));
            memcpy(fTo + H264_OPTS_FRM_LEN, XPR_StreamBlockData(ntb), n);
        }
        else {
            memcpy(fTo, XPR_StreamBlockData(ntb), fFrameSize);
        }
#if 1
        if (mStream->discreteInput() == 1) {
            // Force PTS to ntb->pts
            int64_t usecs = XPR_StreamBlockPTS(ntb);
            fPresentationTime.tv_sec += usecs / 1000000;
            fPresentationTime.tv_usec = usecs % 1000000;
        }
        else {
            // Setup PTS with ntb->pts
            if (fPresentationTime.tv_sec == 0 && fPresentationTime.tv_usec == 0) {
                gettimeofday(&fPresentationTime, NULL);
                fDurationInMicroseconds = 10000;
            }
            else {
                int64_t usecs = XPR_StreamBlockPTS(ntb) - mLastPTS;
                if (usecs) {
                    usecs += fPresentationTime.tv_usec;
                    fPresentationTime.tv_sec += usecs / 1000000;
                    fPresentationTime.tv_usec = usecs % 1000000;
                }
            }
        }
        mLastPTS = XPR_StreamBlockPTS(ntb);
#else
        // Auto filled by H264VideoSteamFramer
        fPresentationTime.tv_sec = 0;
        fPresentationTime.tv_usec = 0;
#endif
        mStream->releaseVideoFrame(ntb);
        if (fNumTruncatedBytes > 0) {
            DBG(DBG_L2,
                "XPR_RTSP: H265VideoFramedSource(%p): %u bytes truncated.",
                this, fNumTruncatedBytes);
        }
    }
    else {
        // Should never run here
        DBG(DBG_L1, "XPR_RTSP: H265VideoFramedSource(%p): BUG: %s:%d\n", this,
            __FILE__, __LINE__);
        // Clear
        fFrameSize = 0;
        fNumTruncatedBytes = 0;
    }
    afterGetting(this);
}

Stream* H265VideoFramedSource::stream(void) const
{
    return mStream;
}

void H265VideoFramedSource::getNextFrame(void* ptr)
{
    if (ptr) {
        ((H265VideoFramedSource*)ptr)->fetchFrame();
    }
}

Boolean H265VideoFramedSource::isH265VideoStreamFramer() const
{
    return True;
}

// H265VideoServerMediaSubsession
//============================================================================
H265VideoServerMediaSubsession::H265VideoServerMediaSubsession(
    UsageEnvironment& env, FramedSource* source, Stream* stream)
    : OnDemandServerMediaSubsession(env, True)
    , mSource(source)
    , mSink(NULL)
    , mStream(stream)
    , mDoneFlag(0)
    , mAuxSDPLine(NULL)
{
    DBG(DBG_L5,
        "XPR_RTSP: "
        "H265VideoServerMediaSubsession::H265VideoServerMediaSubsession(%p, "
        "%p) = %p",
        &env, source, this);
}

H265VideoServerMediaSubsession::~H265VideoServerMediaSubsession(void)
{
    DBG(DBG_L5,
        "XPR_RTSP: "
        "H265VideoServerMediaSubsession::~H265VideoServerMediaSubsession() = "
        "%p",
        this);
    if (mAuxSDPLine) {
        free(mAuxSDPLine);
        mAuxSDPLine = NULL;
    }
    mSource = NULL;
    mSink = NULL;
    mDoneFlag = 0;
}

void H265VideoServerMediaSubsession::afterPlayingDummy(void* clientData)
{
    H265VideoServerMediaSubsession* subsess =
        (H265VideoServerMediaSubsession*)clientData;
    subsess->afterPlayingDummy1();
}

void H265VideoServerMediaSubsession::afterPlayingDummy1()
{
    // Unschedule any pending 'checking' task:
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    // Signal the event loop that we're done:
    setDoneFlag();
}

void H265VideoServerMediaSubsession::setDoneFlag()
{
    mDoneFlag = ~0x00;
}

void H265VideoServerMediaSubsession::checkForAuxSDPLine(void* clientData)
{
    H265VideoServerMediaSubsession* subsess =
        (H265VideoServerMediaSubsession*)clientData;
    subsess->checkForAuxSDPLine1();
}

void H265VideoServerMediaSubsession::checkForAuxSDPLine1()
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
        DBG(DBG_L4,
            "XPR_RTSP: H265VideoServerMediaSubsession(%p): AuxSDPLine [%s]",
            this, mAuxSDPLine);
        // Signal the event loop that we're done:
        setDoneFlag();
    }
    else if (!mDoneFlag) {
        // try again after a brief delay:
        int uSecsToDelay = 10000; // 10 ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            uSecsToDelay, (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const*
H265VideoServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink,
                                              FramedSource* inputSource)
{
    if (mAuxSDPLine) {
        return mAuxSDPLine;
    }
    if (mSink == NULL) {
        // we're not already setting it up for another, concurrent stream
        // Note: For H264 video files, the 'config' information
        // ("profile-level-id" and "sprop-parameter-sets") isn't known until we
        // start reading the file.  This means that "rtpSink"s "auxSDPLine()"
        // will be NULL initially, and we need to start reading data from our
        // file until this changes.
        mSink = rtpSink;
        // Start reading the file:
        mSink->startPlaying(*inputSource, afterPlayingDummy, this);
        // Check whether the sink's 'auxSDPLine()' is ready:
        checkForAuxSDPLine(this);
    }
    envir().taskScheduler().doEventLoop(&mDoneFlag);
    return mAuxSDPLine;
}

FramedSource*
H265VideoServerMediaSubsession::createNewStreamSource(unsigned clientSessionId,
                                                      unsigned& estBitrate)
{
    estBitrate = 5000;
    H265VideoFramedSource* src = new H265VideoFramedSource(envir(), mStream);
    if (mStream->discreteInput()) {
        DBG(DBG_L4,
            "XPR_RTSP: H265VideoServerMediaSubsession(%p): Use Discrete Input!",
            this);
        return H264VideoStreamDiscreteFramer::createNew(envir(), src, False);
    }
    return H264VideoStreamFramer::createNew(envir(), src, False);
}

RTPSink* H265VideoServerMediaSubsession::createNewRTPSink(
    Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
    FramedSource* inputSource)
{
    OutPacketBuffer::maxSize = 320000;
    return H264VideoRTPSink::createNew(envir(), rtpGroupsock,
                                       rtpPayloadTypeIfDynamic);
}

H265VideoServerMediaSubsession*
H265VideoServerMediaSubsession::createNew(UsageEnvironment& env,
                                          FramedSource* source, Stream* stream)
{
    return new H265VideoServerMediaSubsession(env, source, stream);
}

// JPEGVideoFramedSource
//============================================================================
JPEGVideoFramedSource::JPEGVideoFramedSource(UsageEnvironment& env,
                                             Stream* stream)
    : FramedSource(env), mStream(stream), mLastPTS(0)
{
    DBG(DBG_L5,
        "XPR_RTSP: JPEGVideoFramedSource::JPEGVideoFramedSource(%p) = %p", &env,
        this);
}

JPEGVideoFramedSource::JPEGVideoFramedSource(const JPEGVideoFramedSource& rhs)
    : FramedSource(rhs.envir()), mStream(rhs.stream()), mLastPTS(rhs.mLastPTS)
{
}

JPEGVideoFramedSource::~JPEGVideoFramedSource(void)
{
    DBG(DBG_L5,
        "XPR_RTSP: JPEGVideoFramedSource::~JPEGVideoFramedSource() = %p", this);
    envir().taskScheduler().unscheduleDelayedTask(mCurrentTask);
}

void JPEGVideoFramedSource::doGetNextFrame()
{
    fetchFrame();
}

unsigned int JPEGVideoFramedSource::maxFrameSize() const
{
    return XPR_RTSP_JPEG_MAX_FRAME_SIZE;
}

void JPEGVideoFramedSource::fetchFrame()
{
    // Check video queue, if empty, delay for next.
    if (!mStream->hasVideoFrame()) {
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            5000, getNextFrame, this);
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
        // Auto filled by JPEGVideoSteamFramer
        fPresentationTime.tv_sec = 0;
        fPresentationTime.tv_usec = 0;
#endif
        mStream->releaseVideoFrame(ntb);
        if (fNumTruncatedBytes > 0) {
            DBG(DBG_L2,
                "XPR_RTSP: JPEGVideoFramedSource(%p): %u bytes truncated.",
                this, fNumTruncatedBytes);
        }
    }
    else {
        // Should never run here
        DBG(DBG_L1, "XPR_RTSP: JPEGVideoFramedSource(%p): BUG: %s:%d\n", this,
            __FILE__, __LINE__);
        // Clear
        fFrameSize = 0;
        fNumTruncatedBytes = 0;
    }
    afterGetting(this);
}

Stream* JPEGVideoFramedSource::stream(void) const
{
    return mStream;
}

void JPEGVideoFramedSource::getNextFrame(void* ptr)
{
    if (ptr) {
        ((JPEGVideoFramedSource*)ptr)->fetchFrame();
    }
}

Boolean JPEGVideoFramedSource::isJPEGVideoStreamFramer() const
{
    return True;
}

// JPEGVideoServerMediaSubsession
//============================================================================
JPEGVideoServerMediaSubsession::JPEGVideoServerMediaSubsession(
    UsageEnvironment& env, FramedSource* source, Stream* stream)
    : OnDemandServerMediaSubsession(env, True)
    , mSource(source)
    , mSink(NULL)
    , mStream(stream)
    , mDoneFlag(0)
    , mAuxSDPLine(NULL)
{
    DBG(DBG_L5,
        "XPR_RTSP: "
        "JPEGVideoServerMediaSubsession::JPEGVideoServerMediaSubsession(%p, "
        "%p) = %p",
        &env, source, this);
}

JPEGVideoServerMediaSubsession::~JPEGVideoServerMediaSubsession(void)
{
    DBG(DBG_L5,
        "XPR_RTSP: "
        "JPEGVideoServerMediaSubsession::~JPEGVideoServerMediaSubsession() = "
        "%p",
        this);
    if (mAuxSDPLine) {
        free(mAuxSDPLine);
        mAuxSDPLine = NULL;
    }
    mSource = NULL;
    mSink = NULL;
    mDoneFlag = 0;
}

void JPEGVideoServerMediaSubsession::afterPlayingDummy(void* clientData)
{
    JPEGVideoServerMediaSubsession* subsess =
        (JPEGVideoServerMediaSubsession*)clientData;
    subsess->afterPlayingDummy1();
}

void JPEGVideoServerMediaSubsession::afterPlayingDummy1()
{
    // Unschedule any pending 'checking' task:
    envir().taskScheduler().unscheduleDelayedTask(nextTask());
    // Signal the event loop that we're done:
    setDoneFlag();
}

void JPEGVideoServerMediaSubsession::setDoneFlag()
{
    mDoneFlag = ~0x00;
}

void JPEGVideoServerMediaSubsession::checkForAuxSDPLine(void* clientData)
{
    JPEGVideoServerMediaSubsession* subsess =
        (JPEGVideoServerMediaSubsession*)clientData;
    subsess->checkForAuxSDPLine1();
}

void JPEGVideoServerMediaSubsession::checkForAuxSDPLine1()
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
        DBG(DBG_L4,
            "XPR_RTSP: JPEGVideoServerMediaSubsession(%p): AuxSDPLine [%s]",
            this, mAuxSDPLine);
        // Signal the event loop that we're done:
        setDoneFlag();
    }
    else if (!mDoneFlag) {
        // try again after a brief delay:
        int uSecsToDelay = 10000; // 10 ms
        nextTask() = envir().taskScheduler().scheduleDelayedTask(
            uSecsToDelay, (TaskFunc*)checkForAuxSDPLine, this);
    }
}

char const*
JPEGVideoServerMediaSubsession::getAuxSDPLine(RTPSink* rtpSink,
                                              FramedSource* inputSource)
{
    if (mAuxSDPLine) {
        return mAuxSDPLine;
    }
    if (mSink == NULL) {
        // we're not already setting it up for another, concurrent stream
        // until we start reading the file.  This means that "rtpSink"s
        // "auxSDPLine()" will be NULL initially, and we need to start reading
        // data from our file until this changes.
        mSink = rtpSink;
        // Start reading the file:
        mSink->startPlaying(*inputSource, afterPlayingDummy, this);
        // Check whether the sink's 'auxSDPLine()' is ready:
        checkForAuxSDPLine(this);
    }
    envir().taskScheduler().doEventLoop(&mDoneFlag);
    return mAuxSDPLine;
}

FramedSource*
JPEGVideoServerMediaSubsession::createNewStreamSource(unsigned clientSessionId,
                                                      unsigned& estBitrate)
{
    estBitrate = 5000;
    JPEGVideoFramedSource* src = new JPEGVideoFramedSource(envir(), mStream);
    // FIXME:
    // return JPEGVideoStreamFramer::createNew(envir(), src, False);
    return src;
}

RTPSink* JPEGVideoServerMediaSubsession::createNewRTPSink(
    Groupsock* rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
    FramedSource* inputSource)
{
    OutPacketBuffer::maxSize = XPR_RTSP_JPEG_MAX_FRAME_SIZE;
    return JPEGVideoRTPSink::createNew(envir(), rtpGroupsock);
}

JPEGVideoServerMediaSubsession*
JPEGVideoServerMediaSubsession::createNew(UsageEnvironment& env,
                                          FramedSource* source, Stream* stream)
{
    return new JPEGVideoServerMediaSubsession(env, source, stream);
}

// Server
//============================================================================
Server::Server(int id, Port* parent)
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
    DBG(DBG_L5, "XPR_RTSP: Server::Server(%d, %p) = %p", id, parent, this);
    memset(mStreams, 0, sizeof(mStreams));
    memset(mWorkers, 0, sizeof(mWorkers));
}

Server::~Server(void)
{
    DBG(DBG_L5, "XPR_RTSP: Server::~Server(id, %p) = %p", id(), parent(), this);
}

int Server::isPortValid(int port)
{
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

int Server::open(int port, const char* url)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): open(%08X,%s)", this, port, url);
    if (isPortValid(port) == XPR_FALSE || url == NULL) {
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    }
    // Open server if XPR_RTSP_PORT_MINOR_NUL
    if (XPR_RTSP_PORT_MINOR(port) == XPR_RTSP_PORT_MINOR_NUL)
        return openServer(url);
    return openStream(port, url);
}

int Server::close(int port)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): close(%08X)", this, port);
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    // Close server if XPR_RTSP_PORT_MINOR_NUL
    if (XPR_RTSP_PORT_MINOR(port) == XPR_RTSP_PORT_MINOR_NUL)
        return closeServer();
    return closeStream(port);
}

int Server::start(int port)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): start(%08X)", this, port);
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    // Start server if XPR_RTSP_PORT_MINOR_NUL
    if (XPR_RTSP_PORT_MINOR(port) == XPR_RTSP_PORT_MINOR_NUL)
        return startServer();
    return startStream(port);
}

int Server::stop(int port)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): stop(%08X)", this, port);
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    // Stop server if XPR_RTSP_PORT_MINOR_NUL
    if (XPR_RTSP_PORT_MINOR(port) == XPR_RTSP_PORT_MINOR_NUL)
        return stopServer();
    return stopStream(port);
}

int Server::pushData(int port, XPR_StreamBlock* stb)
{
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    int minor = XPR_RTSP_PORT_MINOR(port);
    // Not support push data if minor ==
    //   XPR_RTSP_PORT_MINOR_NUL ||
    //   XPR_RTSP_PORT_MINOR_ALL ||
    //   XPR_RTSP_PORT_MINOR_ANY
    if (minor == XPR_RTSP_PORT_MINOR_NUL || minor == XPR_RTSP_PORT_MINOR_ALL ||
        minor == XPR_RTSP_PORT_MINOR_ANY)
        return XPR_ERR_GEN_NOT_SUPPORT;
    return mStreams[minor]->pushData(port, stb);
}

int Server::postEvent(int port, const XPR_RTSP_EVD* evd)
{
    int err = XPR_ERR_OK;
    if (isPortValid(port) == XPR_FALSE) {
        printf("PORT %d invalid\n", port);
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    }
    for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
        Callback* ncb = &mCallbacks[i];
        if (ncb->evcb) {
            err = ncb->evcb(ncb->evcb_opaque, port, evd);
            if (err != XPR_ERR_OK)
                break;
        }
    }
    return err;
}

RTSPServer* Server::rtspServer(void)
{
    return mRTSPServer;
}

Stream** Server::streams(void)
{
    return mStreams;
}

Worker* Server::worker(int index) const
{
    return mWorkers[index];
}

Worker** Server::workers(void)
{
    return mWorkers;
}

const char* Server::getBindAddress(void) const
{
    return mBindAddress.c_str();
}

void Server::setBindAddress(const char* bindAddress)
{
    mBindAddress = bindAddress;
}

uint16_t Server::getBindPort(void) const
{
    return mBindPort;
}

void Server::setBindPort(uint16_t bindPort)
{
    mBindPort = bindPort;
}

size_t Server::getMaxStreams(void) const
{
    return mMaxStreams;
}

void Server::setMaxStreams(size_t maxStreams)
{
    mMaxStreams = MIN(maxStreams, XPR_RTSP_PORT_STREAM_MAX);
}

size_t Server::getMaxStreamTracks(void) const
{
    return mMaxStreamTracks;
}

void Server::setMaxStreamTracks(size_t maxStreamTracks)
{
    mMaxStreamTracks = MIN(maxStreamTracks, XPR_RTSP_PORT_TRACK_MAX);
}

size_t Server::getMaxWorkers(void) const
{
    return mMaxWorkers;
}

void Server::setMaxWorkers(size_t maxWorkers)
{
    mMaxWorkers = MIN(maxWorkers, XPR_RTSP_MAX_WORKERS);
}

bool Server::isValidStreamId(int streamId)
{
    return false;
}

bool Server::isValidStreamTrackId(int streamTrackId)
{
    return false;
}

int Server::openServer(const char* url)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): openServer(%s)", this, url);
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

int Server::closeServer(void)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): closeServer()", this);
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

int Server::startServer(void)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): startServer()", this);
    if (!(activeFlags() & PortFlags::PORT_FLAG_OPEN))
        return XPR_ERR_GEN_SYS_NOTREADY;
    if (!mUsername.empty()) {
        if (!mAuthDB->lookupPassword(mUsername.c_str())) {
            mAuthDB->addUserRecord(mUsername.c_str(), mPassword.c_str());
        }
    }
    for (size_t i = 0; i < mMaxWorkers; i++) {
        if (mWorkers[i]) {
            mWorkers[i]->start();
        }
    }
    activeFlags(PortFlags::PORT_FLAG_START, 0);
    return 0;
}

int Server::stopServer(void)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): stopServer()", this);
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

int Server::openStream(int port, const char* url)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): openStream(%08X,%s)", this, port, url);
    int streamId = XPR_RTSP_PORT_STREAM(port);
    Stream* stream = mStreams[streamId];
    if (stream)
        return stream->open(port, url);
    return XPR_ERR_GEN_UNEXIST;
}

int Server::closeStream(int port)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): closeStream(%08X)", this, port);
    int streamId = XPR_RTSP_PORT_STREAM(port);
    Stream* stream = mStreams[streamId];
    if (stream)
        return stream->close(port);
    return XPR_ERR_GEN_UNEXIST;
}

int Server::startStream(int port)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): startStream(%08X)", this, port);
    int streamId = XPR_RTSP_PORT_STREAM(port);
    Stream* stream = mStreams[streamId];
    if (stream)
        return stream->start(port);
    return XPR_ERR_GEN_UNEXIST;
}

int Server::stopStream(int port)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): stopStream(%08X)", this, port);
    int streamId = XPR_RTSP_PORT_STREAM(port);
    Stream* stream = mStreams[streamId];
    if (stream)
        return stream->stop(port);
    return XPR_ERR_GEN_UNEXIST;
}

int Server::setupServer(const char* url)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): setupServer(%s)", this, url);
    // 解析地址参数
    XPR_Url* u = XPR_UrlParse(url, -1);
    if (u == NULL)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    //
    mUrl = url;
    // 设定用户名及密码
    if (XPR_UrlHaveUsername(u))
        mUsername = XPR_UrlGetUsername(u);
    if (XPR_UrlHavePassword(u))
        mPassword = XPR_UrlGetPassword(u);
    DBG(DBG_L3, "XPR_RTSP: Server(%p): username=\"%s\", password=\"%s\"", this,
        mUsername.c_str(), mPassword.c_str());
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

int Server::clearServer(void)
{
    DBG(DBG_L4, "XPR_RTSP: Server(%p): clearServer()", this);
    mUrl.clear();
    mUsername.clear();
    mPassword.clear();
    mBindAddress.clear();
    return XPR_ERR_OK;
}

void Server::configServer(const char* query)
{
    if (query && query[0]) {
        DBG(DBG_L3, "XPR_RTSP: Server(%p): configuration query: \"%s\"", this,
            query);
        xpr_foreach_s(query, -1, "&", Server::handleServerConfig, this);
    }
}

void Server::configServer(const char* key, const char* value)
{
    DBG(DBG_L3, "XPR_RTSP: Server(%p): configuration: \"%s\"=\"%s\"", this, key,
        value);
    if (strcmp(key, "maxStreams") == 0)
        setMaxStreams(strtol(value, NULL, 10));
    else if (strcmp(key, "maxStreamTracks") == 0)
        setMaxStreamTracks(strtol(value, NULL, 10));
    else if (strcmp(key, "maxWorkers") == 0)
        setMaxWorkers(strtol(value, NULL, 10));
    else {
        DBG(DBG_L3, "XPR_RTSP: Server(%p): configuration: \"%s\" unsupported.",
            this, key);
    }
}

void Server::setupStreams(void)
{
    for (size_t i = XPR_RTSP_PORT_STREAM_MIN; i <= mMaxStreams; i++) {
        mStreams[i] = new Stream(i, this);
    }
}

void Server::clearStreams(void)
{
    for (size_t i = XPR_RTSP_PORT_STREAM_MIN; i <= mMaxStreams; i++) {
        if (mStreams[i]) {
            delete mStreams[i];
            mStreams[i] = NULL;
        }
    }
}

void Server::configStream(int port, const char* query)
{
    // FIXME:
}

void Server::configStream(int port, const char* key, const char* value)
{
    // FIXME:
}

void Server::setupWorkers(void)
{
    for (size_t i = 0; i < mMaxWorkers; i++) {
        mWorkers[i] = new Worker(i, this);
    }
}

void Server::clearWorkers(void)
{
    for (size_t i = 0; i < mMaxWorkers; i++) {
        if (mWorkers[i]) {
            delete mWorkers[i];
            mWorkers[i] = NULL;
        }
    }
}

void Server::setupRTSPServer(void)
{
    UserAuthenticationDatabase* authDB = nullptr;
    mAuthDB = new UserAuthenticationDatabase();
#if defined(DEBUG) || defined(_DEBUG)
    mAuthDB->addUserRecord("test", "test");
#endif
    if (!mUsername.empty()) {
        mAuthDB->addUserRecord(mUsername.c_str(), mPassword.c_str());
        authDB = mAuthDB;
    }
    mRTSPServer =
        MyRTSPServer::createNew(this, mWorkers[0]->env(), mBindPort, authDB);
    if (mRTSPServer == NULL) {
        DBG(DBG_L1,
            "XPR_RTSP: Server(%p): Failed to create RTSPServer, you "
            "should try to run in root again!",
            this);
    }
}

void Server::clearRTSPServer(void)
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

void Server::handleServerConfig(void* opaque, char* seg)
{
    if (opaque && seg) {
        char* key = NULL;
        char* value = NULL;
        if (xpr_split_to_kv(seg, &key, &value) == XPR_ERR_OK)
            ((Server*)opaque)->configServer(key, value);
    }
}

// Stream
//============================================================================
Stream::Stream(int id, Port* parent)
    : Port(id, parent)
    , mSMS(NULL)
    , mAudioQ(NULL)
    , mVideoQ(NULL)
    , mAQL(30)
    , mVQL(30)
    , mAsrcId(1)
    , mVsrcId(2)
    , mAudioProfile(1)
    , mAudioFreqIdx(4)
    , mAudioChannels(2)
    , mAppendOriginPTS(0)
    , mDiscreteInput(0)
{
    DBG(DBG_L5, "XPR_RTSP: Stream::Stream(%d, %p) = %p", id, parent, this);
}

Stream::~Stream(void)
{
    DBG(DBG_L5, "XPR_RTSP: Stream::~Stream(%d, %p) = %p", id(), parent(), this);
}

int Stream::open(int port, const char* url)
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
    DBG(DBG_L4, "XPR_RTSP: Stream(%p): Stream Name: %s", this, path);
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
int Stream::close(int port)
{
    if (mAudioQ) {
        XPR_FifoDestroy(mAudioQ);
        mAudioQ = NULL;
    }
    if (mVideoQ) {
        XPR_FifoDestroy(mVideoQ);
        mVideoQ = NULL;
    }
    return 0;
}

int Stream::start(int port)
{
    Server* server = (Server*)parent();
    if (mSMS == NULL || server->rtspServer() == NULL)
        return XPR_ERR_GEN_SYS_NOTREADY;
    server->rtspServer()->addServerMediaSession(mSMS);
    DBG(DBG_L4, "XPR_RTSP: Stream(%p): Streaming Url: %s", this,
        server->rtspServer()->rtspURL(mSMS));
    return XPR_ERR_OK;
}

int Stream::stop(int port)
{
    Server* server = (Server*)parent();
    if (mSMS == NULL || server->rtspServer() == NULL)
        return XPR_ERR_GEN_SYS_NOTREADY;
    server->rtspServer()->removeServerMediaSession(mSMS);
    mSMS = NULL;
    return XPR_ERR_OK;
}

int Stream::pushData(int port, XPR_StreamBlock* stb)
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

ServerMediaSession* Stream::sms(void) const
{
    return mSMS;
}

int Stream::appendOriginPTS(void) const
{
    return mAppendOriginPTS;
}

int Stream::discreteInput(void) const
{
    return mDiscreteInput;
}

bool Stream::hasAudioFrame(void) const
{
    return !XPR_FifoIsEmpty(mAudioQ);
}

bool Stream::hasVideoFrame(void) const
{
    return !XPR_FifoIsEmpty(mVideoQ);
}

XPR_StreamBlock* Stream::getAudioFrame(void) const
{
    XPR_StreamBlock* ntb = (XPR_StreamBlock*)XPR_FifoGetAsAtomic(mAudioQ);
    return ntb;
}

XPR_StreamBlock* Stream::getVideoFrame(void) const
{
    XPR_StreamBlock* ntb = (XPR_StreamBlock*)XPR_FifoGetAsAtomic(mVideoQ);
    return ntb;
}

int Stream::putAudioFrame(XPR_StreamBlock* stb)
{
    if (!stb || !mAudioQ)
        return XPR_ERR_GEN_NULL_PTR;
    if (XPR_FifoIsFull(mAudioQ))
        releaseAudioFrame(getAudioFrame());
    XPR_StreamBlock* ntb = stb;
    if (!(stb->flags & XPR_STREAMBLOCK_FLAG_REFERABLE))
        ntb = XPR_StreamBlockDuplicate(stb);
    int err = XPR_FifoPutAsAtomic(mAudioQ, (uintptr_t)ntb);
    if (XPR_IS_ERROR(err)) {
        releaseAudioFrame(ntb);
    }
    return err;
}

int Stream::putVideoFrame(XPR_StreamBlock* stb)
{
    if (!stb || !mVideoQ)
        return XPR_ERR_GEN_NULL_PTR;
    if (XPR_FifoIsFull(mVideoQ))
        releaseVideoFrame(getVideoFrame());
    XPR_StreamBlock* ntb = stb;
    if (!(stb->flags & XPR_STREAMBLOCK_FLAG_REFERABLE))
        ntb = XPR_StreamBlockDuplicate(stb);
    int err = XPR_FifoPutAsAtomic(mVideoQ, (uintptr_t)ntb);
    if (XPR_IS_ERROR(err)) {
        releaseVideoFrame(ntb);
    }
    return err;
}

void Stream::releaseAudioFrame(XPR_StreamBlock* stb)
{
    if (!stb)
        return;
    if (stb->pf_release)
        XPR_StreamBlockRelease(stb);
    else
        XPR_StreamBlockFree(stb);
}

void Stream::releaseVideoFrame(XPR_StreamBlock* stb)
{
    if (!stb)
        return;
    if (stb->pf_release)
        XPR_StreamBlockRelease(stb);
    else
        XPR_StreamBlockFree(stb);
}

void Stream::configStream(const char* query)
{
    if (query && query[0]) {
        DBG(DBG_L3, "XPR_RTSP: Stream(%p): configuration: \"%s\"", this, query);
        xpr_foreach_s(query, -1, "&", Stream::handleStreamConfig, this);
    }
}

void Stream::configStream(const char* key, const char* value)
{
    DBG(DBG_L3, "XPR_RTSP: Stream(%p): configuration: \"%s\"=\"%s\"", this, key,
        value);
    Server* server = (Server*)parent();
    if (strcmp(key, "sourceType") == 0) {
        mSourceType = value;
    }
    else if (strcmp(key, "track") == 0) {
        mSourceType = "stb";
        mTrackId = value;
    }
    else if (strcmp(key, "mime") == 0) {
        if (strcmp(value, "audio/AAC") == 0) {
            if (mSourceType == "stb") {
                UsageEnvironment& env = server->workers()[0]->env();
                mSMS->addSubsession(ADTSAudioServerMediaSubsession::createNew(
                    env, NULL, this, mAudioProfile, mAudioFreqIdx,
                    mAudioChannels));
            }
        }
        if (strcmp(value, "video/H264") == 0) {
            if (mSourceType == "stb") {
                UsageEnvironment& env = server->workers()[0]->env();
                mSMS->addSubsession(
                    H264VideoServerMediaSubsession::createNew(env, NULL, this));
            }
        }
        if (strcmp(value, "video/H265") == 0) {
            if (mSourceType == "stb") {
                UsageEnvironment& env = server->workers()[0]->env();
                mSMS->addSubsession(
                    H265VideoServerMediaSubsession::createNew(env, NULL, this));
            }
        }
        else if (strcmp(value, "video/JPEG") == 0) {
            if (mSourceType == "stb") {
                UsageEnvironment& env = server->workers()[0]->env();
                mSMS->addSubsession(
                    JPEGVideoServerMediaSubsession::createNew(env, NULL, this));
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
    else if (strcmp(key, "audioProfile") == 0) {
        mAudioProfile = strtol(value, NULL, 10);
    }
    else if (strcmp(key, "audioFreqIdx") == 0) {
        mAudioFreqIdx = strtol(value, NULL, 10);
    }
    else if (strcmp(key, "audioChannles") == 0) {
        mAudioChannels = strtol(value, NULL, 10);
    }
    else if (strcmp(key, "appendOriginPTS") == 0) {
        mAppendOriginPTS = strtol(value, NULL, 10);
    }
    else if (strcmp(key, "discreteInput") == 0) {
        mDiscreteInput = strtol(value, NULL, 10);
    }
    else {
        DBG(DBG_L3, "XPR_RTSP: Stream(%p): configuration: \"%s\" unsupported.",
            this, key);
    }
}

void Stream::handleStreamConfig(void* opaque, char* seg)
{
    if (opaque && seg) {
        char* key = NULL;
        char* value = NULL;
        if (xpr_split_to_kv(seg, &key, &value) == XPR_ERR_OK)
            ((Stream*)opaque)->configStream(key, value);
    }
}

} // namespace rtsp

} // namespace xpr

#endif // defined(HAVE_XPR_RTSP_DRIVER_LIVE)

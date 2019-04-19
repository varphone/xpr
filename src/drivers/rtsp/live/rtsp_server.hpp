#ifndef XPR_RTSP_DRIVER_LIVE_RTSP_SERVER_HPP
#define XPR_RTSP_DRIVER_LIVE_RTSP_SERVER_HPP

#include <stdint.h>
#include <string>
#include <live/BasicUsageEnvironment.hh>
#include <live/FramedSource.hh>
#include <live/liveMedia.hh>
#include <xpr/xpr_fifo.h>
#include "rtsp.hpp"

#define XPR_RTSP_ADTS_MAX_FRAME_SIZE    96000
#define XPR_RTSP_ADTS_AUDIO_BUFFER_SIZE (1024*4)

#define XPR_RTSP_H264_MAX_FRAME_SIZE    320000
#define XPR_RTSP_H264_VIDEO_BUFFER_SIZE (1024*1024*2)

#define XPR_RTSP_JPEG_MAX_FRAME_SIZE    512000
#define XPR_RTSP_JPEG_VIDEO_BUFFER_SIZE (1024*1024*2)

namespace xpr
{

namespace rtsp
{

// 前置声明
class ADTSAudioFramedSource;
class H264VideoFramedSource;
class H264VideoServerMediaSubsession;
class JPEGVideoFramedSource;
class JPEGVideoServerMediaSubsession;
class Server;
class ServerManager;
class Stream;
class Worker;

class ADTSAudioFramedSource : public FramedSource {
public:
    ADTSAudioFramedSource(UsageEnvironment& env, Stream* stream,
                          u_int8_t profile, u_int8_t samplingFrequencyIndex,
                          u_int8_t channelConfiguration);
    ADTSAudioFramedSource(const ADTSAudioFramedSource& rhs);
    virtual ~ADTSAudioFramedSource(void);

public:
    // FramedSource interfaces
    virtual void doGetNextFrame();
    virtual unsigned int maxFrameSize() const;

    // Methods
    void fetchFrame();

    // Properties
    Stream* stream(void) const;

    unsigned samplingFrequency() const
    {
        return mSamplingFrequency;
    }

    unsigned numChannels() const
    {
        return mNumChannels;
    }

    char const* configStr() const
    {
        return mConfigStr;
    }

    //
    static void getNextFrame(void* ptr);

private:
    TaskToken mCurrentTask;
    Stream* mStream;
    int64_t mLastPTS;
    unsigned mSamplingFrequency;
    unsigned mNumChannels;
    unsigned mUSecsPerFrame;
    char mConfigStr[5];
};

/// ADTS 音频服务媒体会话
class ADTSAudioServerMediaSubsession : public OnDemandServerMediaSubsession {
public:
    virtual ~ADTSAudioServerMediaSubsession(void);

    // Override OnDemandServerMediaSubsession interfaces
    virtual FramedSource*
    createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                      unsigned char rtpPayloadTypeIfDynamic,
                                      FramedSource* inputSource);

    // Used to implement "getAuxSDPLine()":
    void checkForAuxSDPLine1();
    void afterPlayingDummy1();
    void setDoneFlag();

    // Static Methods
    static ADTSAudioServerMediaSubsession*
    createNew(UsageEnvironment& env, FramedSource* source, Stream* stream,
              u_int8_t profile, u_int8_t samplingFrequencyIndex,
              u_int8_t channelConfiguration);

    static void afterPlayingDummy(void* ptr);
    static void checkForAuxSDPLine(void* ptr);

protected:
    ADTSAudioServerMediaSubsession(UsageEnvironment& env, FramedSource* source,
                                   Stream* stream, u_int8_t profile,
                                   u_int8_t samplingFrequencyIndex,
                                   u_int8_t channelConfiguration);

    // Override OnDemandServerMediaSubsession interfaces
    virtual char const* getAuxSDPLine(RTPSink* rtpSink,
                                      FramedSource* inputSource);

private:
    FramedSource* mSource;
    RTPSink* mSink;
    Stream* mStream;
    u_int8_t mProfile;
    u_int8_t mSamplingFrequencyIndex;
    u_int8_t mChannelConfiguration;
    char mDoneFlag;
    char* mAuxSDPLine;
};

/// 基于帧的 H264 视频源
class H264VideoFramedSource : public FramedSource
{
public:
    H264VideoFramedSource(UsageEnvironment& env, Stream* stream);
    H264VideoFramedSource(const H264VideoFramedSource& rhs);
    virtual ~H264VideoFramedSource(void);

public:
    // FramedSource interfaces
    virtual void doGetNextFrame();
    virtual unsigned int maxFrameSize() const;

    // Methods
    void fetchFrame();

    // Properties
    Stream* stream(void) const;

    //
    static void getNextFrame(void* ptr);

private:
    virtual Boolean isH264VideoStreamFramer() const;

private:
    TaskToken   mCurrentTask;
    Stream*     mStream;
    uint8_t*    mBuffer;
    size_t      mBufferOffset;
    size_t      mBufferSize;
    XPR_Fifo*   mFreeList;
    XPR_Fifo*   mDataList;
    int64_t     mLastPTS;
};

/// H264 视频服务媒体会话
class H264VideoServerMediaSubsession : public OnDemandServerMediaSubsession
{
public:
    virtual ~H264VideoServerMediaSubsession(void);

    // Override OnDemandServerMediaSubsession interfaces
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
                                                unsigned& estBitrate); // "estBitrate" is the stream's estimated bitrate, in kbps
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                      unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

    // Used to implement "getAuxSDPLine()":
    void checkForAuxSDPLine1();
    void afterPlayingDummy1();
    void setDoneFlag();

    // Static Methods
    static H264VideoServerMediaSubsession* createNew(UsageEnvironment& env,
                                                     FramedSource* source, Stream* stream);

    static void afterPlayingDummy(void* ptr);
    static void checkForAuxSDPLine(void* ptr);

protected:
    H264VideoServerMediaSubsession(UsageEnvironment& env, FramedSource* source,
                                   Stream* stream);

    // Override OnDemandServerMediaSubsession interfaces
    virtual char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource);

private:
    FramedSource*       mSource;
    RTPSink*            mSink;
    Stream*             mStream;
    char                mDoneFlag;
    char*               mAuxSDPLine;
};

/// 基于帧的 JPEG 视频源
class JPEGVideoFramedSource : public FramedSource
{
public:
    JPEGVideoFramedSource(UsageEnvironment& env, Stream* stream);
    JPEGVideoFramedSource(const JPEGVideoFramedSource& rhs);
    virtual ~JPEGVideoFramedSource(void);

public:
    // FramedSource interfaces
    virtual void doGetNextFrame();
    virtual unsigned int maxFrameSize() const;

    // Methods
    void fetchFrame();

    // Properties
    Stream* stream(void) const;

    //
    static void getNextFrame(void* ptr);

private:
    virtual Boolean isJPEGVideoStreamFramer() const;

private:
    TaskToken   mCurrentTask;
    Stream*     mStream;
    uint8_t*    mBuffer;
    size_t      mBufferOffset;
    size_t      mBufferSize;
    XPR_Fifo*   mFreeList;
    XPR_Fifo*   mDataList;
    int64_t     mLastPTS;
};

/// JPEG 视频服务媒体会话
class JPEGVideoServerMediaSubsession : public OnDemandServerMediaSubsession
{
public:
    virtual ~JPEGVideoServerMediaSubsession(void);

    // Override OnDemandServerMediaSubsession interfaces
    virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
                                                unsigned& estBitrate); // "estBitrate" is the stream's estimated bitrate, in kbps
    virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                      unsigned char rtpPayloadTypeIfDynamic, FramedSource* inputSource);

    // Used to implement "getAuxSDPLine()":
    void checkForAuxSDPLine1();
    void afterPlayingDummy1();
    void setDoneFlag();

    // Static Methods
    static JPEGVideoServerMediaSubsession* createNew(UsageEnvironment& env,
                                                     FramedSource* source, Stream* stream);

    static void afterPlayingDummy(void* ptr);
    static void checkForAuxSDPLine(void* ptr);

protected:
    JPEGVideoServerMediaSubsession(UsageEnvironment& env, FramedSource* source,
                                   Stream* stream);

    // Override OnDemandServerMediaSubsession interfaces
    virtual char const* getAuxSDPLine(RTPSink* rtpSink, FramedSource* inputSource);

private:
    FramedSource*       mSource;
    RTPSink*            mSink;
    Stream*             mStream;
    char                mDoneFlag;
    char*               mAuxSDPLine;
};

/// 服务器
class Server : public Port
{
public:
    Server(int id, Port* parent);
    virtual ~Server(void);

    // Port interfaces
    virtual int isPortValid(int port);
    virtual int open(int port, const char* url);
    virtual int close(int port);
    virtual int start(int port);
    virtual int stop(int port);
    virtual int pushData(int port, XPR_StreamBlock* stb);

    // Properties
    RTSPServer* rtspServer(void);
    Stream** streams(void);
    Worker* worker(int index) const;
    Worker** workers(void);

    // Methods
    const char* getBindAddress(void) const;
    void setBindAddress(const char* bindAddress);

    uint16_t getBindPort(void) const;
    void setBindPort(uint16_t port);

    size_t getMaxStreams(void) const;
    void setMaxStreams(size_t maxStreams);

    size_t getMaxStreamTracks(void) const;
    void setMaxStreamTracks(size_t maxStreamTracks);

    size_t getMaxWorkers(void) const;
    void setMaxWorkers(size_t maxWorkers);

    bool isValidStreamId(int streamId);
    bool isValidStreamTrackId(int streamTrackId);

private:
    int openServer(const char* url);
    int closeServer(void);

    int startServer(void);
    int stopServer(void);

    int openStream(int port, const char* url);
    int closeStream(int port);

    int startStream(int port);
    int stopStream(int port);

    int setupServer(const char* url);
    int clearServer(void);
    void configServer(const char* query);
    void configServer(const char* key, const char* value);

    void setupStreams(void);
    void clearStreams(void);
    void configStream(int port, const char* query);
    void configStream(int port, const char* key, const char* value);

    void setupWorkers(void);
    void clearWorkers(void);

    void setupRTSPServer(void);
    void clearRTSPServer(void);

    static void handleServerConfig(void* opaque, char* seg);

private:
    std::string         mBindAddress;
    uint16_t            mBindPort;
    size_t              mMaxStreams;
    size_t              mMaxStreamTracks;
    size_t              mMaxWorkers;
    Stream*                         mStreams[XPR_RTSP_PORT_MINOR_MAX + 2];
    Worker*                         mWorkers[XPR_RTSP_MAX_WORKERS];
    UserAuthenticationDatabase*     mAuthDB;
    RTSPServer*                     mRTSPServer;
};

class Stream : public Port
{
public:
    Stream(int id, Port* parent);
    virtual ~Stream(void);

    // Override Port interfaces
    virtual int open(int port, const char* url);
    virtual int close(int port);
    virtual int start(int port);
    virtual int stop(int port);
    virtual int pushData(int port, XPR_StreamBlock* stb);

    // Properties
    ServerMediaSession* sms(void) const;
    int asrcId(void) const;
    int vsrcId(void) const;

    bool hasAudioFrame(void) const;
    bool hasVideoFrame(void) const;

    XPR_StreamBlock* getAudioFrame(void) const;
    XPR_StreamBlock* getVideoFrame(void) const;

    int putAudioFrame(XPR_StreamBlock* stb);
    int putVideoFrame(XPR_StreamBlock* stb);

    void releaseAudioFrame(XPR_StreamBlock* stb);
    void releaseVideoFrame(XPR_StreamBlock* stb);

private:
    void configStream(const char* query);
    void configStream(const char* key, const char* value);

    static void handleStreamConfig(void* opaque, char* seg);

private:
    ServerMediaSession* mSMS;
    //FramedSource*     mFramedSources[XPR_RTSP_PORT_TRACK_MAX+1];
    XPR_Fifo*           mAudioQ;
    XPR_Fifo*           mVideoQ;
    // Cached configurations
    std::string         mSourceType;
    std::string         mTrackId;
    int                 mAQL;
    int                 mVQL;
    int                 mAsrcId;
    int                 mVsrcId;
    int                 mAudioProfile;
    int                 mAudioFreqIdx;
    int                 mAudioChannels;
};

class Worker
{
public:
    Worker(int id, Server* server);
    virtual ~Worker(void);

    // Methods
    void run(void);
    int start(void);
    int stop(void);
    int terminate(void);

    // Properties

    int id(void) const;

    Server& server(void);

    TaskScheduler& scheduler(void);

    UsageEnvironment& env(void);

private:

    static void* thread(void* opaque, XPR_Thread* thread);

private:
    int                 mId;
    Server*             mServer;
    TaskScheduler*      mScheduler;
    UsageEnvironment*   mEnv;
    XPR_Thread*         mThread;
    bool                mExitLoop;
};

} // namespace xpr::rtsp

} // namespace xpr

#endif // XPR_RTSP_DRIVER_LIVE_RTSP_SERVER_HPP

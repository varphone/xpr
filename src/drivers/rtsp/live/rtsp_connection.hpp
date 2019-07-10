#ifndef XPR_RTSP_DRIVER_LIVE_RTSP_CONNECTION_HPP
#define XPR_RTSP_DRIVER_LIVE_RTSP_CONNECTION_HPP

#include "rtsp.hpp"
#include <live/liveMedia.hh>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_utils.h>
#include <string>

#define XPR_RTSP_MAX_FRAME_SIZE (1024*512)

namespace xpr
{
namespace rtsp
{

class Connection;
class GenericFrameMerger;
class Worker;

// Define a class to hold per-stream state that we maintain throughout each
// stream's lifetime:
class StreamClientState {
public:
    StreamClientState();
    virtual ~StreamClientState();

public:
    MediaSubsessionIterator* iter;
    MediaSession* session;
    MediaSubsession* subsession;
    TaskToken streamTimerTask;
    double duration;
    int tracks;
};

class MyRTSPClient : public RTSPClient {
public:
    Authenticator* getAuthenticator(void);
    void setAuthenticator(const char* usename, const char* password);

    bool isStreamUsingTCP(void) const;
    void setStreamUsingTCP(bool streamUsingTCP);
    void setConnectTiemout(int conTimeout);
    void setReceiveTimeout(int rxTimeout);
    void setUseFrameMerger(bool useFrameMerger);
    void setOutputFormats(const std::string& outputFormats);

    Connection* getParent(void);

    void handleError(int err);

    bool isPlaying(void) const;
    void setPlaying(bool yes = true);

    bool isTimeouted(void) const;
    void setTimeouted(bool yes = true);

    void setUnreached(bool yes);

    void updateLATS(void);

    // Static functions
public:
    static MyRTSPClient* createNew(Connection* parent);

    // RTSP 'response handlers':
    static void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode,
                                      char* resultString);
    static void continueAfterSETUP(RTSPClient* rtspClient, int resultCode,
                                   char* resultString);
    static void continueAfterPLAY(RTSPClient* rtspClient, int resultCode,
                                  char* resultString);
    static void continueAfterGET_PARAMETER(RTSPClient* rtspClient, int resultCode,
                                char* resultString);

    // Other event handler functions:
    // called when a stream's subsession (e.g., audio or video substream) ends
    static void subsessionAfterPlaying(void* clientData);

    // called when a RTCP "BYE" is received for a subsession
    static void subsessionByeHandler(void* clientData);

    // called at the end of a stream's expected duration (if the stream has not
    // already signaled its end using a RTCP "BYE")
    static void streamTimerHandler(void* clientData);

    // The main streaming routine (for each "rtsp://" URL):
    static void openURL(UsageEnvironment& env, char const* progName,
                        char const* rtspURL);

    // Used to iterate through each stream's 'subsessions', setting up each one:
    static void setupNextSubsession(RTSPClient* rtspClient);

    // Used to shut down and close a stream (including its "RTSPClient" object):
    static void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

    // Used to keep connection alive
    static void keepAlive(RTSPClient* rtspClient);

    // Used to cancel keep alive task
    static void keepAliveCancel(RTSPClient* rtspClient);

    // Used to mark the stream error status
    static void streamError(RTSPClient* rtspClient, int err);

    // Use to check timeout
    static void timeoutCheck(RTSPClient* rtspClient);

    // Use to cancel timeout check task
    static void timeoutCheckCancel(RTSPClient* rtspClient);

protected:
    explicit MyRTSPClient(UsageEnvironment& env, const char* url,
                          Connection* parent);
    virtual ~MyRTSPClient();

public:
    Connection* mParent;
    Authenticator* mAuth;
    StreamClientState mScs;
    bool mStreamUsingTCP;
    int mConTimeout;
    int mRxTimeout;
    bool mUseFrameMerger;
    std::string mOutputFormats;
    bool mIsPlaying;
    int64_t mLastActiveTS;
    TaskToken mKeepAliveTask;
    TaskToken mTimeoutCheckTask;
};

class DummySink : public MediaSink {
public:
    static DummySink* createNew(UsageEnvironment& env, MyRTSPClient* client,
                                MediaSubsession* subsession, int trackId);

    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);
    static void afterMergedFrame(void* clientData, XPR_StreamBlock const& stb);

private:
    DummySink(UsageEnvironment& env, MyRTSPClient* client,
              MediaSubsession* subsession, int trackId);

    virtual ~DummySink();

    void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                           struct timeval presentationTime,
                           unsigned durationInMicroseconds);
    void afterMergedFrame(XPR_StreamBlock const& stb);

    // MediaSink interfaces
private:
    virtual Boolean continuePlaying();

private:
    MyRTSPClient* mClient;
    MediaSubsession* mSubsession;
    GenericFrameMerger* mFrameMerger;
    bool mOutputAdts;
    uint8_t* mBuffer;
    uint32_t mMaxFrameSize;
    uint32_t mFourcc;
    void* mMeta;
    int64_t mPTS;
    int mTrackId;
    XPR_StreamBlock mStreamBlock;
};

class Connection : public Port {
public:
    Connection(int id, Port* parent);
    virtual ~Connection(void);

    // Override Port interfaces
    virtual int open(int port, const char* url);
    virtual int close(int port);
    virtual int start(int port);
    virtual int stop(int port);
    virtual int pushData(int port, XPR_StreamBlock* stb);
    virtual int postEvent(int port, const XPR_RTSP_EVD* evd);
    virtual int runTask(int port, TaskId task);
    virtual int stateChanged(int port, StateTransition transition);

    // Methods
    int restart(int port);

    // Properties
    Worker* worker(void);

private:
    void configConnection(const char* query);
    void configConnection(const char* key, const char* value);

    int startInTask(int port);
    int stopInTask(int port);
    int restartInTask(int port);

    static void afterRestart(void* clientData);
    static void handleConnectionConfig(void* opaque, char* seg);

private:
    MyRTSPClient* mClient;
    Worker* mWorker;
    TaskToken mRestartTask;
    // Cached configurations
    bool mRtpOverTcp;
    int mConTimeout; // Connect timeout in us
    int mRxTimeout; // Receive timeout in us
    bool mUseFrameMerger; // Use FrameMerger to merge same pts frames
    bool mAutoRestart; // Auto restart when src stopped or timeouted
    int mRestartDelay; // Auto restart delay in us
    std::string mOutputFormats; // Set output formats: adts or ...
};

} // namespace xpr::rtsp

} // namespace xpr

#endif // XPR_RTSP_DRIVER_LIVE_RTSP_CONNECTION_HPP
#ifndef XPR_RTSP_DRIVER_LIVE_RTSP_CLIENT_HPP
#define XPR_RTSP_DRIVER_LIVE_RTSP_CLIENT_HPP

#include "rtsp_connectionmanager.hpp"

#include <live/liveMedia.hh>

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:
class StreamClientState
{
public:
    StreamClientState();
    virtual ~StreamClientState();

public:
    MediaSubsessionIterator* iter;
    MediaSession* session;
    MediaSubsession* subsession;
    TaskToken streamTimerTask;
    double duration;
    int lastTrackId;
};

// If you're streaming just a single stream (i.e., just from a single URL, once), then you can define and use just a single
// "StreamClientState" structure, as a global variable in your application.  However, because - in this demo application - we're
// showing how to play multiple streams, concurrently, we can't do that.  Instead, we have to have a separate "StreamClientState"
// structure for each "RTSPClient".  To do this, we subclass "RTSPClient", and add a "StreamClientState" field to the subclass:

class ourRTSPClient : public RTSPClient
{
public:
    static ourRTSPClient* createNew(UsageEnvironment& env, int port);

    void setAuth(const char* usename, const char* password);

protected:
    ourRTSPClient(UsageEnvironment& env, int port, const char* url);
    // called only by createNew();
    virtual ~ourRTSPClient();

public:
    int port;
    Authenticator* auth;
    StreamClientState scs;
};

// Define a data sink (a subclass of "MediaSink") to receive the data for each subsession (i.e., each audio or video 'substream').
// In practice, this might be a class (or a chain of classes) that decodes and then renders the incoming audio or video.
// Or it might be a "FileSink", for outputting the received data into a file (as is done by the "openRTSP" application).
// In this example code, however, we define a simple 'dummy' sink that receives incoming data, but does nothing with it.

class DummySink : public MediaSink
{
public:
    static DummySink* createNew(UsageEnvironment& env, ourRTSPClient* client,
                                MediaSubsession& subsession, int port, int trackId = 0);

private:
    DummySink(UsageEnvironment& env, ourRTSPClient* client,
              MediaSubsession& subsession, int port, int trackId);

    // called only by "createNew()"
    virtual ~DummySink();

    static void afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime,
                                  unsigned durationInMicroseconds);

    void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
                           struct timeval presentationTime,
                           unsigned durationInMicroseconds);

private:
    // redefined virtual functions:
    virtual Boolean continuePlaying();

private:
    ourRTSPClient* fClient;
    u_int8_t* fReceiveBuffer;
    u_int8_t* fMergeBuffer;
    unsigned fMergedSize;
    int fFrameType;
    int64_t fPTS;
    MediaSubsession& fSubsession;
    int fPort;
    int fCodec;
    int fTrackId;
};

#define RTSP_CLIENT_VERBOSITY_LEVEL 0 // by default, print verbose output from each "RTSPClient"

namespace xpr
{
namespace rtsp
{

} // namespace xpr::rtsp

} // namespace xpr

#endif // XPR_RTSP_DRIVER_LIVE_RTSP_CLIENT_HPP

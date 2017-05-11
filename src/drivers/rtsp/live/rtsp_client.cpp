#if defined(XPR_RTSP_DRIVER_LIVE) && defined(XPR_RTSP_HAVE_CLIENT)

#if defined(_MSC_VER)
#pragma comment(lib, "BasicUsageEnvironment-mt-s-vc140.lib")
#pragma comment(lib, "groupsock-mt-s-vc140.lib")
#pragma comment(lib, "liveMedia-mt-s-vc140.lib")
#pragma comment(lib, "UsageEnvironment-mt-s-vc140.lib")
#pragma comment(lib, "ws2_32.lib")
#endif // defined(_MSC_VER)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_streamblock.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_pes.h>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_sys.h>
#include <xpr/xpr_url.h>

#include <live/BasicUsageEnvironment.hh>

#if defined(XPR_RTSP_HAVE_HKMI)
#include <PlayM4.h>
#endif // defined(XPR_RTSP_HAVE_HKMI)

#if defined(XPR_RTSP_HAVE_XD_STREAM_API)
#include <XD_Common.h>
#include <XD_Stream.h>
#endif // defined(XPR_RTSP_HAVE_XD_STREAM_API)

#include "rtsp.hpp"
#if defined(XPR_RTSP_HAVE_CLIENT)
#include "rtsp_client.hpp"
#endif // defined(XPR_RTSP_HAVE_CLIENT)
#if defined(XPR_RTSP_HAVE_SERVER)
#include "rtsp_client.hpp"
#endif // defined(XPR_RTSP_HAVE_SERVER)

static XPR_RTSP* xpr_rtsp = 0;

PortContext* XPR_RTSP_GetPortContext(int port)
{
    int major = XPR_RTSP_PORT_MAJOR(port);
    int minor = XPR_RTSP_PORT_MINOR(port);
    if (major == XPR_RTSP_PORT_MAJOR_CLI) {
        if (minor > 0 && minor <= XPR_RTSP_MAX_CLIENTS)
            return &xpr_rtsp->client_ports[minor];
    }
    else if (major == XPR_RTSP_PORT_MAJOR_SVR) {
        if (minor > 0 && minor <= XPR_RTSP_MAX_CLIENTS)
            return &xpr_rtsp->stream_ports[minor];
    }
    return 0;
}

int XPR_RTSP_GetAvailClientPortIndex(void)
{
    for (int i = 1; i <= XPR_RTSP_MAX_CLIENTS; i++) {
        if (xpr_rtsp->client_ports[i].usr_flags == 0 &&
            xpr_rtsp->client_ports[i].act_flags == 0)
            return i;
    }
    return -1;
}

int XPR_RTSP_GetAvailStreamPortIndex(void)
{
    for (int i = 1; i <= XPR_RTSP_MAX_STREAMS; i++) {
        if (xpr_rtsp->stream_ports[i].usr_flags == 0 &&
            xpr_rtsp->stream_ports[i].act_flags == 0)
            return i;
    }
    return -1;
}

// Update port last active timestamp
void XPR_RTSP_UpdatePortLATS(int port)
{
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc)
        pc->lats = XPR_SYS_GetCTS();
}

// Update port last liveness timestamp
void XPR_RTSP_UpdatePortLVTS(int port)
{
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        pc->lvts = XPR_SYS_GetCTS();
        pc->act_flags &= ~XPR_RTSP_FLAG_KPL;
    }
}

#ifdef XPR_RTSP_HAVE_HKMI
static void FillPortHKMI(int port, const char* mime)
{
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (!pc || !mime)
        return;
    if (strncmp(mime, "AAC", 3) == 0) {
        pc->out_pes_hkmi.audio_format = AUDIO_MPEG;
        pc->out_pes_hkmi.audio_bitrate = 64000;
        pc->out_pes_hkmi.audio_channels = 1;
        pc->out_pes_hkmi.audio_bits_per_sample = 16;
        pc->out_pes_hkmi.audio_samplesrate = 22500;
    }
    else if (strncmp(mime, "PCMA", 4) == 0) {
        pc->out_pes_hkmi.audio_format = AUDIO_G711_A;
        pc->out_pes_hkmi.audio_bitrate = 64000;
        pc->out_pes_hkmi.audio_channels = 1;
        pc->out_pes_hkmi.audio_bits_per_sample = 16;
        pc->out_pes_hkmi.audio_samplesrate = 8000;
    }
    else if (strncmp(mime, "PCMU", 4) == 0) {
        pc->out_pes_hkmi.audio_format = AUDIO_G711_U;
        pc->out_pes_hkmi.audio_bitrate = 64000;
        pc->out_pes_hkmi.audio_channels = 1;
        pc->out_pes_hkmi.audio_bits_per_sample = 16;
        pc->out_pes_hkmi.audio_samplesrate = 8000;
    }
    else if (strncmp(mime, "H264", 4) == 0) {
        pc->out_pes_hkmi.video_format = VIDEO_H264;
    }
    else if (strncmp(mime, "JPEG", 4) == 0) {
        pc->out_pes_hkmi.video_format = VIDEO_MJPEG;
    }
}
#endif

// Forward function definitions:

// RTSP 'response handlers':
void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode,
                           char* resultString);
void continueAfterSETUP(RTSPClient* rtspClient, int resultCode,
                        char* resultString);
void continueAfterPLAY(RTSPClient* rtspClient, int resultCode,
                       char* resultString);

// Other event handler functions:
void subsessionAfterPlaying(void*
                            clientData); // called when a stream's subsession (e.g., audio or video substream) ends
void subsessionByeHandler(void*
                          clientData); // called when a RTCP "BYE" is received for a subsession
void streamTimerHandler(void* clientData);
// called at the end of a stream's expected duration (if the stream has not already signaled its end using a RTCP "BYE")

// The main streaming routine (for each "rtsp://" URL):
void openURL(UsageEnvironment& env, char const* progName, char const* rtspURL);

// Used to iterate through each stream's 'subsessions', setting up each one:
void setupNextSubsession(RTSPClient* rtspClient);

// Used to shut down and close a stream (including its "RTSPClient" object):
void shutdownStream(RTSPClient* rtspClient, int exitCode = 1);

// A function that outputs a string that identifies each stream (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env,
                             const RTSPClient& rtspClient)
{
    return env << "[URL:\"" << rtspClient.url() << "\"]: ";
}

// A function that outputs a string that identifies each subsession (for debugging output).  Modify this if you wish:
UsageEnvironment& operator<<(UsageEnvironment& env,
                             const MediaSubsession& subsession)
{
    return env << subsession.mediumName() << "/" << subsession.codecName();
}

void usage(UsageEnvironment& env, char const* progName)
{
    env << "Usage: " << progName << " <rtsp-url-1> ... <rtsp-url-N>\n";
    env << "\t(where each <rtsp-url-i> is a \"rtsp://\" URL)\n";
}

static unsigned rtspClientCount =
    0; // Counts how many streams (i.e., "RTSPClient"s) are currently in use.

ourRTSPClient* openURL(UsageEnvironment& env, int port)
{
    // Begin by creating a "RTSPClient" object.  Note that there is a separate "RTSPClient" object for each stream that we wish
    // to receive (even if more than stream uses the same "rtsp://" URL).
    ourRTSPClient* rtspClient = ourRTSPClient::createNew(env, port);
    if (rtspClient == NULL) {
        //env << "Failed to create a RTSP client for port \"" << port << "\": " << env.getResultMsg() << "\n";
        return NULL;
    }
    // Next, send a RTSP "DESCRIBE" command, to get a SDP description for the stream.
    // Note that this command - like all RTSP commands - is sent asynchronously; we do not block, waiting for a response.
    // Instead, the following function call returns immediately, and we handle the RTSP response later, from within the event loop:
    rtspClient->sendDescribeCommand(continueAfterDESCRIBE, rtspClient->auth);
    return rtspClient;
}


// Implementation of the RTSP 'response handlers':

void continueAfterDESCRIBE(RTSPClient* rtspClient, int resultCode,
                           char* resultString)
{
    do {
        UsageEnvironment& env = rtspClient->envir(); // alias
        StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
        if (resultCode != 0) {
            //env << *rtspClient << "Failed to get a SDP description: " << resultString << "\n";
            delete[] resultString;
            break;
        }
        char* const sdpDescription = resultString;
        //env << *rtspClient << "Got a SDP description:\n" << sdpDescription << "\n";
        // Create a media session object from this SDP description:
        scs.session = MediaSession::createNew(env, sdpDescription);
        delete[] sdpDescription; // because we don't need it anymore
        if (scs.session == NULL) {
            //env << *rtspClient << "Failed to create a MediaSession object from the SDP description: " << env.getResultMsg() << "\n";
            break;
        }
        else if (!scs.session->hasSubsessions()) {
            //env << *rtspClient << "This session has no media subsessions (i.e., no \"m=\" lines)\n";
            break;
        }
        // Then, create and set up our data source objects for the session.  We do this by iterating over the session's 'subsessions',
        // calling "MediaSubsession::initiate()", and then sending a RTSP "SETUP" command, on each one.
        // (Each 'subsession' will have its own data source.)
        scs.iter = new MediaSubsessionIterator(*scs.session);
        setupNextSubsession(rtspClient);
        XPR_RTSP_UpdatePortLATS(((ourRTSPClient*)rtspClient)->port);
        return;
    }
    while (0);
    // An unrecoverable error occurred with this stream.
    shutdownStream(rtspClient);
}

// By default, we request that the server stream its data using RTP/UDP.
// If, instead, you want to request that the server stream via RTP-over-TCP, change the following to True:
#define REQUEST_STREAMING_OVER_TCP True

void setupNextSubsession(RTSPClient* rtspClient)
{
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
    Boolean streamUsingTCP = REQUEST_STREAMING_OVER_TCP;
    Boolean forceMulticastOnUnspecified = False;
    PortContext* pc = XPR_RTSP_GetPortContext(((ourRTSPClient*)rtspClient)->port);
    if (pc) {
        if (pc->trspec == XPR_RTSP_TRSPEC_RTP_AVP_UDP)
            streamUsingTCP = false;
        if (pc->trspec == XPR_RTSP_TRSPEC_RTP_AVP_MCAST)
            forceMulticastOnUnspecified = True;
    }
    scs.subsession = scs.iter->next();
    if (scs.subsession != NULL) {
        if (!scs.subsession->initiate()) {
            //env << *rtspClient << "Failed to initiate the \"" << *scs.subsession << "\" subsession: " << env.getResultMsg() << "\n";
            setupNextSubsession(
                rtspClient); // give up on this subsession; go to the next one
        }
        else {
            //env << *rtspClient << "Initiated the \"" << *scs.subsession << "\" subsession (";
            if (scs.subsession->rtcpIsMuxed()) {
                //env << "client port " << scs.subsession->clientPortNum();
            }
            else {
                //env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
            }
            //env << ")\n";
            // Continue setting up this subsession, by sending a RTSP "SETUP" command:
            rtspClient->sendSetupCommand(*scs.subsession, continueAfterSETUP, False,
                                         streamUsingTCP);
        }
        scs.lastTrackId++;
        XPR_RTSP_UpdatePortLATS(((ourRTSPClient*)rtspClient)->port);
        return;
    }
    // We've finished setting up all of the subsessions.  Now, send a RTSP "PLAY" command to start the streaming:
    if (scs.session->absStartTime() != NULL) {
        // Special case: The stream is indexed by 'absolute' time, so send an appropriate "PLAY" command:
        rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY,
                                    scs.session->absStartTime(), scs.session->absEndTime());
    }
    else {
        scs.duration = scs.session->playEndTime() - scs.session->playStartTime();
        rtspClient->sendPlayCommand(*scs.session, continueAfterPLAY);
    }
    XPR_RTSP_UpdatePortLATS(((ourRTSPClient*)rtspClient)->port);
}

void continueAfterSETUP(RTSPClient* rtspClient, int resultCode,
                        char* resultString)
{
    do {
        UsageEnvironment& env = rtspClient->envir(); // alias
        StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
        if (resultCode != 0) {
            //env << *rtspClient << "Failed to set up the \"" << *scs.subsession << "\" subsession: " << resultString << "\n";
            break;
        }
        //env << *rtspClient << "Set up the \"" << *scs.subsession << "\" subsession (";
        if (scs.subsession->rtcpIsMuxed()) {
            //env << "client port " << scs.subsession->clientPortNum();
        }
        else {
            //env << "client ports " << scs.subsession->clientPortNum() << "-" << scs.subsession->clientPortNum()+1;
        }
        //env << ")\n";
        // Having successfully setup the subsession, create a data sink for it, and call "startPlaying()" on it.
        // (This will prepare the data sink to receive data; the actual flow of data from the client won't start happening until later,
        // after we've sent a RTSP "PLAY" command.)
        scs.subsession->sink = DummySink::createNew(env, (ourRTSPClient*)rtspClient,
                                                    *scs.subsession, ((ourRTSPClient*)rtspClient)->port, scs.lastTrackId);
        // perhaps use your own custom "MediaSink" subclass instead
        if (scs.subsession->sink == NULL) {
            //env << *rtspClient << "Failed to create a data sink for the \"" << *scs.subsession
            //<< "\" subsession: " << env.getResultMsg() << "\n";
            break;
        }
        //env << *rtspClient << "Created a data sink for the \"" << *scs.subsession << "\" subsession\n";
        scs.subsession->miscPtr =
            rtspClient; // a hack to let subsession handler functions get the "RTSPClient" from the subsession
        scs.subsession->sink->startPlaying(*(scs.subsession->readSource()),
                                           subsessionAfterPlaying, scs.subsession);
        // Also set a handler to be called if a RTCP "BYE" arrives for this subsession:
        if (scs.subsession->rtcpInstance() != NULL) {
            scs.subsession->rtcpInstance()->setByeHandler(subsessionByeHandler,
                                                          scs.subsession);
        }
    }
    while (0);
    delete[] resultString;
    // Set up the next subsession, if any:
    setupNextSubsession(rtspClient);
    XPR_RTSP_UpdatePortLATS(((ourRTSPClient*)rtspClient)->port);
}

void continueAfterPLAY(RTSPClient* rtspClient, int resultCode,
                       char* resultString)
{
    Boolean success = False;
    do {
        UsageEnvironment& env = rtspClient->envir(); // alias
        StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
        if (resultCode != 0) {
            //env << *rtspClient << "Failed to start playing session: " << resultString << "\n";
            break;
        }
        // Set a timer to be handled at the end of the stream's expected duration (if the stream does not already signal its end
        // using a RTCP "BYE").  This is optional.  If, instead, you want to keep the stream active - e.g., so you can later
        // 'seek' back within it and do another RTSP "PLAY" - then you can omit this code.
        // (Alternatively, if you don't want to receive the entire stream, you could set this timer for some shorter value.)
        if (scs.duration > 0) {
            unsigned const delaySlop =
                2; // number of seconds extra to delay, after the stream's expected duration.  (This is optional.)
            scs.duration += delaySlop;
            unsigned uSecsToDelay = (unsigned)(scs.duration * 1000000);
            scs.streamTimerTask = env.taskScheduler().scheduleDelayedTask(uSecsToDelay,
                                  (TaskFunc*)streamTimerHandler, rtspClient);
        }
        //env << *rtspClient << "Started playing session";
        if (scs.duration > 0) {
            //env << " (for up to " << scs.duration << " seconds)";
        }
        //env << "...\n";
        success = True;
    }
    while (0);
    delete[] resultString;
    if (!success) {
        // An unrecoverable error occurred with this stream.
        shutdownStream(rtspClient);
    }
    XPR_RTSP_UpdatePortLATS(((ourRTSPClient*)rtspClient)->port);
}


// Implementation of the other event handlers:

void subsessionAfterPlaying(void* clientData)
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
        if (subsession->sink != NULL) return; // this subsession is still active
    }
    // All subsessions' streams have now been closed, so shutdown the client:
    shutdownStream(rtspClient);
}

void subsessionByeHandler(void* clientData)
{
    MediaSubsession* subsession = (MediaSubsession*)clientData;
    RTSPClient* rtspClient = (RTSPClient*)subsession->miscPtr;
    UsageEnvironment& env = rtspClient->envir(); // alias
    //env << *rtspClient << "Received RTCP \"BYE\" on \"" << *subsession << "\" subsession\n";
    // Now act as if the subsession had closed:
    subsessionAfterPlaying(subsession);
}

void streamTimerHandler(void* clientData)
{
    ourRTSPClient* rtspClient = (ourRTSPClient*)clientData;
    StreamClientState& scs = rtspClient->scs; // alias
    printf("streamTimerHandler(%p)\n", clientData);
    scs.streamTimerTask = NULL;
    // Shut down the stream:
    shutdownStream(rtspClient);
}

void shutdownStream(RTSPClient* rtspClient, int exitCode)
{
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
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
                    subsession->rtcpInstance()->setByeHandler(NULL,
                                                              NULL); // in case the server sends a RTCP "BYE" while handling "TEARDOWN"
                }
                someSubsessionsWereActive = True;
            }
        }
        if (someSubsessionsWereActive) {
            // Send a RTSP "TEARDOWN" command, to tell the server to shutdown the stream.
            // Don't bother handling the response to the "TEARDOWN".
            rtspClient->sendTeardownCommand(*scs.session, NULL);
        }
    }
    //env << *rtspClient << "Closing the stream.\n";
    Medium::close(rtspClient);
    // Note that this will also cause this stream's "StreamClientState" structure to get reclaimed.
}

void continueAfterGET_PARAMETER(RTSPClient* rtspClient, int resultCode,
                                char* resultString)
{
    if (rtspClient) {
        printf("GET_PARAMETER %d, %s\n", resultCode, resultString);
        XPR_RTSP_UpdatePortLVTS(((ourRTSPClient*)rtspClient)->port);
    }
}

void keepAlive(RTSPClient* rtspClient)
{
    if (!rtspClient)
        return;
    UsageEnvironment& env = rtspClient->envir(); // alias
    StreamClientState& scs = ((ourRTSPClient*)rtspClient)->scs; // alias
    if (!scs.session)
        return;
    rtspClient->sendGetParameterCommand(*scs.session,
                                        continueAfterGET_PARAMETER,
                                        NULL/*"KEEP-ALIVE"*/,
                                        NULL/*((ourRTSPClient*)rtspClient)->auth*/);
}

// Implementation of "ourRTSPClient":

ourRTSPClient* ourRTSPClient::createNew(UsageEnvironment& env, int port)
{
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    XPR_Url* xu = XPR_UrlParse(pc->url, -1);
    char url[1024] = { 0 };
    if (xu) {
        if (XPR_UrlGetPort(xu) != 0)
            sprintf(url, "rtsp://%s:%d/%s", XPR_UrlGetHost(xu), XPR_UrlGetPort(xu),
                    XPR_UrlGetPath(xu));
        else
            sprintf(url, "rtsp://%s/%s", XPR_UrlGetHost(xu), XPR_UrlGetPath(xu));
        if (XPR_UrlHaveQuery(xu)) {
            strcat(url, "?");
            strcat(url, XPR_UrlGetQuery(xu));
        }
        ourRTSPClient* c = new ourRTSPClient(env, port, url);
        if (XPR_UrlHaveUsername(xu)) {
            c->setAuth(XPR_UrlGetUsername(xu), XPR_UrlGetPassword(xu));
        }
        else if (pc->username[0]) {
            c->setAuth(pc->username, pc->password);
        }
        XPR_UrlDestroy(xu);
        return c;
    }
    return NULL;
}

ourRTSPClient::ourRTSPClient(UsageEnvironment& env, int port, const char* url)
    : RTSPClient(env, url, 0, NULL, 0, -1)
    , port(port)
    , auth(NULL)
{
    XPR_RTSP_UpdatePortLATS(port);
}

ourRTSPClient::~ourRTSPClient()
{
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        if (!(pc->act_flags & XPR_RTSP_FLAG_EVTMOPD)) {
            XPR_RTSP_EVD evd;
            evd.event = XPR_RTSP_EVT_TIMEOUT;
            evd.data = NULL;
            evd.data_size = 0;
            pc->act_flags |= XPR_RTSP_FLAG_EVTMOPD;
            XPR_RTSP_PostEvent(port, &evd);
        }
        pc->act_flags |= XPR_RTSP_FLAG_CLOSE;
    }
    if (auth)
        delete auth;
}

void ourRTSPClient::setAuth(const char* username, const char* password)
{
    if (auth)
        delete auth;
    auth = new Authenticator(username, password, false);
}

// Implementation of "StreamClientState":

StreamClientState::StreamClientState()
    : iter(NULL), session(NULL), subsession(NULL), streamTimerTask(NULL),
      duration(0.0)
{
}

StreamClientState::~StreamClientState()
{
    delete iter;
    if (session != NULL) {
        // We also need to delete "session", and unschedule "streamTimerTask" (if set)
        UsageEnvironment& env = session->envir(); // alias
        env.taskScheduler().unscheduleDelayedTask(streamTimerTask);
        Medium::close(session);
    }
}


// Implementation of "DummySink":

// Even though we're not going to be doing anything with the incoming data, we still need to receive it.
// Define the size of the buffer that we'll use:
#define DUMMY_SINK_RECEIVE_BUFFER_SIZE 600000

DummySink* DummySink::createNew(UsageEnvironment& env, ourRTSPClient* client,
                                MediaSubsession& subsession, int port, int trackId)
{
    return new DummySink(env, client, subsession, port, trackId);
}

DummySink::DummySink(UsageEnvironment& env, ourRTSPClient* client,
                     MediaSubsession& subsession, int port, int trackId)
    : MediaSink(env)
    , fClient(client)
    , fPTS(0)
    , fSubsession(subsession)
    , fPort(port)
    , fCodec(0)
    , fTrackId(trackId)
{
    fReceiveBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
    fMergeBuffer = new u_int8_t[DUMMY_SINK_RECEIVE_BUFFER_SIZE];
    fMergedSize = 0;
    const char* cn = subsession.codecName();
    if (strcmp(cn, "H264") == 0)
        fCodec = AV_FOURCC_H264;
    else if (strcmp(cn, "JPEG") == 0)
        fCodec = AV_FOURCC_JPEG;
    else if (strcmp(cn, "PCMA") == 0)
        fCodec = AV_FOURCC_PCMA;
    else if (strcmp(cn, "PCMU") == 0)
        fCodec = AV_FOURCC_PCMU;
#ifdef XPR_RTSP_HAVE_HKMI
    FillPortHKMI(fPort, cn);
#endif
}

DummySink::~DummySink()
{
    delete[] fReceiveBuffer;
    delete[] fMergeBuffer;
}

void DummySink::afterGettingFrame(void* clientData, unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime, unsigned durationInMicroseconds)
{
    DummySink* sink = (DummySink*)clientData;
    sink->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime,
                            durationInMicroseconds);
}

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
    //
    //printf("%02hhx %02hhx %02hhx %02hhx\n", data[0], data[1], data[2], data[3]);
    b |= size > 1 ? (uint32_t)data[0] << 24 : 0;
    b |= size > 2 ? (uint32_t)data[1] << 16 : 0;
    b |= size > 3 ? (uint32_t)data[2] << 0 : 0;
    b |= size > 4 ? (uint32_t)data[3] : 0;
    //
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
    //printf("first_mb_in_slice_zn = %d\n", first_mb_in_slice_zn);
    //
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
        //printf("slice_type_zn: %d, %02hhx %02hhx\n", slice_type_zn, data[0], data[1]);
        b &= (0xffffffff >> (first_mb_in_slice_zn * 2 + 1));
        slice_type = (b >> (n - first_mb_in_slice_zn * 2 - 1 - slice_type_zn * 2 - 1)) -
                     1;
    }
    //
    slice_type &= 0xf;
    //printf("slice_type: %d\n", slice_type);
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

// If you don't want to see debugging output for each received frame, then comment out the following line:
//#define DEBUG_PRINT_EACH_RECEIVED_FRAME 1

void DummySink::afterGettingFrame(unsigned frameSize,
                                  unsigned numTruncatedBytes,
                                  struct timeval presentationTime, unsigned /*durationInMicroseconds*/)
{
    // We've just received a frame of data.  (Optionally) print out information about it:
#ifdef DEBUG_PRINT_EACH_RECEIVED_FRAME
    if (fStreamId != NULL) envir() << "Stream \"" << fStreamId << "\"; ";
    envir() << fSubsession.mediumName() << "/" << fSubsession.codecName() <<
            ":\tReceived " << frameSize << " bytes";
    if (numTruncatedBytes > 0) envir() << " (with " << numTruncatedBytes <<
                                           " bytes truncated)";
    char uSecsStr[6 +
                  1]; // used to output the 'microseconds' part of the presentation time
    sprintf(uSecsStr, "%06u", (unsigned)presentationTime.tv_usec);
    envir() << ".\tPresentation time: " << (int)presentationTime.tv_sec << "." <<
            uSecsStr;
    if (fSubsession.rtpSource() != NULL &&
        !fSubsession.rtpSource()->hasBeenSynchronizedUsingRTCP()) {
        envir() <<
                "!"; // mark the debugging output to indicate that this presentation time is not RTCP-synchronized
    }
#ifdef DEBUG_PRINT_NPT
    envir() << "\tNPT: " << fSubsession.getNormalPlayTime(presentationTime);
#endif
    envir() << "\n";
#endif
    PortContext* pc = XPR_RTSP_GetPortContext(fPort);
    uint8_t* ptr = NULL;
    int64_t pts = presentationTime.tv_sec;
    pts *= 1000000;
    pts += presentationTime.tv_usec;
    //printf("ms %d, fPTS %lld, pts %lld\n", fMergedSize, fPTS, pts);
    // If pts difference, post the frame out
    if (fMergedSize > 0 && fPTS != pts) {
        XPR_StreamBlock stb;
        memset(&stb, 0, sizeof(stb));
        stb.buffer = stb.data = fMergeBuffer;
        stb.bufferSize = stb.dataSize = fMergedSize;
        stb.flags = fFrameType;
        if (fCodec == AV_FOURCC_H264)
            stb.flags |= XPR_STREAMBLOCK_FLAG_VIDEO_FRAME;
        stb.pts = stb.dts = pts;
        stb.codec = fCodec;
        stb.track = fTrackId;
        XPR_RTSP_PostData(fPort, &stb);
        fFrameType = 0;
        fMergedSize = 0;
    }
    // Post H264 SPS_NALU or PPS_NALU
    if (fCodec == AV_FOURCC_H264) {
        int naluType = fReceiveBuffer[0] & 0x1f;
        if (naluType == 7 ||
            naluType == 8) {
            XPR_StreamBlock stb;
            memset(&stb, 0, sizeof(stb));
            stb.buffer = stb.data = fReceiveBuffer;
            stb.bufferSize = stb.dataSize = frameSize;
            stb.flags = XPR_STREAMBLOCK_FLAG_HEADER;
            stb.pts = stb.dts = pts;
            stb.codec = fCodec;
            stb.track = fTrackId;
            XPR_RTSP_PostData(fPort, &stb);
        }
    }
    // Copy to merge buffer to combine frame fragments
    if (fMergedSize + frameSize + 4 < DUMMY_SINK_RECEIVE_BUFFER_SIZE) {
        int naluType = 0;
        if (fCodec == AV_FOURCC_H264) {
            // Check startcode, if not exists, append to merge buffer;
            if (!(fReceiveBuffer[0] == 0 &&
                  fReceiveBuffer[1] == 0 &&
                  fReceiveBuffer[2] == 0 &&
                  fReceiveBuffer[3] == 1)) {
                if (pc->h264_flags & H264_FLAG_START_CODE) {
                    fMergeBuffer[fMergedSize++] = 0;
                    fMergeBuffer[fMergedSize++] = 0;
                    fMergeBuffer[fMergedSize++] = 0;
                    fMergeBuffer[fMergedSize++] = 1;
                }
                // Detect frame type (P/I) from NALU
                naluType = fReceiveBuffer[0] & 0x1f;
                if (fFrameType == 0 && naluType < 6) {
                    fFrameType = H264_SliceTypeToFrameType(H264_DetectSliceType(fReceiveBuffer,
                                                                                frameSize));
                }
                ptr = fReceiveBuffer;
            }
            else {
                // Detect frame type (P/I) from NALU
                naluType = fReceiveBuffer[4] & 0x1f;
                if (fFrameType == 0 && naluType < 6) {
                    fFrameType = H264_SliceTypeToFrameType(H264_DetectSliceType(fReceiveBuffer + 4,
                                                                                frameSize - 4));
                }
                if (!(pc->h264_flags & H264_FLAG_START_CODE)) {
                    ptr = fReceiveBuffer + 4;
                    frameSize -= 4;
                }
            }
            // If pts difference, post the frame out
            if (!(pc->h264_flags & H264_FLAG_SINGLE_FRAME)) {
                memcpy(fMergeBuffer + fMergedSize, ptr, frameSize);
                fMergedSize += frameSize;
                XPR_StreamBlock stb;
                memset(&stb, 0, sizeof(stb));
                stb.buffer = stb.data = fMergeBuffer;
                stb.bufferSize = stb.dataSize = fMergedSize;
                stb.flags = fFrameType;
                stb.flags |= XPR_STREAMBLOCK_FLAG_VIDEO_FRAME;
                stb.pts = stb.dts = pts;
                stb.codec = fCodec;
                stb.track = fTrackId;
                XPR_RTSP_PostData(fPort, &stb);
                ptr = 0;
                frameSize = 0;
                fFrameType = 0;
                fMergedSize = 0;
            }
        }
        else {
            ptr = fReceiveBuffer;
        }
        //
        if (ptr && frameSize) {
            memcpy(fMergeBuffer + fMergedSize, ptr, frameSize);
            fMergedSize += frameSize;
        }
    }
    else {
        fFrameType = 0;
        fMergedSize = 0;
    }
    fPTS = pts;
    // Update port last active timestamp for timeout checker;
    XPR_RTSP_UpdatePortLATS(fPort);
    // Then continue, to request the next frame of data:
    continuePlaying();
}

Boolean DummySink::continuePlaying()
{
    if (fSource == NULL) return False; // sanity check (should not happen)
    PortContext* pc = XPR_RTSP_GetPortContext(fPort);
    if (!(pc->act_flags & XPR_RTSP_FLAG_START)) {
        return False;
    }
    // Request the next frame of data from our input source.  "afterGettingFrame()" will get called later, when it arrives:
    fSource->getNextFrame(fReceiveBuffer, DUMMY_SINK_RECEIVE_BUFFER_SIZE,
                          afterGettingFrame, this,
                          onSourceClosure, this);
    return True;
}

static void* ThreadLoop(void* opaque, XPR_Thread* thread)
{
    int idx = (long)opaque;
    // Begin by setting up our usage environment:
    BasicTaskScheduler* scheduler = BasicTaskScheduler::createNew();
    UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
    unsigned cnt = 0;
    while (!xpr_rtsp->exit_loop) {
        int64_t cts = XPR_SYS_GetCTS();
        //
        if ((cnt % XPR_RTSP_MAX_WORKERS) == idx) {
            int port = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, (cnt & 0xff), 0, 0);
            PortContext* pc = XPR_RTSP_GetPortContext(port);
            if (pc) {
                if ((pc->act_flags == 0) &&
                    (pc->usr_flags & XPR_RTSP_FLAG_START) &&
                    pc->url[0] != 0) {
                    pc->usr_flags &= ~XPR_RTSP_FLAG_START;
                    pc->act_flags |= XPR_RTSP_FLAG_START;
#ifdef XPR_RTSP_HAVE_PES
                    pc->out_pes = 0;
#endif
                    pc->rtsp_priv = openURL(*env, port);
                    //printf("Open port %x, %p\n", port, pc->rtsp_client);
#ifdef XPR_RTSP_HAVE_HKMI
                    memset(&pc->out_pes_hkmi, 0, sizeof(pc->out_pes_hkmi));
                    pc->out_pes_hkmi.media_fourcc = FOURCC_HKMI;
                    pc->out_pes_hkmi.media_version = 0x0101;
                    pc->out_pes_hkmi.device_id = 0;
                    pc->out_pes_hkmi.system_format = SYSTEM_MPEG2_PS;
                    pc->out_pes_hkmi.reserved[0] = 0;
                    pc->out_pes_hkmi.reserved[1] = 0;
                    pc->out_pes_hkmi.reserved[2] = 0;
                    pc->out_pes_hkmi.reserved[3] = 0;
#endif
                }
                else if (pc->usr_flags & XPR_RTSP_FLAG_STOP) {
                    pc->usr_flags &= ~XPR_RTSP_FLAG_STOP;
                    pc->act_flags &= ~XPR_RTSP_FLAG_START;
                    pc->act_flags |= XPR_RTSP_FLAG_STOP;
                    if (pc->rtsp_priv) {
                        //printf("Stop port %x, %p\n", port, pc->rtsp_client);
                        shutdownStream((RTSPClient*)pc->rtsp_priv);
                    }
                    else {
                        pc->act_flags &= ~XPR_RTSP_FLAG_STOP;
                        pc->rtsp_priv = 0;
                    }
                }
                else if ((pc->usr_flags & XPR_RTSP_FLAG_KPL) &&
                         !(pc->act_flags & XPR_RTSP_FLAG_KPL)) {
                    pc->usr_flags &= ~XPR_RTSP_FLAG_KPL;
                    pc->act_flags |= XPR_RTSP_FLAG_KPL;
                    keepAlive((RTSPClient*)pc->rtsp_priv);
                }
                //
                if (pc->act_flags & XPR_RTSP_FLAG_CLOSE) {
#ifdef XPR_RTSP_HAVE_PES
                    if (pc->out_pes) {
                        XPR_PES_Close(pc->out_pes);
                        pc->out_pes = 0;
                    }
                    pc->out_pes_port = 0;
                    pc->out_pes_stb = 0;
#endif
                    pc->act_flags = 0;
                    pc->usr_flags = 0;
                    pc->lats = 0;
                    pc->lvts = 0;
                    pc->rtsp_priv = 0;
                    pc->out_fourcc = 0;
                    memset(pc->cbs, 0, sizeof(pc->cbs));
                }
                else if (pc->act_flags & XPR_RTSP_FLAG_START) {
                    int64_t diff = cts - pc->lats;
                    if (diff > XPR_RTSP_TMO_US) {
                        pc->usr_flags |= XPR_RTSP_FLAG_STOP;
                    }
                    // Check last
                    diff = cts - pc->lvts;
                    if (diff > XPR_RTSP_LVI_US &&
                        !(pc->act_flags & XPR_RTSP_FLAG_KPL)) {
                        pc->usr_flags |= XPR_RTSP_FLAG_KPL;
                    }
                }
            }
        }
        //
        scheduler->SingleStep(1000);
        cnt++;
    }
    return 0;
}

int XPR_RTSP_Init(void)
{
    if (xpr_rtsp == NULL) {
        xpr_rtsp = (XPR_RTSP*)calloc(sizeof(*xpr_rtsp), 1);
        for (int i = 0; i < XPR_RTSP_ACT_WORKERS; i++) {
            xpr_rtsp->threads[i] = XPR_ThreadCreate(ThreadLoop, 0, (void*)i);
        }
    }
    return 0;
}

int XPR_RTSP_Fini(void)
{
    return 0;
}

int XPR_RTSP_IsPortValid(int port)
{
    int major = XPR_RTSP_PORT_MAJOR(port);
    int minor = XPR_RTSP_PORT_MINOR(port);
    if (major != 1 || minor < 1 || minor > XPR_RTSP_MAX_CLIENTS)
        return XPR_FALSE;
    return XPR_TRUE;
}

int XPR_RTSP_Open(int port, const char* url)
{
    int major = XPR_RTSP_PORT_MAJOR(port);
    int minor = XPR_RTSP_PORT_MINOR(port);
    if (minor == XPR_RTSP_PORT_MINOR_ANY) {
        minor = XPR_RTSP_GetAvailClientPortIndex();
        if (minor < 0)
            return -1;
        port = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, minor, 0, 0);
    }
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        strcpy(pc->url, url);
        pc->usr_flags |= XPR_RTSP_FLAG_OPEN;
    }
    return port;
}

int XPR_RTSP_Close(int port)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        pc->usr_flags &= ~(XPR_RTSP_FLAG_OPEN | XPR_RTSP_FLAG_START);
        pc->usr_flags |= XPR_RTSP_FLAG_STOP | XPR_RTSP_FLAG_CLOSE;
    }
    return 0;
}

int XPR_RTSP_Start(int port)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc)
        pc->usr_flags |= XPR_RTSP_FLAG_START;
    return 0;
}

int XPR_RTSP_Stop(int port)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        pc->usr_flags &= ~XPR_RTSP_FLAG_START;
        pc->usr_flags |= XPR_RTSP_FLAG_STOP;
    }
    return 0;
}

int XPR_RTSP_AddDataCallback(int port, XPR_RTSP_DCB cb, void* opaque)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
            Callback* ncb = &pc->cbs[i];
            if (ncb->dcb)
                continue;
            ncb->dcb = cb;
            ncb->dcb_opaque = opaque;
            break;
        }
    }
    return 0;
}



int XPR_RTSP_DelDataCallback(int port, XPR_RTSP_DCB cb, void* opaque)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
            Callback* ncb = &pc->cbs[i];
            if (ncb->dcb == cb && ncb->dcb_opaque == opaque) {
                ncb->dcb = 0;
                ncb->dcb_opaque = 0;
                break;
            }
        }
    }
    return 0;
}

int XPR_RTSP_AddEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
            Callback* ncb = &pc->cbs[i];
            if (ncb->evcb)
                continue;
            ncb->evcb = cb;
            ncb->evcb_opaque = opaque;
            break;
        }
    }
    return 0;
}

int XPR_RTSP_DelEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
            Callback* ncb = &pc->cbs[i];
            if (ncb->evcb == cb && ncb->evcb_opaque == opaque) {
                ncb->evcb = 0;
                ncb->evcb_opaque = 0;
                break;
            }
        }
    }
    return 0;
}

#ifdef XPR_RTSP_HAVE_XD_STREAM_API
int XPR_RTSP_AddDataCallbackXD(int port, XD_StreamDataCallback cb, void* opaque)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
            Callback* ncb = &pc->cbs[i];
            if (ncb->dcb_xd)
                continue;
            ncb->dcb_xd = cb;
            ncb->dcb_opaque_xd = opaque;
            break;
        }
    }
    return 0;
}

int XPR_RTSP_DelDataCallbackXD(int port, XD_StreamDataCallback cb, void* opaque)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
            Callback* ncb = &pc->cbs[i];
            if (ncb->dcb_xd == cb && ncb->dcb_opaque_xd == opaque) {
                ncb->dcb_xd = 0;
                ncb->dcb_opaque_xd = 0;
                break;
            }
        }
    }
    return 0;
}

int XPR_RTSP_AddEventCallbackXD(int port, XD_StreamEventCallback cb,
                                void* opaque)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
            Callback* ncb = &pc->cbs[i];
            if (ncb->evcb_xd)
                continue;
            ncb->evcb_xd = cb;
            ncb->evcb_opaque_xd = opaque;
            break;
        }
    }
    return 0;
}

int XPR_RTSP_DelEventCallbackXD(int port, XD_StreamEventCallback cb,
                                void* opaque)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
            Callback* ncb = &pc->cbs[i];
            if (ncb->evcb_xd == cb && ncb->evcb_opaque_xd == opaque) {
                ncb->evcb_xd = 0;
                ncb->evcb_opaque_xd = 0;
                break;
            }
        }
    }
    return 0;
}
#endif

int XPR_RTSP_SetAuth(int port, const char* username, const char* password,
                     int pwdIsMD5)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        strcpy(pc->username, username ? username : "");
        strcpy(pc->password, password ? password : "");
    }
    return 0;
}

int XPR_RTSP_SetOutputFormat(int port, int fourcc)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        pc->out_fourcc = fourcc;
    }
    return 0;
}

#if defined(XPR_RTSP_HAVE_PES)
typedef struct PES_PH_CTX {
    int port;
    XPR_StreamBlock* stb;
} PES_PH_CTX;

static int PESPacketHandler(const uint8_t* data, int length, void* opaque)
{
    PortContext* pc = (PortContext*)opaque;
    XPR_StreamBlock stb = { 0 };
    stb.buffer = stb.data = (uint8_t*)data;
    stb.bufferSize = stb.dataSize = length;
    stb.codec = AV_FOURCC_MP2P;
    stb.flags = pc->out_pes_stb->flags;
    stb.flags |= (pc->out_pes_stb->codec == AV_FOURCC_PCMA ||
                  pc->out_pes_stb->codec == AV_FOURCC_PCMU) ? XPR_STREAMBLOCK_FLAG_AUDIO_FRAME :
                 0;
    stb.flags |= (pc->out_pes_stb->codec == AV_FOURCC_H264 ||
                  pc->out_pes_stb->codec == AV_FOURCC_JPEG) ? XPR_STREAMBLOCK_FLAG_VIDEO_FRAME :
                 0;
    stb.pts = pc->out_pes_stb->pts;
    stb.dts = pc->out_pes_stb->dts;
    stb.track = 0;
    stb.next = 0;
    stb.pf_release = 0;
    XPR_RTSP_PostData(pc->out_pes_port, &stb);
    return length;
}
#endif // defined(XPR_RTSP_HAVE_PES)

int XPR_RTSP_PostData(int port, XPR_StreamBlock* stb)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
#if defined(XPR_RTSP_HAVE_HKMI)
        if (!(pc->act_flags & XPR_RTSP_FLAG_HKMIPD)) {
            pc->act_flags |= XPR_RTSP_FLAG_HKMIPD;
            XPR_StreamBlock nstb;
            memset(&nstb, 0, sizeof(nstb));
            nstb.buffer = nstb.data = (uint8_t*)&pc->out_pes_hkmi;
            nstb.bufferSize = nstb.dataSize = sizeof(pc->out_pes_hkmi);
            nstb.codec = AV_FOURCC_HKMI;
            XPR_RTSP_PostData(port, &nstb);
        }
#endif // defined(XPR_RTSP_HAVE_HKMI)
        if (pc->out_fourcc == AV_FOURCC_MP2P && stb->codec != AV_FOURCC_MP2P) {
#if defined(XPR_RTSP_HAVE_PES)
            pc->out_pes_port = port;
            pc->out_pes_stb = stb;
            if (pc->out_pes == NULL) {
                pc->out_pes = XPR_PES_Open("file://dummy");
                XPR_PES_AddWriteCallback(pc->out_pes, PESPacketHandler, pc);
            }
            if (pc->out_pes) {
                XPR_PES_WriteFrame(pc->out_pes, stb->data, stb->dataSize, stb->codec, stb->pts);
            }
#else
            return -1;
#endif // defined(XPR_RTSP_HAVE_PES)
        }
        else {
            for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
                Callback* cb = &pc->cbs[i];
                if (cb->dcb)
                    cb->dcb(cb->dcb_opaque, port, stb);
#if defined(XPR_RTSP_HAVE_XD_STREAM_API)
                if (cb->dcb_xd)
                    cb->dcb_xd((XD_StreamBlock*)stb, cb->dcb_opaque_xd);
#endif // defined(XPR_RTSP_HAVE_XD_STREAM_API)
            }
        }
    }
    return 0;
}

int XPR_RTSP_PostEvent(int port, const XPR_RTSP_EVD* evd)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        for (int i = 0; i < XPR_RTSP_MAX_CALLBACKS; i++) {
            Callback* cb = &pc->cbs[i];
            if (cb->evcb)
                cb->evcb(cb->evcb_opaque, port, evd);
#ifdef XPR_RTSP_HAVE_XD_STREAM_API
            if (cb->evcb_xd) {
                if (evd->event == XPR_RTSP_EVT_TIMEOUT)
                    cb->evcb_xd(XD_StreamRTSPTMO, cb->evcb_opaque_xd);
                else if (evd->event == XPR_RTSP_EVT_SRC_STARTED)
                    cb->evcb_xd(XD_StreamSourceStarted, cb->evcb_opaque_xd);
            }
#endif
        }
        //printf("!!! Port %x, evt %d\n", port, evd->event);
    }
    return 0;
}

int XPR_RTSP_SetTrSpec(int port, XPR_RTSP_TRSPEC trspec)
{
    if (XPR_RTSP_IsPortValid(port) != XPR_TRUE)
        return -1;
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    if (pc) {
        pc->trspec = trspec;
    }
    return 0;
}

int XPR_RTSP_GetParam(int port, XPR_RTSP_PARAM param, void* buffer, int* size)
{
    return 0;
}

int XPR_RTSP_SetParam(int port, XPR_RTSP_PARAM param, const void* data,
                      int length)
{
    PortContext* pc = XPR_RTSP_GetPortContext(port);
    switch (param) {
    case XPR_RTSP_PARAM_UNKNOWN:
        break;
    case XPR_RTSP_PARAM_CACHE_TIME:
        break;
    case XPR_RTSP_PARAM_FOURCC:
        break;
    case XPR_RTSP_PARAM_CHANNELS:
        break;
    case XPR_RTSP_PARAM_SAMPLERATE:
        break;
    case XPR_RTSP_PARAM_PPS_DATA:
        break;
    case XPR_RTSP_PARAM_SPS_DATA:
        break;
    case XPR_RTSP_PARAM_AUTO_SDP:
        break;
    case XPR_RTSP_PARAM_JPEG_QFACTOR:
        break;
    case XPR_RTSP_PARAM_MAX_SESSIONS:
        break;
    case XPR_RTSP_PARAM_H264_ADD_AUD:
        pc->h264_flags |= (int)data ? H264_FLAG_ADD_AUD : 0;
        break;
    case XPR_RTSP_PARAM_H264_SINGLE_FRAME:
        pc->h264_flags |= (int)data ? H264_FLAG_SINGLE_FRAME : 0;
        break;
    case XPR_RTSP_PARAM_H264_STARTCODE:
        pc->h264_flags |= (int)data ? H264_FLAG_START_CODE : 0;
        break;
    case XPR_RTSP_PARAM_MAX:
        break;
    default:
        break;
    }
    return 0;
}

#ifdef XPR_RTSP_HAVE_XD_STREAM_API
void libXD_Stream_Config(int request, const char* optVal, size_t optLen)
{
}

int libXD_Stream_Init(void)
{
    return XPR_RTSP_Init();
}

void libXD_Stream_Fini(void)
{
    (void)XPR_RTSP_Fini();
}

const char* libXD_Stream_Version(void)
{
    return "v1.0(xpr)";
}

int libXD_Stream_VersionNumber(void)
{
    return 0x00010000;
}

int XD_StreamOpen(const char* url)
{
    printf("xd_stream: open %s\n", url);
    return XPR_RTSP_Open(XPR_RTSP_PORT_CLI_ANY, url);
}

int XD_StreamClose(int port)
{
    return XPR_RTSP_Close(port);
}

int XD_StreamStart(int port)
{
    return XPR_RTSP_Start(port);
}

int XD_StreamStop(int port)
{
    return XPR_RTSP_Stop(port);
}

int XD_StreamAddDataCallback(int port, XD_StreamDataCallback callback,
                             void* opaque)
{
    return XPR_RTSP_AddDataCallbackXD(port, callback, opaque);
}

int XD_StreamDelDataCallback(int port, XD_StreamDataCallback callback,
                             void* opaque)
{
    return XPR_RTSP_DelDataCallbackXD(port, callback, opaque);
}

int XD_StreamAddEventCallback(int port, XD_StreamEventCallback callback,
                              void* opaque)
{
    return XPR_RTSP_AddEventCallbackXD(port, callback, opaque);
}

int XD_StreamDelEventCallback(int port, XD_StreamEventCallback callback,
                              void* opaque)
{
    return XPR_RTSP_DelEventCallbackXD(port, callback, opaque);
}

int XD_StreamSetAuth(int port, const char* username, const char* password,
                     int pwdIsMD5)
{
    return XPR_RTSP_SetAuth(port, username, password, pwdIsMD5);
}

int XD_StreamSetOFPM(int port, int yes)
{
    return 0;
}

int XD_StreamSetOutputFormat(int port, int fourcc)
{
    return XPR_RTSP_SetOutputFormat(port, fourcc);
}

int XD_StreamSetTimeout(int port, int msecs)
{
    return 0;
}

int XD_StreamSetTransferMode(int port, int mode)
{
    return 0;
}
#else
void libXD_Stream_Config(int request, const char* optVal, size_t optLen)
{
}
int libXD_Stream_Init(void)
{
    return -1;
}
void libXD_Stream_Fini(void)
{
}
const char* libXD_Stream_Version(void)
{
    return "Unknown";
}
int libXD_Stream_VersionNumber(void)
{
    return 0xFFFFFFFF;
}
int XD_StreamOpen(const char* url)
{
    return -1;
}
int XD_StreamClose(int port)
{
    return -1;
}
int XD_StreamStart(int port)
{
    return -1;
}
int XD_StreamStop(int port)
{
    return -1;
}
int XD_StreamAddDataCallback(int port, void* callback, void* opaque)
{
    return -1;
}
int XD_StreamDelDataCallback(int port, void* callback, void* opaque)
{
    return -1;
}
int XD_StreamAddEventCallback(int port, void* callback, void* opaque)
{
    return -1;
}
int XD_StreamDelEventCallback(int port, void* callback, void* opaque)
{
    return -1;
}
int XD_StreamSetAuth(int port, const char* username, const char* password,
                     int pwdIsMD5)
{
    return -1;
}
int XD_StreamSetOFPM(int port, int yes)
{
    return -1;
}
int XD_StreamSetOutputFormat(int port, int fourcc)
{
    return -1;
}
int XD_StreamSetTimeout(int port, int msecs)
{
    return -1;
}
int XD_StreamSetTransferMode(int port, int mode)
{
    return -1;
}
#endif

#endif // defined(XPR_RTSP_DRIVER_LIVE) && defined(XPR_RTSP_HAVE_CLIENT)
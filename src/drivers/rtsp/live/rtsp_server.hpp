#ifndef XPR_RTSP_DRIVER_LIVE_RTSP_SERVER_HPP
#define XPR_RTSP_DRIVER_LIVE_RTSP_SERVER_HPP

#include <stdint.h>
#include <string>
#include <live/BasicUsageEnvironment.hh>
#include <live/FramedSource.hh>
#include <live/LiveMedia.hh>
#include "rtsp.hpp"

#define XPR_RTSP_MAX_SERVERS			4
#define XPR_RTSP_MAX_STREAMS			16
#define XPR_RTSP_MAX_STREAM_TRACKS		4
#define XPR_RTSP_MAX_WORKERS			4
#define XPR_RTSP_STREAM_MAX_FRAME_SIZE	1048576

namespace xpr {

	namespace rtsp {

		// Ç°ÖÃÉùÃ÷
		class Server;
		class ServerManager;
		class Worker;

		class H264VideoFramedSource : public FramedSource
		{
		public:
			H264VideoFramedSource(UsageEnvironment& env);
			virtual ~H264VideoFramedSource(void);

		public:
			// FramedSource interfaces
			virtual void doGetNextFrame();
			virtual unsigned int maxFrameSize() const;

			// Published interface
			void fetchFrame();

			//
			static void getNextFrame(void * ptr);

		private:
			TaskToken	mCurrentTask;
		};

		class H264VideoServerMediaSession : public OnDemandServerMediaSubsession
		{
		public:
			virtual ~H264VideoServerMediaSession(void);

			// Override OnDemandServerMediaSubsession interfaces
			virtual char const * getAuxSDPLine(RTPSink * rtpSink, FramedSource * inputSource);
			virtual FramedSource * createNewStreamSource(unsigned clientSessionId, unsigned & estBitrate); // "estBitrate" is the stream's estimated bitrate, in kbps  
			virtual RTPSink * createNewRTPSink(Groupsock * rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic, FramedSource * inputSource);

			// Methods
			void chkForAuxSDPLine1();

			// Static Methods
			static H264VideoServerMediaSession* createNew(UsageEnvironment & env, FramedSource* source);

			static void afterPlayingDummy(void* ptr);

			static void chkForAuxSDPLine(void* ptr);

		protected:
			H264VideoServerMediaSession(UsageEnvironment& env, FramedSource* source);
		};

		class Worker {
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
			int					mId;
			Server*				mServer;
			TaskScheduler*		mScheduler;
			UsageEnvironment*	mEnv;
			XPR_Thread*			mThread;
			bool				mExitLoop;
		};

		class Server {
		public:
			Server(void);
			virtual ~Server(void);

			int start(void);
			int stop(void);

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
			void setupWorkers(void);
			void clearWorkers(void);

		private:
			std::string			mBindAddress;
			uint16_t			mBindPort;
			size_t				mMaxStreams;
			size_t				mMaxStreamTracks;
			size_t				mMaxWorkers;
			Worker*				mWorkers[XPR_RTSP_MAX_WORKERS];
		};

		class ServerManager : public Port {
		public:
			ServerManager(void);
			virtual ~ServerManager(void);

			// Port interfaces
			virtual bool isPortValid(int port);
			virtual int open(int port, const char* url);
			virtual int close(int port);
			virtual int start(int port);
			virtual int stop(int port);

		private:
			int setupServer(const char* url);
			int clearServer(void);
			void configServer(const char* query);
			void configServer(const char* key, const char* value);
			int setupStream(int port, const char* url);
			int clearStream(int port);

			friend void handleServerConfig(void* opaque, char* seg);

		private:
			Server*		mServer;
		};

	} // namespace xpr::rtsp

} // namespace xpr

#endif // XPR_RTSP_DRIVER_LIVE_RTSP_SERVER_HPP
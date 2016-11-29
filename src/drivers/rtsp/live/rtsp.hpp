#ifndef XPR_RTSP_DRIVER_LIVE_RTSP_HPP
#define XPR_RTSP_DRIVER_LIVE_RTSP_HPP

#include <string>
#include <vector>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_utils.h>

#define XPR_RTSP_MAX_CALLBACKS      4
#define XPR_RTSP_MAX_CLIENTS        256
#define XPR_RTSP_MAX_STREAMS        256
#define XPR_RTSP_MAX_WORKERS        8
#define XPR_RTSP_ACT_WORKERS        4
#define XPR_RTSP_TMO_US             30000000
#define XPR_RTSP_LVI_US             30000000

#define H264_FLAG_ADD_AUD			0x00000001
#define H264_FLAG_SINGLE_FRAME		0x00000002
#define H264_FLAG_START_CODE		0x00000004

/// 回调函数集
typedef struct Callback {
	XPR_RTSP_DCB			dcb;			///< 数据回调
	void*					dcb_opaque;		///< 数据回调关联数据
	XPR_RTSP_EVCB			evcb;			///< 事件回调
	void*					evcb_opaque;	///< 事件回调关联数据
#ifdef HAVE_XPR_RTSP_XD_STREAM_API
	XD_StreamDataCallback	dcb_xd;			///< 数据回调 (XD)
	void*					dcb_opaque_xd;	///< 数据回调关联数据 (XD)
	XD_StreamEventCallback	evcb_xd;		///< 事件回调 (XD)
	void*					evcb_opaque_xd;	///< 事件回调关联数据 (XD)
#endif
} Callback;

/// 端口上下文
typedef struct PortContext {
	int					usr_flags;		///< 用户设定标志
	int					act_flags;		///< 当前生效标志
	int64_t				lats;			///< 最后活动时间
	int64_t				lvts;			///< 最后心跳时间
	uint64_t			total_frames;	///< 传输帧数统计
	char				url[1024];		///< 目标地址
	char				username[64];	///< 用于认证的用户名称
	char				password[64];	///< 用于认证的用户密码
	Callback			cbs[XPR_RTSP_MAX_CALLBACKS];
	void*				rtsp_priv;
	int					out_fourcc;		///< RTSP 客户端输出数据格式
#ifdef HAVE_XPR_RTSP_HKMI
	HIK_MEDIAINFO		out_pes_hkmi;	///< 海康 PES 头信息
#endif

	XPR_RTSP_TRSPEC		trspec;			///< RTSP 客户端传输方式

	uint32_t h264_flags;

#ifdef HAVE_XPR_RTSP_PES
	XPR_PES*			out_pes;
	int					out_pes_port;
	XPR_StreamBlock*	out_pes_stb;
#endif

} PortContext;

typedef struct XPR_RTSP {
	int exit_loop;
	PortContext client_ports[XPR_RTSP_MAX_CLIENTS + 2];
	PortContext stream_ports[XPR_RTSP_MAX_STREAMS + 2];
	XPR_Thread* threads[XPR_RTSP_MAX_WORKERS];
} XPR_RTSP;

/// 获取可用的客户端端口索引
int XPR_RTSP_GetAvailClientPortIndex(void);

/// 获取可用的服务流端口索引
int XPR_RTSP_GetAvailStreamPortIndex(void);

/// 更新端口的最后活动时间
void XPR_RTSP_UpdatePortLATS(int port);

/// 更新端口的最后心跳时间
void XPR_RTSP_UpdatePortLVTS(int port);

namespace xpr {

	namespace rtsp {

		// 前置声明
		class Port;
		//

		class QueryParams {
		public:
			QueryParams(const std::string& qs)
			{
				parse(qs);
			}

			~QueryParams(void)
			{

			}

		private:
			void parse(const std::string& qs)
			{

			}

			void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
			{
				std::string::size_type pos1, pos2;
				pos2 = s.find(c);
				pos1 = 0;
				while (std::string::npos != pos2)
				{
					v.push_back(s.substr(pos1, pos2 - pos1));

					pos1 = pos2 + c.size();
					pos2 = s.find(c, pos1);
				}
				if (pos1 != s.length())
					v.push_back(s.substr(pos1));
			}

			std::pair<std::string, std::string> splitKV()
			{

			}

		private:
			std::vector<std::pair<std::string, std::string>>	mParams;
		};

		class Port {
		public:
			Port(void)
			{
				// Nothing TODO
			}

			virtual ~Port(void)
			{
			}

			virtual bool isPortValid(int port)
			{
				return false;
			}

			virtual int open(int port, const char* url)
			{
				return XPR_ERR_GEN_NOT_SUPPORT;
			}

			virtual int close(int port)
			{
				return XPR_ERR_GEN_NOT_SUPPORT;
			}

			virtual int start(int port)
			{
				return XPR_ERR_GEN_NOT_SUPPORT;
			}

			virtual int stop(int port)
			{
				return XPR_ERR_GEN_NOT_SUPPORT;
			}

			virtual int addDataCallback(int port, XPR_RTSP_DCB cb, void* opaque)
			{
				Port* p = getPort(port);
				if (p) {
					for (int i = 0; i<XPR_RTSP_MAX_CALLBACKS; i++) {
						Callback* ncb = &p->mCallbacks[i];
						if (ncb->dcb)
							continue;
						ncb->dcb = cb;
						ncb->dcb_opaque = opaque;
						return XPR_ERR_OK;
					}
				}
				return XPR_ERR_GEN_UNEXIST;
			}

			virtual int delDataCallback(int port, XPR_RTSP_DCB cb, void* opaque)
			{
				Port* p = getPort(port);
				if (p) {
					for (int i = 0; i<XPR_RTSP_MAX_CALLBACKS; i++) {
						Callback* ncb = &p->mCallbacks[i];
						if (ncb->dcb == cb && ncb->dcb_opaque == opaque) {
							ncb->dcb = 0;
							ncb->dcb_opaque = 0;
							return XPR_ERR_OK;
						}
					}
				}
				return XPR_ERR_GEN_UNEXIST;
			}

			virtual int addEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque)
			{
				Port* pc = getPort(port);
				if (pc) {
					for (int i = 0; i<XPR_RTSP_MAX_CALLBACKS; i++) {
						Callback* ncb = &pc->mCallbacks[i];
						if (ncb->evcb)
							continue;
						ncb->evcb = cb;
						ncb->evcb_opaque = opaque;
						return XPR_ERR_OK;
					}
				}
				return XPR_ERR_GEN_UNEXIST;
			}

			virtual int delEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque)
			{
				Port* p = getPort(port);
				if (p) {
					for (int i = 0; i<XPR_RTSP_MAX_CALLBACKS; i++) {
						Callback* ncb = &p->mCallbacks[i];
						if (ncb->evcb == cb && ncb->evcb_opaque == opaque) {
							ncb->evcb = NULL;
							ncb->evcb_opaque = NULL;
							return XPR_ERR_OK;
						}
					}
				}
				return XPR_ERR_GEN_UNEXIST;
			}

			virtual int setAuth(int port, const char* username, const char* password, int pwdIsMD5)
			{
				mUsername = username;
				mPassword = password;
				mPwdIsMD5 = pwdIsMD5;
				return XPR_ERR_OK;
			}

			virtual int setOutputFormat(int port, uint32_t fourcc)
			{
				return XPR_ERR_GEN_NOT_SUPPORT;
			}

			virtual int setTrSpec(int port, XPR_RTSP_TRSPEC trspec)
			{
				return XPR_ERR_GEN_NOT_SUPPORT;
			}

			virtual int postData(int port, const XPR_StreamBlock* stb)
			{
				return XPR_ERR_GEN_NOT_SUPPORT;
			}

			virtual int postEvent(int port, const XPR_RTSP_EVD* evd)
			{
				return XPR_ERR_GEN_NOT_SUPPORT;
			}

			virtual int getParam(int port, XPR_RTSP_PARAM param, void* buffer, int* size)
			{
				return XPR_ERR_GEN_NOT_SUPPORT;
			}

			virtual int setParam(int port, XPR_RTSP_PARAM param, const void* data, int length)
			{
				return XPR_ERR_GEN_NOT_SUPPORT;
			}

			/// @brief 获取端口上下文
			/// @param [in] port		端口句柄
			/// return 成功返回端口的指针，失败返回 NULL
			static Port* getPort(int port);

			/// 获取可用的流端口编号
			/// @param [in] majorPort		主端口
			static int getAvailStreamId(int majorPort);

			/// 获取可用的流轨道端口编号
			/// @param [in] streamPort		流端口
			static int getAvailStreamTrackId(int streamPort);

		protected:
			uint32_t		mActiveFlags;
			uint32_t		mUserFlags;
			int64_t			mLastActiveTS;
			int64_t			mLastLivenessTS;
			uint32_t		mOutputFourCC;
			XPR_RTSP_TRSPEC	mTrSpec;
			uint64_t		mTotalFrames;
			std::string		mUrl;
			std::string		mUsername;
			std::string		mPassword;
			int				mPwdIsMD5;
			Callback		mCallbacks[XPR_RTSP_MAX_CALLBACKS];
		};

	} // namespace xpr::rtsp

} // namespace xpr


#endif // XPR_RTSP_DRIVER_LIVE_RTSP_HPP

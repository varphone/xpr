#include <d3d9.h>
#include <d3dx9.h>
#include <hash_map>
#include <memory>
#include <set>
#include <atlbase.h>
#include <xpr/xpr_mcvr.h>
#include <xpr/xpr_avframe.h>
#include "xpr_mcvr_autolock.hpp"

// A macro to disallow the copy constructor and operator= functions 
// This should be used in the private:declarations for a class
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);         \
    TypeName& operator=(const TypeName&)
#endif

// 通道状态字符表
#pragma pack(8)
typedef struct _XPR_MCVR_StateString {
    char no_device_string[24];      ///< 无设备字符串
    char buffering_string[24];      ///< 正在缓冲字符串
    char interrupt_string[24];      ///< 断开连接字符串
    char stopped_string[24];        ///< 停止播放字符串
} XPR_MCVR_StateString;
#pragma pack()

class SingleEvent;
class TextWriter;
class LinePainter;
struct D3DDestTexture;
namespace XD
{
    class D3DEffect;
    class D3DMainChannel;
    class D3DSubChannel;
    class D3DRenderer
    {
    private:
        DISALLOW_COPY_AND_ASSIGN(D3DRenderer);
    public:
        explicit D3DRenderer(void *hwnd);
        ~D3DRenderer();
        //功能操作
        bool bindPort(int port, void *hwnd, int rows, int cols);
        bool inputData(int port, XPR_AVFrame *frame_info);
        bool zoomIn(int src_port, int dest_port);
        int getZoomPort(int src_port);
        bool resetPort(int port);
        int getCurrentPort(void);
        bool getVideoSize(int port, int *width, int *height);
        bool snapShot(int port, const char *path);
        void setAspectRatio(float ratio);
        float getAspectRatio(void);
        void setScale(int port, float factor);
        float getScale(int port);
        bool setRate(int port, float rate);
        //渲染主循环
        void loopPresenting(void);
        //状态
        bool setRenderLevel(int level);
        int getRenderLevel() const;
        bool setTitleFlag(int flag);
        int getTitleFlag(void) const;
        bool setHintFlag(int flag);
        int getHintFlag(void) const;
        bool setChannelState(int port, XPR_MCVR_ChannelState state);
        int setChannelString(int port, XPR_MCVR_StringType type, char *strings);
        int setChannelStrings(int port, XPR_MCVR_StringType type, char **strings, int count);
        XPR_MCVR_ChannelState getChannelState(int port);
        //渲染效果
        int setChannelEffect(int port, XPR_MCVR_EffectType effect, int value);
        int setChannelEffectF(int port, XPR_MCVR_EffectType effect, float value);
        int getChannelEffect(int port, XPR_MCVR_EffectType effect);
        float getChannelEffectF(int port, XPR_MCVR_EffectType effect);
        //响应设备窗口的消息
        LRESULT respondMainHwnd(HWND hWnd, int uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT respondChannelHwnd(HWND hWnd, int uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR id);
        //事件
        bool addEventCallback(XPR_MCVR_EventCallback callback, void *user);
        bool delEventCallback(XPR_MCVR_EventCallback callback, void *user);
        bool attachEvent(XPR_MCVR_EventType ev);
        bool attachAllEvents(void);
        bool detachEvent(XPR_MCVR_EventType ev);
        bool detachAllEvents(void);

    private:
        //D3D设备初始化
        bool createDevice(HWND hwnd);
        void fillPresentParameters(D3DPRESENT_PARAMETERS *pp, HWND hwnd, UINT width, UINT height);
        bool initPipeline(void);
        bool initVertices(void);
        bool resetDevice(int wnd_size);
        void initStateString();
        //通道操作
        bool addMainChannel(int main_port, HWND hwnd, int rows, int cols);
        void removeMainChannel(int main_port);
        void resetCurrentPort();
        //渲染循环
        bool beginRenderLoop(void);
        int presentAllChannels(void);
        int presentChannel(D3DMainChannel *main_channel);
        bool drawChannel(D3DMainChannel *main_channel, const int sub_port);
        void saveTextureToFile(IDirect3DTexture9 *texture, const char *path);
        bool enhanceImageY(D3DMainChannel *main_channel, int sub_port, D3DDestTexture *dest_texture);
        std::shared_ptr<D3DMainChannel> getMainChannel(int port);
        std::shared_ptr<D3DSubChannel> getSubChannel(int port);
        std::shared_ptr<D3DDestTexture> getDestTexture(UINT params);
        bool setCenterVertex(void);
        bool setFullVertex(void);
        bool setRenderTarget(IDirect3DTexture9 *target_tex);
        void adjustAspectRatio(float wnd_ratio, float video_ratio);
        void drawText(D3DMainChannel *main_channel);
        void drawSubText(XPR_MCVR_ChannelState state, RECT rect, char *title);
        //功能函数
        void optimizePerformance(bool is_improving);
        bool isMainChanValid(int port);
        bool isSubChanValid(int port);
        bool isFallingSleep(void);
        void genericEvent(XPR_MCVR_EventType event, int port, void *data);

    private:
        IDirect3DDevice9 *_device;
        HWND _main_hwnd;
        int _wnd_size; //High word for height, low word for width.
        int _current_port;
        IDirect3DVertexBuffer9 *_center_vertex;
        IDirect3DVertexBuffer9 *_full_vertex;
        D3DEffect *_d3d_effect;
        float _aspect_ratio;
        stdext::hash_map<UINT, std::shared_ptr<D3DDestTexture>> _texture_map;
        stdext::hash_map<int, std::shared_ptr<D3DMainChannel>> _channels_map;
        std::set<std::pair<XPR_MCVR_EventCallback, void*>> _callback_set;
        std::set<XPR_MCVR_EventType> _event_set;
        SectionLock _callback_lock;
        SectionLock _event_lock;
        IDirect3DTexture9 *_pixel_table;
        int _buffer_count;
        DWORD _previous_time;
        SingleEvent *_end_event;
        TextWriter *_text_writer;
        typedef enum _D3D_DEVICE_STATE {
            D3D_DEVICE_STATE_NORMAL   = 0,
            D3D_DEVICE_STATE_INVALID  = 1,
            D3D_DEVICE_STATE_RESETING = 2,
        } D3D_DEVICE_STATE;
        D3D_DEVICE_STATE _d3d_device_state;
        XPR_MCVR_StateString *_camera_state_string;
        int _render_level;
        volatile bool _is_rendering;
        bool _is_show_title;
        bool _is_show_hint;
    };

    class D3DEffect
    {
    private:
        DISALLOW_COPY_AND_ASSIGN(D3DEffect);
    public:
        explicit D3DEffect(IDirect3DDevice9 *device);
        ~D3DEffect();
        bool setOffset(const int width, const int height);
        bool setRatio(const float wnd_ratio, const float dest_ratio);
        bool convertColor(XPR_AVPixelFormat format, bool is_alarming);
        bool drawRGB(void);
        bool sharpenImage(int val);
        bool brightenImage(int val);
        bool defogImage(int mode, int val, float *range);
        bool stretchHist(float *range);
        bool equalHist();
        void onLostDevice(void);
        void onResetDevice(void);

    private:
        bool initEffect(void);

    private:
        IDirect3DDevice9 *_device;
        ID3DXEffect *_effect;
        
        D3DXHANDLE _planar_tech;
        D3DXHANDLE _uyvy_tech;
        D3DXHANDLE _x_offset_handle;
        D3DXHANDLE _y_offset_handle;
        D3DXHANDLE _wnd_ratio_handle;
        D3DXHANDLE _dest_ratio_handle;
        D3DXHANDLE _alarm_handle;
        D3DXHANDLE _sharp_handle;
        D3DXHANDLE _bright_handle;
        D3DXHANDLE _defog_handle;
        D3DXHANDLE _hist_handle;

        int _tex_width;
        int _tex_height;
        float _wnd_ratio;
        float _dest_ratio;
        float _alarm_factor;
        bool _is_alarming;
    };

} // namespace XD

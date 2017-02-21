#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d9.h>
#include <hash_map>
#include <memory>
#include <queue>
#include <xpr/xpr_mcvr.h>
#include "autolock.hpp"

// A macro to disallow the copy constructor and operator= functions 
// This should be used in the private:declarations for a class
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);         \
    TypeName& operator=(const TypeName&)
#endif
//常量枚举
enum {
    kTextureLevel = 0,
    kMinBufferCount = 2,
    kMaxBufferCount = 5,
    kMaxChannelCount= 256,
};

class Semaphore;
class SingleEvent;
class LinePainter;
class ChannelAttribute;
struct CUSTOMVERTEX_TEXTURE;
struct D3DSrcTexture;
struct D3DDestTexture
{
    IDirect3DTexture9 *y_texture;
    IDirect3DTexture9 *u_texture;
    IDirect3DTexture9 *v_texture;
    IDirect3DTexture9 *target_tex;

    D3DDestTexture()
        :y_texture(nullptr)
        ,u_texture(nullptr)
        ,v_texture(nullptr)
        ,target_tex(nullptr)
    {
    }
    ~D3DDestTexture()
    {
        if(y_texture) {
            y_texture->Release();
            y_texture = nullptr;
        }
        if(u_texture) {
            u_texture->Release();
            u_texture = nullptr;
        }
        if(v_texture) {
            v_texture->Release();
            v_texture = nullptr;
        }
        if(target_tex) {
            target_tex->Release();
            target_tex = nullptr;
        }
    }
};

typedef enum _FRAME_TRANSFER_ACTION {
    FRAME_TRANSFER_NONE       = -1,
    FRAME_TRANSFER_DISPLAY    = 0,
    FRAME_TRANSFER_DROP       = 1,
    FRAME_TRANSFER_HOLD       = 2,
}FRAME_TRANSFER_ACTION;

typedef enum _CHANNEL_MISSION_TYPE
{
    MAIN_CHANNEL_RESIZE = 0,
    MAIN_CHANNEL_LAYOUT = 1,
    MAIN_CHANNEl_ZOOM   = 2,
    MAIN_CHANNEL_CHOOSE = 3,
    MAIN_CHANNEL_BINDING  = 4,
}CHANNEL_MISSION_TYPE;

typedef struct _MAIN_CHANNEL_MISSION
{
    CHANNEL_MISSION_TYPE type;
    //Resize  - invalid data.
    //Layout  - high word for rows, low word for cols.
    //Zoom    - Source zoom port.
    //Choose  - Chosen sub port.
    //Binding - A new window handle.
    int data;
}MAIN_CHANNEL_MISSION;

namespace XD
{
    class D3DMainChannel;
    class D3DSubChannel
    {
    private:
        DISALLOW_COPY_AND_ASSIGN(D3DSubChannel);
    public:
        explicit D3DSubChannel(D3DMainChannel *main_hannnel, int port, IDirect3DDevice9 *device, int bufs);
        ~D3DSubChannel();
        bool inputData(XPR_AVFrame *av_frame);
        void resetChannel(void);
        XPR_AVPixelFormat getFormat(void) const
        {
            return _pixel_format;
        }
        UINT getParams(void);
        int getWidth(void) const
        {
            return _frame_width;
        }
        int getHeight(void) const
        {
            return _frame_height;
        }
        float getAspectRatio(void) const
        {
            return _aspect_ratio;
        }
        void setRate(float rate);
        void resetPTS(void);
        bool updateDestTexture(D3DDestTexture *dest_texture);
        void takeSnapshot(const char *path);
        void resetSnapshotFlag(void);
        bool getSnapShotFlag(void) const
        {
            return _is_shotting;
        }
        char *getSnapshotPath(void) const
        {
            return _snapshot_path;
        }
        float *getHistFactor(); 
        bool updatePixelTable(IDirect3DTexture9 *dest_table);
        bool setHighPerfromance(void);
        bool setLowConsumption(void);

    private:
        bool createTextures(void);
        D3DSrcTexture *createYUV420PTexture(void);
        bool fillData(XPR_AVFrame *av_frame);
        bool fillYUV420PTexture(D3DSrcTexture *src_texture, XPR_AVFrame *av_frame);
        FRAME_TRANSFER_ACTION getFrameAction(D3DSrcTexture *src_tex);
        void setCameraResolution(const int width, const int height);
        void defogImage(D3DSrcTexture *src_texture);
        bool stretchHist(D3DSrcTexture *src_texture, int value);
        bool equalHist(D3DSrcTexture *src_texture);
        bool bindOperation(int level);
        void releaseOperation(int level);
        void pushToBack(D3DSrcTexture *src_texture);
        D3DSrcTexture *popFromBack(void);
        void pushToFront(D3DSrcTexture *src_texture);
        D3DSrcTexture *popFromFront(void);

    private:
        int _operation_level;
        SectionLock _operation_lock;
        static int _tex_pool_size;
        D3DMainChannel *_main_channnel;
        IDirect3DDevice9 *_device;
        int _channel_port;
        std::queue<D3DSrcTexture*> _back_tex_que;
        std::deque<D3DSrcTexture*> _front_tex_que;
        int _buffer_count;
        SectionLock _front_lock;
        SectionLock _back_lock;
        int _frame_width;
        int _frame_height;
        float _aspect_ratio;
        XPR_AVPixelFormat _pixel_format;

        int _hist_count;
        float _hist_factor[2];
        IDirect3DTexture9 *_pixel_table;
        
        float _renderer_rate;
        DWORD _device_time;
        DWORD _local_time;
        DWORD _base_local_time;
        DWORD _base_device_time;
        int _max_drop_frames;
        int _drop_times;

        char *_snapshot_path;
        bool _is_created;
        bool _is_shotting;
    }; // class D3DSubChannel

    class D3DMainChannel
    {
    private:
        DISALLOW_COPY_AND_ASSIGN(D3DMainChannel);
    public:
        explicit D3DMainChannel(IDirect3DDevice9 *device, void *hwnd, int rows, int cols, int bufs);
        ~D3DMainChannel(void);
        void createLinePainter(HWND hwnd); //主通道创建之后立即被调用
        bool inputData(int sub_port, XPR_AVFrame *av_frame);
        ///主通道状态
        void onLostDevice(void);
        void onResetDevice(void);
        int clickOnWnd(LPARAM lParam);
        bool waitData(DWORD ms);
        void postData(void);
        void postMission(MAIN_CHANNEL_MISSION *mission);
        void excuteMissions(void);
        void resetSubChannels(int port);
        ///主通道渲染
        IDirect3DTexture9 *getTargetTexture(void);
        HWND getHwnd(void);
        int getChannelsCount(void);
        void setSourcePort(int port);
        int getSourcePort(void)
        {
            return _source_port;
        }
        std::shared_ptr<D3DSubChannel> getSubChannel(int sub_port);
        bool setRegionSource(int sub_port);
        bool isAdjusting(int sub_port); //是否调整图像属性
        bool isEnhancing(int sub_port); //是否执行图像增强
        bool isAlarming(int sub_port);  //是否正在报警
        int getDefogFactor(int sub_port);
        int getDefogMode(int sub_port);
        float getRegionRatio(void) const
        {
            return _region_ratio;
        }
        float getZoomRatio(void) const
        {
            return _zoom_ratio;
        }
        RECT getSubRect(int port);
        RECT getFullRect(void);
        RECT getCenterRect(void);
        void drawLines();
        ///性能优化
        bool setHighPerfromance(void);
        bool setLowConsumption(void);
        /// 子通道属性
        //目标通道号
        void setZoomPort(int src_port, int dest_port);
        int getZoomPort(int sub_port);
        //字符显示
        void setChannelState(int sub_port, XPR_MCVR_ChannelState state);
        XPR_MCVR_ChannelState getChannelState(int sub_port);
        void setChannelTitle(int sub_port, char *title);
        char *getChannelTitle(int sub_port);
        //渲染效果
        int setChannelEffect(int sub_port, XPR_MCVR_EffectType effect, int value);
        int setChannelEffectF(int sub_port, XPR_MCVR_EffectType effect, float value);
        int getChannelEffect(int sub_port, XPR_MCVR_EffectType effect);
        float getChannelEffectF(int sub_port, XPR_MCVR_EffectType effect);

    private:
        bool initVertices(void);
        void releaseVertices(void);
        void setChannelWindow(HWND hwnd);
        bool setChannelLayout(int rows, int cols);
        bool fillSectionVertices(CUSTOMVERTEX_TEXTURE *vertices, const int index, const int rows, const int cols);
        void createTargetTexture();

    private:
        IDirect3DDevice9 *_device;
        IDirect3DVertexBuffer9 *_region_vertex;
        std::queue<MAIN_CHANNEL_MISSION> _mission_queue;
        SectionLock _mission_lock;
        int _source_port;
        HWND _channel_hwnd;
        int _rows_count;
        int _cols_count;
        int _buffer_count;
        stdext::hash_map<int, std::shared_ptr<D3DSubChannel>> _channels_map;
        int _current_port;
        IDirect3DTexture9 *_target_texture;
        float _region_ratio;
        float _zoom_ratio;
        LinePainter *_line_painter;
        Semaphore *_data_semaphore;
        char _default_title[8];
        ChannelAttribute *_channel_attributes[kMaxChannelCount+1];
        bool _is_resizing;
    }; // MainChannel

    //判断主通道号/子通道号是否有效
    inline bool isPortValid(int port)
    {
        //1~256为有效通道号
        if((port-1)*(kMaxChannelCount-port) >= 0) {
            return true;
        }
        return false;
    }

} // namespace XD

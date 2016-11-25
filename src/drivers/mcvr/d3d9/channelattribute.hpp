#pragma once

#include <xpr/xpr_mcvr.h>

// A macro to disallow the copy constructor and operator= functions 
// This should be used in the private:declarations for a class
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);         \
    TypeName& operator=(const TypeName&)
#endif

enum {
    kMaxTitleLength = 88,
};

typedef enum _DEFOG_MODE {
    DEFOG_STRETCH_HIST = 0,    //直方图拉伸透雾
    DEFOG_EQUAL_HIST   = 1,    //直方图均衡化透雾
} DEFOG_MODE;

class ChannelAttribute
{
private:
    DISALLOW_COPY_AND_ASSIGN(ChannelAttribute);
public:
    explicit ChannelAttribute(XPR_MCVR_ChannelState state, int port);
    ~ChannelAttribute(void);

    void setZoomPort(int main_port);
    int getZoomPort(void) const
    {
        return _zoom_port;
    }
    void setChannelTitle(const char *title);
    char *getChannelTitle(void);
    bool setChannelState(XPR_MCVR_ChannelState state);
    XPR_MCVR_ChannelState getChannelState(void);
    int setEffectFlag(int value);
    int getEffectFlag(void) const;
    int setPropertyFlag(int value);
    int getPropertyFlag(void) const;
    int setLuminance(int value);
    int getLuminance(void);
    int setContrast(int value);
    int getContrast(void);
    int setGamma(int value);
    int getGamma(void);
    int setHue(int value);
    int getHue(void);
    int setSaturation(int value);
    int getSaturation(void);
    int setEnhancementFlag(int value);
    int getEnhancementFlag(void) const;
    int setSharpeness(int value);
    int getSharpeness(void);
    int setBrightness(int value);
    int getBrightness(void);
    int setDefogMode(DEFOG_MODE mode);
    DEFOG_MODE getDefogMode(void) const;
    int setDefogFactor(int value);
    int getDefogFactor(void);
    int setSpecialsFlag(int value);
    int getSpecialsFlag(void) const;
    int setAlarm(int value);
    int getAlarm(void) const;
    
    bool isAdjusting(void);
    bool isEnhancing(void);
    bool isAlarming(void);

private:
    int _channel_port;
    //放大目标通道号
    //-1     不放大
    //0      所在主通道全窗口放大
    //1~256  其他主通道全窗口
    //0xFFFF 当前主通道中心区域
    int _zoom_port;
    char _channel_title[kMaxTitleLength];
    XPR_MCVR_ChannelState _channel_state;
    //图像属性
    int _luminance_value;
    int _contrast_value;
    int _gamma_value;
    int _hue_value;
    int _saturation_value;
    //图像增强
    float _sharp_value;
    float _bright_value;
    float _defog_value;
    DEFOG_MODE _defog_mode;
    //特殊效果
    int _alarm_flag;
    //效果启用标识
    bool _is_effect_on;
    bool _is_property_on;
    bool _is_enhancement_on;
    bool _is_specials_on;
};
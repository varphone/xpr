#include <string.h>
#include <cstdio>
#include <Windows.h>
#include <xpr/xpr_mcvr.h>
#include "channelattribute.hpp"
//#include "ChannelPlayerHeader.h"

ChannelAttribute::ChannelAttribute(XPR_MCVR_ChannelState state, int port)
    :_channel_port(port)
    ,_zoom_port(-1)
    ,_channel_state(state)
    ,_luminance_value(50)
    ,_contrast_value(50)
    ,_gamma_value(50)
    ,_hue_value(50)
    ,_saturation_value(50)
    ,_sharp_value(0.0f)
    ,_bright_value(0.0f)
    ,_defog_value(0.0f)
    ,_defog_mode(DEFOG_STRETCH_HIST)
    ,_alarm_flag(0)
    ,_is_effect_on(true)
    ,_is_property_on(false)
    ,_is_enhancement_on(true)
    ,_is_specials_on(true)
{
    sprintf_s(_channel_title, kMaxTitleLength, "<%d>", _channel_port);
}

ChannelAttribute::~ChannelAttribute()
{

}

void ChannelAttribute::setZoomPort(int main_port)
{
    _zoom_port = main_port;
}

bool ChannelAttribute::setChannelState(XPR_MCVR_ChannelState state)
{
    if(state == _channel_state) {
        return false;
    }

    _channel_state = state;
    return true;
}

void ChannelAttribute::setChannelTitle(const char *title)
{
    if(title) {
        memcpy_s(_channel_title, kMaxTitleLength, title, strlen(title)+1);
    }
}

XPR_MCVR_ChannelState ChannelAttribute::getChannelState(void)
{
    return _channel_state;
}

char *ChannelAttribute::getChannelTitle()
{
    return _channel_title;
}

int ChannelAttribute::setEffectFlag(int value)
{
    _is_effect_on = value>0 ? true : false;
    return 1;
}

int ChannelAttribute::getEffectFlag(void) const
{
    return _is_effect_on ? 1 : 0;
}

int ChannelAttribute::setPropertyFlag(int value)
{
    _is_property_on = value>0 ? true : false;
    return 1;
}

int ChannelAttribute::getPropertyFlag(void) const
{
    return _is_property_on ? 1 : 0;
}

int ChannelAttribute::setLuminance(int value)
{
    _luminance_value = value;
    return 1;
}

int ChannelAttribute::getLuminance(void)
{
    return _luminance_value;
}

int ChannelAttribute::setContrast(int value)
{
    _contrast_value = value;
    return 1;
}

int ChannelAttribute::getContrast()
{
    return _contrast_value;
}

int ChannelAttribute::setGamma(int value)
{
    _gamma_value = value;
    return 1;
}

int ChannelAttribute::getGamma()
{
    return _gamma_value;
}

int ChannelAttribute::setHue(int value)
{
    _hue_value = value;
    return 1;
}

int ChannelAttribute::getHue()
{
    return _hue_value;
}

int ChannelAttribute::setSaturation(int value)
{
    _saturation_value = value;
    return 1;
}

int ChannelAttribute::getSaturation()
{
    return _saturation_value;
}

int ChannelAttribute::setEnhancementFlag(int value)
{
    _is_enhancement_on = value>1 ? true : false;
    return 1;
}

int ChannelAttribute::getEnhancementFlag()  const
{
    return _is_enhancement_on ? 1 : 0;
}

int ChannelAttribute::setSharpeness(int value)
{
    _sharp_value = static_cast<float>(value) / 100.0f;
    return 1;
}

int ChannelAttribute::getSharpeness()
{
    return static_cast<int>(_sharp_value*100.0f);
}

int ChannelAttribute::setBrightness(int value)
{
    _bright_value = static_cast<float>(value/100.0f);
    return 1;
}

int ChannelAttribute::getBrightness()
{
    return static_cast<int>(_bright_value*100.0f);
}

int ChannelAttribute::setDefogMode(DEFOG_MODE mode)
{
    _defog_mode = mode;
    return 1;
}

DEFOG_MODE ChannelAttribute::getDefogMode() const
{
    return _defog_mode;
}

int ChannelAttribute::setDefogFactor(int value)
{
    _defog_value = static_cast<float>(value) / 100.0f;
    return 1;
}

int ChannelAttribute::getDefogFactor()
{
    return static_cast<int>(_defog_value*100.0f);
}

int ChannelAttribute::setSpecialsFlag(int value)
{
    _is_specials_on = value>0 ? true : false;
    return 1;
}

int ChannelAttribute::getSpecialsFlag() const
{
    return _is_specials_on ? 1 : 0;
}

int ChannelAttribute::setAlarm(int value)
{
    _alarm_flag = value>0 ? 1 : 0;
    return 1;
}

int ChannelAttribute::getAlarm() const
{
    return _alarm_flag;
}

//Reserved.
bool ChannelAttribute::isAdjusting()
{
    if(true == (_is_effect_on&_is_property_on)) {
        return false;
    }

    return false;
}

bool ChannelAttribute::isEnhancing()
{
    if(true == (_is_effect_on&_is_effect_on)) {
        float factor = _sharp_value + _bright_value + _defog_value;
        return factor>0.01f ? true : false;
    }

    return false;
}

bool ChannelAttribute::isAlarming()
{
    return true == (_is_effect_on&_is_specials_on&(_alarm_flag>0));
}


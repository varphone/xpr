#if defined(XPR_MCVR_DRIVER_D3D)
#include "singleevent.hpp"

SingleEvent::SingleEvent(wchar_t *event_name)
    :_event(NULL)
{
    _event = CreateEventW(NULL, FALSE, FALSE, event_name);
}

SingleEvent::~SingleEvent(void)
{
    if(NULL != _event) {
        CloseHandle(_event);
        _event = NULL;
    }
}

void SingleEvent::setEvent(void)
{
    SetEvent(_event);
}

bool SingleEvent::waitEvent(DWORD ms)
{
    DWORD wait_code = WaitForSingleObject(_event, ms);
    if(WAIT_OBJECT_0 == wait_code) {
        return true;
    }

    return false;
}

void SingleEvent::resetEvent()
{
    if(NULL != _event) {
        ResetEvent(_event);
    }
}
#endif // defined(XPR_MCVR_DRIVER_D3D)
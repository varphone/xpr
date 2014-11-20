#pragma once
#include <event.h>

// A macro to disallow the copy constructor and operator= functions 
// This should be used in the private:declarations for a class
#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName&);         \
    TypeName& operator=(const TypeName&)
#endif

class SingleEvent
{
private:
    DISALLOW_COPY_AND_ASSIGN(SingleEvent);
public:
    explicit SingleEvent(wchar_t *event_name);
    ~SingleEvent(void);
    void setEvent(void);
    bool waitEvent(DWORD ms);
    void resetEvent();

private:
    HANDLE _event;
};
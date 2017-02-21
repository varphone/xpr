#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

//信号量封装
class Semaphore
{
public:
    Semaphore(int val = 0);
    ~Semaphore(void);

public:
    void post(void);
    bool wait(DWORD ms);
    void reset(void);

private:
    int fInitial;
    HANDLE fHandle;
};
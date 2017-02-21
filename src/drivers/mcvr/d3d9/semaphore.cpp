#include <cstdio>
#include "semaphore.hpp"

Semaphore::Semaphore(int val)
    : fInitial(val)
{
    fHandle = CreateSemaphore(nullptr, fInitial, 0xFFFF, nullptr);
}

Semaphore::~Semaphore(void)
{
    if (fHandle) {
        CloseHandle(fHandle);
        fHandle = nullptr;
    }
}

void Semaphore::post(void)
{
    ReleaseSemaphore(fHandle, 1, nullptr);
}

bool Semaphore::wait(DWORD ms)
{
    DWORD ret = WaitForSingleObject(fHandle, ms);
    return WAIT_OBJECT_0==ret ? true : false;
}

void Semaphore::reset(void)
{
    if (fHandle) {
        CloseHandle(fHandle);
        fHandle = nullptr;
    }
    fHandle = CreateSemaphore(nullptr, fInitial, 0xFFFF, nullptr);
}
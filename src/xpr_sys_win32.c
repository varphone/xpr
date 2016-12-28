#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#if defined(WIN32) || defined(WIN64)
#include <Windows.h>
#endif // defined(WIN32) || defined(WIN64)
#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_sys.h>

// FIXME
XPR_API void XPR_SYS_Poweroff(void)
{
}

// FIXME
XPR_API void XPR_SYS_Reboot(void)
{
}

// FIXME
XPR_API void XPR_SYS_DelayedReboot(int secs)
{
}

// FIXME
XPR_API int XPR_SYS_EnableThreadRealtimeSchedule(int64_t tid)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

// FIXME
XPR_API int XPR_SYS_EnableProcessRealtimeSchedule(int64_t pid)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}

// FIXME
XPR_API int XPR_SYS_SetAudioClockFrequency(int freq)
{
	return XPR_ERR_GEN_NOT_SUPPORT;
}


typedef NTSTATUS(NTAPI* NtQueryPerformanceCounterProc)(
	PLARGE_INTEGER PerformanceCounter,
	PLARGE_INTEGER PerformanceFrequency);

static NtQueryPerformanceCounterProc ntqpcp = 0;

XPR_API int64_t XPR_SYS_GetCTS(void)
{
	LARGE_INTEGER ticks;
	LARGE_INTEGER freq;
	if (!ntqpcp) {
		ntqpcp = (NtQueryPerformanceCounterProc)GetProcAddress(
				 GetModuleHandleA("ntdll.dll"), "NtQueryPerformanceCounter");
	}
	if (ntqpcp) {
		ntqpcp(&ticks, &freq);
	}
	else {
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&ticks);
	}
	return ticks.QuadPart / (freq.QuadPart / 1000000);
}

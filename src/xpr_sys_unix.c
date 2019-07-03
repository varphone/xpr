#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <xpr/xpr_sys.h>

void XPR_SYS_Poweroff(void)
{
    reboot(RB_POWER_OFF);
}

void XPR_SYS_Reboot(void)
{
    reboot(RB_AUTOBOOT);
}

void* DelayedReboot(void* arg)
{
    int secs = (int)(long)arg;
    pthread_detach(pthread_self());
    sleep(secs);
    reboot(RB_AUTOBOOT);
    return (void*)0;
}

void XPR_SYS_DelayedReboot(int secs)
{
    pthread_t tid = 0;
    if (secs > 0) {
        pthread_create(&tid, NULL, DelayedReboot, (void*)(long)secs);
    }
    else {
        reboot(RB_AUTOBOOT);
    }
}

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
XPR_API int XPR_SYS_EnableThreadRealtimeSchedule(int64_t tid)
{
    struct sched_param param;
    int policy = SCHED_RR;
    int priority = 90;
    if (!tid)
        return -1;
    memset(&param, 0, sizeof(param));
    param.sched_priority = priority;
    if (pthread_setschedparam(tid, policy, &param) < 0)
        perror("pthread_setschedparam()");
    pthread_getschedparam(tid, &policy, &param);
    if (param.sched_priority != priority)
        return -1;
    return 0;
}
#else
#pragma message                                                                \
    "HAVE_PTHREAD_H not defined, XPR_SYS_EnableThreadRealtimeSchedule() will not work."
XPR_API int XPR_SYS_EnableThreadRealtimeSchedule(int64_t tid)
{
    return 0;
}
#endif // HAVE_PHTREAD_H

#ifdef HAVE_SCHED_H
#include <sched.h>
XPR_API int XPR_SYS_EnableProcessRealtimeSchedule(int64_t pid)
{
    struct sched_param param;
    int maxpri = sched_get_priority_max(SCHED_FIFO);
    if (maxpri < 0) {
        perror("sched_get_priority_max()");
        return -1;
    }
    param.sched_priority = maxpri;
    if (sched_setscheduler(pid, SCHED_FIFO, &param) < 0) {
        perror("sched_setscheduler");
        return -1;
    }
    return 0;
}
#else
#pragma message                                                                \
    "HAVE_SCHED_H not defined, XPR_SYS_EnableProcessRealtimeSchedule() will not work."
XPR_API int XPR_SYS_EnableProcessRealtimeSchedule(int64_t pid)
{
    return 0;
}
#endif // HAVE_SCHED_H

#if defined(BOARD_MAJOR_A5S)
#include <ambarella/a5s/amba_api.h>
XPR_API int XPR_SYS_SetAudioClockFrequency(int freq)
{
    int fd = open("/dev/iav", O_RDWR);
    if (ioctl(fd, IAV_IOC_SET_AUDIO_CLK_FREQ_EX, freq) < 0) {
        fprintf(stderr, "ioctl(IAV_IOC_SET_AUDIO_CLK_FREQ_EX) error: %d\n",
                errno);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}
#else
XPR_API int XPR_SYS_SetAudioClockFrequency(int freq)
{
    return 0;
}
#endif // defined(BOARD_MAJOR_A5S)

#if defined(HAVE_TIME_H)
#include <time.h>
XPR_API int64_t XPR_SYS_GetCTS(void)
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (int64_t)tp.tv_sec * 1000000 + (int64_t)tp.tv_nsec / 1000;
}
#else
#pragma message "HAVE_TIME_H not defined, XPR_SYS_GetCTS() will not work."
XPR_API int64_t XPR_SYS_GetCTS(void)
{
    return 0;
}
#endif // defined(HAVE_TIME_H)

XPR_API int XPR_SYS_WaitKey(int timeout)
{
    fd_set rfds;
    struct timeval tv;
    FD_ZERO(&rfds);
    tv.tv_sec = timeout / XPR_SYS_CTS_UNIT;
    tv.tv_usec = timeout % XPR_SYS_CTS_UNIT;
    FD_SET(STDIN_FILENO, &rfds);
    int n = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
    return n == 0 ? 0 : 1;
}

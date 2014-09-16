#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void XPR_SYS_Poweroff(void)
{
    printf("XPR_SYS_Poweroff()\n");
}

void XPR_SYS_Reboot(void)
{
    printf("XPR_SYS_Reboot()\n");
}

#ifdef HAVE_SCHED_H
#include <sched.h>
#ifdef HAVE_PHTREAD_H
#include <pthread.h>
int XPR_SYS_EnableThreadRealtimeSchedule(int64_t tid)
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
int XPR_SYS_EnableThreadRealtimeSchedule(int64_t tid)
{
    return 0;
}
#endif
int XPR_SYS_EnableProcessRealtimeSchedule(int64_t pid)
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
int XPR_SYS_EnableProcessRealtimeSchedule(int64_t pid)
{
    return 0;
}
#endif

#if defined(BOARD_MAJOR_A5S)
#include <ambarella/a5s/amba_api.h>
int XPR_SYS_SetAudioClockFrequency(int freq)
{
    int fd = open("/dev/iav", O_RDWR);

    if (ioctl(fd, IAV_IOC_SET_AUDIO_CLK_FREQ_EX, freq) < 0) {
        fprintf(stderr, "ioctl(IAV_IOC_SET_AUDIO_CLK_FREQ_EX) error: %d\n", errno);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}
#else
int XPR_SYS_SetAudioClockFrequency(int freq)
{
    return 0;
}
#endif


#include <pthread.h>
#include <unistd.h>
#include <sys/reboot.h>
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
    int secs = (int)arg;
    pthread_detach(pthread_self());
    sleep(secs);
    reboot(RB_AUTOBOOT);
    return (void*)0;
}

void XPR_SYS_DelayedReboot(int secs)
{
    pthread_t tid = 0;
    if (secs > 0) {
        pthread_create(&tid, NULL, DelayedReboot, (void*)secs);
    }
    else {
        reboot(RB_AUTOBOOT);
    }
}

#include "Platform.h"

#ifdef __PLATFORM_LINUX__
//#include <atomic.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

void Sleep(long long nMiniSecods)
{
#if 0
    struct timeval delay;
    delay.tv_sec = 0;
    delay.tv_usec = nMiniSecods * 1000; // 20 ms
    select(0, NULL, NULL, NULL, &delay);
#endif
    if (nMiniSecods > 1800*1000)
    {
        sleep(nMiniSecods/1000);
    }
    else
    {
        usleep(nMiniSecods*1000);
    }
}

// static atomic_t atomic_number;

#else
#include <windows.h>


void gettimeofday(struct timeval* tp)
{
#ifdef NGX_WIN32

    DWORD dt = timeGetTime(); // 错误代码项  
    tp->tv_sec = (long) (dt / 1000);  
    tp->tv_usec = (long) (dt*1000);  
#else  
    ULONGLONG   usec;  
    FILETIME    ft;  
    SYSTEMTIME  st;  

    GetSystemTime(&st);  
    SystemTimeToFileTime(&st, &ft);  

    usec = ft.dwHighDateTime;  
    usec <<= 32;  
    usec |= ft.dwLowDateTime;  
    usec /= 10;  
    usec -= 11644473600000000LL;  

    tp->tv_sec = (long) (usec / 1000000);  
    tp->tv_usec = (long) (usec % 1000000); 
#endif // NGX_WIN32  
}


#endif

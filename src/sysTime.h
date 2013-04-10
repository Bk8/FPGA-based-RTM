#ifndef SYSTIME_H

#define SYSTIME_H

#include <sys/time.h>

class SysTime
{
public:
    SysTime();
    void start();
    float  stop();
    float elapsedS();
    float elapsedMS();
    float elapsedUS();
private:
    struct timeval mStart;
    struct timeval mStop;
};

#endif /* end of include guard: SYSTIME_H */

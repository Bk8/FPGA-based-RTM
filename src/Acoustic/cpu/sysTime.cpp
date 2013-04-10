#include "sysTime.h"
#include <cstddef>
using namespace std;
SysTime::SysTime()
{

}

void SysTime::start()
{
    gettimeofday(&mStart, NULL);
}

float SysTime::stop()
{
    gettimeofday(&mStop, NULL);
    return elapsedMS();
}

float SysTime::elapsedS()
{
    float sec = mStop.tv_sec - mStart.tv_sec;
    float msec = 1.0 * (mStop.tv_usec - mStart.tv_usec) / 1000000.0f;
    return sec + msec;
}

float SysTime::elapsedMS()
{
    float msec = 1.0 * (mStop.tv_sec - mStart.tv_sec) * 1000.0f;
    float usec = 1.0 * (mStop.tv_usec - mStart.tv_usec) / 1000.0f;
    return msec + usec;
}

float SysTime::elapsedUS()
{
    float usec1 = (mStop.tv_sec - mStart.tv_sec) * 1000000.0f;
    float usec2 = (mStop.tv_usec - mStart.tv_usec);
    return usec1 + usec2;
}

#include "time.h"
#ifdef _WIN32
#include <winbase.h>
#elif __linux__
#include <sys/time.h>
#endif

#define IsLeapYear(y) ((!(y % 4)) ? (((y % 400) && !(y % 100)) ? 0 : 1) : 0)

time_t z_time_now()
{
	return time(NULL);
}

void z_sleep(int second)
{
#ifdef _WIN32
	SleepEx(((DWORD)(sec)) * ((DWORD)1000), TRUE);
#elif __linux__
	sleep(second);
#endif
}

#ifdef _WIN32
static void SystemTimeToExpTime(z_time_exp_t *xt, SYSTEMTIME *tm)
{
	static const int dayoffset[12] =
	{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

	/* Note; the caller is responsible for filling in detailed tm_usec,
	 * tm_gmtoff and tm_isdst data when applicable.
	 */
	xt->tm_usec = tm->wMilliseconds * 1000;
	xt->tm_sec  = tm->wSecond;
	xt->tm_min  = tm->wMinute;
	xt->tm_hour = tm->wHour;
	xt->tm_mday = tm->wDay;
	xt->tm_mon  = tm->wMonth - 1;
	xt->tm_year = tm->wYear - 1900;
	xt->tm_wday = tm->wDayOfWeek;
	xt->tm_yday = dayoffset[xt->tm_mon] + (tm->wDay - 1);
	xt->tm_isdst = 0;
	xt->tm_gmtoff = 0;

	/* If this is a leap year, and we're past the 28th of Feb. (the
	 * 58th day after Jan. 1), we'll increment our tm_yday by one.
	 */
	if (IsLeapYear(tm->wYear) && (xt->tm_yday > 58))
		xt->tm_yday++;
}

#elif __linux__

z_time_t z_time_now2(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * Z_USEC_PER_SEC + tv.tv_usec;
}

static void explode_time(struct z_time_exp_t *xt,
	z_time_t t,
	int offset,
	int use_localtime)
{
	struct tm tm;
	time_t tt = (t / Z_USEC_PER_SEC) + offset;
	xt->tm_usec = t % Z_USEC_PER_SEC;

#if Z_HAS_THREADS && defined (_POSIX_THREAD_SAFE_FUNCTIONS)
	if (use_localtime)
		localtime_r(&tt, &tm);
	else
		gmtime_r(&tt, &tm);
#else
	if (use_localtime)
		tm = *localtime(&tt);
	else
		tm = *gmtime(&tt);
#endif

	xt->tm_sec  = tm.tm_sec;
	xt->tm_min  = tm.tm_min;
	xt->tm_hour = tm.tm_hour;
	xt->tm_mday = tm.tm_mday;
	xt->tm_mon  = tm.tm_mon;
	xt->tm_year = tm.tm_year;
	xt->tm_wday = tm.tm_wday;
	xt->tm_yday = tm.tm_yday;
	xt->tm_isdst = tm.tm_isdst;
	xt->tm_gmtoff = 0;
}
#endif


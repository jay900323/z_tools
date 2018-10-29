#ifndef _Z_TIME_H
#define _Z_TIME_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include <time.h>
#include "common.h"

#define Z_TIME_C(val)  (INT64)val
#define Z_USEC_PER_SEC Z_TIME_C(1000000)

struct z_time_exp_t {
	/** microseconds past tm_sec */
	int tm_usec;
	/** (0-61) seconds past tm_min */
	int tm_sec;
	/** (0-59) minutes past tm_hour */
	int tm_min;
	/** (0-23) hours past midnight */
	int tm_hour;
	/** (1-31) day of the month */
	int tm_mday;
	/** (0-11) month of the year */
	int tm_mon;
	/** year since 1900 */
	int tm_year;
	/** (0-6) days since Sunday */
	int tm_wday;
	/** (0-365) days since January 1 */
	int tm_yday;
	/** daylight saving time */
	int tm_isdst;
	/** seconds east of UTC */
	int tm_gmtoff;
};

/*精确到微妙的时间*/
typedef INT64 z_time_t;
	
time_t z_time_now();
void z_sleep(int second);

#ifdef __cplusplus
}
#endif

#endif

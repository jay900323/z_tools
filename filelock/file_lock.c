#include "file_lock.h"
#include <sys/file.h>

int z_checkexit(const char* pfile)
{
	if (pfile == NULL)
	{   
		return -1; 
	}   
	int lockfd = open(pfile, O_RDWR);
	if (lockfd == -1) 
	{   
		return -2; 
	}   
	int iret = flock(lockfd, LOCK_EX | LOCK_NB);
	if (iret == -1) 
	{  
		return -3; 
	}   

	return lockfd;
}
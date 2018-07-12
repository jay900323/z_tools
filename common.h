#ifndef _Z_COMMON_H
#define _Z_COMMON_H

#include <stdio.h>
#ifdef _WINDOWS
#include <windows.h>
#else
#include <stddef.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#endif

#define		SUCCEED		0
#define		FAIL		-1
#define		NOTSUPPORTED	-2
#define		NETWORK_ERROR	-3
#define		TIMEOUT_ERROR	-4
#define		CONFIG_ERROR	-5


#define SUCCEED_OR_FAIL(result) (FAIL != (result) ? SUCCEED : FAIL)

#define Z_KIBIBYTE		1024
#define MAX_ID_LEN		64
#define MAX_FILEPATH_LEN    256	
#define MAX_STRING_LEN		2048	
#define MAX_BUFFER_LEN		65536
#define MAX_ZBX_HOSTNAME_LEN	128
#define MAX_EXECUTE_OUTPUT_LEN	(512 * Z_KIBIBYTE)


#ifndef MAX
#	define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#	define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define z_free(ptr)		\
				\
do				\
{				\
	if (ptr)		\
	{			\
		free(ptr);	\
		ptr = NULL;	\
	}			\
}				\
while (0)

#define z_fclose(file)	\
				\
do				\
{				\
	if (file)		\
	{			\
		fclose(file);	\
		file = NULL;	\
	}			\
}				\
while (0)

#define Z_MIN_PORT 1024u
#define Z_MAX_PORT 65535u

#define ARRSIZE(a)	(sizeof(a) / sizeof(*a))


#endif

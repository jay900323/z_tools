#ifndef ZABBIX_THREADS_H
#define ZABBIX_THREADS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

#if defined(_WINDOWS)
#include <Windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

#if defined(_WINDOWS)

	void	CALLBACK Z_EndThread(ULONG_PTR dwParam);

#define Z_THREAD_ERROR	-1

#define Z_THREAD_HANDLE	HANDLE
#define Z_THREAD_HANDLE_NULL	NULL

#define Z_THREAD_ENTRY_POINTER(pointer_name) \
		unsigned (__stdcall *pointer_name)(void *)

#define Z_THREAD_ENTRY(entry_name, arg_name)	\
		unsigned __stdcall entry_name(void *arg_name)

#define z_thread_exit(status) \
		_endthreadex((unsigned int)(status)); \
		return ((unsigned)(status))

#define z_sleep(sec) SleepEx(((DWORD)(sec)) * ((DWORD)1000), TRUE)

#define z_thread_kill(h) QueueUserAPC(Z_EndThread, h, 0);

#else	/* not _WINDOWS */

#define Z_THREAD_ERROR	-1

#define Z_THREAD_HANDLE	pthread_t
#define Z_THREAD_HANDLE_NULL	0

#define Z_THREAD_ENTRY_POINTER(pointer_name) \
		void *(* pointer_name)(void *)

#define Z_THREAD_ENTRY(entry_name, arg_name)	\
		void * entry_name(void *arg_name)

	/* Calling _exit() to terminate child process immediately is important. See ZBX-5732 for details. */
#define z_thread_exit(status) \
		pthread_exit((int)(status)); \
		return ((unsigned)(status))

#define z_sleep(sec) sleep((sec))

#define z_thread_kill(h) pthread_kill(h, SIGQUIT);

#endif	/* _WINDOWS */

typedef struct
{
	void		*args;
	Z_THREAD_ENTRY_POINTER(entry);
}zbx_thread_args_t;

Z_THREAD_HANDLE	z_thread_start(zbx_thread_args_t *thread_args);
int			z_thread_wait(Z_THREAD_HANDLE thread);
long int		z_get_thread_id();
int			z_thread_close(Z_THREAD_HANDLE thread);

#ifdef __cplusplus
}
#endif

#endif	/* ZABBIX_THREADS_H */

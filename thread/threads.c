#include "threads.h"

#if defined  (_WINDOWS)
#include  <excpt.h>
#endif

#if !defined(_WINDOWS)

static Z_THREAD_ENTRY(z_linux_thread_entry, args)
{
	zbx_thread_args_t	*thread_args = (zbx_thread_args_t *)args;

	return thread_args->entry(thread_args->args);
}

#else

static Z_THREAD_ENTRY(zbx_win_thread_entry, args)
{
	__try
	{
		zbx_thread_args_t	*thread_args = (zbx_thread_args_t *)args;

		return thread_args->entry(thread_args->args);
	}
	__except(EXCEPTION_CONTINUE_SEARCH)
	{
		z_thread_exit(EXIT_SUCCESS);
	}
}
#endif

Z_THREAD_HANDLE	z_thread_start(zbx_thread_args_t *thread_args)
{
	Z_THREAD_HANDLE	thread = Z_THREAD_HANDLE_NULL;

#ifdef _WINDOWS
	unsigned		thrdaddr;

	//thread_args->entry = handler;
	/* NOTE: _beginthreadex returns 0 on failure, rather than 1 */
	if (0 == (thread = (Z_THREAD_HANDLE)_beginthreadex(NULL, 0, zbx_win_thread_entry, thread_args, 0, &thrdaddr)))
	{
		//zlog_error("failed to create a thread: %s", strerror_from_system(GetLastError()));
		thread = (Z_THREAD_HANDLE)Z_THREAD_ERROR;
	}
#else
	//thread_args->entry = handler;
	if(0 != pthread_create(&thread, NULL, z_linux_thread_entry, thread_args)){
		//zlog_error("failed to create a thread: %s", strerror(errno));
		thread = (Z_THREAD_HANDLE)Z_THREAD_ERROR;
	}
#endif
	return thread;
}

Z_THREAD_HANDLE	z_thread_start2(Z_THREAD_ENTRY_POINTER(entry), void *args)
{
	
}

int	z_thread_wait(Z_THREAD_HANDLE thread)
{
	int	status = 0;	/* significant 8 bits of the status */

#ifdef _WINDOWS

	if (WAIT_OBJECT_0 != WaitForSingleObject(thread, INFINITE))
	{
		//zlog_error("Error on thread waiting. [%s]", strerror_from_system(GetLastError()));
		return Z_THREAD_ERROR;
	}

	if (0 == GetExitCodeThread(thread, &status))
	{
		//zlog_error("Error on thread exit code receiving. [%s]", strerror_from_system(GetLastError()));
		return Z_THREAD_ERROR;
	}

	if (0 == CloseHandle(thread))
	{
		//zlog_error("Error on thread closing. [%s]", strerror_from_system(GetLastError()));
		return Z_THREAD_ERROR;
	}

#else	/* not _WINDOWS */

	if(0 != pthread_join(thread, NULL))
	{
		return Z_THREAD_ERROR;
	}
#endif	/* _WINDOWS */

	return status;
}

long int		z_get_thread_id()
{
#ifdef _WINDOWS
	return (long int)GetCurrentThreadId();
#else
	return (long int)pthread_self();
#endif
}

int	z_thread_close(Z_THREAD_HANDLE thread)
{
#ifdef _WINDOWS
	if(thread){
		if(!CloseHandle(thread)){
			return -1;
		}
	}
#endif
	return 0;
}

#ifdef _WINDOWS
void	CALLBACK Z_EndThread(ULONG_PTR dwParam)
{
	_endthreadex(SUCCEED);
}
#endif
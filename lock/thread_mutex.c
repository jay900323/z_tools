#include "thread_mutex.h"

int z_thread_mutex_create(z_thread_mutex_t *mutex, unsigned int flags)
{
	int rv = 0;

	if(mutex == NULL){
		return -1;
	}
#ifdef _WINDOWS
	if (flags & Z_THREAD_MUTEX_UNNESTED) {
		mutex->type = thread_mutex_unnested_event;
		mutex->handle = CreateEvent(NULL, FALSE, TRUE, NULL);
	}
	else{
#ifdef _UNICODE
			mutex->type = thread_mutex_critical_section;
			InitializeCriticalSection(&(mutex->section));
#else
			mutex->type = thread_mutex_nested_mutex;
			mutex->handle = CreateMutex(NULL, FALSE, NULL);
#endif
	}

#else

	if (flags & Z_THREAD_MUTEX_NESTED) {
		pthread_mutexattr_t mattr;

		rv = pthread_mutexattr_init(&mattr);
		if (rv) return rv;

		rv = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
		if (rv) {
			pthread_mutexattr_destroy(&mattr);
			return rv;
		}

		rv = pthread_mutex_init(&mutex->mutex, &mattr);
		pthread_mutexattr_destroy(&mattr);
	} else
		rv = pthread_mutex_init(&mutex->mutex, NULL);

	if (rv) {
		return errno;
	}
#endif

	return 0;
}

int z_thread_mutex_lock(z_thread_mutex_t *mutex)
{
#ifdef _WINDOWS
	if (mutex->type == thread_mutex_critical_section) {
		EnterCriticalSection(&mutex->section);
	}
	else {
		DWORD rv = WaitForSingleObject(mutex->handle, INFINITE);
		if ((rv != WAIT_OBJECT_0) && (rv != WAIT_ABANDONED)) {
			return (rv == WAIT_TIMEOUT) ? Z_MUTEX_BUSY : Z_MUTEX_ERROR;
		}
	}

	return 0;
#else
	int rv;

	rv = pthread_mutex_lock(&mutex->mutex);
	if (rv) {
		rv = errno;
	}
	return rv;
#endif
}

int z_thread_mutex_trylock(z_thread_mutex_t *mutex)
{
#ifdef _WINDOWS
	if (mutex->type == thread_mutex_critical_section) {
		if (!TryEnterCriticalSection(&mutex->section)) {
			return Z_MUTEX_BUSY;
		}
	}
	else {
		DWORD rv = WaitForSingleObject(mutex->handle, 0);
		if ((rv != WAIT_OBJECT_0) && (rv != WAIT_ABANDONED)) {
			return (rv == WAIT_TIMEOUT) ? Z_MUTEX_BUSY : Z_MUTEX_ERROR;
		}
	} 

#else
	int rv;
	rv = pthread_mutex_trylock(&mutex->mutex);
	if (rv) {
		rv = errno;
		return (rv == EBUSY) ? Z_MUTEX_BUSY : rv;
	}
	return rv;
#endif
}

int z_thread_mutex_unlock(z_thread_mutex_t *mutex)
{
#ifdef _WINDOWS
	if (mutex->type == thread_mutex_critical_section) {
		LeaveCriticalSection(&mutex->section);
	}
	else if (mutex->type == thread_mutex_unnested_event) {
		if (!SetEvent(mutex->handle)) {
			return Z_MUTEX_ERROR;
		}
	}
	else if (mutex->type == thread_mutex_nested_mutex) {
		if (!ReleaseMutex(mutex->handle)) {
			return Z_MUTEX_ERROR;
		}
	}else{
		return Z_MUTEX_ERROR;
	}
	return 0;

#else
	int rv;

	rv = pthread_mutex_unlock(&mutex->mutex);
	if (rv) {
		rv = errno;
	}
	return rv;
#endif
}

int z_thread_mutex_destroy(z_thread_mutex_t *mutex)
{
#ifdef _WINDOWS
	if (mutex->type == thread_mutex_critical_section) {
		DeleteCriticalSection(&mutex->section);
	}
	else if (mutex->type == thread_mutex_unnested_event) {
		if (!CloseHandle(mutex->handle)) {
			return Z_MUTEX_ERROR;
		}
	}
	else if (mutex->type == thread_mutex_nested_mutex) {
		if (!CloseHandle(mutex->handle)) {
			return Z_MUTEX_ERROR;
		}
	}
	return 0;
#else
	int rv;
	rv = pthread_mutex_destroy(&mutex->mutex);
	if (rv) {
		rv = errno;
	}
	return rv;
#endif
}
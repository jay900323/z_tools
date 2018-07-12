#ifndef _THREAD_LOCK_H
#define _THREAD_LOCK_H

#include "common.h"


#define Z_MUTEX_BUSY  1
#define Z_MUTEX_ERROR  -1

#define Z_THREAD_MUTEX_NESTED  1  /*The same thread allows nested use*/
#define Z_THREAD_MUTEX_UNNESTED  2

#ifdef _WIN32
typedef enum thread_mutex_type {
	thread_mutex_critical_section,
	thread_mutex_unnested_event,
	thread_mutex_nested_mutex
} thread_mutex_type;

typedef struct  
{
	thread_mutex_type type;
	HANDLE handle;
	CRITICAL_SECTION  section;
}z_thread_mutex_t;

#else

typedef struct  
{
	pthread_mutex_t mutex;
}z_thread_mutex_t;
#endif

int z_thread_mutex_create(z_thread_mutex_t *mutex, unsigned int flags);
int z_thread_mutex_lock(z_thread_mutex_t *mutex);
int z_thread_mutex_trylock(z_thread_mutex_t *mutex);
int z_thread_mutex_unlock(z_thread_mutex_t *mutex);
int z_thread_mutex_destroy(z_thread_mutex_t *mutex);

#endif
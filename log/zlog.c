#include "zlog.h"
#include "pthread.h"

#ifdef _WIN32
#include <stdarg.h>
#include <Windows.h>
#endif

#ifdef _WIN32
static HANDLE		system_log_handle = INVALID_HANDLE_VALUE;
#endif

static char		syslog_app_name[256] = {"fetchxml_log"};
static char		log_filename[256];
static int		log_type = LOG_TYPE_UNDEFINED;
static int		log_level = ZLOG_LEVEL_WARN;
static int backup_file;
static pthread_mutex_t	log_access;
#define LOCK_LOG	pthread_mutex_lock(&log_access)
#define UNLOCK_LOG	pthread_mutex_unlock(&log_access)



#ifndef _WIN32
static sigset_t	orig_mask;

static void	lock_log(void)
{
	sigset_t	mask;

	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	sigaddset(&mask, SIGTERM);	/* block SIGTERM, SIGINT to prevent deadlock on log file mutex */
	sigaddset(&mask, SIGINT);

	if (0 > sigprocmask(SIG_BLOCK, &mask, &orig_mask))
		zbx_error("cannot set sigprocmask to block the user signal");

	LOCK_LOG;
}

static void	unlock_log(void)
{
	UNLOCK_LOG;

	if (0 > sigprocmask(SIG_SETMASK, &orig_mask, NULL))
		zbx_error("cannot restore sigprocmask");
}
#else
static void	lock_log(void)
{
	LOCK_LOG;
}

static void	unlock_log(void)
{
	UNLOCK_LOG;
}
#endif

void	zbx_handle_log(void)
{
	if (LOG_TYPE_FILE != log_type)
		return;

	lock_log();

	rotate_log(log_filename);

	unlock_log();
}

int	zlog_open_log(int type, int level, const char *filename)
{
	FILE	*log_file = NULL;
	log_type = type;
	log_level = level;

	if (LOG_TYPE_SYSTEM == type)
	{
#ifdef __linux__
		openlog(syslog_app_name, LOG_PID, LOG_DAEMON);
#endif
	}
	else if (LOG_TYPE_FILE == type)
	{
		if (MAX_STRING_LEN <= strlen(filename))
		{
			printf("too long path for logfile");
			return -1;
		}

		if (0 != pthread_mutex_init(&log_access, NULL))
		{
			printf("unable to create mutex for log file");
			return -1;
		}

		if (NULL == (log_file = fopen(filename, "a+")))
		{
			printf("unable to open log file [%s]", filename);
			return -1;
		}
		fclose(log_file);
		strcpy(log_filename, filename);
	}
	else if (LOG_TYPE_CONSOLE == type)
	{
		if (0 != pthread_mutex_init(&log_access, NULL))
		{
			printf("unable to create mutex for standard output");
			return -1;
		}

		//fflush(stderr);
		//if (-1 == dup2(STDOUT_FILENO, STDERR_FILENO))
		//printf("cannot redirect stderr to stdout.");
	}

	return 0;
}

void	zabbix_close_log(void)
{
	if (LOG_TYPE_SYSTEM == log_type)
	{
#ifdef __linux__
		closelog();
#endif
	}
	else if (LOG_TYPE_FILE == log_type || LOG_TYPE_CONSOLE == log_type)
	{
		pthread_mutex_destroy(&log_access);
	}
}

void	zabbix_set_log_level(int level)
{
	log_level = level;
}

int t_filesize(const char *filename)
{
	FILE *fp = 0;
	int file_len = 0;

	if((fp=fopen(filename, "rb"))== NULL){
		return 0;
	}
	fseek(fp, 0L, SEEK_END);
	file_len = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	fclose(fp);

	return file_len;
}

const char* get_log_level(int log_level)
{
	switch (log_level)
	{
	case ZLOG_LEVEL_DEBUG:
		return "DEBUG";
	case ZLOG_LEVEL_INFO:
		return "INFO";
	case ZLOG_LEVEL_NOTICE:
		return "NOTICE";
	case ZLOG_LEVEL_WARN:
		return "WARN";
	case ZLOG_LEVEL_ERROR:
		return "ERROR";
	case ZLOG_LEVEL_FATAL:
		return "FATAL";
	default:
		return "UNKNOWN";
	}
}

static void	rotate_log(const char *filename)
{
	int		new_size;

	if (0 == CONFIG_LOG_FILE_SIZE || NULL == filename || '\0' == *filename)
	{
		return;
	}

	new_size = t_filesize(filename);
	
	if (CONFIG_LOG_FILE_SIZE < new_size)
	{
		char	filename_old[MAX_STRING_LEN];

		strcpy(filename_old, filename);
		strcat(filename_old, ".old");
		remove(filename_old);

		if (0 != rename(filename, filename_old))
		{
			FILE	*log_file = NULL;

			if (NULL != (log_file = fopen(filename, "w")))
			{
				time_t t = time(0);   
				char tmp[64];   

				strftime( tmp, sizeof(tmp), "%Y/%m/%d %H:%M:%S",localtime(&t) );

				fprintf(log_file, "%s %s (%s:%d) %s\n", tmp, get_log_level(ZLOG_LEVEL_ERROR), __FILE__, __LINE__, "文件重命名失败.");
				fclose(log_file);
				
			}
		}
	}
}

void zlog(const char *file, long line, int level,const char *format, ...)
{
	FILE		*log_file = NULL;
	char		message[MAX_BUFFER_LEN];
	va_list		args;
#ifdef _WIN32
	WORD		wType;
	wchar_t		thread_id[20], *strings[2];
#endif

	if (LOG_TYPE_FILE == log_type)
	{
		lock_log();

		rotate_log(log_filename);

		if (NULL != (log_file = fopen(log_filename, "a+")))
		{
			time_t t = time(0);   
			char tmp[64];   
			va_list ap;

			strftime( tmp, sizeof(tmp), "%Y/%m/%d %H:%M:%S",localtime(&t) );
			/* 时间日期 等级 (文件名:行号)*/
			va_start(ap, format);
			vsnprintf (message, 4096, format, ap);
			va_end(ap);

			fprintf(log_file, "%s %s (%s:%d) %s\n", tmp, get_log_level(level), file, line, message);
			fprintf(stdout, "%s %s (%s:%d) %s\n", tmp, get_log_level(level), file, line, message);
			fclose(log_file);
		}

		unlock_log();

		return;
	}

	if (LOG_TYPE_CONSOLE == log_type)
	{
		lock_log();

		time_t t = time(0);   
		char tmp[64];   
		char buf[4096] = {0};
		va_list ap;

		strftime( tmp, sizeof(tmp), "%Y/%m/%d %H:%M:%S",localtime(&t) );
		/* 时间日期 等级 (文件名:行号)*/
		va_start(ap, format);
		vsnprintf (buf, 4096, format, ap);
		va_end(ap);

		fprintf(stdout, "%s %s (%s:%d) %s\n", tmp, get_log_level(level), file, line, buf);

		unlock_log();

		return;
	}

	if (LOG_TYPE_SYSTEM == log_type)
	{
		va_list ap;

		va_start(ap, format);
		vsnprintf (message, MAX_BUFFER_LEN, format, ap);
		va_end(ap);

#ifdef __linux__	 /* not _WIN32 */

		/* for nice printing into syslog */
		switch (level)
		{
		case ZLOG_LEVEL_FATAL:
			syslog(LOG_CRIT, "%s", message);
			break;
		case ZLOG_LEVEL_ERROR:
			syslog(LOG_ERR, "%s", message);
			break;
		case ZLOG_LEVEL_WARN:
			syslog(LOG_WARNING, "%s", message);
			break;
		case ZLOG_LEVEL_DEBUG:
			syslog(LOG_DEBUG, "%s", message);
			break;
		case ZLOG_LEVEL_INFO:
			syslog(LOG_INFO, "%s", message);
			break;
		default:
			/* LOG_LEVEL_EMPTY - print nothing */
			break;
		}

#endif	/* _WIN32 */
	}	/* LOG_TYPE_SYSLOG */
}


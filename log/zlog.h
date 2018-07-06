#ifndef _ZLOG_H
#define _ZLOG_H

#ifdef _WIN32
#include <stdio.h>

#define ZLOG_LEVEL_DEBUG 0
#define ZLOG_LEVEL_INFO 1
#define ZLOG_LEVEL_NOTICE 2
#define ZLOG_LEVEL_WARN 3
#define ZLOG_LEVEL_ERROR 4
#define ZLOG_LEVEL_FATAL 5

#define LOG_LEVEL_INFORMATION	127	/* printing in any case no matter what level set */

#define LOG_TYPE_UNDEFINED	0
#define LOG_TYPE_SYSTEM		1
#define LOG_TYPE_FILE		2
#define LOG_TYPE_CONSOLE	3

#define TSERVER_OPTION_LOGTYPE_SYSTEM	"system"
#define TSERVER_OPTION_LOGTYPE_FILE		"file"
#define TSERVER_OPTION_LOGTYPE_CONSOLE	"console"
#define TSERVER_EVENT_SOURCE			"tserver_sys_event"
#define MAX_STRING_LEN   256
#define CONFIG_LOG_FILE_SIZE  10*1024*1024
#define MAX_BACKUP_FILE    5
#define ZBX_MESSAGE_BUF_SIZE	4096
#define MAX_BUFFER_LEN 2048

const char * get_log_level(int log_level);
void zlog(const char *file, long line, int level,const char *format, ...);
static void	rotate_log(const char *filename);
int	zlog_open_log(int type, int level, const char *filename);

#define zlog_debug(...) \
	zlog(__FILE__, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)
#define zlog_info(...) \
	zlog(__FILE__, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)
#define zlog_notice(...) \
	zlog(__FILE__, __LINE__, ZLOG_LEVEL_NOTICE, __VA_ARGS__)
#define zlog_warn(...) \
	zlog(__FILE__, __LINE__, ZLOG_LEVEL_WARN, __VA_ARGS__)
#define zlog_error(...) \
	zlog(__FILE__, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)
#define zlog_fatal(...) \
	zlog(__FILE__, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)
#endif
#endif





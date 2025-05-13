#ifndef __SY_LOG_MESSAGE_H__
#define __SY_LOG_MESSAGE_H__

#include <stdarg.h>

#define MAX_LOG_LINE_NUM        500000
#define TIME_STAMP_BUF_LEN      128
#define LOG_STR_BUF_LEN		    8196

typedef enum
{
	LOG_INFO = 0,
    LOG_WARN,
    LOG_ERROR,
	LOG_FATAL,
    LOG_SYS,
} LOG_TYPE;


// win32
/*
#define logFatal(fmt, ...)   logMsg(SYLOG_FATAL, fmt, __VA_ARGS__)
#define logError(fmt, ...)   logMsg(SYLOG_ERROR, fmt, __VA_ARGS__)
#define logWarn(fmt, ...)    logMsg(SYLOG_WARN,  fmt, __VA_ARGS__)
#define logInfo(fmt, ...)    logMsg(SYLOG_INFO,  fmt, __VA_ARGS__)
*/
// gnu
#define logSys(fmt, ...)     logMsg(LOG_SYS, fmt, ##__VA_ARGS__)
#define logFatal(fmt, ...)   logMsg(LOG_FATAL, fmt, ##__VA_ARGS__)
#define logError(fmt, ...)   logMsg(LOG_ERROR, fmt, ##__VA_ARGS__)
#define logWarn(fmt, ...)    logMsg(LOG_WARN,  fmt, ##__VA_ARGS__)
#define logInfo(fmt, ...)    logMsg(LOG_INFO,  fmt, ##__VA_ARGS__)


void set_log_file_name(const char *file_name);
int log_init();
void log_set_level(int level);
void log_set_stdout_output(bool on);
int log_string(int level, const char *str);
int log_close(void);
const char* log_get_level_name(int level);

void logMsg(int level, const char* fmt, ...);

// write log info to database
//int db_write_log(string account, string module, string action, string operate);


#endif /* __SY_LOG_MESSAGE_H__ */

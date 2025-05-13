#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "logMsg.h"
#include "fileUtils.h"
#include "stringUtils.h"

#define MAX_LOOP_FILE  6
#define LOG_DIR "/app/run/frs/log/"
#define LOG_FILE_SIZE   (1024 * 1024 * 40)  // 40 M

static FILE *pf_log = NULL;
static FILE *pf_log_err = NULL;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
static int log_level = LOG_INFO;
static char last_log_str[LOG_STR_BUF_LEN];
static char log_file_name[MAX_LOOP_FILE][100] = {"frs.1.log", "frs.2.log", "frs.3.log", "frs.4.log", "frs.5.log", "frs.6.log"};
static char err_file_name[MAX_LOOP_FILE][100] = {"frs.1.err", "frs.2.err", "frs.3.err", "frs.4.err", "frs.5.err", "frs.6.err"};
static int currFileSize = 0;
static bool log_stdout_on = true;

/*******************************************************************************
*
*******************************************************************************/
int move_log_file()
{
    int last_index = MAX_LOOP_FILE - 1;


    for (int i = 0; i < MAX_LOOP_FILE; i++)
    {
        string fileCurr = string(LOG_DIR) + string(log_file_name[i]);
        
        if (is_file_exist(fileCurr.c_str()))
        {
            continue;
        }
        else
        {
            last_index = i;
            break;
        }
    }

    string fileLast = string(LOG_DIR) + string(log_file_name[last_index]);

    if (last_index == MAX_LOOP_FILE - 1)
    {
        if (is_file_exist(fileLast.c_str()))
        {
            //move all forward
            for (int i = 1; i < MAX_LOOP_FILE; i++)
            {
                string filePrev = string(LOG_DIR) + string(log_file_name[i-1]);
                string fileNext = string(LOG_DIR) + string(log_file_name[i]);
                
                if (is_file_exist(filePrev.c_str()))
                {
                    remove(filePrev.c_str());
                }
                rename(fileNext.c_str(), filePrev.c_str());
            }
        }        
    }  
    
    return last_index;
}

/*******************************************************************************
*
*******************************************************************************/
int move_err_file()
{
    int last_index = MAX_LOOP_FILE - 1;


    for (int i = 0; i < MAX_LOOP_FILE; i++)
    {
        string fileCurr = string(LOG_DIR) + string(err_file_name[i]);
        
        if (is_file_exist(fileCurr.c_str()))
        {
            continue;
        }
        else
        {
            last_index = i;
            break;
        }
    }

    string fileLast = string(LOG_DIR) + string(err_file_name[last_index]);

    if (last_index == MAX_LOOP_FILE - 1)
    {
        if (is_file_exist(fileLast.c_str()))
        {
            //move all forward
            for (int i = 1; i < MAX_LOOP_FILE; i++)
            {
                string filePrev = string(LOG_DIR) + string(err_file_name[i-1]);
                string fileNext = string(LOG_DIR) + string(err_file_name[i]);
                
                if (is_file_exist(filePrev.c_str()))
                {
                    remove(filePrev.c_str());
                }
                rename(fileNext.c_str(), filePrev.c_str());
            }
        }        
    }  
    
    return last_index;
}

/*******************************************************************************
*
*******************************************************************************/
void logMsg(int level, const char* fmt, ...)
{
	char     outbuf[LOG_STR_BUF_LEN];
  
    memset(outbuf, 0, sizeof(outbuf));
	// parse format message 
	va_list args;
	va_start(args, fmt);
    
	vsnprintf(outbuf, LOG_STR_BUF_LEN, fmt, args);

    log_string(level, outbuf);

    // output standard output 
    if (log_stdout_on)
    {
        printf("%s", outbuf);
    }
    
	va_end(args);
	return;
}

/*******************************************************************************
*
*******************************************************************************/
void set_log_file_name(const char *file_name)
{
    if (file_name == NULL) 
    {
        ;
    }
	//snprintf(log_file_name, sizeof(log_file_name), "%s%s", LOG_DIR, file_name);
}

/*******************************************************************************
*
*******************************************************************************/
int log_init()
{
    char buf[1024];
	memset(buf, 0, sizeof(buf));
    getcwd(buf, 1024);
    
    strcat(buf, LOG_DIR);
    if (false == is_file_exist(buf))
    {
        make_dir(buf);
    }  
    
    char filename[256];
    memset(filename, 0, sizeof(filename));

    // open log file
    int fileIndex =  move_log_file();    
    sprintf(filename, "%s%s", LOG_DIR, log_file_name[fileIndex]);        
	pf_log = fopen(filename, "a+");
	if (pf_log == NULL)
	{
		printf("Can't create log file: %s.\n", filename);
		return (-1);
	}
	currFileSize = 0;
	memset(last_log_str, 0, LOG_STR_BUF_LEN);

    // open error log
    fileIndex =  move_err_file();
    memset(filename, 0, sizeof(filename));
    sprintf(filename, "%s%s", LOG_DIR, err_file_name[fileIndex]);
	pf_log_err = fopen(filename, "a+");

	log_string(LOG_SYS, "LOG START\n");
    
	return 0;
}


/*******************************************************************************
*
*******************************************************************************/
void log_set_level(int level)
{
    if (level >= LOG_INFO && level <= LOG_SYS)
    {
        log_level = level;
    }
}

/*******************************************************************************
*
*******************************************************************************/
void log_set_stdout_output(bool on)
{
    log_stdout_on = on;
}

/*******************************************************************************
*
*******************************************************************************/
int log_string(int level, const char *str)
{
	char log_str[LOG_STR_BUF_LEN];
	int need_log_str = 0;

	pthread_mutex_lock(&log_mutex);

	string curTs = GetCurrentDTstring();

	if (currFileSize >= LOG_FILE_SIZE)
	{
        if (pf_log)
        {
            fclose(pf_log);
        }
        int fileIndex =  move_log_file();
        char filename[1024];
        memset(filename, 0, sizeof(filename));        
		sprintf(filename, "%s%s", LOG_DIR, log_file_name[fileIndex]);

        pf_log = fopen(filename, "a+");
        if (pf_log == NULL)
        {
            printf("Can't create log file: %s.\n", filename);
            pthread_mutex_unlock(&log_mutex);
            return (-1);
        }
        
        currFileSize = 0;
        memset(last_log_str, 0, LOG_STR_BUF_LEN);

        // log err
        if (pf_log_err) 
        {
            fclose(pf_log_err);
        }
        fileIndex =  move_err_file();
        memset(filename, 0, sizeof(filename));        
        sprintf(filename, "%s%s", LOG_DIR, err_file_name[fileIndex]);
        pf_log_err = fopen(filename, "a+");
	}

	// only log the string in case it is different then the last one
	if (strcmp(str, last_log_str))
	{
		switch (level)
		{
		case LOG_INFO:
			if (log_level <= LOG_INFO)
			{
				need_log_str = 1;
				snprintf(log_str, LOG_STR_BUF_LEN, "[%s] [INFO ] %s", curTs.c_str(), str);
			}
			break;
		case LOG_WARN:
			if (log_level <= LOG_WARN)
			{
				need_log_str = 1;
				snprintf(log_str, LOG_STR_BUF_LEN, "[%s] [WARN ] %s", curTs.c_str(), str);
			}
			break;
		case LOG_ERROR:
			if (log_level <= LOG_ERROR)
			{
				need_log_str = 1;
				snprintf(log_str, LOG_STR_BUF_LEN, "[%s] [ERROR] %s", curTs.c_str(), str);
                if (pf_log_err)
                {
                    fprintf(pf_log_err, "%s", log_str);
                    fflush(pf_log_err);
                }
			}
			break;
		case LOG_FATAL:
			if (log_level <= LOG_FATAL)
			{
				need_log_str = 1;
				snprintf(log_str, LOG_STR_BUF_LEN, "[%s] [FATAL] %s", curTs.c_str(), str);
                if (pf_log_err)
                {
                    fprintf(pf_log_err, "%s", log_str);
                    fflush(pf_log_err);
                }
			}
			break;
		case LOG_SYS:
			need_log_str = 1;
			snprintf(log_str, LOG_STR_BUF_LEN, "[%s] %s", curTs.c_str(), str);
			break;
		}

		if (need_log_str)
		{
			if (pf_log)
			{
                fprintf(pf_log, "%s", log_str);
			}
            
			memset(last_log_str, 0, LOG_STR_BUF_LEN);
			strcpy(last_log_str, str);
			currFileSize += strlen(log_str);
		}
	}

    if (pf_log)
    {
        fflush(pf_log);
    }
        
	pthread_mutex_unlock(&log_mutex);
	return 0;
}

/*******************************************************************************
*
*******************************************************************************/
int log_close(void)
{
    log_string(LOG_SYS, "LOG END\n");
    
	if (pf_log != NULL)
	{
		fclose(pf_log);
        pf_log = NULL;
	}

	if (pf_log_err != NULL)
	{
		fclose(pf_log_err);
        pf_log_err = NULL;
	}
    
	return 0;
}

/*******************************************************************************
*
*******************************************************************************/
const char* log_get_level_name(int level)
{
    switch (level)
    {
        case LOG_INFO:   return "INFO";
        case LOG_WARN:   return "WARN";
        case LOG_ERROR:  return "ERROR";
        case LOG_FATAL:  return "FATAL";
        case LOG_SYS:    return "SYS";
        default:         return "UNKOWN";
    }
}



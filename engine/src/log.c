#include "log.h"
#include "filesystem.h"
#include <stdarg.h>
#include <stdio.h>

filehandle_t* logFile_Handle;

#define LOG_TO_CONSOLE 1
#define LOG_TO_FILE 1
void Log_Init(void)
{
	logFile_Handle = FS_OpenFile("shmup_log.txt","a");

	Log_Printf("\n\n");

	if (LOG_TO_CONSOLE)
		Log_Printf("****[Warning, logging to console   . This should be disabled in prod.]\n");

	if (LOG_TO_FILE)
		Log_Printf("****[Warning, logging to filesystem. This should be disabled in prod.]\n");

}


int Log_Printf(const char *fmt,...){
	
    va_list ap;

    va_start(ap, fmt);

	if (LOG_TO_FILE)
		vfprintf(logFile_Handle->hFile, fmt, ap);

	if (LOG_TO_CONSOLE){
		vprintf(fmt,ap);
		fflush(stdout);
	}
    va_end(ap);

    return 0;
}
#ifndef _LOG_H
#define _LOG_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>


void TraceLog(const char * Fmt,...);
char * changeDt(long *datetime);
int GetDt(char *datetime);

#endif 

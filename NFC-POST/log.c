#include "log.h"

#define      LOG_FILE_PATH     "/home/buptzjw/test/log.txt"

/**
***将time()函数取得的秒数转成具体日期时间.
***参数datetime是传进的time()函数取得的秒数
***返回时间格式为YYYY/MM/DD-hh:mm:ss的字符串
**/
char * changeDt(long *datetime)
{
    struct tm *T;
    char buf[101],temp[5];

    T=localtime(datetime);

    sprintf(buf,"%d/",T->tm_year+1900);
    if(T->tm_mon+1<10)
        strcat(buf,"0");
    memset(temp,0x00,sizeof(temp));
    sprintf(temp,"%d/",T->tm_mon+1);
    strcat(buf,temp);
    if(T->tm_mday<10)
        strcat(buf,"0");
    memset(temp,0x00,sizeof(temp));
    sprintf(temp,"%d-",T->tm_mday);
    strcat(buf,temp);
    if(T->tm_hour<10)
        strcat(buf,"0");
    memset(temp,0x00,sizeof(temp));
    sprintf(temp,"%d:",T->tm_hour);
    strcat(buf,temp);
    if(T->tm_min<10)
        strcat(buf,"0");
    memset(temp,0x00,sizeof(temp));
    sprintf(temp,"%d:",T->tm_min);
    strcat(buf,temp);
    if(T->tm_sec<10)
        strcat(buf,"0");
    memset(temp,0x00,sizeof(temp));
    sprintf(temp,"%d",T->tm_sec);
    strcat(buf,temp);
    return(buf);
}

/**
***取系统日期和时间
**/
int GetDt(char *datetime)
{

    time_t rawtime;
    struct tm *timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    sprintf(datetime,"%s",asctime(timeinfo));

    return 0;
}

void  TraceLog(const char * Fmt,...)
{
	char  buffer[512] ;
	char  timebuf[64]  ;

 	//open the log file !
	 FILE * fp = NULL ;
	     if (!(fp = fopen(LOG_FILE_PATH,"a+")))
	     {
	     		 printf("can't open the LogFile!"); 
			 exit (1); 
	     } 

        GetDt(timebuf);
	       
        va_list arg_ptr;

        va_start(arg_ptr, Fmt);

        vsprintf(buffer,Fmt,arg_ptr); 
	
        va_end(arg_ptr);
	
        fprintf(fp,"%s%s\n",timebuf,buffer) ;
	
	fclose(fp) ;
        
}



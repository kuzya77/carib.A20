#include "types.h"
#include "mem.h"
#include "log.h"
#include "stuff.h"

#include <stdlib.h>
#include <time.h>

#include <unistd.h>
#include <linux/limits.h>

static FILE* log_file=NULL;

static u32_t log_mask = LOG_DEFAULT;

bool logInit(const char* file_name)
{
    char name[PATH_MAX];
    if(!log_file)
        logDone();
    if(file_name==NULL)
    {
	ssize_t size = readlink("/proc/self/exe", name, PATH_MAX);
	if(size!=-1)
		strcpy(&name[size], ".log");
	else
		strcpy(name, "logfile.log");
        file_name=name;
    }
    log_file=fopen(file_name,"wt");
    if(log_file)
    {
        log_mask=LOG_DEFAULT;
        return true;
    }  
    return false;
}

void logDone()
{
    if(log_file!=stdout)
        NULL_DESTROY(fclose,log_file);
}

void logEnable(u32_t mask)
{
    log_mask |= mask;
}

void logDisable(u32_t mask)
{
    log_mask ^= mask;
}

inline bool check(u32_t mask)
{
    return (log_mask & mask)==mask && log_file;
}

static void _log_puts(const char* msg,const char* prefix="")
{
//  if (check (LOG_FILE) && log_file)
    fprintf (log_file,"%s%s\n",prefix,msg);

//  if (check (LOG_CONSOLE))
//    conPrintf ("%s%s\n",prefix,msg);
}

static const char* get_prefix (LogLevel level)
{
  if (check (LOG_LEVEL_PREFIX))
  {
    switch (level)
    {
      case LOG_ERROR:   return "!> "; 
      case LOG_DEBUG:   return "#> ";
      case LOG_WARNING: return "?> ";
      default:
      case LOG_MESSAGE: return ">> ";
    }
  }
  else return "";
}

static void _log_print (LogLevel level,const char* msg)
{
    char tmp[50];
    const char* prefix = "";

    if(check (LOG_TIME))
    {
        if(check (LOG_FULL_TIME))
        {
            time_t ltime;

            time(&ltime);

            char* time_msg = ctime (&ltime); 

            time_msg[strlen(time_msg)-1]=0;    

            
            sprintf(tmp,"[%s]%s",time_msg,get_prefix (level));
        }
        else 
            sprintf(tmp,"[%08d]%s",clock(),get_prefix (level));
        prefix=tmp;
    }
    else 
        prefix = get_prefix (level);

    for (char* end;end=(char*)strchr(msg,'\n');msg=end+1)
    {
        *end = 0;
        _log_puts (msg, prefix);
    }

    if(*msg)
        _log_puts (msg, prefix);

    fflush (log_file);      
    fflush (stdout);
}

void logVPrintf(LogLevel level,const char* fmt,va_list list)
{
    if(check (level))
    {
        char sbuff[512];
        if(vsnprintf(sbuff, sizeof(sbuff)-1, fmt,list)<0)
        	sbuff[sizeof(sbuff)-1]='\0';
        _log_print(level,sbuff);
    }
}

void logPrintf (LogLevel level,const char* fmt,...)
{
    if(check (level))
    {
        va_list list;
        char sbuff[512];
        if(vsnprintf(sbuff, sizeof(sbuff)-1, fmt,list)<0)
        	sbuff[sizeof(sbuff)-1]='\0';
        _log_print(level,sbuff);
        va_end (list);
    }
}

void logPrint (LogLevel level,const char* msg)
{
    if(check (level))
        _log_print(level,msg);
}

void logDebug(const char* fmt,...)
{
    va_list list;
    va_start   (list,fmt);
    logVPrintf (LOG_DEBUG,fmt,list);
    va_end     (list);
}

void logMessage (const char* fmt,...)
{
    va_list list;
    va_start   (list,fmt);
    logVPrintf (LOG_MESSAGE,fmt,list);
    va_end     (list);
}

void logWarning (const char* fmt,...)
{
    va_list list;
    va_start   (list,fmt);
    logVPrintf (LOG_WARNING,fmt,list);
    va_end     (list);
}

void logError (const char* fmt,...)
{
    va_list list;

    va_start   (list,fmt);
    logVPrintf (LOG_ERROR,fmt,list);

    va_end     (list);
}

void logTitle (const char* fmt,...)
{
  const char* header = "---------------------------------------------";

  char buf[128];

  va_list list;

  va_start (list,fmt);
  if(vsnprintf (buf, sizeof(buf)-1, fmt, list)<0)
  	buf[sizeof(buf)-1] = '\0';

  va_end   (list);

    int first = (strlen (header)-strlen (buf))/2;

    if(first < 0)
        first = 0;

    char* space=buf+strlen (buf)+1,*s=space;
    for (;first--;*s++=' ');

    *s = 0;

  logMessage (header);
  logMessage ("%s%s",space,buf);
  logMessage (header);
}

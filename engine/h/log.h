#ifndef __BLITZ_LOG_STREAM__
#define __BLITZ_LOG_STREAM__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <types.h>
#include <stdarg.h>

//-------------------------------------------------------------------------------------
// Поток отладочных сообщений.

////////////////////////////////////////////////////////////////////////////////////////////
///Уровень сообщений / флаги лога
////////////////////////////////////////////////////////////////////////////////////////////
enum LogLevel
{
  LOG_ERROR   = 1,  //ошибка
  LOG_WARNING = 2,  //предупреждение
  LOG_MESSAGE = 4,  //сообщение
  LOG_DEBUG   = 8,  //отладочное сообщение
};

enum LogFlags
{
  LOG_FILE      = 16,  //выводить лог в файл
  LOG_CONSOLE   = 32,  //выводить лог в консоль
  LOG_TIME      = 64,  //выводить в лог время (clock'и)
  LOG_FULL_TIME = 128, //выводить в лог полное время
  LOG_LEVEL_PREFIX = 256, //выводить префикс (>>, !>, ?>, #>)

  LOG_ALL        = -1,
  LOG_ALL_LEVELS = LOG_ERROR|LOG_WARNING|LOG_MESSAGE|LOG_DEBUG,
  LOG_DEFAULT    = LOG_FILE|LOG_CONSOLE|LOG_TIME|LOG_ALL_LEVELS|LOG_LEVEL_PREFIX
};

// Инициализация, завершение ----------------------------------------------------------------------
bool logInit(const char* file_name=NULL);       // by default prog_name.log
void logDone();

// Вывод -------------------------------------------------------------------------

void logPrintf  (LogLevel level,const char* fmt,...);
void logVPrintf (LogLevel level,const char* fmt,va_list);
void logPrint   (LogLevel level,const char* msg);

void logDebug   (const char* msg,...);
void logMessage (const char* msg,...);
void logWarning (const char* msg,...);
void logError   (const char* msg,...);

void logTitle   (const char* msg,...);

// Фильтр ---------------------------------------------------------------------------------------

void logEnable    (uint32 mask);
void logDisable   (uint32 mask);
bool logIsEnabled (uint32 mask);

#endif

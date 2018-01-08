/*
 * ulog.h
 *
 *  Created on: 10/1/17
 *      Author: Oleg F., fedorov.ftf@gmail.com
 */

#ifndef CORE_ULOG_H_
#define CORE_ULOG_H_

#include <stdarg.h>
#include <stdio.h>

#define ERROR   "\x1B[31m"
#define INFO    "\x1B[32m"
#define WARN    "\x1B[33m"
#define ENDLINE "\033[0m\n"

#ifdef ZLOG
  #include <zlog.h>
#else
  enum zlog_level {
    ZLOG_LEVEL_INFO=0,
    ZLOG_LEVEL_NOTICE,
    ZLOG_LEVEL_WARN,
    ZLOG_LEVEL_ERROR
  };
#endif

#define BUILD_BUFFER(buffer, format) do {\
		va_list args;\
		va_start(args, format);\
		vsnprintf(buffer, sizeof(buffer), format, args);\
		va_end(args);\
} while (0)

#ifdef __cplusplus
extern "C" {
#endif

void ulog_init();
void ulog_clean();
void ulog_info(const char* category, const char* file_name, const char* func_name, int line, const char* format, ...);
void ulog_error(const char* category, const char* file_name, const char* func_name, int line, const char* format, ...);
void ulog_common(enum zlog_level level, const char* category, const char* file_name, const char* func_name, int line, const char* format, ...);

#ifndef ZLOG
#define ULOG_CATEGORY "ulog"
#endif

#ifndef ULOG_SUCCESS
#define ULOG_SUCCESS "Success"
#endif

#ifndef ULOG_FAIL
#define ULOG_FAIL "Error"
#endif

#define mdebug_error(format, ...) ulog_error(ULOG_CATEGORY, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define mdebug_notice(format, ...) ulog_common(ZLOG_LEVEL_NOTICE, ULOG_CATEGORY, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define mdebug_info(format, ...) ulog_info(ULOG_CATEGORY, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)
#define mdebug_warn(format, ...) ulog_common(ZLOG_LEVEL_WARN, ULOG_CATEGORY, __FILE__, __FUNCTION__, __LINE__, format, ##__VA_ARGS__)


#ifdef __cplusplus
}
#endif

#endif /* CORE_ULOG_H_ */


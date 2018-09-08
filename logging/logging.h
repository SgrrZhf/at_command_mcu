
#ifndef __LOGGING_H
#define __LOGGING_H

#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL LOG_INFO
#endif /* ifndef LOG_LOCAL_LEVEL */

#define LOG_NONE 0
#define LOG_ERROR 1
#define LOG_WARN 2
#define LOG_INFO 3
#define LOG_DEBUG 4
#define LOG_VERBOSE 5

#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D       LOG_COLOR(LOG_COLOR_CYAN)
#define LOG_COLOR_V

void logging_write(const char *format, ...);

#define __Logging_Format(letter,format) LOG_COLOR_ ## letter #letter " (%d) %s: "  format LOG_RESET_COLOR "\n"

#if (LOG_LOCAL_LEVEL >= LOG_ERROR)
#define __Logging_E(tag,format,...)      logging_write(__Logging_Format(E,format),HAL_GetTick(),tag,##__VA_ARGS__)
#else
#define __Logging_E(tag,format,...)
#endif

#if (LOG_LOCAL_LEVEL >= LOG_WARN)
#define __Logging_W(tag,format,...)     logging_write(__Logging_Format(W,format),HAL_GetTick(),tag,##__VA_ARGS__)
#else
#define __Logging_W(tag,format,...)
#endif

#if (LOG_LOCAL_LEVEL >= LOG_INFO)
#define __Logging_I(tag,format,...)      logging_write(__Logging_Format(I,format),HAL_GetTick(),tag,##__VA_ARGS__)
#else
#define __Logging_I(tag,format,...)
#endif

#if (LOG_LOCAL_LEVEL >= LOG_DEBUG)
#define __Logging_D(tag,format,...)     logging_write(__Logging_Format(D,format),HAL_GetTick(),tag,##__VA_ARGS__)
#else
#define __Logging_D(tag,format,...)
#endif

#if (LOG_LOCAL_LEVEL >= LOG_VERBOSE)
#define __Logging_V(tag,format,...)      logging_write(__Logging_Format(V,format),HAL_GetTick(),tag,##__VA_ARGS__)
#else
#define __Logging_V(tag,format,...)
#endif

#endif /* ifndef __LOGGING_H
 */

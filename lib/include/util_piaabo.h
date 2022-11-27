#ifndef __UTIL_PIAABO
#define __UTIL_PIAABO
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#define ANSI_COLOR_ERROR "\x1b[41m"
#define ANSI_COLOR_SUCCESS "\x1b[42m"
#define ANSI_COLOR_WARNING "\x1b[43m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define	ANSI_COLOR_Black "\x1b[30m"
#define	ANSI_COLOR_Red "\x1b[31m"
#define	ANSI_COLOR_Green "\x1b[32m"
#define	ANSI_COLOR_Yellow "\x1b[33m"
#define	ANSI_COLOR_Blue "\x1b[34m"
#define	ANSI_COLOR_Magenta "\x1b[35m"
#define	ANSI_COLOR_Cyan "\x1b[36m"
#define	ANSI_COLOR_White "\x1b[37m"
#define	ANSI_COLOR_Bright_Black_Grey "\x1b[90m"
#define	ANSI_COLOR_Bright_Red "\x1b[91m"
#define	ANSI_COLOR_Bright_Green "\x1b[92m"
#define	ANSI_COLOR_Bright_Yellow "\x1b[93m"
#define	ANSI_COLOR_Bright_Blue "\x1b[94m"
#define	ANSI_COLOR_Bright_Magenta "\x1b[95m"
#define	ANSI_COLOR_Bright_Cyan "\x1b[96m"
#define	ANSI_COLOR_Bright_White "\x1b[97m"

pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;
/* This log functionality marks the thread id, so that log messages are linked to the request thread */
#define log(...) {\
  pthread_mutex_lock(&log_mutex);\
  fprintf(stdout,"[%s0x%lX%s]: ",\
    ANSI_COLOR_Cyan,pthread_self(),ANSI_COLOR_RESET);\
  fprintf(stdout,__VA_ARGS__);\
  pthread_mutex_unlock(&log_mutex);\
}
/* This log functionality marks the thread id, so that log messages are linked to the request thread */
#define log_err(...) {\
  pthread_mutex_lock(&log_mutex);\
  fprintf(stderr,"[%s0x%lX%s]: %sERROR%s: ",\
    ANSI_COLOR_Cyan,pthread_self(),ANSI_COLOR_RESET,\
    ANSI_COLOR_ERROR,ANSI_COLOR_RESET);\
  fprintf(stdout,__VA_ARGS__);\
  pthread_mutex_unlock(&log_mutex);\
}
/* This log functionality marks the thread id, so that log messages are linked to the request thread */
#define log_warn(...) {\
  pthread_mutex_lock(&log_mutex);\
  fprintf(stderr,"[%s0x%lX%s]: %sWARNING%s: ",\
    ANSI_COLOR_Cyan,pthread_self(),ANSI_COLOR_RESET,\
    ANSI_COLOR_WARNING,ANSI_COLOR_RESET);\
  fprintf(stdout,__VA_ARGS__);\
  pthread_mutex_unlock(&log_mutex);\
}
/* this function prints boolean values */
#define print_bool(dinput) dinput? "True":"False"
/* functionaly to encodebase64 */
static char *encode_base64(const char *message){
  /* valid characters */
  const char *lookup =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  /* temporal variables */
  unsigned long l;
  char *b64;
  size_t length = strlen (message);
  /* allocate return (must be free) */
  b64 = (char*)malloc (length * 2);
  if (NULL == b64)
    return b64;
  /* initialzie return */
  b64[0] = 0;
  /* encode base64 */
  for (size_t i = 0; i < length; i += 3){
    l = (((unsigned long) message[i]) << 16)
      | (((i + 1) < length) ? (((unsigned long) message[i + 1]) << 8) : 0)
      | (((i + 2) < length) ? ((unsigned long) message[i + 2]) : 0);
    strncat (b64, &lookup[(l >> 18) & 0x3F], 1);
    strncat (b64, &lookup[(l >> 12) & 0x3F], 1);
    if (i + 1 < length)
      strncat (b64, &lookup[(l >> 6) & 0x3F], 1);
    if (i + 2 < length)
      strncat (b64, &lookup[l & 0x3F], 1);
  }
  /* standarize */
  if (length % 3)
    strncat (b64, "===", 3 - length % 3);
  /* return base64 */
  return b64;
}

#endif
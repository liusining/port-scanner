#ifndef __LOGGER_H
#define __LOGGER_H
#include <stdio.h> /* fprintf(), ... */
#include <stdlib.h> /* exit() */
#include <errno.h> /* errno */
#include <string.h> /* strerror() */
#define FATALF(title,msg) \
do { \
  fprintf(stderr, "%s: %s\n", title, msg); \
  exit(EXIT_FAILURE); \
} while(0)
#define ERRORF(title) \
do { \
  fprintf(stderr, "%s: %s, errno: %d\n", title, strerror(errno), errno); \
  exit(EXIT_FAILURE); \
} while(0)
#endif

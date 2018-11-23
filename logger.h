#ifndef __LOGGER_H
#define __LOGGER_H
#include <stdio.h>
#include <stdlib.h> /* exit() */
#include <errno.h> /* errno */
#include <string.h> /* strerror() */
#define FATALF(title,msg) \
do { \
  printf("%s: %s\n", title, msg); \
  exit(1); \
} while(0)
#define ERRORF(title) \
do { \
  printf("%s: %s, errno: %d\n", title, strerror(errno), errno); \
  exit(1); \
} while(0)
#endif

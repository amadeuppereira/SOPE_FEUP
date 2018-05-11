#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include <string.h> 
#include <time.h>
#include <pthread.h>

#define MAX_ROOM_SEATS 9999
#define MAX_CLI_SEATS 99
#define DELAY() sleep(5)
#define MAX_PREFERED_SEATS 99
#define WIDTH_SEAT 4
#define WIDTH_PID 5
#define WIDTH_XXNN 5
#define MAX_BUFFER_SIZE 500

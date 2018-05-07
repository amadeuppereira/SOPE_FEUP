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

#define MAX_CLI_SEATS 5
#define DELAY() sleep(5)
#define MAX_MSG_LEN 100

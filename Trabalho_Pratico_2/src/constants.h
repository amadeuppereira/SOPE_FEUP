#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <signal.h>
#include <errno.h>
#include <string.h> 
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h> 

#define MAX_ROOM_SEATS 9999
#define MAX_CLI_SEATS 99
#define DELAY() sleep(1)
#define MAX_PREFERED_SEATS 99
#define WIDTH_SEAT 4
#define WIDTH_PID 5
#define WIDTH_XXNN 5
#define MAX_BUFFER_SIZE 500

#define FIFO_SERVER "requests" 
#define SERVER_LOG "slog.txt"
#define SERVER_BOOKINGS "sbook.txt"
#define CLIENTS_LOG "clog.txt"
#define CLIENTS_BOOKINGS "cbook.txt"

#define MAX -1
#define NST -2
#define IID -3
#define ERR -4
#define NAV -5
#define FUL -6
#define OUT -7

/**
 * @brief Struct with a request's information
 */
struct Request {
  int processed;

  int clientID;
  int num_wanted_seats;
  int num_prefered_seats;
  int prefered_seats[MAX_PREFERED_SEATS];
};
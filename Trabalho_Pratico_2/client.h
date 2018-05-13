#include "constants.h"

#define CLIENT_LOG "clog.txt"
#define CLIENT_BOOKINGS "cbook.txt"

char fifo_name[MAX_BUFFER_SIZE];
FILE *clog_file;

struct Request {
  int processed;

  int clientID;
  int num_wanted_seats;
  int num_prefered_seats;
  int prefered_seats[MAX_PREFERED_SEATS];
};

int readParameters(int *time_out, int *num_wanted_seats, int pref_seat_list[], int *num_prefered_seats, char *argv[]);
int destroyFIFO(char* name);
void alarm_handler(int signo);
void answerFifoName(char *name);
void getFullClientId(char *fn, int id);
void getFullSeatNumber(char *fn, int n);
void clogAnswer(int ret, int num_wanted_seats, int * reserved_seats);
void cbookReservations(int ret,int num_wanted_seats, int * reserved_seats);
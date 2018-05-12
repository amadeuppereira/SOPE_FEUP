#include "constants.h"

char fifo_name[MAX_BUFFER_SIZE];

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
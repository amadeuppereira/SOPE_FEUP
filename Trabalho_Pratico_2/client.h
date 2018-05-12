#include "constants.h"

struct Request {
  int processed;

  int clientID;
  int num_wanted_seats;
  int num_prefered_seats;
  int prefered_seats[MAX_PREFERED_SEATS];
};

int readParameters(int *time_out, int *num_wanted_seats, int pref_seat_list[], char *argv[]);
int destroyFIFO(char* name);
void alarm_handler(int signo);
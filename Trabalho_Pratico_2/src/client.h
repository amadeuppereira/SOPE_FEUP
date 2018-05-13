#include "constants.h"

char fifo_name[MAX_BUFFER_SIZE];
int timeout = 0;
FILE *clog_file;

/**
 * @brief Reads the given parameters, puting them in the correspondent given variable.
 * 
 * @param time_out 
 * @param num_wanted_seats 
 * @param pref_seat_list 
 * @param num_prefered_seats 
 * @param argv 
 * @return int 
 */
int readParameters(int *time_out, int *num_wanted_seats, int pref_seat_list[], int *num_prefered_seats, char *argv[]);
int destroyFIFO(char* name);
void alarm_handler(int signo);
void answerFifoName(char *name);
void getFullClientId(char *fn, int id);
void getFullSeatNumber(char *fn, int n);
void getFullXXNN(char *fn, int n1, int n2);
void clogAnswer(int ret, int num_wanted_seats, int * reserved_seats);
void cbookReservations(int ret,int num_wanted_seats, int * reserved_seats);
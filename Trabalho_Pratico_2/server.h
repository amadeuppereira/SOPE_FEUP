#include "constants.h"

#define SERVER_LOG "slog.txt"
#define SERVER_BOOKINGS "sbook.txt"

int num_room_seats;
int timeout = 0;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
FILE *slog;

struct Seat{
  int seatNumber;
  int occupied; //0 - free, 1 - occupied
  int clientID;
};

struct Request {
  int processed;

  int clientID;
  int num_wanted_seats;
  int num_prefered_seats;
  int prefered_seats[MAX_PREFERED_SEATS];
};

struct Seat roomSeats[MAX_ROOM_SEATS];
struct Request buffer[1];

int readParameters(int *num_room_seats, int *num_ticket_offices, int *open_time, char *argv[]);
void createSeats(int num_room_seats);
int createFIFO(char* name);
int destroyFIFO(char* name);
void alarm_handler(int signo);
int checkRequest(struct Request r, int reserved_seats[]);
void *ticketOffice(void *arg);
int isSeatFree(struct Seat *seats, int seatNum);
void bookSeat(struct Seat * seats, int seatNum, int clientID);
void freeSeat(struct Seat *seats, int seatNum);
int isRoomFull();

void slogOpenClose(int no, int open);
void slogRequest(int no, struct Request r, int ret, int reserved_seats[]);
void getFullSeatNumber(char *fn, int n);
void getFullClientId(char *fn, int id);
void answerFifoName(char *name, int clientID);
void sbookReservations();
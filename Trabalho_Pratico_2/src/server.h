#include "constants.h"

int num_room_seats;
int timeout = 0;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mut_files = PTHREAD_MUTEX_INITIALIZER;
FILE *slog;

/**
 * @brief Struct holding all the information for a seat
 */
struct Seat{
  int seatNumber;
  int occupied; //0 - free, 1 - occupied
  int clientID;
};

struct Seat roomSeats[MAX_ROOM_SEATS];
struct Request buffer[1];

/**
 * @brief Reads the given parameters, puting them in the corresponding given variable.
 * 
 * @param num_room_seats pointer for the variable that will get the number of seats
 * @param num_ticket_offices pointer for the variable that will get the number of ticket offices
 * @param open_time pointer for the variable that will get the working time of the server
 * @param argv array with all the given parameters
 * @return int 1 if invalid parameters, 0 otherwise
 */
int readParameters(int *num_room_seats, int *num_ticket_offices, int *open_time, char *argv[]);

/**
 * @brief Create a given number of seats
 * 
 * @param num_room_seats number of seats
 */
void createSeats(int num_room_seats);

/**
 * @brief Creates a fifo with a given name
 * 
 * @param name fifo's name
 * @return int 0 if success, 1 if there already exists a fifo with that name, 2 other error
 */
int createFIFO(char* name);

/**
 * @brief Destroy a fifo that has a given name.
 * 
 * @param name fifo's name
 * @return int 0 if sucess, 1 otherwise
 */
int destroyFIFO(char* name);

/**
 * @brief Handler for SIGALRM.
 * 
 * @param signo signal number
 */
void alarm_handler(int signo);

/**
 * @brief Analyze a given request
 * 
 * @param r request
 * @param reserved_seats array that will hold the reserved seats
 * @return int negative number if request no possible, 0 otherwise
 */
int checkRequest(struct Request r, int reserved_seats[]);

/**
 * @brief Thread function
 * 
 * @param arg argument
 * @return void* void pointer
 */
void *ticketOffice(void *arg);

/**
 * @brief Check if a seat is free
 * 
 * @param seats pointer for the Seat array to go through
 * @param seatNum seat number to check
 * @return int 1 if free, 0 otherwise
 */
int isSeatFree(struct Seat *seats, int seatNum);

/**
 * @brief Book a seat
 * 
 * @param seats pointer for the Seat array to go through
 * @param seatNum seat number to book
 * @param clientID client id that booked the seat
 */
void bookSeat(struct Seat * seats, int seatNum, int clientID);

/**
 * @brief Free a seat
 * 
 * @param seats pointer for the Seat array to go through
 * @param seatNum seat number to free
 */
void freeSeat(struct Seat *seats, int seatNum);

/**
 * @brief Free a given number of seats
 * 
 * @param n number of seats to free
 * @param seats array with the seat numbers to free
 */
void freeSeats(int n, int seats[]);

/**
 * @brief Check if the room is full
 * 
 * @return int 1 if free, 0 otherwise
 */
int isRoomFull();

/**
 * @brief Write a thread open/close message in the SERVER_LOG file
 * 
 * @param no thread number
 * @param open flag (1 -> open, 0 -> close)
 */
void slogOpenClose(int no, int open);

/**
 * @brief Write a request in the SERVER_LOG file
 * 
 * @param no thread number
 * @param r request
 * @param ret request's answer from server 
 * @param reserved_seats array with reserved seats
 */
void slogRequest(int no, struct Request r, int ret, int reserved_seats[]);

/**
 * @brief Get the full seat number, having in consideration WIDTH_SEAT (fill with 0).
 * 
 * @param fn pointer where the full seat number will be saved
 * @param n seat number
 */
void getFullSeatNumber(char *fn, int n);

/**
 * @brief Get the full clientID, having in consideration WIDTH_PID (fill with 0).
 * 
 * @param fn pointer where the full client id will be saved
 * @param id client id
 */
void getFullClientId(char *fn, int id);

/**
 * @brief Get the full name for the fifo that that will send the server's answer to the client (ex:ans01234), having in consideration WIDTH_PID.
 * 
 * @param name pointer where the name will be saved
 * @param clientID client id
 */
void answerFifoName(char *name, int clientID);

/**
 * @brief Write all the reserved seats in the SERVER_BOOKINGS file.
 */
void sbookReservations();

/**
 * @brief Clear SERVER_BOOKING and SERVER_LOG files
 */
void resetClientFiles();
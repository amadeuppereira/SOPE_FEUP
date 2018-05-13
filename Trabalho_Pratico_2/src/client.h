#include "constants.h"

char fifo_name[MAX_BUFFER_SIZE];
int timeout = 0;
FILE *clog_file;

/**
 * @brief Reads the given parameters, puting them in the corresponding given variable.
 * 
 * @param time_out pointer for the variable that will get the max waiting time for a server's answer
 * @param num_wanted_seats pointer for the variable that will get the number of seats wanted
 * @param pref_seat_list array for all the prefered seats
 * @param num_prefered_seats pointer for the variable that will get the number of prefered seats
 * @param argv array with all the given parameters
 * @return int 1 if invalid parameters, 0 otherwise
 */
int readParameters(int *time_out, int *num_wanted_seats, int pref_seat_list[], int *num_prefered_seats, char *argv[]);

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
 * @brief Get the full name for the fifo that will receive the server's answer (ex:ans01234), having in consideration WIDTH_PID.
 * 
 * @param name pointer where the name will be saved
 */
void answerFifoName(char *name);

/**
 * @brief Get the full clientID, having in consideration WIDTH_PID (fill with 0).
 * 
 * @param fn pointer where the full client id will be saved
 * @param id client id
 */
void getFullClientId(char *fn, int id);

/**
 * @brief Get the full seat number, having in consideration WIDTH_SEAT (fill with 0).
 * 
 * @param fn pointer where the full seat number will be saved
 * @param n seat number
 */
void getFullSeatNumber(char *fn, int n);

/**
 * @brief Get the full XXNN (n1.n2), having in consideration WIDTH_XXNN (fill with 0).
 * 
 * @param fn pointer where the full xxnn will be saved
 * @param n1 left number
 * @param n2 right number
 */
void getFullXXNN(char *fn, int n1, int n2);

/**
 * @brief Write the server answer in the CLIENTS_LOG file.
 * 
 * @param ret server return value
 * @param num_wanted_seats number of wanted seats
 * @param reserved_seats array with the reserved seats
 */
void clogAnswer(int ret, int num_wanted_seats, int * reserved_seats);

/**
 * @brief Write all the reserved seats in the CLIENTS_BOOKINGS file.
 * 
 * @param ret server return value
 * @param num_wanted_seats number of wanted seats
 * @param reserved_seats array with the reserved seats
 */
void cbookReservations(int ret,int num_wanted_seats, int * reserved_seats);
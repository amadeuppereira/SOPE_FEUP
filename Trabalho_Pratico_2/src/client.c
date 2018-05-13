#include "client.h"

int main(int argc, char* argv[]){
  if (argc != 4) {
    printf("Usage: %s <time_out> <num_wanted_seats> <pref_seat_list>\n", argv[0]);
    return 1;
  }

   //Reading the parameters
  int time_out;
  int num_wanted_seats;
  int num_prefered_seats;
  int prefered_seats[MAX_PREFERED_SEATS];

  if(readParameters(&time_out, &num_wanted_seats, prefered_seats, &num_prefered_seats, argv) != 0) {
    printf("Invalid Arguments!\n");
    return 1;
  }

  //Opening FIFO's
  int requests=open(FIFO_SERVER, O_WRONLY);
  if(requests == -1){
   printf("Ticket offices closed!\n");
   return 2;
  }

  clog_file = fopen(CLIENTS_LOG, "a");

  //Alarm handler
  struct sigaction action;
  action.sa_handler = alarm_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGALRM, &action, NULL);

  //Making FIFO to get the answer from the server
  answerFifoName(fifo_name);
  
  if(mkfifo(fifo_name,0660) < 0){
    if (errno == EEXIST)
      printf("FIFO '%s' already exists\n", fifo_name);
    else
      printf("Can't create FIFO\n");
  }

  int fd_answer;
  if((fd_answer=open(fifo_name, O_RDONLY | O_NONBLOCK)) == -1) {
    printf("FIFO 'ans' failed to open in READONLY mode\n");
    return 2;
  }

  //Writing to FIFO requests
  struct Request request;
  request.clientID = getpid();
  request.num_wanted_seats = num_wanted_seats;
  request.num_prefered_seats = num_prefered_seats;
  int i;
  for(i = 0; i < num_prefered_seats; i++){
    request.prefered_seats[i] = prefered_seats[i];
  }
  request.processed = 0;

  write(requests, &request, sizeof(struct Request));
  //Starting Time Count
  alarm(time_out);

  close(requests);

  int ret, n, reserved_seats[num_wanted_seats];
  while((n = read(fd_answer, &ret, sizeof(int))) <= 0 && !timeout);
  alarm(0);

  if(timeout) ret = OUT;

  if(ret >= 0) {
    for(i = 0; i < num_wanted_seats; i++) {
      while((n = read(fd_answer, &ret, sizeof(int))) <= 0);
      reserved_seats[i] = ret;
    }
  }

  close(fd_answer);

  clogAnswer(ret, num_wanted_seats, reserved_seats);
  cbookReservations(ret, num_wanted_seats, reserved_seats);

  //Destroying FIFO
  destroyFIFO(fifo_name);

  fclose(clog_file);

  return 0;
}

int readParameters(int *time_out, int *num_wanted_seats, int pref_seat_list[], int *num_prefered_seats, char *argv[]) {
  *time_out = atoi(argv[1]);
  *num_wanted_seats = atoi(argv[2]);


  int count = 0;
  char* tmp = argv[3];
  char* last_space = 0;
  char delim[2];
  delim[0] = ' ';
  delim[1] = 0;

  while(*tmp) {
    if(*tmp == ' '){
      count ++;
      last_space = tmp;
    }
    tmp++;
  }

  count += last_space < (argv[3] + strlen(argv[3] - 1));
  count++;

  size_t idx = 0;
  char* token = strtok(argv[3], delim);
  while(token){
    *(pref_seat_list + idx++) = atoi(strdup(token));
    token = strtok(0, delim);
  }
  *(pref_seat_list + idx) = 0;

  *num_prefered_seats = idx;

  if(*time_out <= 0 || *num_wanted_seats <= 0)
    return 1;
  
  return 0;
}

int destroyFIFO(char* name) {
    if (unlink(name) < 0) {
      printf("Error when destroying FIFO '%s'\n", name);
      return 1;
    }
    return 0;
}

void alarm_handler(int signo) {
  timeout = 1;
}

void answerFifoName(char *name) {
  char id[WIDTH_PID + 1];
  getFullClientId(id, getpid());
  sprintf(name, "ans%s", id);
}

void getFullClientId(char *fn, int id) {
  sprintf(fn, "");
  char clientID_s[WIDTH_PID + 1];
  sprintf(clientID_s, "%d", id);
  int i;
  for(i = strlen(clientID_s); i < WIDTH_PID; i++) {
    sprintf(fn, "%s%d", fn, 0);
  }
  sprintf(fn, "%s%s", fn, clientID_s);
}

void getFullSeatNumber(char *fn, int n) {
  sprintf(fn, "");
  char seat[WIDTH_SEAT + 1];
  sprintf(seat, "%d", n);
  int i;
  for(i = strlen(seat); i < WIDTH_SEAT; i++) {
    sprintf(fn, "%s%d", fn, 0);
  }
  sprintf(fn, "%s%s", fn, seat);
}

void getFullXXNN(char *fn, int n1, int n2) {
  int width = (WIDTH_XXNN - 1) / 2;

  sprintf(fn, "");
  char fn1[width];
  sprintf(fn1, "%d", n1);
  int i;
  for(i = strlen(fn1); i < width; i++) {
    sprintf(fn, "%s%d", fn, 0);
  }
  sprintf(fn, "%s%d.", fn, n1);

  char fn2[width];
  sprintf(fn2, "%d", n2);
  for(i = strlen(fn2); i < width; i++) {
    sprintf(fn, "%s%d", fn, 0);
  }
  sprintf(fn, "%s%d", fn, n2);
}

// WRITE TO FILE FUNCTIONS

void clogAnswer(int ret, int num_wanted_seats, int * reserved_seats) {

  char clientID_s[WIDTH_PID + 1];
  getFullClientId(clientID_s, getpid());

  int i;

  switch(ret) {
    case MAX: fprintf(clog_file, "%s MAX\n", clientID_s); break;
    case NST: fprintf(clog_file, "%s NST\n", clientID_s); break;
    case IID: fprintf(clog_file, "%s IID\n", clientID_s); break;
    case ERR: fprintf(clog_file, "%s ERR\n", clientID_s); break;
    case NAV: fprintf(clog_file, "%s NAV\n", clientID_s); break;
    case FUL: fprintf(clog_file, "%s FUL\n", clientID_s); break;
    case OUT: fprintf(clog_file, "%s OUT\n", clientID_s); break;
    default:
      for(i = 0; i < num_wanted_seats; i++) {
        char xxnn[WIDTH_XXNN + 1];
        getFullXXNN(xxnn, i+1, num_wanted_seats);

        char seat[WIDTH_SEAT + 1];
        getFullSeatNumber(seat, reserved_seats[i]);

        fprintf(clog_file, "%s %s %s\n", clientID_s, xxnn, seat);
      }
      break;
  }
} 

void cbookReservations(int ret, int num_wanted_seats, int * reserved_seats) {

  if(ret >= 0){
    FILE *cbook = fopen(CLIENTS_BOOKINGS, "a");

    int i;
    for(i = 0; i < num_wanted_seats; i++) {
      char seat[WIDTH_SEAT + 1];
      getFullSeatNumber(seat, reserved_seats[i]);
      fprintf(cbook, "%s\n", seat);
    }

    fclose(cbook);
  }
}
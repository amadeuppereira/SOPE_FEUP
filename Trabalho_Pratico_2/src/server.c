#include "server.h"

int main(int argc, char* argv[]){

  if (argc != 4) {
    printf("Usage: %s <num_room_seats> <num_ticket_offices> <open_time>\n", argv[0]);
    return 1;
  }

  int num_ticket_offices, open_time;
  if(readParameters(&num_room_seats, &num_ticket_offices, &open_time, argv) != 0) {
    printf("Invalid Arguments!\n");
    return 1;
  }

  slog = fopen(SERVER_LOG, "w");

  //Alarm handler
  struct sigaction action;
  action.sa_handler = alarm_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGALRM, &action, NULL);
  
  //Creating the Room Seats
  createSeats(num_room_seats);

  //Making FIFO
  if(createFIFO(FIFO_SERVER) != 0)
    return 2;

  //Open FIFO
  int requests = open(FIFO_SERVER, O_RDONLY | O_NONBLOCK);
  if(requests < 0) {
    printf("FIFO '%s' failed to open in READONLY mode\n", FIFO_SERVER);
     if(destroyFIFO(FIFO_SERVER) != 0) {
      return 4;
     }
    return 3;
  }

  resetClientFiles();
  
  struct Request request;
  request.processed = 1;
  buffer[0] = request;

  //Starting Time Count
  alarm(open_time);

  //Create threads
  pthread_t tid[num_ticket_offices];
  int i;
  for(i = 0; i < num_ticket_offices; i++) {

    slogOpenClose(i+1, 1);
    int rc = pthread_create(&tid[i], NULL, ticketOffice, (void*) (size_t) i + 1);
    if(rc) {
      perror("Thread");
      return 5;
    }
  }
  //Reading FIFO
  int n;
  while(!timeout) {
    if(buffer[0].processed) {
      if((n = read(requests, &request, sizeof(struct Request))) > 0) {
        buffer[0] = request;
      }
    }
  }
  
  close(requests);

  for(i = 0; i < num_ticket_offices; i++) {
    pthread_join(tid[i], NULL);
    slogOpenClose(i+1, 0);
  }
  
  sbookReservations();

  //Destroying FIFO
  if(destroyFIFO(FIFO_SERVER) != 0) {
    return 4;
  }

  pthread_mutex_destroy(&mut);
  pthread_mutex_destroy(&mut_files);

  fprintf(slog, "SERVER CLOSED");
  fclose(slog);
  return 0;
}

int readParameters(int *num_room_seats, int *num_ticket_offices, int *open_time, char *argv[]) {
  *num_room_seats = atoi(argv[1]);
  *num_ticket_offices = atoi(argv[2]);
  *open_time = atoi(argv[3]);

  if(*num_room_seats <= 0 || *num_ticket_offices <= 0 || *open_time <= 0)
    return 1;
  
  return 0;
}

void createSeats(int num_room_seats) {
  int n;
  for(n = 0; n < num_room_seats; n++){
    roomSeats[n].seatNumber = n+1;
    roomSeats[n].occupied = 0;
    roomSeats[n].clientID = 0;
  }
}

int createFIFO(char *name) {
   if(mkfifo(name,0660) < 0){
    if (errno == EEXIST) {
      printf("FIFO '%s' already exists\n", name);
      return 1;
    }
    else {
      perror("FIFO");
      return 2;
    }
  }
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
  printf("timeout\n");
}

void *ticketOffice(void *arg) {
  int t_no = (int) arg;
  struct Request r;
  while(!timeout) {
    int status = pthread_mutex_trylock(&mut);
    if(status == EBUSY) continue;
    while(buffer[0].processed && !timeout) {}
    if (!buffer[0].processed) {
      r = buffer[0];
      buffer[0].processed = 1;

      int reserved_seats[r.num_wanted_seats];
      int ret = checkRequest(r, reserved_seats);
      pthread_mutex_unlock(&mut);

      char fifo_name[MAX_BUFFER_SIZE];
      answerFifoName(fifo_name, r.clientID);

      int fd = open(fifo_name, O_WRONLY);
      if (fd < 0) {
        ret = OUT;
        int i;
        for(i = 0; i < r.num_wanted_seats; i++) {
          freeSeat(roomSeats, reserved_seats[i]);
        }
      }
      else {
        if (ret < 0) {
          write(fd, &ret, sizeof(int));
        }
        else {
          write(fd, &r.num_wanted_seats, sizeof(int));
          int i;
          for (i = 0; i < r.num_wanted_seats; i++) {
            write(fd, &reserved_seats[i], sizeof(int));
          }
        }
      }

      pthread_mutex_lock(&mut_files);
      slogRequest(t_no, r, ret, reserved_seats);
      pthread_mutex_unlock(&mut_files);
    }
    else {
      pthread_mutex_unlock(&mut);
    }
  }
  return NULL;
}

void answerFifoName(char *name, int clientID) {
  char id[WIDTH_PID + 1];
  getFullClientId(id, clientID);
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

int isRoomFull() {
  int i;
  for(i = 0; i < num_room_seats; i++) {
    if(isSeatFree(roomSeats, i)) {
      return 0;
    }
  }
  return 1;
}

int checkRequest(struct Request r, int reserved_seats[]) {
  
  if(r.num_wanted_seats > MAX_CLI_SEATS) {
    return MAX;
  }

  if(!(r.num_prefered_seats >= r.num_wanted_seats && r.num_prefered_seats <= MAX_CLI_SEATS)) {
    return NST;
  }

  if(isRoomFull()) {
    return FUL;
  }

  int i, num_reserved_seats = 0;
  for(i = 0; i < r.num_prefered_seats; i++) {
    if(!(r.prefered_seats[i] > 0 && r.prefered_seats[i] <= num_room_seats)) {
      freeSeats(num_reserved_seats, reserved_seats);
      return IID;
    }

    if(isSeatFree(roomSeats, r.prefered_seats[i]) && num_reserved_seats < r.num_wanted_seats) {
      bookSeat(roomSeats, r.prefered_seats[i], r.clientID);
      reserved_seats[num_reserved_seats] = r.prefered_seats[i];
      num_reserved_seats++;
    }
  }

  if(num_reserved_seats < r.num_wanted_seats) {
    freeSeats(num_reserved_seats, reserved_seats);
    return NAV;
  }

  if(r.num_wanted_seats < 0 || r.clientID < 0) {
    return ERR;
  }

  return 0;
}

void freeSeats(int n, int seats[]) {
  int i;
  for(i = 0; i < n; i++) {
      freeSeat(roomSeats, seats[i]);
    }
}

int isSeatFree(struct Seat *seats, int seatNum){
  int i;
  for(i = 0; i < num_room_seats; i++){
    if(seats->seatNumber == seatNum){
      if(seats->occupied == 0){
        DELAY();
        return 1;
      }
      else{
        DELAY();
        return 0;
      }
    }
    seats++;
  }
  DELAY();
  return 0;
}

void bookSeat(struct Seat * seats, int seatNum, int clientID){
  int i;
  for(i = 0; i < num_room_seats; i++){
    if(seats->seatNumber == seatNum){
      if(seats->occupied == 0){
        seats->occupied = 1;
        seats->clientID = clientID;
        DELAY();
        return;
      }
    }
    seats++;
  }
  DELAY();
}

void freeSeat(struct Seat *seats, int seatNum){
  int i;
  for(i = 0; i < num_room_seats; i++){
    if(seats->seatNumber == seatNum){
      seats->occupied = 0;
      DELAY();
      return;
    }
    seats++;
  }
  DELAY();
}

// WRITE TO FILE FUNCTIONS

void resetClientFiles() {
  FILE *f = fopen(CLIENTS_BOOKINGS, "w");
  fprintf(f, "");
  fclose(f);

  f = fopen(CLIENTS_LOG, "w");
  fprintf(f, "");
  fclose(f);
}

void slogOpenClose(int no, int open) {
  if(no < 10)
      fprintf(slog, "0");
  fprintf(slog, "%d", no);
  if(open)
      fprintf(slog, "-OPEN\n");
  else
      fprintf(slog, "-CLOSED\n");
}

void slogRequest(int no, struct Request r, int ret, int reserved_seats[]) {
  if(no < 10)
    fprintf(slog, "0");
  fprintf(slog, "%d-", no);

  char clientID_s[WIDTH_PID + 1];
  getFullClientId(clientID_s, r.clientID);
  fprintf(slog, "%s-", clientID_s);

  if(r.num_wanted_seats < 10)
    fprintf(slog, "0");
  fprintf(slog, "%d: ", r.num_wanted_seats);  

  int i, j;
  for(i = 0; i < r.num_prefered_seats; i++) {
    char seat[WIDTH_SEAT + 1];
    getFullSeatNumber(seat, r.prefered_seats[i]);
    fprintf(slog, "%s ", seat);
    if(i == MAX_CLI_SEATS - 1) break;
  }

  for(i = 0; i < MAX_CLI_SEATS - r.num_prefered_seats; i++) {
    for(j = 0; j <= WIDTH_SEAT; j++) {
      fprintf(slog, " ");
    }
  }

  fprintf(slog, "- ");

  switch(ret) {
    case MAX: fprintf(slog, "MAX"); break;
    case NST: fprintf(slog, "NST"); break;
    case IID: fprintf(slog, "IID"); break;
    case ERR: fprintf(slog, "ERR"); break;
    case NAV: fprintf(slog, "NAV"); break;
    case FUL: fprintf(slog, "FUL"); break;
    case OUT: fprintf(slog, "OUT"); break;
    case 0:
      for(i = 0; i < r.num_wanted_seats; i++) {
        char seat[WIDTH_SEAT + 1];
        getFullSeatNumber(seat, reserved_seats[i]);
        fprintf(slog, "%s ", seat);
      }

      for(i = 0; i < MAX_CLI_SEATS - r.num_wanted_seats; i++) {
        for(j = 0; j <= WIDTH_SEAT; j++) {
          fprintf(slog, " ");
        }
      }
    default: break;
  }
 
  fprintf(slog, "\n");

}

void sbookReservations() {

  FILE *sbook = fopen(SERVER_BOOKINGS, "w");

  int i;
  for(i = 0; i < num_room_seats; i++) {
    if(roomSeats[i].occupied) {
      char seat[WIDTH_SEAT + 1];
      getFullSeatNumber(seat, i + 1);
      fprintf(sbook, "%s\n", seat);
    }
  }
  fclose(sbook);
}
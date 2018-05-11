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
  
  struct Request request;
  request.processed = 1;
  buffer[0] = request;

  //Starting Time Count
  alarm(open_time);

  //Create threads
  pthread_t tid[num_ticket_offices];
  int i;
  for(i = 0; i < num_ticket_offices; i++) {
    int rc = pthread_create(&tid[i], NULL, ticketOffice, NULL);
    if(rc) {
      perror("Thread");
      return 5;
    }
  }
  
  //Reading FIFO
  while(!timeout) {
    if(buffer[0].processed) {
      if(read(requests, &request, sizeof(struct Request)) < 0) {
        perror("READ");
      }
      buffer[0] = request;
    }
  }
  
  close(requests);

  for(i = 0; i < num_ticket_offices; i++) {
    pthread_join(tid[i], NULL);
  }
  
  //Destroying FIFO
  if(destroyFIFO(FIFO_SERVER) != 0) {
    return 4;
  }

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
    roomSeats[n].seatNumber = n + 1;
    roomSeats[n].occupied = 0;
    roomSeats[n].clientID = 0;
  }
}

int createFIFO(char *name) {
   if(mkfifo(name,0777) < 0){
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
  struct Request r;
  while(!timeout) {
    int status = pthread_mutex_trylock(&mut);
    if (!buffer[0].processed && status != EBUSY) {
      r = buffer[0];
      buffer[0].processed = 1;

      int reserved_seats[r.num_wanted_seats];
      int ret = checkRequest(r, reserved_seats);
      pthread_mutex_unlock(&mut);

      char fifo_name[MAX_STR_LEN];
      answerFifoName(fifo_name, r.clientID);

      int fd = open(fifo_name, O_WRONLY);
      if (fd < 0) {
        printf("FIFO '%s' failed to open in WRITEONLY mode\n", fifo_name);
        exit(1);
      }

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
  }

  return NULL;
}

void answerFifoName(char *name, int clientID) {
  char clientID_s[WIDTH_PID];

  strcat(name, "ans");
  sprintf(clientID_s, "%d", clientID);
  int i;
  for(i = strlen(clientID_s); i < WIDTH_PID; i++) {
    strcat(name, "0");
  }
  strcat(name, clientID_s);
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
    return -1;
  }

  if(!(r.num_prefered_seats >= r.num_wanted_seats && r.num_prefered_seats <= MAX_CLI_SEATS)) {
    return -2;
  }

  int i, num_reserved_seats = 0;
  for(i = 0; i < r.num_prefered_seats; i++) {
    if(!(r.prefered_seats[i] > 0 && r.prefered_seats[i] <= num_room_seats)) {
      return -3;
    }
    if(isSeatFree(roomSeats, r.prefered_seats[i]) && num_reserved_seats < r.num_wanted_seats) {
      bookSeat(roomSeats, r.prefered_seats[i], r.clientID);
      reserved_seats[num_reserved_seats] = r.prefered_seats[i];
      num_reserved_seats++;
    }
  }

  if(num_reserved_seats < r.num_wanted_seats) {
    for(i = 0; i < num_reserved_seats; i++) {
      freeSeat(roomSeats, reserved_seats[i]);
    }

    return -5;
  }

  if(r.num_wanted_seats < 0 || r.clientID < 0) {
    return -4;
  }

  if(isRoomFull()) {
    return -6;
  }

  return 0;
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
    }
    seats++;
  }
  DELAY();
}
#include "constants.h"

int num_room_seats;

struct Seat{
  int seatNumber;
  int occupied; //0 - free, 1 - occupied
  int clientID;
};

//return 0 if the seat is free, -1 if the seat is occupied and -2 if the seat doens't exist
int isSeatFree(struct Seat *seats, int seatNum){
  int i;
  for(i = 0; i < num_room_seats; i++){
    if(seats->seatNumber == seatNum){
      if(seats->occupied == 0){
        DELAY();
        return 0;
      }
      else{
        DELAY();
        return -1;
      }
    }
    seats++;
  }
  DELAY();
  return -2;
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

int main(int argc, char* argv[]){
  if (argc < 2) {
    printf("Usage: %s <num_room_seats> <num_ticket_offcies> <open_time>\n", argv[0]);
    return 1;
  }
  if (argc < 4){
    printf("Missing arguments!\n");
    return 1;
  }

  num_room_seats = atoi(argv[1]);
  int num_ticket_offcies = atoi(argv[2]);
  int open_time = atoi(argv[3]);

  //Creating the Room Seats
  struct Seat roomSeats[num_room_seats];
  int n;
  for(n = 0; n < num_room_seats; n++){
    roomSeats[n].seatNumber = n + 1;
    roomSeats[n].occupied = 0;
    roomSeats[n].clientID = 0;
  }

  //Starting Time Count
  time_t start, end;
  double elapsed;
  start= time(NULL);

  //Making FIFO
  int fd;
  
  if(mkfifo("requests",0660) < 0){
    if (errno == EEXIST)
      printf("FIFO 'requests' already exists\n");
    else
      printf("Can't create FIFO\n");
  }

  if((fd=open("requests", O_RDONLY)) == -1)
    printf("FIFO 'requests' failed to open in WRITEONLY mode\n");

  int m;
  char str[MAX_MSG_LEN];
  
  //Reading FIFO
  do{
    m = read(fd, str, MAX_MSG_LEN);
    //ver aqui como vamos passar a informação pelos fifos
    
    if(n>0)
      printf("%s", str);
    
    end = time(NULL);
    elapsed = difftime(end, start);
  } while (elapsed <= open_time);

  close(fd);

  //Destroying FIFO
  if (unlink("requests") < 0)
    printf("Error when destroying FIFO 'requests'\n");

  exit(0);
}
#include "constants.h"

int main(int argc, char* argv[]){
  if (argc < 2) {
    printf("Usage: %s <time_out> <num_wanted_seats> <pref_seat_list>\n", argv[0]);
    return 1;
  }
  if (argc < 4){
    printf("Missing arguments!\n");
    return 1;
  }

  int time_out = atoi(argv[1]);
  int num_wanted_seats = atoi(argv[2]);

//////////////////////////////////////////////////////////
// Code to get pref_seat_list
  char newString[sizeof(argv[3])][sizeof(argv[3])];
  int i, j=0, ctr=0;
  for(i=0; i <= (strlen(argv[3])); i++){
    if(argv[3][i] == ' ' || argv[3][i]=='\0'){
      newString[ctr][j]='\0';
      ctr++;
      j=0;
    }
    else{
      newString[ctr][j]= argv[3][i];
      j++;
    }
  }
  int pref_seat_list[ctr];
  for(i=0;i < ctr;i++){
    pref_seat_list[i] = atoi(newString[i]);
  }
//////////////////////////////////////////////////////////

  //Making FIFO to get the answer from the server
  int fd_answer;
  char fifo_name[10];

  char *end = fifo_name;
  end += sprintf(end, "ans%ld", (long)getpid());
  
  if(mkfifo(fifo_name,0660) < 0){
    if (errno == EEXIST)
      printf("FIFO '%s' already exists\n", fifo_name);
    else
      printf("Can't create FIFO\n");
  }


  //Writing to FIFO requests
  int fd_requests, messagelen;
  char message[100];

  fd_requests=open("requests", O_WRONLY);
  if(fd_requests == -1){
   printf("Ticket offices closed!\n");
   exit(1);
  }

  sprintf(message, "I am process no. %d, i want %d seats\n", getpid(), num_wanted_seats);
  //falta enviar os pref_seat_list
  messagelen = strlen(message) +1;
  write(fd_requests, message, messagelen);
  close(fd_requests);


  //Reading FIFO Answers
  if((fd_answer=open(fifo_name, O_RDONLY)) == -1)
    printf("FIFO 'ans' failed to open in READONLY mode\n");

  close(fd_answer);

    //Destroying FIFO
  if (unlink(fifo_name) < 0)
    printf("Error when destroying FIFO '%s'\n", fifo_name);

  return 0;
}

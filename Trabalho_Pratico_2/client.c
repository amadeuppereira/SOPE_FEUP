#include "client.h"

int timeout = 0;
char fifo_name[10];

int main(int argc, char* argv[]){
  if (argc < 2) {
    printf("Usage: %s <time_out> <num_wanted_seats> <pref_seat_list>\n", argv[0]);
    return 1;
  }
  if (argc < 4){
    printf("Missing arguments!\n");
    return 1;
  }

   //Alarm handler
  struct sigaction action;
  action.sa_handler = alarm_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGALRM, &action, NULL);

  //Reading the parameters
  int time_out;
  int num_wanted_seats;
  int* pref_seat_list = malloc(MAX_PREFERED_SEATS*sizeof(int));

  if(readParameters(&time_out, &num_wanted_seats, pref_seat_list, argv) != 0) {
    printf("Invalid Arguments!\n");
    return 1;
  }


  //Making FIFO to get the answer from the server
  int fd_answer;

  char *end = fifo_name;
  end += sprintf(end, "ans%ld", (long)getpid());
  
  if(mkfifo(fifo_name,0660) < 0){
    if (errno == EEXIST)
      printf("FIFO '%s' already exists\n", fifo_name);
    else
      printf("Can't create FIFO\n");
  }

  //Starting Time Count
  alarm(time_out);

  //Writing to FIFO requests
  int requests=open("requests", O_WRONLY);
  if(requests == -1){
   printf("Ticket offices closed!\n");
   exit(1);
  }

  struct Request request;
  request.clientID = getpid();
  request.num_wanted_seats = num_wanted_seats;
  //request.prefered_seats = pref_seat_list;

  write(requests, &request, sizeof(struct Request));
  close(requests);


  //Reading FIFO Answers
  if((fd_answer=open(fifo_name, O_RDONLY)) == -1)
    printf("FIFO 'ans' failed to open in READONLY mode\n");

  close(fd_answer);

    //Destroying FIFO
  // if (unlink(fifo_name) < 0)
  //   printf("Error when destroying FIFO '%s'\n", fifo_name);

  return 0;
}

int readParameters(int *time_out, int *num_wanted_seats, int pref_seat_list[], char *argv[]) {
  *time_out = atoi(argv[1]);
  *num_wanted_seats = atoi(argv[2]);
  
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
  for(i=0;i < ctr;i++){
    pref_seat_list[i] = atoi(newString[i]);
  }

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

  //Destroying FIFO
  if(destroyFIFO(fifo_name) != 0) {
    exit(4);
  }
  printf("timeout\n");
  exit(1);
}
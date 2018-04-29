#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constantes.h"

int main(int argc, char* argv[]){
  if (argc < 2) {
    printf("Usage: %s <num_room_seats> <num_ticket_offcies> <open_time>\n", argv[0]);
    return 1;
  }
  if (argc < 4){
    printf("Missing arguments!\n");
    return 1;
  }

  int num_room_seats = atoi(argv[1]);
  int num_ticket_offcies = atoi(argv[2]);
  int open_time = atoi(argv[3]);






  return 0;
}

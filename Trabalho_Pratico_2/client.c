#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constantes.h"

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


//ver DELAY()



  return 0;
}

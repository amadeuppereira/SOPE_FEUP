#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define BUFFER_SIZE 512

int main(int argc, char* argv[])
{
   if (argc < 2)
   {
     printf("Error on number of arguments.\n");
     return -1;
   }


   int file;
   file = open(argv[argc - 1], O_RDONLY) // O_RDONLY para ler

   if(file == -1)
   {
     perror(argv[argc - 1]);
     close(file);
     return -1;
   }

   close(file);
}

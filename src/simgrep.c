#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>

#define BUFFER_SIZE 512
#define DEFAULT_PATH "(standart input)"

//Global variables hold options state
int optionI = 0;
int optionL = 0;
int optionN = 0;
int optionC = 0;
int optionW = 0;
int optionR = 0;
//Global variable with pattern
char givenPattern[BUFFER_SIZE];
//Global variable with file/dir
char givenPath[BUFFER_SIZE] = DEFAULT_PATH;

//Checks if a given path is valid
int validPath(const char *path);
//Returns if a given path is a regular file
int isFile(const char *path);
//Reads given parameters
int readParameters(int argc, char* argv[]);
//Processes a given file looking for a given pattern
void processFile(const char *path, const char* pattern);
/*
Checks if a given string(str1) constains another string(str2)
taking in consideration the given options
*/
int strContains(const char* str1, const char* str2);

int main(int argc, char* argv[]) {
  int p;
  if (argc < 2 || (p = readParameters(argc, argv)) != 0) {
    printf("Usage: %s [options] pattern [file/dir]\n", argv[0]);
    return 1;
  }

  if(strcmp(givenPath, DEFAULT_PATH) != 0 && !validPath(givenPath)) {
    return 2;
  }

  if((strcmp(givenPath, DEFAULT_PATH) == 0) || isFile(givenPath)) {
    processFile(givenPath, givenPattern);
  }
  else {
    if(!optionW) {
      printf("%s: Is a directory\n", givenPath);
    }
    else {
      /*
      TODO: -w option
      */
    }
  }

  return 0;
}

int validPath(const char *path) {
  struct stat stat_buf;
  if (stat(path, &stat_buf) != 0) {
    perror(path);
    return 0;
  }
  return 1;
}

int isFile(const char *path) {

  struct stat stat_buf;
  if (stat(path, &stat_buf) != 0) {
    perror(path);
    return 0;
  }
  return S_ISREG(stat_buf.st_mode);
}

void processFile(const char* path, const char* pattern) {

    FILE* f;
    if(strcmp(path, DEFAULT_PATH) == 0) {
      f = stdin;
    }
    else {
     f = fopen(path, "r"); //opens file
     if(f == NULL) {
       perror(path);
       return;
     }
    }

    char line[BUFFER_SIZE];
    int lineNumber = 1;
    int totalLines = 0;
    while(fgets(line, BUFFER_SIZE, f) != NULL) {
      if(strContains(line, pattern)) {
        if(optionL) {
          printf("%s\n", path);
          break;
        }
        else if(optionN) {
          printf("%d:%s", lineNumber, line);
        }
        else if(!optionC) {
          printf("%s", line);
        }

        totalLines++;
      }
      lineNumber++;
    }

    if(optionC) {
      printf("%d\n", totalLines);
    }


    fclose(f);
}

int isDelimiterChar(char c) {
  return !((c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'Z') ||
         (c >= 'a' && c <= 'z') ||
         (c == '_'));
}

int strContains(const char* str1, const char* str2) {

  if(optionI && !optionW) { // -i
    //checks if str2 is a substring of str1 (case insensitive)
    if(strcasestr(str1, str2) != NULL) {
      return 1;
    }
    else {
      return 0;
    }
  }
  else if(optionW) { // -w

    const char* s = str1;
    char* temp;
    size_t len = strlen(str2);

    while (1) {
      if(optionI) {
        temp = strcasestr(s,str2);
      }
      else {
        temp = strstr(s,str2);
      }
      if(temp == NULL) break;
      if((temp == str1 || isDelimiterChar(temp[-1])) && isDelimiterChar(temp[len])) {
        return 1;
      }
      else {
        s++;
      }
    }

    return 0;
  }
  else { //no options given
    //checks if str2 is a substring of str1 (case sensitive)
    if(strstr(str1, str2) != NULL) {
      return 1;
    }
    else {
      return 0;
    }
  }

  return 0;
}

int readParameters(int argc, char* argv[]) {
  int flag = 0; //if 1 -> all options have been read

  for(int i = 1; i < argc; i++) {
    if(strcmp(argv[i], "-i") == 0) {
      if(flag) {
        return 1;
      }
      optionI = 1;
    }
    else if(strcmp(argv[i], "-l") == 0) {
      if(flag) {
        return 1;
      }
      optionL = 1;
    }
    else if(strcmp(argv[i], "-n") == 0) {
      if(flag) {
        return 1;
      }
      optionN = 1;
    }
    else if(strcmp(argv[i], "-c") == 0) {
      if(flag) {
        return 1;
      }
      optionC = 1;
    }
    else if(strcmp(argv[i], "-w") == 0) {
      if(flag) {
        return 1;
      }
      optionW = 1;
    }
    else if(strcmp(argv[i], "-r") == 0) {
      if(flag) {
        return 1;
      }
      optionR = 1;
    }
    else {
      if(flag) {
        if(strcmp(givenPath, DEFAULT_PATH) != 0) {
          return 1;
        }
        strcpy(givenPath, argv[i]);
      }
      else {
        flag = 1;
        strcpy(givenPattern, argv[i]);
      }
    }
  }
  //no not option string given
  if(!flag) {
    return 1;
  }
  return 0;
}

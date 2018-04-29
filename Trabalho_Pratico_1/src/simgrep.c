#define _GNU_SOURCE
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <ctype.h>
#include <dirent.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

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
//Global variable with the parent pid
int parentPid;
//Global variable with the time when the program has started
struct timeval startTime;
//Global variable with the logfile
char* logfileName;
FILE *logfile;

//------------------------------------------

//Checks if a given path is valid
int validPath(const char *path);
//Returns if a given path is a regular file
int isFile(const char *path);
//Reads given parameters
int readParameters(int argc, char* argv[]);
//Processes a given file looking for a given pattern
void processFile(const char *path, const char* pattern);
//Processes a given directory
void processDirectory(const char* path);
//Checks if a given string(str1) constains another string(str2)
//taking in consideration the given options
int strContains(const char* str1, const char* str2);
//Get logfile path
int getLogfile();
//Write in the logfile
void logPrint(const char* act);

//---------------------------------------------

void sig_handler(int signo) {

  switch(signo) {
    case SIGUSR1:
      logPrint("SIGUSR1");
      exit(0);
    case SIGINT:
      logPrint("SIGINT");
      while(parentPid == getpid()) {
        printf("\nAre you sure you want to terminate the program? (Y/N)\n");
        char op;
        scanf(" %c", &op);
        while(getchar() != '\n');
        if(op == 'Y' || op == 'y') kill(-parentPid, SIGUSR1);
        else if (op == 'N' || op == 'n') {
          kill(-parentPid, SIGUSR2);
          return;
        }
      }
      if(parentPid != getpid()) {
        sigset_t mask;
        sigemptyset(&mask);
        sigsuspend(&mask);
      }
      break;
    case SIGUSR2:
      logPrint("SIGUSR2");
      break;
    default:
      break;
  }
}

int main(int argc, char* argv[], char* envp[]) {
  gettimeofday(&startTime, NULL);
  setbuf(stdout, NULL);

  if(getLogfile() != 0) {
    return 3;
  }

  char msg[128] = "COMANDO ";
	int i;
	for (i = 0; i < argc; ++i) // Generate command to output to log file
	{
		strcat(msg, argv[i]);
		if (i != argc-1)
			strcat(msg, " ");
	}
	logPrint(msg);

  int p;
  if (argc < 2 || (p = readParameters(argc, argv)) != 0) {
    printf("Usage: %s [options] pattern [file/dir]\n", argv[0]);
    fclose(logfile);
    return 1;
  }


  if(strcmp(givenPath, DEFAULT_PATH) != 0 && !validPath(givenPath)) {
    fclose(logfile);
    return 2;
  }

  parentPid = getpid();

  struct sigaction action;
  action.sa_handler = sig_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGUSR1, &action, NULL);
  sigaction(SIGUSR2, &action, NULL);


  if((strcmp(givenPath, DEFAULT_PATH) == 0) || isFile(givenPath)) {
    processFile(givenPath, givenPattern);
  }
  else {
    if(!optionR) {
      printf("%s: Is a directory\n", givenPath);
    }
    else {
      processDirectory(givenPath);
    }
  }

  fclose(logfile);
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

void processDirectory(const char* path) {

  DIR *d;
  struct dirent *dir;

  d = opendir(path);
  if (d) {
    while ((dir = readdir(d))) {
      char newPath[BUFFER_SIZE];
      sprintf(newPath, "%s%s%s", path, "/", dir->d_name);
      if (dir->d_type == DT_REG) {
        processFile(newPath, givenPattern);
      }
      else if(dir->d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0) {
        pid_t pid = fork();
        if(pid < 0) {
          perror("Fork");
        }
        else if(pid == 0) {
          processDirectory(newPath);
          exit(0);
        }
      }
    }
  }

  closedir(d);
}

void processFile(const char* path, const char* pattern) {
    int flag = 1;

    FILE* f;
    if(strcmp(path, DEFAULT_PATH) == 0) {
      flag = 0;
      f = stdin;
    }
    else {
      char msg[128];
      sprintf(msg, "ABERTO %s", path);
      logPrint(msg);
      f = fopen(path, "r"); //opens file
      if(f == NULL) {
        perror(path);
        return;
      }
    }

    char line[BUFFER_SIZE];
    int lineNumber = 1;
    int totalLines = 0;
    while(1) {
      if(fgets(line, BUFFER_SIZE, f) == NULL) {
        if(errno == EINTR) {
          errno = 0;
          continue;
        }
        break;
      }
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
      printf("%s: %d\n", path, totalLines);
    }

    if(flag) {
      char msg[128];
      sprintf(msg, "FECHADO %s", path);
      logPrint(msg);
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
        char buf[BUFFER_SIZE + 1];
        realpath(argv[i], buf);
        strcpy(givenPath, buf);
      }
      else {
        flag = 1;
        strcpy(givenPattern, argv[i]);
      }
    }
  }
  //no option string given
  if(!flag) {
    return 1;
  }
  return 0;
}

int getLogfile() {
  if((logfileName = getenv("LOGFILENAME")) == NULL) {
    printf("LOGFILENAME not found!\n");
    return 1;
  }


  char buf[BUFFER_SIZE + 1];
  realpath(logfileName, buf);
  logfile = fopen(buf, "a");
  if(logfile == NULL) {
    perror(buf);
    return 1;
  }
  return 0;
}

void logPrint(const char* act) {
  struct timeval now;
  gettimeofday(&now, NULL);
  double diff = (double)(now.tv_usec - startTime.tv_usec);
  fprintf(logfile, "%.2f - %d - %s\n",diff, getpid(), act);
}

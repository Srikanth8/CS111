#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>

/* Flag set by ‘--verbose’. */
static int verbose_flag = 0;
static int append_flag = 0;
static int cloexec_flag = 0;
static int creat_flag = 0;
static int directory_flag = 0;
static int dsync_flag = 0;
static int excl_flag = 0;
static int nofollow_flag = 0;
static int nonblock_flag = 0;
static int rsync_flag = 0;
static int sync_flag = 0;
static int trunc_flag = 0;

void verbose(int argc, char* argv[], int position);

int isNumber(char* string);

void addElement(int element, int* array, int size);

int ORoflags(int append_flag, int cloexec_flag, int creat_flag, int directory_flag, int dsync_flag, int excl_flag, int nofollow_flag, int nonblock_flag, int rsync_flag, int sync_flag, int trunc_flag);

void signal_handler(int signal_number);

int main(int argc, char* argv[])
{ 
  int c;
  int* FDarray = malloc(1*sizeof(int));
  int FDsize = 0;
  int oflags = 0;
  
  static struct option long_options[] =
    {
      /* These options set a flag. */
      {"verbose", no_argument, &verbose_flag, 1},
      {"append", no_argument, &append_flag, 1},
      {"cloexec", no_argument, &cloexec_flag, 1},
      {"creat", no_argument, &creat_flag, 1},
      {"directory", no_argument, &directory_flag, 1},
      {"dsync", no_argument, &dsync_flag, 1},
      {"excl", no_argument, &excl_flag, 1},
      {"nofollow", no_argument, &nofollow_flag, 1},
      {"nonblock", no_argument, &nonblock_flag, 1},
      {"rsync", no_argument, &rsync_flag, 1},
      {"sync", no_argument, &sync_flag, 1},
      {"trunc", no_argument, &trunc_flag, 1},
      /* These options don’t set a flag.
	 We distinguish them by their indices. */
      {"rdonly",  required_argument, 0, 'r'},
      {"wronly",  required_argument, 0, 'w'},
      {"rdwr", required_argument, 0, 'd'},
      {"command", no_argument, 0, 'c'},
      {"pipe", no_argument, 0, 'p'},
      {"pause", no_argument, 0, 's'},
      {"catch", required_argument, 0, 'h'},
      {0, 0, 0, 0}
    };
  
  while ((c = getopt_long(argc, argv, "cr:w:psh:", long_options, NULL)) != -1)
    {
      int fd;
      switch (c)
	{
	case 'r':	  
	  if (verbose_flag == 1)
	    verbose(argc, argv, optind-2);

	  oflags = ORoflags(append_flag, cloexec_flag, creat_flag, directory_flag, dsync_flag, excl_flag, nofollow_flag, nonblock_flag, rsync_flag, sync_flag, trunc_flag);

	  if (creat_flag == 1)
	    fd = open(optarg, O_RDONLY | oflags, 0666);
	  else
	    fd = open(optarg, O_RDONLY | oflags);
	  
	  if (optarg[0] == '-' && optarg[1] == '-')
	    {
	      fprintf(stderr, "Missing operand for --rdonly option!\n");
	      break;
	    }
	  if (fd == -1)
	    {
	      fprintf(stderr, "%s %s\n", "Error opening file", optarg);
	      break;
	    }
	  FDsize++;
	  addElement(fd, FDarray, FDsize);
	  break;
	  
	case 'w':
	  if (verbose_flag == 1)
	    verbose(argc, argv, optind-2);
	  
	  oflags = ORoflags(append_flag, cloexec_flag, creat_flag, directory_flag, dsync_flag, excl_flag, nofollow_flag, nonblock_flag, rsync_flag, sync_flag, trunc_flag);

	  if (creat_flag == 1)
	    fd = open(optarg, O_WRONLY | oflags, 0666);
	  else
	    fd = open(optarg, O_WRONLY | oflags);
	    
	  if (optarg[0] == '-' && optarg[1] == '-')
	    {
	      fprintf(stderr, "Missing operand for --wronly option!\n");
	      break;
	    }
	  if (fd == -1)
	    {
	      fprintf(stderr, "%s %s\n", "Error opening file", optarg);
	      break;
	    }
	  FDsize++;
	  addElement(fd, FDarray, FDsize);
	  break;

	case 'd': //RDWR
	  if(verbose_flag)
	      verbose(argc, argv, optind-2);

	  oflags = ORoflags(append_flag, cloexec_flag, creat_flag, directory_flag, dsync_flag, excl_flag, nofollow_flag, nonblock_flag, rsync_flag, sync_flag, trunc_flag);

	  if (creat_flag == 1)
	    fd = open(optarg, O_RDWR | oflags, 0666);
	  else
	    fd = open(optarg, O_RDWR | oflags);

	  if (optarg[0] == '-' && optarg[1] == '-')
	    {
	      fprintf(stderr, "Missing operand for --wronly option!\n");
	      break;
	    }
	  if (fd == -1)
	    {
	      fprintf(stderr, "%s %s\n", "Error opening file", optarg);
	      break;
	    }
	  FDsize++;
	  addElement(fd, FDarray, FDsize);
	  //pipeSize++;
	  //addElement (-1, pipeArray, pipeSize);
	  break;	  

	case 'p':
	  if (verbose_flag)
	    verbose(argc, argv, optind-1);

	  int temp_fd[2];
	  pipe(temp_fd);
	  
	  FDsize++;
	  addElement (temp_fd[0], FDarray, FDsize);
	  //pipeSize++;
	  //addElement (0, pipeArray, pipeSize);
	  
	  FDsize++;
	  addElement (temp_fd[1], FDarray, FDsize);
	  //pipeSize++;
	  //addElement (1, pipeArray, pipeSize);
	  break;

	case 's':
	  if (verbose_flag)
	    verbose(argc, argv, optind-1);
	  pause();
	  break;

	case 'h':
	  if (verbose_flag)
	    verbose(argc, argv, optind-1);

	  int signal_number = atoi(optarg);
	  if (signal(signal_number, signal_handler) == SIG_ERR)
	    fprintf(stderr, "Unable to catch signal!\n");
	  break;
	  
	case 'c':
	  if (verbose_flag)
	    verbose(argc, argv, optind-1);

	  int i = atoi(argv[optind]);
	  int o = atoi(argv[optind+1]);
	  int e = atoi(argv[optind+2]);

	  if (isNumber(argv[optind])*isNumber(argv[optind+1])*isNumber(argv[optind+2]) == 0)
	    {
	      fprintf(stderr, "The first three arguments to --command must be nonnegative integers\n");
	      break;
	    }
	  
	  if (!(i < FDsize && o < FDsize && e < FDsize && i >= 0 && o >= 0 && e >= 0))
	    {
	      fprintf(stderr, "Invalid file descriptor(s) used in --command option!\n");
	      break;
	    }
	  
	  char** cmdArray = malloc(1*sizeof(char*));	  
	  int n = 0;
	  int x;
	  for (x=optind+3; x < argc; x++)
	    {
	      if (argv[x][0] == '-' && argv[x][1] == '-')
		break;
	      n++;
	      char** tempcmd=realloc(cmdArray, n*sizeof(char*));
	      if (tempcmd != NULL)
		{		  
		  cmdArray=tempcmd;
		}
	      else
		{
		  fprintf(stderr, "Error allocating memory!\n");
		  exit(1);
		}
	      cmdArray[n-1] = argv[x];
	    }

	  n++;
	  char** tempcmd=realloc(cmdArray, n*sizeof(char*));
	  if (tempcmd != NULL)
	    {		  
	      cmdArray=tempcmd;
	    }
	  else
	    {
	      fprintf(stderr, "Error allocating memory!\n");
	      exit(1);
	    }
	  cmdArray[n-1] = NULL;

	  int yy;
	  for (yy=0; cmdArray[yy]!=NULL; yy++)
	    printf("%s\n", cmdArray[yy]);
	  
	  if (n == 0)
	    {
	      fprintf(stderr, "No command argument has been provided for the --command option\n");
	      break;
	    }
	  
	  pid_t pid;
	  pid = fork();

	  if (pid == 0)
	    {	      
	      int saved_stderr = dup(2);
	      
	      dup2(FDarray[i], 0);
	      dup2(FDarray[o], 1);
	      dup2(FDarray[e], 2);

	      int k;
	      for (k=0; k<FDsize; k++)
		  close(FDarray[k]);
	      
	      int ret = execvp (cmdArray[0], cmdArray);

	      if (ret == -1)
		{
		  dup2(saved_stderr, 2);
		  fprintf(stderr, "Command could not be executed!\n");
		  exit(1);
		}	      
	    }
	  else if (pid == -1)
	    {
	      fprintf(stderr, "Error forking the process!\n");
	    }
	  break;
	  	  
	  case 0:
	    /*getopt_long() set a variable, just keep going*/
	  break;
	  }
    }
  int m;
  for (m=0; m<FDsize; m++)
    close(FDarray[m]);
  exit (0);
}

void verbose(int argc, char* argv[], int position)
{
  printf("%s ", argv[position]);
  position++;
  while (position < argc)
    {
      if (argv[position][0] == '-' && argv[position][1] == '-')
	break;
      printf("%s ", argv[position]);
      position++;
    }
  printf("%c", '\n');
}

int isNumber(char* string)
{
  int isNumber=1;
  int i;
  for (i=0; string[i] != '\0'; i++)
      if (isdigit(string[i]) == 0 && string[i] != '0')
	isNumber = 0;
  return isNumber;
}

void addElement(int element, int* array, int size)
{
  int *temp=realloc(array, size*sizeof(int));
  if (temp != NULL)
    {
      array=temp;
    }
  else
    {
      fprintf(stderr, "Error allocating memory!\n");
      exit(1);
    }
  array[size-1] = element;
}

int ORoflags(int append_flag, int cloexec_flag, int creat_flag, int directory_flag, int dsync_flag, int excl_flag, int nofollow_flag, int nonblock_flag, int rsync_flag, int sync_flag, int trunc_flag)
{
  int oflags = append_flag*O_APPEND | cloexec_flag*O_CLOEXEC | creat_flag*O_CREAT | directory_flag*O_DIRECTORY | dsync_flag*O_DSYNC | excl_flag*O_EXCL | nofollow_flag*O_NOFOLLOW | nonblock_flag*O_NONBLOCK | rsync_flag*O_RSYNC | sync_flag*O_SYNC | trunc_flag*O_TRUNC;

append_flag = 0;
cloexec_flag = 0;
creat_flag = 0;
directory_flag = 0;
dsync_flag = 0;
excl_flag = 0;
nofollow_flag = 0;
nonblock_flag = 0;
rsync_flag = 0;
sync_flag = 0;
trunc_flag = 0;

 return oflags;
}

void signal_handler(int signal_number)
{
  fprintf(stderr, "%d caught\n", signal_number);
  exit(signal_number);
}


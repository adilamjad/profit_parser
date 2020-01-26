#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define GEN_OUTPUT_FILES 	0
#define PRINT_ROW 			1
#define FRAME_COUNT 		10
#define PKTS_PER_FRAME 		256

#define SINK_01     1
//==================================================================================================//
// Parsing Functions

int readline(int fd, char *buf, int nbytes)
{
	int numread = 0;
	int returnval;

	while(numread < nbytes - 1 )
	{
		returnval = read(fd, buf + numread, 1);
		if((returnval == -1) && (errno == EINTR) )
			continue;

		if( (returnval == 0) && (numread == 0) )
			return 0;

		if(returnval == 0)
			break;

		if(returnval == -1)
			return -1;

		numread++;

		if(buf[numread-1] == '\n')
		{
			buf[numread] = '\0';
			return numread;
		}
	}
	errno = EINVAL;
	return -1;
}

int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;

   if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      errno = EINVAL;
      return -1;
   }
   *argvp = NULL;
   snew = s + strspn(s, delimiters);         /* snew is real start of string */
   if ((t = (char*)malloc(strlen(snew) + 1)) == NULL)
      return -1;
   strcpy(t, snew);
   numtokens = 0;
   if (strtok(t, delimiters) != NULL)     /* count the number of tokens in s */
      for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

                             /* create argument array for ptrs to the tokens */
   if ((*argvp = (char**)malloc((numtokens + 1)*sizeof(char *))) == NULL) {
      error = errno;
      free(t);
      errno = error;
      return -1;
   }
                        /* insert pointers to tokens into the argument array */
   if (numtokens == 0)
      free(t);
   else {
      strcpy(t, snew);
      **argvp = strtok(t, delimiters);
      for (i = 1; i < numtokens; i++)
          *((*argvp) + i) = strtok(NULL, delimiters);
    }
    *((*argvp) + numtokens) = NULL;             /* put in final NULL pointer */
    return numtokens;
}

int getID(char argv[])
{
	int data;

	//int len;
	//len = strlen(argv);
	//printf("argv[%d]=%s\n", len, argv);

	data = atoi(argv);
	return data;
}

int getData(char argv[])
{
	int data;
	int i = 0;
	int len;
	char *ptr;

	len = strlen(argv);
	argv[len-1] = 0;
	ptr = argv;
	while(i<len)
	{
		if(argv[i]==':')
			ptr= &argv[i+1];
		i++;
	}

	data = atoi(ptr);
	return data;
}

void getAddressData(char argv[], char* optr)
{
	int i = 0;
	int len;
	char *ptr = argv;

	len = strlen(argv);
	argv[len-2] = 0;
	while(i<len)
	{
		if(argv[i]==':')
			ptr= &argv[i+1];
		i++;
	}
	strcpy(optr, ptr);

}

//==================================================================================================//
// Data Structures

typedef struct _sender_data
{
	int sysTime;
	int nID;
	int sink;
	int sender;
	int seq_no;
	int priority;
}Sender_Data;

typedef struct _sink_data
{
	int sysTime;
	int nID;
	int priority;
	int total_packets;
	int s1_packets;
	int s2_packets;
	int sender;
	int seq_no;
}Sink_Data;

typedef struct _dropped_pkt_data
{
	int sysTime;
	int nID;
	int total_dropped;
	int hi_dropped;
	int lo_dropped;
}Dropped_Packet_Data;

//==================================================================================================//
// Data Acquisition Functions
int findPacketSendTime(Sender_Data sender_array[], int sender, int seq_no, int total_array_length, int *index)
{
	int time = 0;

	for(int i=0; i<total_array_length; i++)
	{
		if ((sender_array[i].seq_no == seq_no) && (sender_array[i].sender == sender))
		{
			time = sender_array[i].sysTime;
			*index = i;
			break;
		}
	}

	return time;
}


int findPacketRecvTime(Sink_Data sink_array[], int sender, int seq_no, int total_array_length, int *index)
{
	int time = 0;

	for(int i=0; i<total_array_length; i++)
	{
		if ((sink_array[i].seq_no == seq_no) && (sink_array[i].sender == sender))
		{
			time = sink_array[i].sysTime;
			*index = i;
			break;
		}
	}

	return time;
}

void print_array(char **argv1, char *buff, int numtoken)
{
	printf("Buff[%d] %s\n", numtoken, buff);

	for (int i = 0; i < numtoken; i++)
	{
		printf("\t%d:\t%s\n", i, argv1[i]);
	}
	printf("\n");
}



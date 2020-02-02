#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define FRAME_COUNT         1
#define PKTS_PER_FRAME 		1000
#define SINK_01             1

#define GEN_OUTPUT_FILES 	0
#define PRINT_ROW 			1
#define PRINT_OUTPUT        0

#define SENDER_LINE 	(numtoken1 == 5) && (numtoken2 == 10) && ((strcmp(argv1[3],"SENDER") == 0 ))
#define SINK_LINE 		(numtoken1 == 5) && (numtoken2 == 20) && ((strcmp(argv1[3],"SINK") == 0 ))
#define DROP_LINE		(numtoken1 == 5) && (numtoken2 == 15) && ((strcmp(argv1[3],"DROPPED") == 0 ))


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
	int line;
}Sender_Data;

typedef struct _sink_data
{
	int sysTime;
	int nID;
	int seq_no;
	int sender;
	int hi_pkts_received;
	int lo_pkts_received;
	int total_packets;
	int priority;
	int line;
}Sink_Data;

typedef struct _dropped_pkt_data
{
	int sysTime;
	int nID;
	int total_dropped;
	int hi_dropped;
	int lo_dropped;
	int seq;
	int priority;
	int line;
}Dropped_Packet_Data;

typedef struct _transmission_time_
{
	int tx_time;
	int sink;
}TRANSMISSION_TIME;

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

//=================================================================================================================================//
// 	Calculating old Results


#if PRINT_OUTPUT
	// Getting packets received at sink
	printf("All stats parsed from input file %s\n", filename);
	typedef struct _tx_time_
	{
		int tx_time;
		int sink;
	}TX_TIME;


	int total_packets_sent = 0;
	int pkts_sent_sink1 = 0;
	int pkts_recv_sink1 = 0;
	int total_sink1_time = 0;
	int sink1_dropped = 0;
	TX_TIME pkts_tx_time[MAX_SOURCES][PKTS_PER_FRAME];

	int x;

	for(x=0; x<MAX_SOURCES;x++)
	{
        for (j=0; j<PKTS_PER_FRAME; j++)
        {
            send_time = findPacketSendTime(send_array, sources[x], j, nSendingLines, &send_index);
            recv_time = findPacketRecvTime(recv_array, sources[x], j, nSinkLines, &recv_index);

            if (send_time != 0)
            {
                // packet was sent

                total_packets_sent++;

                if(send_array[send_index].sink == SINK_01) // sink 1
                {
                    pkts_sent_sink1++;
                }

                if (recv_time != 0)
                {
                    // packet reached sink
                    pkts_tx_time[x][j].tx_time = recv_time - send_time;
                    pkts_tx_time[x][j].sink = recv_array[recv_index].nID;
                    if (pkts_tx_time[x][j].tx_time < 0)
                    {
                    	printf("negative\n");
                    }

                    if(recv_array[recv_index].nID == SINK_01) // sink 1
                    {
                        pkts_recv_sink1++;
                    }

                }
                else
                {
                    // packet was dropped

                    if(send_array[send_index].sink == SINK_01) // sink 1
                    {
                        sink1_dropped++;
                    }


                    //printf("Packet dropped frm: %d, seq: %d\n",  i, j);

                    pkts_tx_time[x][j].tx_time = 0;
                }
            }
            else
            {

                // packet was not sent
                pkts_tx_time[x][j].tx_time = 0;
            }

		}
	}

#if 1//DEADLINE
#define DEADLINES 10

    int sink1_deadline = 100;
    int sink1_reached_in_time[DEADLINES];
	int sink1_reached_late[DEADLINES];

    memset(sink1_reached_in_time, 0, DEADLINES*sizeof(int));
    memset(sink1_reached_late, 0, DEADLINES*sizeof(int));

    for(x=0; x<MAX_SOURCES;x++)
	{
		for (j=0; j<PKTS_PER_FRAME; j++)
		{
			if (pkts_tx_time[x][j].tx_time > 0)
			{
				if (pkts_tx_time[x][j].sink == SINK_01)
				{
				   total_sink1_time += pkts_tx_time[x][j].tx_time;

					for (k=0; k<DEADLINES; k++)
					{
						if (pkts_tx_time[x][j].tx_time < (sink1_deadline*(1+k)))
						{
							sink1_reached_in_time[k]++;
						}
						else
						{
							sink1_reached_late[k]++;
						}
					}
				}
			}
		}

    }

#endif //DEADLINE

	#define TIME_SLOTS 20

	int tc_sink1_received[TIME_SLOTS];
	int tc_sink1_sent[TIME_SLOTS];
	memset(tc_sink1_received, 0, TIME_SLOTS*sizeof(int));
	memset(tc_sink1_sent, 0, TIME_SLOTS*sizeof(int));

	int send_start_time = send_array[0].sysTime;
	int recv_start_time = recv_array[0].sysTime;
	int pkt_tx_time = recv_start_time - send_start_time;
	int recv_end_time = recv_array[nSinkLines-1].sysTime;
	int transmission_time_span = recv_end_time - send_start_time;
	//int time_span_slot = transmission_time_span / TIME_SLOTS;
	int  s1_packets = 0;
	int  time_span_slot = 10000;


	int slot = 1;

	for (k = 0; k < nSinkLines && slot < (TIME_SLOTS + 1); k++)
	{
		if (recv_array[k].nID == 1)
		{
			s1_packets++;
		}

		if (recv_array[k].sysTime > (send_start_time + (time_span_slot)*slot))
		{
			tc_sink1_received[slot-1] = s1_packets;
			//printf("tc_sink1_received[%d]: %d\t", slot - 1, tc_sink1_received[slot-1]);
			//printf("tc_sink2_received[%d]: %d\n", slot - 1, tc_sink2_received[slot-1]);
			slot++;
		}
	}

	slot = 1;
	int sink1_sents = 0;

	for (k = 0; k < nSendingLines && slot < (TIME_SLOTS + 1); k++)
	{
		if (send_array[k].sink == SINK_01)
		{
			sink1_sents++;
		}

		if (send_array[k].sysTime > (send_start_time + (time_span_slot)*slot))
		{
			tc_sink1_sent[slot-1] = sink1_sents;
			//printf("tc_sink1_sent[%d]: %d\t", slot-1, tc_sink1_sent[slot-1]);
			//printf("tc_sink2_sent[%d]: %d\n", slot-1, tc_sink2_sent[slot-1]);
			slot++;

		}
	}

	if (SINKS == 1)
	{
		if (tc_sink1_sent[TIME_SLOTS-1] == 0)
		{
			tc_sink1_sent[TIME_SLOTS-1] = nSendingLines;
		}

    }
	else
	{
		if (tc_sink1_sent[TIME_SLOTS-1] == 0)
		{
			tc_sink1_sent[TIME_SLOTS-1] = nSendingLines/2;
		}

    }

//	for (k = 0; k < nDroppedLines; k++)
//	{
//
//	}


	printf("\n");
	printf("Simulation Results:\n");
	printf("\tTotal packets created:\t\t%d\n", (FRAME_COUNT * PKTS_PER_FRAME) * MAX_SOURCES);
	printf("\tTotal packets not sent:\t\t%d\n", (FRAME_COUNT * PKTS_PER_FRAME * MAX_SOURCES) - total_packets_sent);
	printf("\tTotal packets sent:\t\t%d\n", total_packets_sent);
	printf("\tTotal packets received:\t\t%d\n", nSinkLines);
	printf("\tTotal time (ms):\t\t%d\n", total_time);
	printf("\tTotal dropped:\t\t\t%d\n", total_packets_sent - pkts_recv_sink1);
	printf("\tSink 1 Packets Received:\t%d\n", pkts_recv_sink1);
	printf("\tSink 1 Total time:\t\t%d\n", total_sink1_time);

	if (pkts_recv_sink1 > 0)
	{
		printf("\tAvg Sink1 Tx time(ms):\t\t%d\n", total_sink1_time/pkts_recv_sink1);
		printf("\tOverall Sink1 Pkt Ratio:\t%0.2f\n", (float)pkts_recv_sink1/(float)pkts_sent_sink1);
	}
	else
	{
		printf("\tAvg Sink1 Tx time(ms):\t\tINF\n");
		printf("\tOverall Sink1 Pkt Ratio:\tINF\n");
	}

	printf("\tTime\tS1 Sent\tS1 Recv\tS1 PR\n");
	for (k = 0; k < TIME_SLOTS; k++)
	{
		printf("\t%d\t%d\t%d\t",
				time_span_slot*(k+1),
				tc_sink1_sent[k],
				tc_sink1_received[k]
				);
		if (!(tc_sink1_sent[k] == 0))
		{
			printf("%0.2f\n",(float)tc_sink1_received[k]/(float)tc_sink1_sent[k]);
		}
		else
		{
			printf("INF\n");
		}
	}


    printf("\tS1_DL\tS1 time\tS1 late\n");
    for (k=0; k<DEADLINES; k++)
    {
        printf("\t%d\t%d\t%d\n", sink1_deadline*(k+1), sink1_reached_in_time[k], sink1_reached_late[k]);
    }

	printf("send_start_time:%d, recv_end_time:%d, transmission_time_span:%d, time_span_slot:%d, pkt_tx_time:%d\n", send_start_time, recv_end_time, transmission_time_span, time_span_slot, pkt_tx_time);

#if 1//PRINT_ROW
	FILE* fd5;
	char row1[2048];
	memset(row1, 0, 2048*sizeof(char));

	fd5 = fopen("row.csv", "a+");

	if (SINKS == 1)
	{
    	if (tc_sink1_received[0] == 0)
    	{
    		sprintf(row1, "No P1 packets received\n");
    	}
	}

    if (tc_sink1_received[0] != 0)
    {
		sprintf(row1, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
						(PKTS_PER_FRAME) * MAX_SOURCES,                         // 1
						(PKTS_PER_FRAME * MAX_SOURCES) - total_packets_sent,    // 2
						total_packets_sent,                                     // 3
						nSinkLines,                                             // 4
						total_time,                                             // 5
						total_packets_sent - (pkts_recv_sink1),                 // 6
						pkts_recv_sink1,                                        // 7
						total_sink1_time                                        // 8
						);

		if (pkts_recv_sink1 > 0)
		{
			sprintf(row1, "%s\t%d\t%0.2f",
						row1,
						total_sink1_time/pkts_recv_sink1,                       // 9
						(float)pkts_recv_sink1/(float)pkts_sent_sink1);         // 10
		}
		else
		{
			sprintf(row1, "%s\tINF\tINF", row1) ;
		}

		for (k = 0; k < TIME_SLOTS; k++)
		{
			sprintf(row1, "%s\t%d\t%d\t%d",
					row1,
					time_span_slot*(k+1),                                       // 11
					tc_sink1_sent[k],                                           // 12
					tc_sink1_received[k]                                        // 13
					);

		    if (tc_sink1_sent[k] != 0)
		    {
		        sprintf(row1, "%s\t%0.2f", row1, (float)tc_sink1_received[k]/(float)tc_sink1_sent[k]); // 14
		    }
		    else
		    {
		        sprintf(row1, "%s\t0", row1);
		    }

		}

		for (k=0; k<DEADLINES; k++)
		{
			sprintf(row1, "%s\t%d\t%d\t%d",
					row1,
					sink1_deadline*(k+1),                                                  // 15
					sink1_reached_in_time[k],                                              // 16
					sink1_reached_late[k]                                                  // 17
					);
        }
	}
	else
	{
	    printf("INVALID SINK COUNT\n");
	    exit(0);
	}
	fprintf(fd5, "%s\n", row1);

	printf("Row:\n");
	printf("............................................................\n");
	printf("%s\n", row1);
	printf("............................................................\n");

	fclose(fd5);

#endif // PRINT_ROW

	//printf("Total:\t\t%d\n", p1_packets_received+p2_packets_received+p2_not_sent+p1_not_sent+p1_dropped+p2_dropped);
#if DROPPED_FILE
	char powerfile[50];
	sprintf(powerfile, "powerfile.txt");

	FILE* pf = fopen("pf.csv", "w+");

	for (i=0; i<nDroppedLines;i++)
	{
		fprintf(pf, "%d\t%d\t%d\t%d\t%d\n",
					dropped_array[i].sysTime,
					dropped_array[i].nID,
					dropped_array[i].total_dropped,
					dropped_array[i].hi_dropped,
					dropped_array[i].lo_dropped);
	}
	fclose(pf);
#endif
#endif // PRINT_OUTPUT
//=================================================================================================================================//
// 	Dumping Output in csv files

#if GEN_OUTPUT_FILES

	int fd2, fd3, fd4;
	char filename1[50];
	char filename2[50];
	char filename3[50];

	sprintf(filename1, "%s.sen.csv", filename);
	sprintf(filename2, "%s.col.csv", filename);
	sprintf(filename3, "%s.sin.csv", filename);

	printf("Going to generate files:\n");

	printf("%s\n", filename1);
	printf("%s\n", filename2);
	printf("%s\n", filename3);

	fd2 = open(filename1, O_CREAT|O_TRUNC|O_RDWR, 0x0200 );
	fd3 = open(filename2, O_CREAT|O_TRUNC|O_RDWR, 0x0200 );
	fd4 = open(filename3, O_CREAT|O_TRUNC|O_RDWR, 0x0200 );

	char line[256];

	sprintf(line, "Time, Node ID, Priority, Frame No, Seq No, Retransmissions, Collisions, Packet\n");

	write(fd2, line, strlen(line));

	sprintf(line, "Time, Node ID, Priority, T1, T2, Factor, Backoff, Retransmissions, Collisions, Address\n");

	write(fd3, line, strlen(line));

	sprintf(line, "Time, Node ID, Priority, Total Packets, P1 Packets, P2 Packets, Frame No, Seq No \n");

	write(fd4, line, strlen(line));

	printf("Printing Packet Sending Stats.....");

	for(i = 0; i < nSendingLines; i++)
	{
		sprintf(line, "%d,%d,%d,%d,%d,%d,%d,%s\n",
									sending_array[i].sysTime,
									sending_array[i].nID,
									sending_array[i].priority,
									sending_array[i].nFrame,
									sending_array[i].nSequence,
									sending_array[i].nReTx,
									sending_array[i].nCollision,
									sending_array[i].address);

		write(fd2, line, strlen(line));
	}

	printf(".....DONE\n");

	printf("Printing Packet Collisions Stats.....");

	for(j = 0; j < nCollisionLines; j++)
	{
		sprintf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%s\n",
									collisions_array[j].sysTime,
									collisions_array[j].nID,
									collisions_array[j].priority,
									collisions_array[j].t1,
									collisions_array[j].t2,
									collisions_array[j].comp,
									collisions_array[j].backoff,
									collisions_array[j].retransmissions,
									collisions_array[j].collisions,
									collisions_array[j].address);

		write(fd3, line, strlen(line));

	}

	printf(".....DONE\n");

	printf("Printing Sink Stats.....");

	for(k = 0; k < nSinkLines; k++)
	{
		sprintf(line, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
									recv_array[k].sysTime,
									recv_array[k].nID,
									recv_array[k].priority,
									recv_array[k].total_packets,
									recv_array[k].p1_packets,
									recv_array[k].p2_packets,
									recv_array[k].sender,
									recv_array[k].nFrame,
									recv_array[k].nSeq);

		write(fd4, line, strlen(line));
	}

	printf(".....DONE\n");

	close(fd2);
	close(fd3);
	close(fd4);

#endif



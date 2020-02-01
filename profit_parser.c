#include "profit_parser.h"

//==================================================================================================//
// Main Program

int main(int argc, char *argv[])
{

	if (argc < 3)
	{
		printf("%d\n", argc);
		printf("Usage: ./auto_parser <log_file> <number_of_sinks> <number_of_sources> [source-id-1] [source-id-2]\n");
		exit(0);
	}

	int MAX_SOURCES = atoi(argv[3]);
	int SINKS = atoi(argv[2]);
	printf("argv2: %d, argc: %d\n", SINKS, argc);
	int fd, numread, numtoken1, numtoken2;
	char buff[256];
	char **argv1;
	char **argv2;
	int nSendingLines = 0;
	int nDroppedLines = 0;
	int nSinkLines = 0;
	int h, i, j, k;
	char filename[50];
	int sources[MAX_SOURCES];

    for (i = 0; i < MAX_SOURCES; i++)
    {
        sources[i] = atoi(argv[4+i]);
    }

	strcpy(filename, argv[1]);

	fd = open(filename, O_RDONLY);

	if (fd < 0)
	{
        printf("Input file not found. Exiting.\n");
        exit(-1);
	}
	printf("\n------------------------------------------------------------\n");
	printf("Processing file %s\n", filename);

	while( (numread = readline(fd, buff, 256))>0)
	{
		numtoken1 = makeargv(buff," ", &argv1);
		numtoken2 = makeargv(buff, ":",&argv2);
#if 0
		printf("Argv1:\n");
		print_array(argv1, buff, numtoken1);
		printf("Argv2:\n");
		print_array(argv2, buff, numtoken2);
#endif

		if (SENDER_LINE)
		{
			nSendingLines++;
			continue;
		}

		if (SINK_LINE)
		{
			nSinkLines++;
			continue;
		}

		if (DROP_LINE)
		{
			nDroppedLines++;
			continue;
		}
	}

	close(fd);

	Sender_Data send_array[nSendingLines];
	Sink_Data recv_array[nSinkLines];
	Dropped_Packet_Data dropped_array[nDroppedLines];

	memset(send_array, 0, nSendingLines*sizeof(Sender_Data));
	memset(recv_array, 0, nSinkLines*sizeof(Sink_Data));
	memset(dropped_array, 0, nDroppedLines*sizeof(Dropped_Packet_Data));

	printf("nSendingLines = %d\tnSinkLines = %d\tnDroppedLines = %d\n",
			nSendingLines,
			nSinkLines,
			nDroppedLines);

	fd = open(filename, O_RDONLY);

	h = 0, i = 0, j = 0, k = 0;

	while( (numread = readline(fd, buff, 256))>0)
	{
		numtoken1 = makeargv(buff," ", &argv1);
		numtoken2 = makeargv(buff, ":",&argv2);
#if 0
		printf("Argv1:\n");
		print_array(argv1, buff, numtoken1);
		printf("Argv2:\n");
		print_array(argv2, buff, numtoken2);
#endif

		if (SENDER_LINE)
		{

			send_array[h].sysTime = atoi(argv2[0])/1000;
			send_array[h].nID = atoi(argv2[1]);
			send_array[h].sink = atoi(argv2[8]);
			send_array[h].sender = atoi(argv2[1]);
			send_array[h].seq_no = atoi(argv2[4]);
			send_array[h].priority = atoi(argv2[6]);

/*			printf("send_array[%d].sysTime = %d\n", h, send_array[h].sysTime);
			printf("send_array[%d].nID = %d\n", h, send_array[h].nID);
			printf("send_array[%d].sink = %d\n", h, send_array[h].sink);
			printf("send_array[%d].sender = %d\n", h, send_array[h].sender);
			printf("send_array[%d].frm_no = %d\n", h, send_array[h].frm_no);
			printf("send_array[%d].seq_no = %d\n", h, send_array[h].seq_no);
*/
			h++;
			continue;
		}

		else if (SINK_LINE)
		{
			recv_array[k].sysTime = atoi(argv2[0])/1000;
			recv_array[k].nID = atoi(argv2[1]);
			recv_array[k].total_packets++;
			recv_array[k].sender = atoi(argv2[16]);
			recv_array[k].seq_no = atoi(argv2[4]);

/*			printf("recv_array[%d].sysTime = %d\n", k, recv_array[k].sysTime);
			printf("recv_array[%d].nID = %d\n", k, recv_array[k].nID);
			printf("recv_array[%d].total_packets = %d\n", k, recv_array[k].total_packets);
			printf("recv_array[%d].sender = %d\n", k, recv_array[k].sender);
			printf("recv_array[%d].nFrame = %d\n", k, recv_array[k].nFrame);
			printf("recv_array[%d].nSeq = %d\n", k, recv_array[k].nSeq);
*/
			k++;
			continue;
		}

		else if (DROP_LINE)
		{
			dropped_array[i].sysTime = getID(argv2[0])/1000;
			dropped_array[i].nID = getID(argv2[1]);
			dropped_array[i].total_dropped = getID(argv2[5]);
			dropped_array[i].hi_dropped = getID(argv2[7]);
			dropped_array[i].lo_dropped = getID(argv2[9]);
			dropped_array[i].seq = getID(argv2[11]);

/*			printf("dropped_array[%d].sysTime: %d\t", i, dropped_array[i].sysTime);
			printf("nID: %d\t", dropped_array[i].nID);
			printf("time: %d\t", dropped_array[i].time);
			printf("cpu: %d\t", dropped_array[i].cpu);
			printf("lpm: %d\t", dropped_array[i].lpm);
			printf("transmit: %d\t", dropped_array[i].transmit);
			printf("listen: %d\n", dropped_array[i].listen);
*/
			i++;
			continue;
		}
	}


	close(fd);


//=================================================================================================================================//
// 	Calculating Results

	printf("All stats parsed from input file %s\n", filename);
#if 1//PRINT_OUTPUT
	// Getting packets received at sink

	typedef struct _tx_time_
	{
		int tx_time;
		int sink;
	}TX_TIME;


	int total_packets_sent = 0;
	int pkts_sent_sink1 = 0;
	int pkts_recv_sink1 = 0;
	int total_time = recv_array[nSinkLines-1].sysTime - send_array[0].sysTime;
	int total_sink1_time = 0;
	int send_time;
	int recv_time;
	int sink1_dropped = 0;
	TX_TIME pkts_tx_time[MAX_SOURCES][PKTS_PER_FRAME];

	int x;
	int send_index = 0;
	int recv_index = 0;

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
#define DEADLINES 20

    int sink1_deadline = 2000;
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


	printf("\nProgram Completed Successfully\n\n");
	printf("===================================================================================\n\n");
	return 0;
}



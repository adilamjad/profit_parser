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
	int hPktsRecv = 0;
	int lPktsRecv = 0;
	int hPktsSent = 0;
	int lPktsSent = 0;
	int hPktsDrop = 0;
	int lPktsDrop = 0;
	float avgTxInt = 0;

	int h, i, j, k, l;
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

	h = 0, i = 0, j = 0, k = 0, l = 0;

	while( (numread = readline(fd, buff, 256))>0)
	{
		l++;
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

			send_array[h].sysTime = atoi(argv2[0]);
			send_array[h].nID = atoi(argv2[1]);
			send_array[h].sink = atoi(argv2[8]);
			send_array[h].sender = atoi(argv2[1]);
			send_array[h].seq_no = atoi(argv2[4]);
			send_array[h].priority = atoi(argv2[6]);
			send_array[h].line = l;

			if (send_array[h].priority == 1)
			{
				hPktsSent++;
			}
			else if (send_array[h].priority == 2)
			{
				lPktsSent++;
			}

			if (h > 1)
			{
				avgTxInt = (avgTxInt + (send_array[h].sysTime - send_array[h-1].sysTime))/2;
			}

			h++;
			continue;
		}

		else if (SINK_LINE)
		{
			recv_array[k].sysTime = atoi(argv2[0]);
			recv_array[k].nID = atoi(argv2[1]);
			recv_array[k].total_packets = k + 1;
			recv_array[k].sender = atoi(argv2[16]);
			recv_array[k].priority = atoi(argv2[18]);
			recv_array[k].seq_no = atoi(argv2[4]);
			recv_array[k].line = l;

			if (recv_array[k].priority == 1)
			{
				hPktsRecv++;
			}
			else if (recv_array[k].priority == 2)
			{
				lPktsRecv++;
			}
			k++;
			continue;
		}

		else if (DROP_LINE)
		{
			dropped_array[i].sysTime = getID(argv2[0]);
			dropped_array[i].nID = getID(argv2[1]);
			dropped_array[i].total_dropped = getID(argv2[5]);
			dropped_array[i].hi_dropped = getID(argv2[7]);
			dropped_array[i].lo_dropped = getID(argv2[9]);
			dropped_array[i].seq = getID(argv2[11]);
			dropped_array[i].priority = getID(argv2[13]);
			dropped_array[i].line = l;

			if (dropped_array[i].priority == 1)
			{
				hPktsDrop++;
			}
			else if (dropped_array[i].priority == 2)
			{
				lPktsDrop++;
			}

			i++;
			continue;
		}
	}


	close(fd);

//=================================================================================================================================//
// 	print arrays
#if 1
	FILE* fa = fopen("file_arrays.csv", "w+");

	for (i = 0; i < nSendingLines; i++)
	{
		fprintf(fa, "send_array[%d].sysTime = %d\tnID = %d\tsink = %d\tsender = %d\tseq_no = %d\tpriority = %d\tline = %d\n",
					i,
					send_array[i].sysTime,
					send_array[i].nID,
					send_array[i].sink,
					send_array[i].sender,
					send_array[i].seq_no,
					send_array[i].priority,
					send_array[i].line);
	}

	for (i = 0; i < nSinkLines; i++)
	{
		fprintf(fa, "recv_array[%d].sysTime = %d\tnID = %d\ttotal_packets = %d\tsender = %d\tseq_no = %d\tpr = %d\tline = %d\n",
					i,
					recv_array[i].sysTime,
					recv_array[i].nID,
					recv_array[i].total_packets,
					recv_array[i].sender,
					recv_array[i].seq_no,
					recv_array[i].priority,
					recv_array[i].line);
	}

	for (i = 0; i < nDroppedLines; i++)
	{
		fprintf(fa, "dropped_array[%d].sysTime = %d\tnID = %d\ttotal = %d\thi = %d\tlo = %d\tseq = %d\tpr = %d\tline = %d\n",
					i,
					dropped_array[i].sysTime,
					dropped_array[i].nID,
					dropped_array[i].total_dropped,
					dropped_array[i].hi_dropped,
					dropped_array[i].lo_dropped,
					dropped_array[i].seq,
					dropped_array[i].priority,
					dropped_array[i].line);
	}

	fclose(fa);
#endif


//=================================================================================================================================//
// 	Calculating Results
	TRANSMISSION_TIME tx_time_hi[hPktsRecv];
	TRANSMISSION_TIME tx_time_lo[lPktsRecv];
	int send_time;
	int recv_time;
	int send_index = 0;
	int recv_index = 0;
	int pkt_sent = 0;
	int pkt_recv = 0;
	int hIndex = 0;
	int lIndex = 0;
	int pkt_dropped = 0;
	int pkt_not_sent = 0;
	int total_time = recv_array[nSinkLines-1].sysTime - send_array[0].sysTime;

	char row[1000];
	memset(tx_time_hi, 0, sizeof(tx_time_hi));
	memset(tx_time_lo, 0, sizeof(tx_time_lo));

	for(i = 0; i < MAX_SOURCES; i++)
	{
		for (j = 0; j < PKTS_PER_FRAME; j++)
		{
			send_time = findPacketSendTime(send_array, sources[i], j, nSendingLines, &send_index);
			recv_time = findPacketRecvTime(recv_array, sources[i], j, nSinkLines, &recv_index);

			if (send_time > 0)
			{
				pkt_sent++;
			}
			else
			{
				pkt_not_sent++;
			}

			if (recv_time > 0)
			{
				pkt_recv++;
			}
			else
			{
				pkt_dropped++;
				continue;
			}

			if ((send_time > 0) && (send_time > 0))
			{
				if (recv_array[recv_index].priority == 1)
				{
					tx_time_hi[hIndex].tx_time = recv_time - send_time;
					tx_time_hi[hIndex].sink = recv_array[recv_index].nID;
					hIndex++;
				}
				if (recv_array[recv_index].priority == 2)
				{
					tx_time_lo[lIndex].tx_time = recv_time - send_time;
					tx_time_lo[lIndex].sink = recv_array[recv_index].nID;
					lIndex++;
				}
			}

		}
	}

	int tot_tx_time_hi = 0;
	int tot_tx_time_lo = 0;

	for (i = 0; i < hPktsRecv; i++)
	{
		tot_tx_time_hi += tx_time_hi[i].tx_time;
	}
	for (i = 0; i < lPktsRecv; i++)
	{
		tot_tx_time_lo += tx_time_lo[i].tx_time;
	}


	int result_tot_packets = (FRAME_COUNT * PKTS_PER_FRAME) * MAX_SOURCES;

	printf("\n");
	printf("PRoFIT Simulation Results:\n");
	printf("\tTotal packets created:\t\t%d\n", result_tot_packets);
	printf("\tTotal packets not sent:\t\t%d\t(%d)\n", pkt_not_sent, (FRAME_COUNT * PKTS_PER_FRAME * MAX_SOURCES) - pkt_sent);
	printf("\tTotal packets sent:\t\t%d\n", pkt_sent);
	printf("\tTotal packets received:\t\t%d\t(%d)\n", pkt_recv, nSinkLines);
	printf("\tTotal time (ms):\t\t%d\n", total_time);
	printf("\tTotal dropped:\t\t\t%d\t(%d)\n", pkt_dropped, pkt_sent - pkt_recv);
	printf("\tAverage Tx Interval(ms):\t%0.2f\n", avgTxInt);

	strcpy(row, "");
	sprintf(row, "%s%d", row, result_tot_packets);
	sprintf(row, "%s\t%d", row, pkt_not_sent);
	sprintf(row, "%s\t%d", row, pkt_sent);
	sprintf(row, "%s\t%d", row, pkt_recv);
	sprintf(row, "%s\t%d", row, total_time);
	sprintf(row, "%s\t%d", row, pkt_dropped);
	sprintf(row, "%s\t%0.2f", row, avgTxInt);

	if (hPktsRecv > 0)
	{
		printf("\tAvg Tx Time Hi(ms):\t\t%0.2f\n", (float)tot_tx_time_hi/(float)hPktsRecv);
		sprintf(row, "%s\t%0.2f", row, (float)tot_tx_time_hi/(float)hPktsRecv);

	}
	else
	{
		printf("\tAvg Tx Time Hi(ms):\t\tINF\n");
		sprintf(row, "%s\tINF", row);
	}


	if (lPktsRecv > 0)
	{
		printf("\tAvg Tx Time Lo(ms):\t\t%0.2f\n", (float)tot_tx_time_lo/(float)lPktsRecv);
		sprintf(row, "%s\t%0.2f", row, (float)tot_tx_time_lo/(float)lPktsRecv);
	}
	else
	{
		printf("\tAvg Tx Time Lo(ms):\t\tINF\n");
		sprintf(row, "%s\tINF", row);
	}

	if (hPktsSent > 0)
	{
		printf("\tPkt Delivery Ratio Hi:\t\t%0.2f\t(%d/%d)\n", (float)hPktsRecv/(float)hPktsSent, hPktsRecv, hPktsSent);
		sprintf(row, "%s\t%0.2f", row, (float)hPktsRecv/(float)hPktsSent);
	}
	else
	{
		printf("\tPkt Delivery Ratio Hi:\t\tINF\n");
		sprintf(row, "%s\tINF", row);
	}

	if (lPktsSent > 0)
	{
		printf("\tPkt Delivery Ratio Lo:\t\t%0.2f\t(%d/%d)\n", (float)lPktsRecv/(float)lPktsSent, lPktsRecv, lPktsSent);
		sprintf(row, "%s\t%0.2f", row, (float)lPktsRecv/(float)lPktsSent);
	}
	else
	{
		printf("\tPkt Delivery Ratio lo:\t\tINF\n");
		sprintf(row, "%s\tINF", row);
	}

	printf("==============================================================================================================\n");
	printf("%s\n==============================================================================================================\n", row);

	FILE *fr = fopen("row.csv", "a+");
	fprintf(fr, "%s\n", row);
	printf("\nProgram Completed Successfully\n\n");
	printf("===================================================================================\n\n");
	return 0;
}



/*
 *	PROGRAM 4 Server - Alexander Alfonso (aja0167)
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#pragma pack(1)		//	pack structure into 5 bytes to send struct to client
struct tcpHeader
{
	unsigned short int src;
	unsigned short int dest;
	unsigned int seqNum;
	unsigned int ack;
	unsigned short int hdr_flags;
	unsigned short int recvWindow;
	unsigned short int chksum;
	unsigned short int urgptr;
	unsigned int options;
};
#pragma pack(0)		//	turn packing off

void printSegment(struct tcpHeader seg);
unsigned int computeChecksum(struct tcpHeader seg);
void fprintSegment(FILE *fp, struct tcpHeader seg);

int main(int argc, char *argv[])
{
	int portno, clilen, sockfd, clientfd, n;
	unsigned int y = 0;
	unsigned short int temp;
	struct sockaddr_in servaddr, cliaddr;
	struct tcpHeader segment;

	FILE *fp = fopen("server.out", "w");

	if(argc != 2)
	{
		printf("usage: ./executable <portno>\n");
		exit(0);
	}
	portno = atoi(argv[1]);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("socket fail");
		exit(0);
	}

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(portno);

    int on = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	if(bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
	{
		perror("bind fail");
	}

	if(listen(sockfd, 5) < 0)
	{
		perror("listen");
		exit(0);
	}

	clilen = sizeof(struct sockaddr_in);

	if((clientfd = accept(sockfd, (struct sockaddr*) &cliaddr, (socklen_t*) &clilen)) < 0)
	{
		perror("accept error");
		exit(1);
	}

	printf("Opening TCP Connection\n");
	printf("------------------------\n");

	fprintf(fp, "Opening TCP Connection\n");
	fprintf(fp, "------------------------\n");

	if(n = read(clientfd, &segment, sizeof(segment)) < 0)
	{
		perror("read");
		exit(0);
	}
	else
	{
		//	swap source/destination ports
		temp = segment.src;
		segment.src = segment.dest;
		segment.dest = temp;

		//	print to terminal
		printf("received SYN bit from client...\n");
		printSegment(segment);

		//	print to server.out
		fprintf(fp, "received SYN bit from client...\n");
		fprintSegment(fp, segment);	
	}

	segment.ack = segment.seqNum+1;

	y = rand() % 1000;
	segment.seqNum = y;

	//	set SYN bit to 1, set ACK bit to 1
	segment.hdr_flags = segment.hdr_flags | 0x0012;

	//	compute checksum
	segment.chksum = 0;
	segment.chksum = computeChecksum(segment);

	//	sending response to client
	if(n = write(clientfd, &segment, sizeof(segment)) < 0)
	{
		perror("write");
		exit(0);
	}
	else
	{
		printf("\nsent response segment to client\n");
		printSegment(segment);

		fprintf(fp, "\nsent response segment to client\n");
		fprintSegment(fp, segment);
	}
	
	if(n = read(clientfd, &segment, sizeof(segment)) < 0)
	{
		perror("read");
		exit(0);
	}
	else
	{
		//	swap source/destination ports
		temp = segment.src;
		segment.src = segment.dest;
		segment.dest = temp;

		printf("\nreceived ACK bit from client... \n");
		printSegment(segment);

		fprintf(fp, "\nreceived ACK bit from client... \n");
		fprintSegment(fp, segment);

		printf("\n*	Granting connection... \n");
	}

	/*
		CLOSE TCP CONNECTION
	*/
	printf("\nClosing TCP Connection\n");
	printf("------------------------\n");

	fprintf(fp, "\nClosing TCP Connection\n");
	fprintf(fp, "------------------------\n");

	if(n = read(clientfd, &segment, sizeof(segment)) < 0)
	{
		perror("read");
		exit(0);
	}
	else
	{
		//	swap source/destination ports
		temp = segment.src;
		segment.src = segment.dest;
		segment.dest = temp;

		printf("received close request with FIN bit set...\n");
		printSegment(segment);

		fprintf(fp, "received close request with FIN bit set...\n");
		fprintSegment(fp, segment);
	}


	unsigned int cli_seqNum = segment.seqNum;
	segment.ack = segment.seqNum+1;
	segment.seqNum = 512;

	//	SET ACK BIT TO 1
	segment.hdr_flags = segment.hdr_flags | 0x0010;
	//	*	reset FIN bit to 0
	segment.hdr_flags = segment.hdr_flags & 0xFFFE;

	segment.chksum = 0;
	segment.chksum = computeChecksum(segment);

	
	if(n = write(clientfd, &segment, sizeof(segment)) < 0)
	{
		perror("write");
		exit(0);
	}
	else
	{
		printf("\nsending ACK segment to client...\n");
		printSegment(segment);

		fprintf(fp, "\nsending ACK segment to client...\n");
		fprintSegment(fp, segment);
	}

	segment.seqNum = 512;
	segment.ack = cli_seqNum + 1;

	//	reset ACK bit to 0
	segment.hdr_flags = segment.hdr_flags & 0xFFEF;

	//	set FIN bit to 1
	segment.hdr_flags = segment.hdr_flags | 0x0001;

	//	compute checksum
	segment.chksum = 0;
	segment.chksum = computeChecksum(segment);
	
	if(n = write(clientfd, &segment, sizeof(segment)) < 0)
	{
		perror("write");
		exit(0);
	}
	else
	{
		printf("\nsending FIN segment to client...\n");
		printSegment(segment);

		fprintf(fp, "\nsending FIN segment to client...\n");
		fprintSegment(fp, segment);
	}
	
	if(n = read(clientfd, &segment, sizeof(segment)) < 0)
	{
		perror("read");
		exit(0);
	}
	else
	{
		//	swap source/destination ports
		temp = segment.src;
		segment.src = segment.dest;
		segment.dest = temp;

		printf("\nreceiving closing ACK from client...\n");
		printSegment(segment);

		fprintf(fp, "\nreceiving closing ACK from client...\n");
		fprintSegment(fp, segment);
	}

	printf("\n*	connection closed...\n");
	fprintf(fp, "\n*	connection closed...\n");

	close(clientfd);
	close(sockfd);
	close(fp);

	printf("\nexiting program...\n");
	return 0;
}

/*
	FUNCTIONS
*/
void printSegment(struct tcpHeader seg)
{
	printf("----------------\n");
	printf("TCP Field Values\n");
	printf("----------------\n");
	printf("src: %d\t", seg.src); // Printing all values
	printf("des: %d\n", seg.dest);
	printf("seq: %d\n", seg.seqNum);
	printf("ack: %d\n", seg.ack);
	printf("hdr: 0x%04X\t", seg.hdr_flags);
	printf("rec: 0x%04X\n", seg.recvWindow);
	printf("chk: 0x%04X\t", seg.chksum);
	printf("urg: 0x%04X\n", seg.urgptr);
	printf("opt: 0x%08X\n", seg.options);
	printf("----------------\n");
}

void fprintSegment(FILE *fp, struct tcpHeader seg)
{
	fprintf(fp, "----------------\n");
	fprintf(fp, "TCP Field Values\n");
	fprintf(fp, "----------------\n");
	fprintf(fp, "src: %d\t", seg.src); // Printing all values
	fprintf(fp, "des: %d\n", seg.dest);
	fprintf(fp, "seq: %d\n", seg.seqNum);
	fprintf(fp, "ack: %d\n", seg.ack);
	fprintf(fp, "hdr: 0x%04X\t", seg.hdr_flags);
	fprintf(fp, "rec: 0x%04X\n", seg.recvWindow);
	fprintf(fp, "chk: 0x%04X\t", seg.chksum);
	fprintf(fp, "urg: 0x%04X\n", seg.urgptr);
	fprintf(fp, "opt: 0x%08X\n", seg.options);
	fprintf(fp, "----------------\n");
}

unsigned int computeChecksum(struct tcpHeader seg)
{
	unsigned short int checksumArr[12];
	unsigned int i, sum=0, cksum, wrap;

	seg.chksum = 0;

	memcpy(checksumArr, &seg, 24);

	for(i = 0;i<12; i++)
	{
		sum = sum + checksumArr[i];
	}

	wrap = sum >> 16;             // Wrap around once
	sum = sum & 0x0000FFFF; 
	sum = wrap + sum;

	wrap = sum >> 16;             // Wrap around once more
	sum = sum & 0x0000FFFF;
	cksum = wrap + sum;

	//printf("Sum Value: 0x%04X\n", cksum);
	//printf("\nChecksum Value: 0x%04X\n", (0xFFFF^cksum));

	cksum = 0xFFFF^cksum;
	return cksum;
}

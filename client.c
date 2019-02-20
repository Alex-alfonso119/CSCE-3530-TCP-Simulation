/*
 *	PROGRAM 4 Client - Alexander Alfonso (aja0167)
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#pragma pack(1)		//	pack structure into 5 bytes to send struct to server
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
	unsigned int x;
	unsigned short int temp;

	FILE *fp = fopen("client.out", "w");

	//	probably send the segment struct to server
	if(argc != 2)
	{
		printf("usage: ./executable <portno>\n");
		exit(0);
	}
	int portno = atoi(argv[1]);

	int sockfd, n, srcPort;
	int len = sizeof(struct sockaddr);

	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("socket error");
		exit(0);
	}

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port=htons(portno);

	inet_pton(AF_INET, "129.120.151.94", &(servaddr.sin_addr));

	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
	{
		perror("connect error");
		exit(0);
	}

	int addrlen = sizeof(servaddr);

	//	get port # of client
	if(getsockname(sockfd, (struct sockaddr *)&servaddr, &addrlen) < 0)
	{
		perror("getsockname");
		exit(0);
	}
	else
	{
		srcPort = ntohs(servaddr.sin_port);
	}

	srand(time(NULL));
	x = rand() % 1000;	//	random seq #

	//	initalize tcp segment
	struct tcpHeader segment = {srcPort, portno, x, 0, 0, 0, 0, 0};

	segment.hdr_flags = segment.hdr_flags | 0x6000;	//	data offset = 6
	segment.hdr_flags = segment.hdr_flags | 0x0002;	//	set SYN bit to 1
	
	//	compute checksum
	segment.chksum = 0;
	segment.chksum = computeChecksum(segment);

	printf("Opening TCP Connection\n");
	printf("------------------------\n");

	fprintf(fp, "Opening TCP Connection\n");
	fprintf(fp, "------------------------\n");

	if(n = write(sockfd, &segment, sizeof(segment)) < 0)
	{
		perror("write");
		exit(0);
	}
	else
	{
		//	print to terminal
		printf("sent initial SYN segment to server\n");
		printSegment(segment);

		//	print to console
		fprintf(fp, "sent initial SYN segment to server\n");
		fprintSegment(fp, segment);
	}

	if(n = read(sockfd, &segment, sizeof(segment)) < 0)
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

		printf("\nreceived SYN bit and ACK bit from server\n");
		printSegment(segment);

		fprintf(fp, "\nreceived SYN bit and ACK bit from server\n");
		fprintSegment(fp, segment);
	}

	segment.ack = segment.seqNum + 1;
	segment.seqNum = x+1;	//	set seqNum = initial seqNum+1

	//	FFED: 1111 1111 1110 1101
	segment.hdr_flags = segment.hdr_flags & 0xFFED;	//	set SYN bit to 0, set ACK bit to 0
	segment.hdr_flags = segment.hdr_flags | 0x0010;	//	set ACK bit to 1

	//	compute checksum
	segment.chksum = 0;
	segment.chksum = computeChecksum(segment);

	if(n = write(sockfd, &segment, sizeof(segment)) < 0)
	{
		perror("write");
		exit(0);
	}
	else
	{
		printf("\nsent ACK to server\n");
		printSegment(segment);

		fprintf(fp, "\nsent ACK to server\n");
		fprintSegment(fp, segment);
		
		printf("\n*	Connection is live...\n");
		fprintf(fp, "\n*	Connection is live...\n");
	}

	/*
		CLOSE TCP CONNECTION
	*/
	printf("\nClosing TCP Connection\n");
	printf("------------------------\n");

	fprintf(fp, "\nClosing TCP Connection\n");
	fprintf(fp, "------------------------\n");

	x = 1024;
	segment.seqNum = x;
	segment.ack = 512;

	//	FFED: 1111 1111 1110 1101
	segment.hdr_flags = segment.hdr_flags & 0xFFEF;	//	set ACK bit to 0
	//	SET FIN BIT TO 1
	segment.hdr_flags = segment.hdr_flags | 0x0001;

	//	compute checksum
	segment.chksum = 0;
	segment.chksum = computeChecksum(segment);

	if(n = write(sockfd, &segment, sizeof(segment)) < 0)
	{
		perror("write");
		exit(0);
	}
	else
	{
		printf("sending close request to server...\n");
		printSegment(segment);

		fprintf(fp, "sending close request to server...\n");
		fprintSegment(fp, segment);
	}

	if(n = read(sockfd, &segment, sizeof(segment)) < 0)
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

		printf("\nreceiving ACK segment from server...\n");
		printSegment(segment);

		fprintf(fp, "\nreceiving ACK segment from server...\n");		
		fprintSegment(fp, segment);
	}

	if(n = read(sockfd, &segment, sizeof(segment)) < 0)
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

		printf("\nreceiving FIN segment from server...\n");
		printSegment(segment);

		fprintf(fp, "\nreceiving FIN segment from server...\n");
		fprintSegment(fp, segment);
	}

	segment.ack = segment.seqNum + 1;	//	set ack = server seqNum + 1
	segment.seqNum = x + 1;	//	set seqNum = client seqNum + 1

	//	set ACK bit to 1
	segment.hdr_flags = segment.hdr_flags | 0x0010;

	//	*	reset FIN bit to 0
	segment.hdr_flags = segment.hdr_flags & 0xFFFE;

	//	checksum
	segment.chksum = 0;
	segment.chksum = computeChecksum(segment);

	if(n = write(sockfd, &segment, sizeof(segment)) < 0)
	{
		perror("write");
		exit(0);
	}
	else
	{
		printf("\nsending closing ACK to server...\n");
		printSegment(segment);

		fprintf(fp, "\nsending closing ACK to server...\n");
		fprintSegment(fp, segment);
	}

	printf("\n*	connection closed...\n");
	fprintf(fp, "\n*	connection closed...\n");

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

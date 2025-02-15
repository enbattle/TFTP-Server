#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define MAXBUFFER 512
#define ADDRBUFFER 128

// opcode  operation
//   1     Read request (RRQ)
//   2     Write request (WRQ)
//   3     Data (DATA)
//   4     Acknowledgment (ACK)
//   5     Error (ERROR)

typedef struct request_packet {
   uint16_t opcode;
   char filename[256];
   uint8_t padding1;
   char mode[20];
   uint8_t padding2;
} request_packet;

typedef struct data_packet {
   uint16_t opcode;
   uint16_t block_number;
   char data[512];
} data_packet;

typedef struct ack_packet {
   uint16_t opcode;
   uint16_t block_number;
} ack_packet;

typedef struct error_packet {
   uint16_t opcode;
   uint16_t error_code;
   char error_message[256];
   uint8_t padding;
} error_packet;

typedef struct tftp_packet{
	int packet_type; // 1 and 2: request_packet, 3: data_packet,...
	request_packet rp; // 1 or 2
	data_packet dp; // 3
	ack_packet ap; // 4
	error_packet ep; // 5
} tftp_packet;


void handle_alarm(int sig) {
	printf("No response after 10 seconds. Program terminated.\n");
	exit(0);
}

int main(int argc, char* argv[]) {
	// Check that there are three command line arguments --- file, start port, and end port
	if(argc != 3) {
		fprintf(stderr, "ERROR: Invalid arguments!\n");
		return EXIT_FAILURE;
	}

	// Installing the alarm handler
	signal(SIGALRM, handle_alarm);

	// Read in the start and end ports
	unsigned short int startPort = atoi(argv[1]);
	unsigned short int endPort = atoi(argv[2]);

	// Quick setvbuf
	setvbuf(stdout, NULL, _IONBF, 0);

	int i;
	int j;

	// Create TFTP socket
	int sd = socket(AF_INET, SOCK_DGRAM, 0);

	if(sd < 0) {
		fprintf(stderr, "ERROR: socket creation failed!\n");
		return EXIT_FAILURE;
	}

	// Create server struct
	struct sockaddr_in server;

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(startPort);

	int length = sizeof(server);

	// Bind server to a port
	if(bind(sd, (struct sockaddr*) &server, length) < 0) {
		fprintf(stderr, "ERROR: Bind to TFTP socket failed!\n");
		return EXIT_FAILURE;
	}

	// Obtain the assigned port number
	if(getsockname(sd, (struct sockaddr*) &server, (socklen_t *) &length) < 0) {
		fprintf(stderr, "ERROR: Failure getting the port number!\n");
		return EXIT_FAILURE;
	}


	//---------Application Level---------------------------------------


	// Server is starting
	printf("TFTP server at port number %d\n", ntohs(server.sin_port));


  	int n;

	while ( 1 )
	{
		char buffer[ MAXBUFFER ];
	  	struct sockaddr_in client;
	  	int len = sizeof(client);

		tftp_packet* client_request = malloc(sizeof(tftp_packet));

	    /* read a datagram from the remote client side (BLOCKING) */
	    n = recvfrom( sd, client_request, sizeof(tftp_packet), 0, (struct sockaddr *) &client,
	                  (socklen_t *) &len );

	    int opcode = ntohs(client_request->packet_type);

	    printf("%d\n", opcode);

	    // Set alarm for 10 seconds, and terminate the program if no response
	    printf("Setting up alarm for 10 seconds.\n");
		alarm(10);

		if ( n == -1 ) 
		{
	      perror( "recvfrom() failed" );
	    }
	    else
	    {
	    	printf( "Rcvd datagram from %s port %d\n",
				inet_ntoa( client.sin_addr ), ntohs( client.sin_port ) );

	    	if(opcode == 1 || opcode == 2) // Request packet (read, write)
	    	{
	    		
	    	}
	    	else 
	    	{
	    		sendto( sd, "Invalid operation!\n", 19, 0, (struct sockaddr *) &client, len );
	    	}
	    }
	}

	return EXIT_SUCCESS;
}
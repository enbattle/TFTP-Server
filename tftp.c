#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAXBUFFER 512
#define ADDRBUFFER 128

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

typedef union {
	int packet_type;
	request_packet rp;
	data_packet dp;
	ack_packet ap;
	error_packet ep;
} tftp_packet;

int main(int argc, char* argv[]) {
	// Check that there are three command line arguments --- file, start port, and end port
	if(argc != 3) {
		fprintf(stderr, "ERROR: Invalid arguments!\n");
		return EXIT_FAILURE;
	}

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
  	char buffer[ MAXBUFFER ];
  	struct sockaddr_in client;
  	int len = sizeof( client );
  	tftp_packet* client_request = malloc(sizeof(tftp_packet));
	while ( 1 )
	{

	    /* read a datagram from the remote client side (BLOCKING) */
	    n = recvfrom( sd, client_request, sizeof(tftp_packet), 0, (struct sockaddr *) &client,
	                  (socklen_t *) &len );

		if ( n == -1 ) 
		{
	      perror( "recvfrom() failed" );
	    }
	    else
	    {
			printf( "Rcvd datagram from %s port %d\n",
				inet_ntoa( client.sin_addr ), ntohs( client.sin_port ) );
			printf( "RCVD %d bytes\n", n );
			// printf("Received request.\n opcode: %u\n filename: %s\n padding1: %u\n mode: %s\n padding2: %u\n",
			// 	client_request.opcode, client_request.filename, client_request.padding1, client_request.mode,
			// 	client_request.padding2);
			printf("Received request.\n");
			printf("Packet type: %d\n", client_request->packet_type);
			printf("opcode: %u\n", client_request->rp.opcode);
			printf("filename: %s\n", client_request->rp.filename);
			printf("padding1: %u\n", client_request->rp.padding1);
			printf("mode: %s\n", client_request->rp.mode);
			printf("padding2: %u\n", client_request->rp.padding2);



			/* echo the data back to the sender/client */
			sendto( sd, buffer, n, 0, (struct sockaddr *) &client, len );

			/* to do: check the return code of sendto() */
	    }
	}

	return EXIT_SUCCESS;
}
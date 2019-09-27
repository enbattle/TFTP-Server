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
	int packet_type; //0: request_packet, 1: data_packet,...
	request_packet rp; // 0
	data_packet dp; // 1
	ack_packet ap; // 2
	error_packet ep; //3
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

	int t = time(NULL);

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

	//Sample data packets to send over

    char* msg1 = "KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK";
    char* msg2 = "LLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL";
    char* msg3 = "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM";
    char* msg4 = "NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN";
    char* msg5 = "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO";
    char* msg6 = "This should break the connection to the client";

    tftp_packet dp1;
    dp1.dp.opcode = 3;
    dp1.dp.block_number = 1;
    strcpy(dp1.dp.data, msg1);

    tftp_packet dp2;
    dp2.dp.opcode = 3;
    dp2.dp.block_number = 2;
    strcpy(dp2.dp.data, msg2);

    tftp_packet dp3;
    dp3.dp.opcode = 3;
    dp3.dp.block_number = 3;
    strcpy(dp3.dp.data, msg3);

    tftp_packet dp4;
    dp4.dp.opcode = 3;
    dp4.dp.block_number = 4;
    strcpy(dp4.dp.data, msg4);

	tftp_packet dp5;
    dp5.dp.opcode = 3;
    dp5.dp.block_number = 5;
    strcpy(dp5.dp.data, msg5);

	tftp_packet dp6;
    dp6.dp.opcode = 3;
    dp6.dp.block_number = 6;
    strcpy(dp6.dp.data, msg6);

    tftp_packet file_data_packets[6] = {dp1, dp2, dp3, dp4, dp5, dp6};

    //Test file name to read from and read to (fake)
    char* test_file = "test.txt";


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
			// printf("Packet type: %d\n", client_request->packet_type);
			// printf("filename: %s\n", client_request->rp.filename);
			// printf("padding1: %u\n", client_request->rp.padding1);
			// printf("mode: %s\n", client_request->rp.mode);
			// printf("padding2: %u\n", client_request->rp.padding2);

	    	if(client_request->packet_type == 0) // Request packet
	    	{
	    		printf("Request packet identified\n");
	    		request_packet packet = client_request->rp;

	    		if(packet.opcode == 1) //RRQ
	    		{
	    			//check if this file exists
	    			//may have a list of files?
	    			if(strcmp(packet.filename,test_file) == 0)
	    			{
	    				//Send the data packets and wait for acknowledgements
	    				tftp_packet file_data_packet, client_packet;
						//TODO: Change this to a while loop iterating through file data
						for(int i = 0; i < 6; i++){
							file_data_packet = file_data_packets[i];
							sendto(sd, &file_data_packet, sizeof(tftp_packet), 0, (struct sockaddr *) &client, len );	
							recvfrom( sd, &client_packet, sizeof(tftp_packet), 0, (struct sockaddr *) &client,
								(socklen_t *) &len );
							if(client_packet.ap.opcode == 4)
								printf("Receivd ack packet\n");
							else
							{
								//TODO: Implement timeout behavior
								printf("Received something wack. Go fix your shit\n");
							}
						}

	    			}
	    			else
	    			{
	    				//TODO: SEND ERROR PACKET
	    				printf("This file doesn't exist\n");
	    			}
	    			break;

	    			//send the first data packet to be read

	    		}
	    		else if(packet.opcode == 2) //WRQ
	    		{
	    			//send an acknowledgement packet
	    			tftp_packet* response_packet = malloc(sizeof(tftp_packet));
	    			response_packet->packet_type = 2;
	    			response_packet->ap.opcode = 4;
	    			response_packet->ap.block_number = 0;
					sendto(sd, response_packet, sizeof(tftp_packet), 0, (struct sockaddr *) &client, len );
					free(response_packet);

					//read the file in blocks of 512 bytes
					int num_bytes;
					tftp_packet client_packet;
					while(1)
					{
						//TODO: IMPLEMENT TIMEOUT BEHAVIOR
						recvfrom( sd, &client_packet, sizeof(tftp_packet), 0, (struct sockaddr *) &client,
							(socklen_t *) &len );
						num_bytes = strlen(client_packet.dp.data);
						printf("RECEIVED (%d) bytes:\n %s\n", num_bytes, client_packet.dp.data);
						//send an acknowledgement for each packet received
		    			response_packet = malloc(sizeof(tftp_packet));
		    			response_packet->packet_type = 2;
		    			response_packet->ap.opcode = 4;
		    			response_packet->ap.block_number = client_packet.dp.block_number;
						sendto(sd, response_packet, sizeof(tftp_packet), 0, (struct sockaddr *) &client, len );	
						free(response_packet);					

						if(num_bytes < 512)
						{
							printf("Client closed connection\n");
							break;
						}
					}

	    		}
	    	}
			/* echo the data back to the sender/client */
			sendto( sd, buffer, n, 0, (struct sockaddr *) &client, len );
			/* to do: check the return code of sendto() */
	    }
	}

	return EXIT_SUCCESS;
}
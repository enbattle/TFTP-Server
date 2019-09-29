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
#include<sys/wait.h> 

#define MAXBUFFER 512
#define ADDRBUFFER 128

// opcode  operation
//   1     Read request (RRQ)
//   2     Write request (WRQ)
//   3     Data (DATA)
//   4     Acknowledgment (ACK)
//   5     Error (ERROR)

//Error codes
   // 0         Not defined, see error message (if any).
   // 1         File not found.
   // 2         Access violation.
   // 3         Disk full or allocation exceeded.
   // 4         Illegal TFTP operation.
   // 5         Unknown transfer ID.
   // 6         File already exists.
   // 7         No such user.

char* file_not_found_str = "File not found.";
char* file_already_exists_str = "File already exists.";
char* illegal_operation_str = "Illegal TFTP operation.";
char* file_not_open_str = "Could not open file";

typedef struct request_packet {
   uint16_t opcode;
   uint8_t filename[256];
} request_packet;

typedef struct data_packet {
   uint16_t opcode;
   uint16_t block_number;
   uint8_t data[512];
} data_packet;

typedef struct ack_packet {
   uint16_t opcode;
   uint16_t block_number;
} ack_packet;

typedef struct error_packet {
   uint16_t opcode;
   uint16_t error_code;
   uint8_t error_message[256];
} error_packet;

void handle_alarm(int sig) {
	printf("No response after 10 seconds. Program terminated.\n");
	exit(0);
}

void handleRequest(unsigned short int startPort, int opcode, struct sockaddr_in* client, 
	int len, request_packet* client_request){

	// Create TFTP socket
	int sd = socket(AF_INET, SOCK_DGRAM, 0);

	if(sd < 0) {
		fprintf(stderr, "CHILD: ERROR socket creation failed!\n");
		exit(1);
	}

	// Create server struct
	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(startPort);
	int length = sizeof(server);

	printf("CHILD: This is the sd we are about to bind to: %d\n", sd);
	printf("CHILD: This is the port we are about to bind to: %d\n", startPort);
	// Bind server to a port
	if(bind(sd, (struct sockaddr*) &server, length) < 0) {
		fprintf(stderr, "CHILD: ERROR Bind to TFTP socket failed!\n");
		exit(1);
	}

	// Obtain the assigned port number
	if(getsockname(sd, (struct sockaddr*) &server, (socklen_t *) &length) < 0) {
		fprintf(stderr, "CHILD: ERROR Failure getting the port number!\n");
		exit(1);
	}

	printf("CHILD: Before looking at the opcode here is the filename: %s\n", client_request->filename);

	if(opcode == 1) //RRQ
	{
		printf("CHILD: This is the filename we need to READ: %s\n", client_request->filename);
		char* filename = (char*)client_request->filename;

		//check if this file exists
		if( access( filename, F_OK ) != -1 ) 
		{
			printf("CHILD: About to open file in RRQ: %s\n", filename);

			//open the file for reading
		    FILE *file = fopen(filename, "r");
		    uint8_t* file_string;
		    size_t n = 0;
		    int c;

		    //send error if could not open file
		    if (file == NULL) 
		    {
				printf("CHILD: ERROR could not open file.\n");

				// Creating an "file not found" error packet
				error_packet* error = malloc(sizeof(error_packet));
				error->opcode = htons(5);
				error->error_code = 4;
				memcpy(error->error_message, (uint8_t*)file_not_open_str, strlen(file_not_open_str));

				// Sending error packet
	    		sendto( sd, error, 4 + strlen(file_not_found_str) + 1, 0, 
	    			(struct sockaddr *) client, len );
	    		free(error);
	    		exit(1);
		    }

		    fseek(file, 0, SEEK_END);
		    long f_size = ftell(file);
		    fseek(file, 0, SEEK_SET);

		    printf("CHILD: This is the length of the file: %ld\n", f_size);

		    data_packet* file_data_packet;
		    ack_packet acknowledgement;
		    if(f_size < 512)
		    {
		    	//send the entire file if it's less than 512 bytes
		    	file_string = calloc(f_size, sizeof(uint8_t));
			    while ((c = fgetc(file)) != EOF)
			    {
			        file_string[n++] = (uint8_t)c;
			    }

			    data_packet* file_data_packet = malloc(sizeof(data_packet));
			    file_data_packet->opcode = htons(3);
			    file_data_packet->block_number = htons(1);
			    memcpy(file_data_packet->data, file_string, n);
			    // printf("This is the the data we are about to send: %s\n", file_data_packet->data);
				sendto(sd, file_data_packet, 4+n, 0, (struct sockaddr *) client, len );	
			    free(file_data_packet);
			    // sendDataPacket(sd, file_string, n, 1, &client, len);
			    free(file_string);

			    //receive ack packet
				recvfrom( sd, &acknowledgement, sizeof(ack_packet), 0, (struct sockaddr *) client,
					(socklen_t *) &len );
				printf("CHILD: Received acknowledgement packet. Opcode: %u, block number: %u\n", 
					ntohs(acknowledgement.opcode), ntohs(acknowledgement.block_number));
		    }
		    else
		    {
		    	//iterate through the file and send every 512 bytes
		    	file_string = calloc(512, sizeof(uint8_t));
		    	int block_number = 1;
		    	while ((c = fgetc(file)) != EOF)
		    	{
		    		if(n == 511)
		    		{
		    			file_string[n++] = (uint8_t)c;
					    file_data_packet = malloc(sizeof(data_packet));
					    file_data_packet->opcode = htons(3);
					    file_data_packet->block_number = htons(block_number);
			    		memcpy(file_data_packet->data, file_string, n);
						sendto(sd, file_data_packet, 4+n, 0, (struct sockaddr *) client, len );	
			    		free(file_data_packet);
			    		memset(file_string,0,n); //empty the string
			    		n = 0;
			    		block_number++;

						//receive ack packet
						recvfrom( sd, &acknowledgement, sizeof(ack_packet), 0, (struct sockaddr *) client,
							(socklen_t *) &len );
						printf("CHILD: Received acknowledgement packet. Opcode: %u, block number: %u\n", 
							ntohs(acknowledgement.opcode), ntohs(acknowledgement.block_number));
		    		}
		    		else
		    		{
		    			file_string[n++] = (uint8_t)c;
		    		}
		    	}

		    	//send leftover string (last block)
		    	if(n != 0)
		    	{ 
				    // file_data_packet = malloc(sizeof(tftp_packet));
				    // file_data_packet->dp.opcode = htons(3);
				    // file_data_packet->dp.block_number = htons(block_number);
    			    file_data_packet = malloc(sizeof(data_packet));
				    file_data_packet->opcode = htons(3);
				    file_data_packet->block_number = htons(block_number);
			    	memcpy(file_data_packet->data, file_string, n);
					sendto(sd, file_data_packet, 4+n, 0, (struct sockaddr *) client, len );	
			    	free(file_data_packet);	

					//receive ack packet
					recvfrom( sd, &acknowledgement, sizeof(ack_packet), 0, (struct sockaddr *) client,
						(socklen_t *) &len );
					printf("CHILD: Received acknowledgement packet. Opcode: %u, block number: %u\n", 
						ntohs(acknowledgement.opcode), ntohs(acknowledgement.block_number));		    		
		    	}
		    	free(file_string);
		    }
		    fclose(file);
		}
		else
		{
			printf("CHILD: ERROR File not found.\n");

			// Creating an "file not found" error packet
			error_packet* error = malloc(sizeof(error_packet));
			error->opcode = htons(5);
			error->error_code = 4;
			memcpy(error->error_message, (uint8_t*)file_not_found_str, strlen(file_not_found_str));

			// Sending error packet
    		sendto( sd, error, 4 + strlen(file_not_found_str) + 1, 0, 
    			(struct sockaddr *) client, len );
    		free(error);
		}
	}
	else if(opcode == 2) //WRQ
	{
		printf("CHILD: This is the filename we need to write to: %s\n", client_request->filename);
		char* filename = (char*)client_request->filename;

		//Cannot write to a file that already exists
		if( access( filename, F_OK ) != -1 )
		{
			printf("CHILD: ERROR File already exists.\n");

			// Creating an "file already exists" error packet
			error_packet* error = malloc(sizeof(error_packet));
			error->opcode = htons(5);
			error->error_code = 4;
			memcpy(error->error_message, (uint8_t*)file_already_exists_str, strlen(file_already_exists_str));

			// Sending error packet
    		sendto( sd, error, 4 + strlen(file_already_exists_str) + 1, 0, 
    			(struct sockaddr *) client, len );
    		free(error);

		}
		else
		{
			printf("About to open file in WRQ: %s\n", filename);

			//Open new file for writing
			FILE *file = fopen(filename ,"w");

		    if(file == NULL)
		    {
		        /* File not created hence exit */
		        printf("CHILD: ERROR Unable to create file.\n");
		        exit(1);
		    }

			//send an acknowledgement packet
			ack_packet acknowledgement;
			acknowledgement.opcode = htons(4);
			acknowledgement.block_number = htons(0);
			sendto(sd, &acknowledgement, sizeof(ack_packet), 0, (struct sockaddr *) client, len );

			//read the file in blocks of 512 bytes
			int num_bytes;
			data_packet* client_packet;
			while(1)
			{
				client_packet = malloc(sizeof(data_packet));
				num_bytes = recvfrom( sd, client_packet, sizeof(data_packet), 0, (struct sockaddr *) client,
					(socklen_t *) &len );
				num_bytes -= 4; //opcode and block number
				fwrite(client_packet->data, 1, num_bytes, file);
			    memset(client_packet->data, 0, 512); //empty the string

				//send an acknowledgement for each packet received
    			acknowledgement.opcode = htons(4);
    			acknowledgement.block_number = client_packet->block_number;
				sendto(sd, &acknowledgement, sizeof(ack_packet), 0, (struct sockaddr *) client, len);
				free(client_packet);

				if(num_bytes < 512)
				{
					printf("CHILD: Client closed connection\n");
					break;
				}
			}

			fclose(file);
		}
	}
}

int main(int argc, char* argv[]) {
	// Check that there are three command line arguments --- file, start port, and end port
	if(argc != 3) {
		fprintf(stderr, "MAIN: ERROR Invalid arguments!\n");
		return EXIT_FAILURE;
	}

	// Installing the alarm handler
	signal(SIGALRM, handle_alarm);

	// Read in the start and end ports
	unsigned short int startPort = atoi(argv[1]);
	unsigned short int endPort = atoi(argv[2]);

	// Quick setvbuf
	setvbuf(stdout, NULL, _IONBF, 0);

	// Create TFTP socket
	int sd = socket(AF_INET, SOCK_DGRAM, 0);

	if(sd < 0) {
		fprintf(stderr, "MAIN: ERROR socket creation failed!\n");
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
		fprintf(stderr, "MAIN: ERROR Bind to TFTP socket failed!\n");
		return EXIT_FAILURE;
	}

	// Obtain the assigned port number
	if(getsockname(sd, (struct sockaddr*) &server, (socklen_t *) &length) < 0) {
		fprintf(stderr, "MAIN: ERROR Failure getting the port number!\n");
		return EXIT_FAILURE;
	}

	//---------Application Level---------------------------------------

	// Server is starting
	printf("MAIN: TFTP server at port number %d\n", ntohs(server.sin_port));

  	int n;
	while ( 1 )
	{
	  	struct sockaddr_in client;
	  	int len = sizeof( client );
	  	request_packet client_request;

	    /* read a datagram from the remote client side (BLOCKING) */
	    n = recvfrom( sd, &client_request, sizeof(request_packet), 0, (struct sockaddr *) &client,
	                  (socklen_t *) &len );

	    int opcode = ntohs(client_request.opcode);
	    printf("MAIN: opcode: %d\n", opcode);
	    printf("MAIN: filename: %s\n", client_request.filename);

	    // Set alarm for 10 seconds, and terminate the program if no response
		alarm(10);

		if ( n == -1 ) 
		{
	      perror( "MAIN: recvfrom() failed" );
	    }
	    else
	    {
	    	printf( "MAIN: Rcvd datagram from %s port %d\n",
				inet_ntoa( client.sin_addr ), ntohs( client.sin_port ) );

	    	if(opcode == 1 || opcode == 2) // Request packet
	    	{
	    		printf("MAIN: Request packet identified\n");
	    		int pid = fork();
	    		startPort++;
	    		if(pid == 0){ //child
	    			handleRequest(startPort, opcode, &client, len, &client_request);
	    			return EXIT_SUCCESS;
	    		}
	    		else {
	    			if(startPort == endPort) {
	    				printf("MAIN: endPort reached!\n");
	    			}
	    		}
	    	}
	    	else 
	    	{
	    		printf("MAIN: ERROR Illegal TFTP operation.\n");

				// Creating an "invalid operation" error packet
				error_packet* error = malloc(sizeof(error_packet));
				error->opcode = htons(5);
				error->error_code = 4;
				memcpy(error->error_message, (uint8_t*)illegal_operation_str, strlen(illegal_operation_str));

				// Sending error packet
	    		sendto( sd, error, 4 + strlen(illegal_operation_str), 0, (struct sockaddr *) 
	    			&client, len );
	    		free(error);
	    	}
	    }
	}

	return EXIT_SUCCESS;
}
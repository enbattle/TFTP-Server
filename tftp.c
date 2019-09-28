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

//Error codes
   // 0         Not defined, see error message (if any).
   // 1         File not found.
   // 2         Access violation.
   // 3         Disk full or allocation exceeded.
   // 4         Illegal TFTP operation.
   // 5         Unknown transfer ID.
   // 6         File already exists.
   // 7         No such user.
//only need to do code 1,4, and 6

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

// typedef struct tftp_packet{
// 	uint16_t packet_type;         
//     uint8_t fname_and_mode[276];
// 	request_packet rp; // 1 and 2
// 	data_packet dp; // 3
// 	ack_packet ap; // 4
// 	error_packet ep; //5
// } tftp_packet;

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
	  	struct sockaddr_in client;
	  	int len = sizeof( client );
	  	request_packet client_request;
	  	// test_request* trq = malloc(sizeof(test_request));

	    /* read a datagram from the remote client side (BLOCKING) */
	    n = recvfrom( sd, &client_request, sizeof(request_packet), 0, (struct sockaddr *) &client,
	                  (socklen_t *) &len );

	    int opcode = ntohs(client_request.opcode);
	    printf("opcode: %d\n", opcode);
	    printf("filename: %s\n", client_request.filename);

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

	    	if(opcode == 1 || opcode == 2) // Request packet
	    	{
	    		printf("Request packet identified\n");

	    		if(opcode == 1) //RRQ
	    		{

	    			printf("This is the filename we need to open: %s\n", client_request.filename);
	    			char* filename = (char*)client_request.filename;
	    			//check if this file exists
	    			if( access( filename, F_OK ) != -1 ) 
	    			{
	    				//open the file for reading
    				    FILE *file = fopen(filename, "r");
    				    char* file_string;
					    size_t n = 0;
					    int c;

					    if (file == NULL) return NULL; //could not open file
					    fseek(file, 0, SEEK_END);
					    long f_size = ftell(file);
					    fseek(file, 0, SEEK_SET);

					    printf("This is the length of the file: %ld\n", f_size);

					    data_packet* file_data_packet;
					    if(f_size < 512)
					    {
					    	//send the entire file if it's less than 512 bytes
					    	file_string = calloc(f_size, sizeof(char));
						    while ((c = fgetc(file)) != EOF)
						    {
						        file_string[n++] = (char)c;
						    }
						    // file_data_packet = malloc(sizeof(tftp_packet));
						    // file_data_packet->dp.opcode = 3;
						    // file_data_packet->dp.block_number = 1;
						    file_data_packet = malloc(sizeof(data_packet));
						    file_data_packet->opcode = htons(3);
						    file_data_packet->block_number = htons(1);
						    // strncpy((uint8_t*)file_data_packet->data, file_string, n+1);
						    memcpy(file_data_packet->data, (uint8_t*)file_string, n+1);
						    printf("This is the the data we are about to send: %s\n", file_data_packet->data);
							sendto(sd, file_data_packet, 4+n+1, 0, (struct sockaddr *) &client, len );	
						    free(file_string);
						    free(file_data_packet);

						    //receive ack packet


					    }
					    else
					    {
		// 			    	//iterate through the file and send every 512 bytes
		// 			    	file_string = calloc(512, sizeof(char));
		// 			    	int block_number = 1;
		// 			    	while ((c = fgetc(file)) != EOF)
		// 			    	{
		// 			    		if(n == 511)
		// 			    		{
		// 						    // file_data_packet = malloc(sizeof(tftp_packet));
		// 						    // file_data_packet->dp.opcode = htons(3);
		// 						    // file_data_packet->dp.block_number = htons(block_number);
		// 						    file_data_packet = malloc(sizeof(data_packet));
		// 						    file_data_packet->opcode = htons(3);
		// 						    file_data_packet->block_number = htons(1);
		// 						    strncpy(file_data_packet->data, file_string, n+1);
		// 						    printf("This is the string I'm about to send: %s\n", file_string);
		// 							sendto(sd, file_data_packet, 4+n+1, 0, (struct sockaddr *) &client, len );
		// 				    		free(file_data_packet);
		// 				    		memset(file_string,0,n); //empty the string
		// 				    		n = 0;
		// 				    		block_number++;
		// 			    		}
		// 			    		else
		// 			    		{
		// 			    			file_string[n++] = (char)c;
		// 			    		}
		// 			    	}

		// 			    	//send leftover string (last bloc)
		// 			    	if(n != 0)
		// 			    	{ 
		// 					    // file_data_packet = malloc(sizeof(tftp_packet));
		// 					    // file_data_packet->dp.opcode = htons(3);
		// 					    // file_data_packet->dp.block_number = htons(block_number);
		// 	    			    file_data_packet = malloc(sizeof(data_packet));
		// 					    file_data_packet->opcode = htons(3);
		// 					    file_data_packet->block_number = htons(1);
		// 					    strncpy(file_data_packet->data, file_string, n+1);
		// 						printf("This is the string I'm about to send: %s\n", file_string);
		// 						sendto(sd, file_data_packet, 4+n+1, 0, (struct sockaddr *) &client, len );	
		// 				    	free(file_data_packet);			    		
		// 			    	}
		// 			    	free(file_string);
					    }
					    fclose(file);
	    			}
	    	// 		else
	    	// 		{
	    	// 			fprintf(stderr, "ERROR: File not found.\n");

	    	// 			// Creating an "file not found" error packet
						// error_packet* error = malloc(sizeof(error_packet));
						// error->opcode = htons(5);
						// error->error_code = 4;
						// strcpy(error->error_message, "File not found.");
						// error->padding = 0;

						// // Sending error packet
			   //  		sendto( sd, error, 4 + strlen(error->error_message) + 1, 0, 
			   //  			(struct sockaddr *) &client, len );
	    			
	    	// 		}



	    	// 		//Cannot write to a file that already exists
	    	// 		if( access( packet.filename, F_OK ) != -1 )
	    	// 		{
	    	// 			fprintf(stderr, "ERROR: File already exists.\n");

	    	// 			// Creating an "file already exists" error packet
						// error_packet* error = malloc(sizeof(error_packet));
						// error->opcode = htons(5);
						// error->error_code = 4;
						// strcpy(error->error_message, "File already exists.");
						// error->padding = 0;

						// // Sending error packet
			   //  		sendto( sd, error, 4 + strlen(error->error_message) + 1, 0, 
			   //  			(struct sockaddr *) &client, len );



    		// 		    if(file == NULL)
					 //    {
					 //        /* File not created hence exit */
					 //        fprintf(stderr, "ERROR: unable to create file.\n");
					 //        return EXIT_FAILURE;
					 //    }












	    		}
	    		else if(opcode == 2) //WRQ
	    		{

	    	// 		//Cannot write to a file that already exists
	    	// 		if( access( packet.filename, F_OK ) != -1 )
	    	// 		{

	    	// 			//TODO: send error packet
	    	// 			printf("File already exists!\n");

	    	// 		}
	    	// 		else
	    	// 		{
	    	// 			//Open new file for writing
	    	// 			FILE *file = fopen(packet.filename ,"w");

    		// 		    if(file == NULL)
					 //    {
					 //        /* File not created hence exit */
					 //        printf("Unable to create file.\n");
					 //        exit(EXIT_FAILURE);
					 //    }

		    // 			//send an acknowledgement packet
		    // 			tftp_packet* response_packet = malloc(sizeof(tftp_packet));
		    // 			response_packet->packet_type = 4;
		    // 			response_packet->ap.opcode = 4;
		    // 			response_packet->ap.block_number = 0;
						// sendto(sd, response_packet, sizeof(tftp_packet), 0, (struct sockaddr *) &client, len );
						// free(response_packet);

						// //read the file in blocks of 512 bytes
						// int num_bytes;
						// tftp_packet client_packet;
						// while(1)
						// {
						// 	recvfrom( sd, &client_packet, sizeof(tftp_packet), 0, (struct sockaddr *) &client,
						// 		(socklen_t *) &len );
						// 	num_bytes = strlen(client_packet.dp.data);
						// 	printf("RECEIVED (%d) bytes:\n %s\n", num_bytes, client_packet.dp.data);
						// 	fputs(client_packet.dp.data, file);
						// 	//send an acknowledgement for each packet received
			   //  			response_packet = malloc(sizeof(tftp_packet));
			   //  			response_packet->packet_type = 4;
			   //  			response_packet->ap.opcode = 4;
			   //  			response_packet->ap.block_number = client_packet.dp.block_number;
						// 	sendto(sd, response_packet, sizeof(tftp_packet), 0, (struct sockaddr *) &client, len );	
						// 	free(response_packet);					

						// 	if(num_bytes < 512)
						// 	{
						// 		printf("Client closed connection\n");
						// 		break;
						// 	}
						// }
						// fclose(file);
	    	// 		}

	    		}
	    	}
	    	else if (opcode == 3) {

	    	}

	    	else 
	    	{
	   //  		fprintf(stderr, "ERROR: Illegal TFTP operation.\n");

				// // Creating an "invalid operation" error packet
				// error_packet* error = malloc(sizeof(error_packet));
				// error->opcode = htons(5);
				// error->error_code = 4;
				// strcpy(error->error_message, "Illegal TFTP operation.");
				// error->padding = 0;

				// // Sending error packet
	   //  		sendto( sd, error, 4 + strlen(error->error_message) + 1, 0, (struct sockaddr *) 
	   //  			&client, len );
	    	}
	    }
	}

	return EXIT_SUCCESS;
}
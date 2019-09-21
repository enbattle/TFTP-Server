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
	int tftp = socket(AF_INET, SOCK_DGRAM, 0);

	if(tftp < 0) {
		fprintf(stderr, "ERROR: socket creation failed!\n");
		return EXIT_FAILURE;
	}

	// Create server struct
	struct sockaddr_in server;

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(startPort);

	int len = sizeof(server);

	// Bind server to a port
	if(bind(tftp, (struct sockaddr*) &server, len) < 0) {
		fprintf(stderr, "ERROR: Bind to TFTP socket failed!\n");
		return EXIT_FAILURE;
	}

	// Obtain the assigned port number
	if(getsockname(tftp, (struct sockaddr*) &server, (socklen_t *) &len) < 0) {
		fprintf(stderr, "ERROR: Failure getting the port number!\n");
		return EXIT_FAILURE;
	}

	// Server is starting
	printf("TFTP server at port number %d\n", ntohs(server.sin_port));

	while(1) {

	}

	return EXIT_SUCCESS;
}
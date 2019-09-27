// Client side implementation of UDP client-server model 
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
  
#define PORT     8080 
#define MAXLINE 1024 

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

int main() { 
    int sockfd; 
    char buffer[MAXLINE]; 
    char *hello = "BROADCAST 15\nHi, how are ya?"; 
    struct sockaddr_in     servaddr; 
  
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
  
    memset(&servaddr, 0, sizeof(servaddr)); 
      
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(8120); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
      
    int n, len; 


    //SENDING MESSAGE TO SERVER----------------------------------------------
    tftp_packet* request = malloc(sizeof(tftp_packet));
    request->packet_type = 0;
    request->rp.opcode = 2;
    strcpy(request->rp.filename, "test.txt");
    request->rp.padding1 = 0;
    strcpy(request->rp.mode, "binary");

    printf("Sending request packet:\n packet_type: %d\n opcode: %u\n filename: %s\n padding1: %u\n mode: %s\n padding2: %u\n",
        request->packet_type, request->rp.opcode, request->rp.filename, request->rp.padding1, request->rp.mode,
        request->rp.padding2);


    sendto(sockfd, request, sizeof(tftp_packet), MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr)); 

    //RECEIVE ACKNOWLEDGEMENT FROM SERVER
    tftp_packet* response_packet = malloc(sizeof(tftp_packet));
    n = recvfrom(sockfd, response_packet, MAXLINE, 0, (struct sockaddr *) &servaddr, (socklen_t*)&len); 
    printf("Received packet from server:\n");
    printf("opcode: %u\n", response_packet->ap.opcode);
    printf("block_number: %u\n", response_packet->ap.block_number); 
    free(response_packet);

    //SEND MESSAGE TO THE SERVER

    char* msg1 = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    char* msg2 = "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB";
    char* msg3 = "CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC";
    char* msg4 = "This should break the connection";

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

    //Send and receive an acknowledge for each packet
    printf("Sending msg: %s\n", msg1);
    sendto(sockfd, &dp1, sizeof(tftp_packet), MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr));
    response_packet = malloc(sizeof(tftp_packet));
    n = recvfrom(sockfd, response_packet, MAXLINE, 0, (struct sockaddr *) &servaddr, (socklen_t*)&len); 
    printf("Received packet from server:\n");
    printf("opcode: %u\n", response_packet->ap.opcode);
    printf("block_number: %u\n", response_packet->ap.block_number); 
    free(response_packet);

    printf("Sending msg: %s\n", msg2);
    sendto(sockfd, &dp2, sizeof(tftp_packet), MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr)); 
    response_packet = malloc(sizeof(tftp_packet));
    n = recvfrom(sockfd, response_packet, MAXLINE, 0, (struct sockaddr *) &servaddr, (socklen_t*)&len); 
    printf("Received packet from server:\n");
    printf("opcode: %u\n", response_packet->ap.opcode);
    printf("block_number: %u\n", response_packet->ap.block_number); 
    free(response_packet);

    printf("Sending msg: %s\n", msg3);
    sendto(sockfd, &dp3, sizeof(tftp_packet), MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr));  
    response_packet = malloc(sizeof(tftp_packet));
    n = recvfrom(sockfd, response_packet, MAXLINE, 0, (struct sockaddr *) &servaddr, (socklen_t*)&len); 
    printf("Received packet from server:\n");
    printf("opcode: %u\n", response_packet->ap.opcode);
    printf("block_number: %u\n", response_packet->ap.block_number); 
    free(response_packet);
  
    close(sockfd); 
    return 0; 
} 
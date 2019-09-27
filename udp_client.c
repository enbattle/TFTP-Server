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
  
// Driver code 
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

    //SENDING MESSAGE----------------------------------------------
    request_packet* r1 = malloc(sizeof(request_packet));
    r1->opcode = 1;
    strcpy(r1->filename, "test.txt");
    r1->padding1 = 0;
    strcpy(r1->mode, "binary");

    printf("Before Sending.\n opcode: %u\n filename: %s\n padding1: %u\n mode: %s\n padding2: %u\n",
        r1->opcode, r1->filename, r1->padding1, r1->mode,
        r1->padding2);

    sendto(sockfd, r1, sizeof(request_packet), MSG_CONFIRM, (const struct sockaddr *) &servaddr,  sizeof(servaddr)); 
    printf("r1 message sent.\n"); 
          
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, (socklen_t*)&len); 
    buffer[n] = '\0'; 
    printf("Server : %s\n", buffer); 
  
    close(sockfd); 
    return 0; 
} 
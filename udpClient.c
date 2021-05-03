#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

void storeText(int socket);
unsigned char* createHeader(unsigned short count, unsigned short seq);
unsigned char* combineText(unsigned char* header, unsigned char* data);
unsigned char* getHeader(unsigned char* response);

int main(){

	int clientSock;
	struct sockaddr_in serverAddress;

	unsigned char clientMessage[256], header[4];

	//gets the name of the file from user
	printf("Enter file name: ");
	scanf("%s", &clientMessage);
	
	clientSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  //creates a socket

	if(clientSock< 0){
		printf("socket() failed");
		return 0;
	}

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8003);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	/*
	if(bind(clientSock, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0){
		printf("bind() failed");
	}
	*/

	strcpy(header, createHeader((unsigned short) strlen(clientMessage), (unsigned short)0));
	
	unsigned char *text = combineText(header, clientMessage);

	sendto(clientSock, text, strlen(text+4) + 4, 0,(struct sockaddr*) &serverAddress, sizeof(serverAddress));

	printf("Sizeof: %ld\n", strlen(text +4) + 4);
	printf("Message Sent\n");
	
	free(text);
	
	//store incoming data for file
	//storeText(clientSock);
	close(clientSock);
	return 0;
}

unsigned char* getHeader(unsigned char* response){
	unsigned char header[4];

	strncpy(header, response, 4);

	unsigned char* returnHeader = malloc(strlen(header)+1);
	strcpy(returnHeader, header);

	return returnHeader;
}

void storeText(int socket){
	/*
	unsigned char responce[84];

	int totalCount = 0;
	int totalPacket = 0;

	FILE * file;
	file = fopen("out.txt", "w"); //write to file out.txt

	int headerCount;
	int dataCount;

	while(1){
		
		//reassemble the char array into two shorts(count and sequence number)
		//unsigned short count = header[0] + (header[1] << 8);
		//unsigned short seq = header[2] + (header[3] << 8);

		//clears out the char array
		bzero(responce, 84);
		//recieves the data
		dataCount = recv(socket, responce, sizeof(responce), 0);

		//if the length of the data is 0 and count is zero, then it's the end of transmission
		if(!strlen(serverResponce) && (count == 0)){
			printf("End of Transmission Packet with sequence number %d received with %d data bytes\n", seq, count);
			break;
		}
		//puts the data into the file
		fputs(serverResponce, file);
		//update value
		totalCount += count;
		++totalPacket;
		printf("Packet %d received with %d data bytes\n", seq, count);		
		
		//signal to move on
		send(socket, "", sizeof(""), 0);

	}
	//close file
	fclose(file);

	printf("Number of data packets received: %d\n", totalPacket);
	printf("Number of data bytes received: %d\n", totalCount);
	*/
}

unsigned char* combineText(unsigned char* header, unsigned char* data){
	unsigned char* combine = malloc(4 + strlen(data) + 1);

	strcpy(combine, header);
	strcat(combine+4, data);

	printf("%s\n", combine+4);
	return combine;
}

unsigned char* createHeader(unsigned short count, unsigned short seq){
	unsigned char header[4];

	//dissambles count and sequence number into a 4 bytes char array
	header[0] = count;
	header[1] = count >> 8;
	header[2] = seq;
	header[3] = seq >> 8;

	

	unsigned char * headerString = malloc(sizeof(header) + 1);
	strcpy(headerString, header);

	//printf("Strlen header: %ld\n", strlen(headerString));
	//printf("Sizeof header: %ld\n", sizeof(headerString));

	return headerString;
}
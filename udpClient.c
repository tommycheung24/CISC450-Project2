#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

void storeText(int socket, struct sockaddr_in server);
unsigned char* createHeader(unsigned short seq, unsigned short count);
unsigned char* combineText(unsigned char* header, unsigned char* data);
unsigned char* getHeader(unsigned char* response);
unsigned short getCount(unsigned char* header);
unsigned short getSequence(unsigned char* header);
unsigned char* getMessage(unsigned char* response);

int main(){

	int clientSock;
	struct sockaddr_in serverAddress;

	unsigned char clientMessage[256], header[4];

	//gets the name of the file from user
	printf("Enter file name: ");
	scanf("%s", clientMessage);
	
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
	unsigned short count = strlen(clientMessage);
	unsigned char *text = malloc(count + 4);
	text[0] = count;
	text[1] = count << 8;
	text[2] = 0;
	text[3] = 0 << 8;

	strcat(text+4, clientMessage);

	sendto(clientSock, text, strlen(text+4) + 4, 0,(struct sockaddr*) &serverAddress, sizeof(serverAddress));
	printf("Size: %ld\n", strlen(text+4) + 4);
	
	//store incoming data for file
	
	storeText(clientSock, serverAddress);

	close(clientSock);
	return 0;
}

unsigned char* getHeader(unsigned char* response){
	unsigned char header[4];

	strncpy(header, response, 4);

	unsigned char* returnHeader = malloc(sizeof(header) + 1);
	strncpy(returnHeader, header, 4);
	returnHeader[4] = '\0';

	return returnHeader;
}
unsigned char* getMessage(unsigned char* response){
	unsigned char *newResponce = malloc(strlen(response+4) + 1);

	strcpy(newResponce, response+4);
	return newResponce;
}

void storeText(int socket, struct sockaddr_in server){

	unsigned char response[85], header[5], ack[3], message[81];
	socklen_t serverSize = sizeof(server);

	int totalCount = 0;
	int totalPacket = 0;

	FILE * file;
	file = fopen("out.txt", "w"); //write to file out.txt

	int headerCount;
	int dataCount;

	while(1){

		bzero(response, 85);
		bzero(message, 81);
		bzero(header, 5);
		bzero(ack, 3);

		dataCount = recvfrom(socket, response, sizeof(response), 0, (struct sockaddr*)&server, &serverSize);
		
		//reassemble the char array into two shorts(count and sequence number)
		unsigned short count = response[0] + (response[1] << 8);
		unsigned short seq = response[2] + (response[3] << 8);

		printf("Seq and Size: %d %d\n", seq, count);
		printf("Message: %s", response+4);

		//puts the data into the file
		fputs(response+4, file);
		
		//recieves the data
		ack[0] = seq;
		ack[1] = seq << 8;

		sendto(socket, ack, 2, 0, (struct sockaddr*)&server, sizeof(server));
		
		//update value
		totalCount += count;
		++totalPacket;	
		

	}
	//close file
	fclose(file);
}
unsigned short getSequence(unsigned char* header){
	unsigned short count = header[2] + (header[3] << 8);
	return count;
}

unsigned short getCount(unsigned char* header){
	unsigned short seq = header[0] + (header[1] << 8);
	return seq;
}


unsigned char* combineText(unsigned char* header, unsigned char* data){
	unsigned char* combine = malloc(4 + strlen(data) + 1);

	strncpy(combine, header, 4);
	strcpy(combine+4, data);

	return combine;
}

unsigned char* createHeader(unsigned short count, unsigned short seq){
	unsigned char header[4];

	//dissambles count and sequence number into a 4 bytes char array
	header[0] = count;
	header[1] = count >> 8;
	header[2] = seq;
	header[3] = seq >> 8;

	unsigned char * headerString = malloc(4+ 1);
	strncpy(headerString, header, 4);
	headerString[4] = '\0';

	return headerString;
}
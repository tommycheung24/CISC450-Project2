#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

void sendText(int socket,unsigned char* textName, struct sockaddr_in client);
unsigned char* createHeader(unsigned short seq, unsigned short count);
unsigned char* getHeader(unsigned char* response);
unsigned char* getMessage(unsigned char* response);
unsigned short getCount(unsigned char* header);
unsigned short getSequence(unsigned char* header);
unsigned char* combineText(unsigned char* header, unsigned char* data);

int main(){

	unsigned char response[256];

	int serverSock;
	struct sockaddr_in servAddress, clientAddress;
	socklen_t addr_size = sizeof(clientAddress);

	serverSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(serverSock< 0){
		printf("socket() failed");
		return 0;
	}

	servAddress.sin_family = AF_INET;
	servAddress.sin_port = htons(8003);
	servAddress.sin_addr.s_addr = INADDR_ANY;

	//binds the socket with the server Address
	if(bind(serverSock, (struct sockaddr *) &servAddress, sizeof(servAddress)) < 0){
		printf("bind() failed");
		return 0;
	}

	if(recvfrom(serverSock, response, sizeof(response), 0,(struct sockaddr*) &clientAddress, &addr_size) <0 ){
		printf("header recv() failed\n");
		return 0;
	}

	printf("Size of message: %ld\n", strlen(response+4) + 4);
	unsigned char header[5];
	strcpy(header, getHeader(response));

	
	unsigned short seq = getSequence(header);
	unsigned short count = getCount(header);

	printf("%d %d\n", seq, count);
	
	sendText(serverSock, getMessage(response), clientAddress);

	close(serverSock);

	return 0;
}
unsigned short getCount(unsigned char* header){
	unsigned short count = header[2] + (header[3] << 8);
	return count;
}

unsigned short getSequence(unsigned char* header){
	unsigned short seq = header[0] + (header[1] << 8);
	return seq;
}

unsigned char* getHeader(unsigned char* response){
	unsigned char header[5];

	strncpy(header, response, 4);
	header[4] = '\0';

	unsigned char* returnHeader = malloc(sizeof(header));
	strcpy(returnHeader, header);

	return returnHeader;
}

unsigned char* getMessage(unsigned char* response){
	unsigned char *newResponce = malloc(strlen(response+4) + 1);

	strcpy(newResponce, response+4);
	return newResponce;
}

void sendText(int socket,unsigned char* textName, struct sockaddr_in client){
	
	//line_buffer is the line in the file, confirm is the response from client 
	unsigned char line_buffer[81], ackMessage[3];
	unsigned short ack = 0;
	socklen_t clientSize = sizeof(client);


	FILE* file;
	file= fopen(textName, "r"); // read file with name textName

	unsigned short seq = 1;
	int totalCount = 0;

	while(fgets(line_buffer, sizeof(line_buffer), file)){
		printf("%s", line_buffer);

		unsigned char newLine[strlen(line_buffer) + 4];
		strcpy(newLine, createHeader(seq, (unsigned short)strlen(line_buffer)));
		strcpy(newLine+4, line_buffer);
		printf("Line sent: %s", newLine+4);
		
		while(ack != seq){
			sendto(socket, newLine, strlen(newLine+4) + 4, 0, (struct sockaddr*)&client, sizeof(client));

			recvfrom(socket, ackMessage, sizeof(ackMessage), 0,(struct sockaddr*)&client, &clientSize);
			ack = getSequence(ackMessage);
		}

		seq = (seq + 1) % 2;
		totalCount += strlen(newLine+4);

		bzero(newLine, strlen(line_buffer) + 4);
		bzero(line_buffer, 80);
	}

	fclose(file);
}

unsigned char* combineText(unsigned char* header, unsigned char* data){
	unsigned char* combine = malloc(4 + strlen(data) + 1);

	strcpy(combine, header);
	strcat(combine+4, data);

	return combine;
}

unsigned char* createHeader(unsigned short seq, unsigned short count){
	unsigned char charSeq[2], charCount[2];

	//dissambles count and sequence number into a 4 bytes char array
	charSeq[0] = seq;
	charSeq[1] = seq >> 8;
	charCount[0] = count;
	charCount[1] = count >> 8;



	unsigned char * headerString = malloc(4+ 1);
	strcpy(headerString, charSeq);
	strcat(headerString, charCount);

	return headerString;
}


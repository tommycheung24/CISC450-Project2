#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

void sendText(int socket,char* textName);
unsigned char* createHeader(unsigned short count, unsigned short sequenceNumber);
unsigned char* getHeader(unsigned char* response);
unsigned char* getMessage(unsigned char* response);

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

	printf("Message recieved\n");
	printf("Size of message: %ld\n", strlen(response));
	unsigned char header[4];
	strcpy(header, getHeader(response));

	printf("%d %d\n", seq, count);

	printf("%s\n", getMessage(response));

	//sendText(clientSock, clientResponce);

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
	unsigned char header[4];

	strncpy(header, response, 4);

	unsigned char* returnHeader = malloc(strlen(header)+1);
	strcpy(returnHeader, header);

	return returnHeader;
}

unsigned char* getMessage(unsigned char* response){

	printf("response: %ld\n", strlen(response+4));
	unsigned char *newResponce = malloc(strlen(response+4) + 1);

	strcpy(newResponce, response+4);

	printf("newResponce: %ld\n", strlen(newResponce));

	return newResponce;
}

void sendText(int socket,unsigned char* textName, struct sockaddr_in client){
	
	//line_buffer is the line in the file, confirm is the response from client 
	char line_buffer[80];
	unsigned short ack = 0;
	socklen_t clientSize = sizeof(client);


	FILE* file;
	file= fopen(textName, "r"); // read file with name textName

	unsigned short seq = 1;
	int totalCount = 0;

	while(fgets(line_buffer, sizeof(line_buffer), file)){

		unsigned char newLine[strlen(line_buffer) + 4];
		strcpy(newLine, createHeader((unsigned short)strlen(line_buffer), seq));
		strcat(newLine+4, line_buffer);
		
		//gets the size in bytes of the new char array
		unsigned short count = (unsigned short) sizeof(newLine);
		
		sendto(socket, newLine, sizeof(newLine), 0, (struct sockaddr*)client, &clientSize);


		seq = (seq + 1) % 2;
		totalCount += strlen(line_buffer);
	}

	fclose(file);
}

unsigned char* combineText(unsigned char* header, unsigned char* data){
	unsigned char* combine = malloc(4 + strlen(data) + 1);

	strcpy(combine, header);
	strcat(combine+4, data);

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


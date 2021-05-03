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
	unsigned short seq = header[0] + (header[1] << 8);
	unsigned short count = header[2] + (header[3] << 8);

	printf("%d %d\n", seq, count);

	printf("%s\n", getMessage(response));

	//sendText(clientSock, clientResponce);

	close(serverSock);

	return 0;
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
/*
void sendText(int socket,char* textName){
	
	//line_buffer is the line in the file, confirm is the response from client 
	char line_buffer[80], confirm[1];

	FILE* file;
	file= fopen(textName, "r"); // read file with name textName

	unsigned short sequenceNumber = 1;
	int totalCount = 0;

	//gets a single line in the file and stores it in line_buffer
	while(fgets(line_buffer, sizeof(line_buffer), file)){

		//cuts the unused space away before sending the data
		char newLine[strlen(line_buffer)];
		strcpy(newLine, line_buffer);
		
		//gets the size in bytes of the new char array
		unsigned short count = (unsigned short) sizeof(newLine);
		
		//sends the header with info, count and sequence number
		sendHeader(socket, count, sequenceNumber, 0);
		//recieves a confirmation from client so the server don't move on until the client is ready
		recv(socket, confirm, sizeof(confirm), 0);
		//sends the actually data after the server recieves confirmation
		send(socket, newLine, sizeof(newLine), 0);

		//add 1 to sequence number, add count to totalCount(total bytes send)
		++sequenceNumber;
		totalCount += count;

		//waits again for confirmation to move on from client
		recv(socket, confirm, sizeof(confirm), 0);
	}

	//since it's out of loop it means that there's no more lines to grab from file
	//time to send the End of Transmission packet
	sendHeader(socket, 0, sequenceNumber, 1);
	//sends an empty data 
	send(socket, "", sizeof(""), 0);

	printf("Number of data packets transmitted: %d\n", sequenceNumber -1);
	printf("Total number of bytes transmitted: %d\n", totalCount);

	fclose(file);
}
*/

unsigned char * createHeader(unsigned short count, unsigned short sequenceNumber){
	unsigned char header[4];

	//putting the two shorts into a 4 bytes char array 
	header[0] = count;
	header[1] = count >> 8;
	header[2] = sequenceNumber;
	header[3] = sequenceNumber >> 8;

	unsigned char * headerString = malloc(sizeof(header) + 1);
	strcpy(headerString, header);

	return headerString;
}


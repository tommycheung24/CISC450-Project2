#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

void storeText(int socket, struct sockaddr_in server, float ratio);
int simulateACKLoss(float ratio);

int main(){

	srand(time(NULL));
	int clientSock;
	struct sockaddr_in serverAddress;
	float ratio;

	unsigned char clientMessage[256];

	//gets the name of the file from user
	printf("Enter file name: ");
	scanf("%s", clientMessage);
	printf("Enter ACK Loss Ratio: ");
	scanf("%f", &ratio);
	
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
	
	storeText(clientSock, serverAddress, ratio);

	close(clientSock);
	return 0;
}

void storeText(int socket, struct sockaddr_in server, float ratio){

	unsigned char response[85], ack[3];
	socklen_t serverSize = sizeof(server);

	int totalCount = 0;
	int totalPacket = 0;
	unsigned short prevSeq = 0;

	FILE * file;
	file = fopen("out.txt", "w"); //write to file out.txt

	while(1){

		bzero(response, 85);
		bzero(ack, 3);

		recvfrom(socket, response, sizeof(response), 0, (struct sockaddr*)&server, &serverSize);
		
		//reassemble the char array into two shorts(count and sequence number)
		unsigned short count = response[0] + (response[1] << 8);
		unsigned short seq = response[2] + (response[3] << 8);

		if(count == 0){
			printf("End of Transmission Packet with sequence number %d recieved\n", seq);
			break;
		}
		if(seq != prevSeq){
			printf("Packet %d recieved with %d data bytes\n", seq, count);
			fputs(response+4, file);
			
			totalCount += count;
			++totalPacket;
			prevSeq = seq;
		}else{
			printf("Duplicate packet %d recieved with %d data bytes\n", seq, count);
		}

		
		//recieves the data
		ack[0] = seq;
		ack[1] = seq << 8;
		printf("ACK %d generated for transmission\n", seq);
		if(simulateACKLoss(ratio) == 0){
			sendto(socket, ack, 2, 0, (struct sockaddr*)&server, sizeof(server));
			printf("ACK %d successfully transmitted\n", seq);
		}else{
			printf("ACK %d lost\n", seq);
		}

	}
	//close file
	fclose(file);
}

int simulateACKLoss(float ratio){
	float n = (float)rand()/ (float)RAND_MAX;

	if(n < ratio){
		return 1;
	}
	return 0;
}



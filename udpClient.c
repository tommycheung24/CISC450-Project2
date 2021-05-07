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
	//used to generate a random number
	srand(time(NULL));
	//socket and server info
	int clientSock;
	struct sockaddr_in serverAddress;
	
	//used for user input
	float ratio;
	unsigned char clientMessage[256];

	//gets the name of the file from user and ack loss ratio
	printf("Enter file name: ");
	scanf("%s", clientMessage);
	printf("Enter ACK Loss Ratio(0-1): ");
	scanf("%f", &ratio);
	
	clientSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  //creates a socket

	if(clientSock< 0){
		printf("socket() failed");
		return 0;
	}

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(8003);
	serverAddress.sin_addr.s_addr = INADDR_ANY;

	//initialize a string for client to send packet, the first 4 will always be for the header
	unsigned short count = strlen(clientMessage);
	unsigned char *text = malloc(count + 5);
	text[0] = count;
	text[1] = count << 8;
	text[2] = 0;
	text[3] = 0 << 8;
	//add the file to read to the string
	strcat(text+4, clientMessage);

	sendto(clientSock, text, strlen(text+4) + 4, 0,(struct sockaddr*) &serverAddress, sizeof(serverAddress));

	free(text);
	
	//store incoming data for file
	
	storeText(clientSock, serverAddress, ratio);

	close(clientSock);
	return 0;
}

void storeText(int socket, struct sockaddr_in server, float ratio){

	//responce and ack string
	unsigned char response[85], ack[3];
	socklen_t serverSize = sizeof(server);

	int totalPacket = 0, duplicatePacket = 0, initialPacket = 0, totalBytes = 0, transmittedACK = 0, lossACK = 0, totalACK = 0; 
	//keeps track of the previous sequence number to see if it's a duplicate or not
	unsigned short prevSeq = 0;

	FILE * file;
	file = fopen("out.txt", "w"); //write to file out.txt

	while(1){
		//flushed the strings
		bzero(response, 85);
		bzero(ack, 3);

		recvfrom(socket, response, sizeof(response), 0, (struct sockaddr*)&server, &serverSize);
		
		//reassemble the char array into two shorts(count and sequence number)
		unsigned short count = response[0] + (response[1] << 8);
		unsigned short seq = response[2] + (response[3] << 8);

		//if count is 0 then will know it's the end of the communication
		if(count == 0){
			printf("End of Transmission Packet with sequence number %d recieved\n", seq);
			break;
		}
		++totalPacket;

		if(seq != prevSeq){
			printf("Packet %d recieved with %d data bytes\n", seq, count);
			fputs(response+4, file);
			printf("Packet %d delivered to user\n", seq);
			
			totalBytes += count;
			++initialPacket; 
			prevSeq = seq;
		}else{
			printf("Duplicate packet %d recieved with %d data bytes\n", seq, count);
			++duplicatePacket;
		}

		
		//send the sequnce number back to server, showing acknowledgement(ack)
		ack[0] = seq;
		ack[1] = seq << 8;
		printf("ACK %d generated for transmission\n", seq);
		++totalACK; 
		if(simulateACKLoss(ratio) == 0){
			sendto(socket, ack, 2, 0, (struct sockaddr*)&server, sizeof(server));
			printf("ACK %d successfully transmitted\n", seq);
			++transmittedACK;
		}else{
			printf("ACK %d lost\n", seq);
			++lossACK;
		}

	}
	//close file
	fclose(file);

	printf("Total number of data packet: %d\n", totalPacket);
	printf("Number of duplicate data packets recieved: %d\n", duplicatePacket);
	printf("Number of initial data packets recieved: %d\n", initialPacket);
	printf("Total Number of bytes delivered to user: %d\n", totalBytes);
	printf("Number of Acks transmitted: %d\n", transmittedACK);
	printf("Number of Dropped Acks: %d\n", lossACK);
	printf("Total Number of Ack generated: %d\n", totalACK);

}

int simulateACKLoss(float ratio){
	//generates a float that is between 0-1
	float n = (float)rand()/ (float)RAND_MAX;

	if(n < ratio){
		return 1;
	}
	return 0;
}



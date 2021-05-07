#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

void sendText(int socket,unsigned char* textName, struct sockaddr_in client, int timeInput, float ratio);
int simulateLoss(float ratio);

int main(){

	//used for random number generater
	srand(time(NULL));

	//used for user input
	unsigned char response[256];
	float ratio;
	int timeout;

	printf("Enter Timeout(1-10): ");
	scanf("%d", &timeout);
	printf("Enter Packet Loss Ratio(0-1):" );
	scanf("%f", &ratio);

	//initialize socket
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

	//recieves the intial message with the fill name that we have to read to client
	if(recvfrom(serverSock, response, sizeof(response), 0,(struct sockaddr*) &clientAddress, &addr_size) <0 ){
		printf("header recv() failed\n");
		return 0;
	}
	
	sendText(serverSock, response +4, clientAddress, timeout, ratio);

	close(serverSock);

	return 0;
}

void sendText(int socket,unsigned char* textName, struct sockaddr_in client, int timeInput, float ratio){
	
	unsigned char line_buffer[80], ackMessage[3];
	unsigned short ack = 0;
	socklen_t clientSize = sizeof(client);

	//timeout Interval is is the 10^n where n is timeInput
	double time = pow(10, timeInput);
	//two set and two timeval because select is destructive so we use a temp
	struct timeval timeout, tempTimeout;
	fd_set currentSock, tempSock;
	//sets the current socket with the our socket it is binded
	FD_ZERO(&currentSock);
	FD_SET(socket, &currentSock);

	timeout.tv_sec = (int) time/pow(10,6);
	timeout.tv_usec = (int) (time - timeout.tv_sec*pow(10,6));

	FILE* file;
	file= fopen(textName, "r"); // read file with name textName

	unsigned short seq = 1;
	int initialPacket =0 , initialBytes =0, totalPacket =0, dropPacket = 0, transmittedPacket=0, recievedACK=0, numTimeout = 0;

	while(fgets(line_buffer, sizeof(line_buffer), file)){

		//redo is to keep track if we are transmitting a new packet or retransmitting an old packet
		int redo = 0;
		//used to store the header info in the first 4 chars, 4 bytes
		unsigned short count = strlen(line_buffer);
		unsigned char *newLine = malloc(count + 5);
		newLine[0] = count;
		newLine[1] = count << 8;
		newLine[2] = seq;
		newLine[3] = seq << 8;
		//adds the data afterwards
		strcat(newLine+4, line_buffer);
		
		while(ack != seq){
			//reassign the temp since select is destructive
			tempSock = currentSock;
			tempTimeout = timeout;

			totalPacket += 1;

			if(redo){
				printf("Packet %d gernerated for retransmission with %d data bytes\n", seq ,count);

			}else{
				printf("Packet %d gernerated for transmission with %d data bytes\n", seq, count);
				initialPacket += 1;
				initialBytes += count;
			}

			if(simulateLoss(ratio) == 0){
				sendto(socket, newLine, strlen(newLine+4) + 4, 0, (struct sockaddr*)&client, sizeof(client));
				printf("Packet %d successfully transmitted with %d data bytes\n", seq, count);
				transmittedPacket += 1;
			}else{
				printf("Packet %d lost\n", seq);
				dropPacket += 1;
			}

			//select listens to see if there is any activity in tempSock within a timeinterval
			if(select(FD_SETSIZE, &tempSock, NULL, NULL, &tempTimeout) < 0){
				printf("select() failed\n");
				break;
			}else if(FD_ISSET(socket, &tempSock)){
				recvfrom(socket, ackMessage, sizeof(ackMessage), 0,(struct sockaddr*)&client, &clientSize);
				ack = ackMessage[0] + (ackMessage[1] >> 8);
				printf("ACK %d recieved\n", ack);
				recievedACK += 1;
			}else{
				//when is reaches here, it means that the time ran out and there is nothing for us
				redo = 1;
				printf("Timeout expired for packet number %d\n", seq);
				numTimeout += 1; 
			}
		}

		seq = (seq + 1) % 2;

		free(newLine);
		bzero(line_buffer, 80);
	}
	//sends the last packet EOT packet
	unsigned char *last = malloc(4);
	last[0] = 0;
	last[1] = 0;
	last[2] = seq;
	last[3]= seq << 8;
	sendto(socket, last, 4, 0, (struct sockaddr*)&client, sizeof(client));


	printf("End of Tranmission Packet with sequence number %d transmitted\n", seq);
	printf("Number of data packets gernerated for initial tranmission: %d\n", initialPacket);
	printf("Total number of data bytes for initial tranmission: %d\n", initialBytes);
	printf("Total number of data packets for transmission: %d\n", totalPacket);
	printf("Number of data packets dropped due to loss: %d\n", dropPacket);
	printf("Number of data packets transmitted successfully: %d\n", transmittedPacket);
	printf("Number of ACKs recieved: %d\n", recievedACK);
	printf("Number of timeouts: %d\n", numTimeout);

	fclose(file);
}

int simulateLoss(float ratio){
	//gets a real number between 0 and 1
	float n = (float)rand()/(float)RAND_MAX;

	if(n < ratio){
		return 1;
	}
	return 0;
}


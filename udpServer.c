#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>

void sendText(int socket,unsigned char* textName, struct sockaddr_in client);

int main(){

	srand(time(NULL));
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
	
	sendText(serverSock, response +4, clientAddress);

	close(serverSock);

	return 0;
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

		unsigned short count = strlen(line_buffer);
		unsigned char *newLine = malloc(count + 5);
		newLine[0] = count;
		newLine[1] = count << 8;
		newLine[2] = seq;
		newLine[3] = seq << 8;
		strcat(newLine+4, line_buffer);
		
		while(ack != seq){
			sendto(socket, newLine, strlen(newLine+4) + 4, 0, (struct sockaddr*)&client, sizeof(client));
			printf("Packet %d gernerated for transmission with %d data bytes\n", seq, count);

			recvfrom(socket, ackMessage, sizeof(ackMessage), 0,(struct sockaddr*)&client, &clientSize);
			ack = ackMessage[0] + (ackMessage[1] >> 8);
		}

		seq = (seq + 1) % 2;
		totalCount += strlen(newLine+4);

		bzero(newLine, strlen(line_buffer) + 4);
		bzero(line_buffer, 80);
	}

	unsigned char *last = malloc(4);
	last[0] = 0;
	last[1] = 0;
	last[2] = seq;
	last[3]= seq << 8;
	sendto(socket, last, 4, 0, (struct sockaddr*)&client, sizeof(client));
	printf("End of Tranmission Packet with sequence number %d transmitted\n", seq);

	fclose(file);
}

int simulateLoss(float ratio){
	float n = (float)rand()/(float)RAND_MAX;

	if(n < ratio){
		return 1;
	}
	return 0;
}


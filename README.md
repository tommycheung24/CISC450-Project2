# CISC450-Project2

# Tommy Cheung
# Project 2 is done alone with no collborations. 
#Project 1 was done with Dontae Esdaile.

How to compile(use these lines to run):

	gcc udpClient.c -o c
	gcc udpServer.c -o s -lm
	./c
	./s

	Note: reason to use -lm is because the system doesn't know where to look for pow();


Explanation:

	udpServer.c
		first it ask for user input(timeout and packet loss ratio). Then initialize udp connection.
		Recieves the initial packet from client with the info of the file name. Then reads the file
		and sends the file back line by line. The function simulateloss() will decide if the line will be sent
		or not. Then goes on to use select() as the way to implement timeout. It will repeatedly send the file(with simulateloss()) and recieve file(select()) until the ack matches the sequence number that was sent. 
		Then it does it again for every single line that is in the file

	udpClient.c
		first it ask for user input(file name and ack packet loss ratio). Then initiaize udp connection. 
		Sends the initial packet to server with the info of the file name. Then waits for the server to send packets back to the client. After receiving a packet from client. Check for duplicate and if it's the EOT packet. Then stores it to the out.txt file. Calls simulateACKloss() to see if the ack should be send or not. Then repeats until the server send the ETO packet.

	Packet Layout:

		The first 4 chars is the header(4 bytes) then it will be the actually data.

# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <stdlib.h>
# include <errno.h>
# include <string.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <netinet/in.h>
# include <fcntl.h>


# define recVideoFile "received-video.mov"
# define BUFSIZE 500 // Restricting payload 

struct packet {
	int seqNum;
	char data[BUFSIZE];
	int size;
};

int main(int argc, char* argv[]) {
	// Two arguments should be provided
	if (argc < 2) {
		perror("Port Number not specified. \nTry ./serv.out 9898\n");
                return 0;

	}

	// Converting string to integer (atoi)
	int PORT = atoi(argv[1]);
	int _bind;
	int _socket;
	struct sockaddr_in address;
	socklen_t addr_length;
        addr_length = sizeof(struct sockaddr_in);

	int recvlen;
	int sendlen;
	int file;
	int fileSize;
	int remainingData;
        int receivedData;
        int length = 5;
	
	struct packet _packet;
	struct packet packets[5];
	int _acks;
	struct packet acks[5];


	// Socket Created
	_socket = socket(AF_INET, SOCK_DGRAM, 0);


	// Error Checking 
	if (_socket < 0) {
		perror("Server Socket could not be created/n");
		return 0;
	}

	// Adding Values to the attributes of the sockaddr_in struct

	memset((char*)& address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);

	// Binding the socket
	_bind = bind(_socket, (struct sockaddr*) & address, sizeof(address));

	// Error Checking
	if (_bind < 0) {
		perror("Could not bind\n");
		return 0;
	}

	// Creating new file to be written if it does not exist (O_CREAT)
	// It can be read from and written to (O_RDWR)
	// Set write rights for others to be true (S_IWOTH)
	file = open(recVideoFile, O_RDWR | O_CREAT, S_IWOTH);

	
	// Receiving the size of the video file
	recvlen = recvfrom(_socket, &fileSize, sizeof(off_t), 0, (struct sockaddr*) & address, &addr_length);
	fprintf(stdout, "Size of Video File to be received: %d bytes\n", fileSize);

	

	recvlen = 1;
	remainingData = fileSize;

	while (remainingData > 0) {

		// setting array of packets to zero
		memset(packets, 0, sizeof(packets));
                for (int i = 0; i < 5; i++){
                        packets[i].size = 0;
                }

		// Trying to receive 5 UDP segments
		for (int i = 0; i < length; i++) {
			recvlen = recvfrom(_socket, &_packet, sizeof(struct packet), 0, (struct sockaddr*) & address, &addr_length );
                         if (_packet.size == -999){ printf("last packet found\n");length = _packet.seqNum + 1; }

			// Successfully received packet 
			if (recvlen > 0) {

				// Keeping the right order of packets by index of the array
				packets[_packet.seqNum] = _packet;
                               
                               
			}
		}

		// Sending Acknowledgements for the packets received only
		_acks = 0;
		for (int i = 0; i < length; i++) {
                        

			// Creating acks for the packets received
			if (packets[i].size !=  0) {
				acks[i].size = -99;
				acks[i].seqNum = packets[i].seqNum;

				// Sending acks to the client
				sendlen = sendto(_socket, &acks[i], sizeof(acks[i]), 0, (struct sockaddr*) & address, addr_length);
				if (sendlen > 0) {
					_acks++;
					fprintf(stdout, "Ack sent: %d\n", acks[i].seqNum);
				}
			}
		}

		// Selective Repeat
		while (_acks < length) {
			recvlen = recvfrom(_socket, &_packet, sizeof(struct packet), 0, (struct sockaddr*) & address,&addr_length );
			if (recvlen > 0) {
				packets[_packet.seqNum] = _packet;
				_acks++;
			}
	
		}
		
                 WRITE:
		// write data into file
		for (int i = 0; i < length; i++) {
			if (packets[i].size != 0 && packets[i].size !=-999)
			{
				fprintf(stdout, "Writing packet: %d\n", packets[i].seqNum);
				write(file, packets[i].data, packets[i].size);
				receivedData = receivedData + packets[i].size;
				remainingData = remainingData - packets[i].size;
			}
		}


		fprintf(stdout, "Total received bytes: %d\nRemaining data: %d bytes\n", receivedData, remainingData);


	}


	/*
	// Receiving data
	for (;;) {
		printf("PORT NUMBER: %d\n", PORT);
		// returns number of bytes. (recvlen)
		// returns data in buf variable
		recvlen = recvfrom(_socket, buf, BUFSIZE, 0, (struct sockaddr*) & address, &addr_length);
		printf("Received %d bytes\n", recvlen);
		if (recvlen > 0) {
			buf[recvlen] = 0;
			printf("received message: %s\n", buf);
		}
		if ((char *)buf == "end") {
			close(_socket);
		}
	}
      */
       close(_socket);
       return 0;


}

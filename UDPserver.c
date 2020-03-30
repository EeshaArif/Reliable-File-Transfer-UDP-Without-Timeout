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
# include <pthread.h>
# include <fcntl.h>
# include <time.h>


# define recVideoFile "received-video.mov"
# define BUFSIZE 500 // Restricting payload 


struct packet {
	int seqNum;
	int size;
	char data[BUFSIZE];
};

// Socket Variables
int PORT;
int _bind;
int _socket;
struct sockaddr_in address;
socklen_t addr_length = sizeof(struct sockaddr_in);

// Video File Variables
int recvlen = 0;
int sendlen = 0;
int file;
int fileSize;
int remainingData = 0;
int receivedData = 0;

// Segment Variables
int length = 5; // Number of packets to be sent at a time
struct packet _packet;
struct packet packets[5];
int _acks;
struct packet acks[5];
int num;




// Thread to receive Packets

// This runs parallel within the main program
void* receiveSegments(void *vargp) {

	// Trying to receive 5 UDP segments
	for (int i = 0; i < length; i++) {
        RECEIVE:
		recvlen = recvfrom(_socket, &_packet, sizeof(struct packet), 0, (struct sockaddr*) & address, &addr_length);

		// If duplicate packet was sent

        if (packets[_packet.seqNum].size != 0 ){ 
            // Reallocating the array
            packets[_packet.seqNum] = _packet;
			// Create an acknowledgement 
			num = _packet.seqNum;
			acks[num].size = 1;
			acks[num].seqNum = packets[num].seqNum;
			// Send the ack to the client
			sendlen = sendto(_socket, &acks[num], sizeof(acks[num]), 0, (struct sockaddr*) & address, addr_length);
			fprintf(stdout,"Duplicate Ack Sent:%d\n",acks[num].seqNum);

			// Receive packet again until a unique packets is sent
			goto RECEIVE;
		}

		// If last packet was sent

		if (_packet.size == -999) {
			printf("last packet found\n");
			// Decrementing the counter of the remaining loops
			length = _packet.seqNum + 1;
		}

		// Successfully received a unique packet 
		if (recvlen > 0) {
			fprintf(stdout, "Packet Received:%d\n", _packet.seqNum);
			// Keeping the correct order of packets by index of the array
			packets[_packet.seqNum] = _packet;
		}
              
	}
	return NULL;
}





int main(int argc, char* argv[]) {
	// Two arguments should be provided
	// Specifying port number from command line
	if (argc < 2) {
		perror("Port Number not specified. \nTry ./serv.out 9898\n");
                return 0;

	}

	// Using the value of the port number received from the command line
	PORT = atoi(argv[1]); // Converting string to integer (atoi)

   	// TimeDelay variables
	struct timespec time1, time2;
	time1.tv_sec = 0;
	time1.tv_nsec = 30000000L;  // 0.03 seconds
    
	// Thread ID
	pthread_t thread_id;

	// Socket Created
	_socket = socket(AF_INET, SOCK_DGRAM, 0);


	// Verifying the creation of socket
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

	// Checking the bind
	if (_bind < 0) {
		perror("Could not bind\n");
		return 0;
	}

	// Creating new file if it does not exist (O_CREAT)
	// It can be read from and written to (O_RDWR)
	file = open(recVideoFile, O_RDWR | O_CREAT, 0755);

	
	// Receiving the size of the video file
	recvlen = recvfrom(_socket, &fileSize, sizeof(off_t), 0, (struct sockaddr*) & address, &addr_length);
	fprintf(stdout, "Size of Video File to be received: %d bytes\n", fileSize);

	

	recvlen = 1;
	remainingData = fileSize;

	while (remainingData > 0 || (length == 5)) {


		// Reinitializing the arrays after writing 5 UDP segments
		
		// Setting array of packets to zero
		memset(packets, 0, sizeof(packets));
        for (int i = 0; i < 5; i++){ packets[i].size = 0; }

		// Setting array of acks to zero
        memset(acks, 0, sizeof(packets));
        for (int i = 0; i < 5; i++){ acks[i].size = 0; }



               
        // The server starts receiving packets ( Thread execution starts )
		pthread_create(&thread_id, NULL, receiveSegments, NULL);

        // Waiting for packets to be received ( The code sleeps for 0.03 seconds )
        nanosleep(&time1, &time2);

		_acks = 0;

		// Sending Acknowledgements for the packets received only

		RESEND_ACK:
		for (int i = 0; i < length; i++) {
			num = packets[i].seqNum;
			// If the ack has not been sent before
			if (acks[num].size != 1 ) {
				// Creating acks for the packets received ONLY
				if (packets[i].size != 0) {
					// Setting condition for an ack to be checked by the client
					acks[num].size = 1;
					acks[num].seqNum = packets[i].seqNum;

					// Sending acks to the client
					sendlen = sendto(_socket, &acks[num], sizeof(acks[num]), 0, (struct sockaddr*) & address, addr_length);
					if (sendlen > 0) {
						_acks++;
						fprintf(stdout, "Ack sent: %d\n", acks[packets[num].seqNum].seqNum);
					}
				}

			}
			
		}

		// Stop n Wait
		// Waiting for acks to be sent and processed by the client
		nanosleep(&time1, &time2);
		nanosleep(&time1, &time2);

		// If all packets were not received
		if (_acks < length) {
			goto RESEND_ACK;
		}
                
		// 5 packets have been received ( The thread executes successfully )
		pthread_join(thread_id, NULL);
                 
		// Write data (packets) into file
		for (int i = 0; i < length; i++) {
			// Data is present in the packets and its not the last packet
			if (packets[i].size != 0 && packets[i].size !=-999)
			{
				fprintf(stdout, "Writing packet: %d\n", packets[i].seqNum);
				write(file, packets[i].data, packets[i].size);
				remainingData = remainingData - packets[i].size;
				receivedData = receivedData + packets[i].size;
				
			}
		}


		fprintf(stdout, "Received data: %d bytes\nRemaining data: %d bytes\n", receivedData, remainingData);

		// Reinitiate the process for the next 5 packets
	}

	// After all the packets have been received
	fprintf(stdout, "\n\nFile Received Successfully.\nThe copied file is named \"received-video\"\nPlease check your ./ repository.\n\n");

	// Close the UDP (server) socket
    close(_socket);
    return 0;


}

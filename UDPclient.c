# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <stdlib.h>
# include <errno.h>
# include <string.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <netinet/in.h>
# include <sys/stat.h>
# include <sys/sendfile.h>
# include <fcntl.h>
# include <time.h>

# define ADDRESS "127.0.0.1"
# define videoFile "video.mp4"
# define BUFSIZE 500 // Restricting payload 

struct packet {
	int seqNum;
	char data[BUFSIZE];
	int size;
};

int main(int argc, char* argv[]) {

	// Two arguments should be provided
	if (argc < 2) {
		// Port Number needs to be specified on the command line
		perror("Port Number not specified.\nTry ./cli.out 9898\n");
                return 0;

	}

	// Socket Variables
	int PORT = atoi(argv[1]); // Converting string to integer (atoi)
	int _socket;
	struct sockaddr_in address;
	socklen_t addr_length = sizeof(struct sockaddr_in);

	// TimeDelay variables
	struct timespec time1, time2;
	time1.tv_sec = 0;
	time1.tv_nsec = 5000000L;

	// File Variables
	int sendlen;
	int len;
	int recvlen;
	int file;
	struct stat fileStat;
	off_t fileSize;

	// Segment Variables
	struct packet packets[5]; // Restricting to 5 UDP segments 
	int _seqNum = 1;
	int _acks;
	struct packet ack;
	struct packet acks[5];
	int length = 5;



	// Socket Created
	_socket = socket(AF_INET, SOCK_DGRAM, 0);


	// Checking created socket 
	if (_socket < 0) {
		perror("Client Socket could not be created/n");
		return 0;
	}

	// Adding Values to the attributes of the sockaddr_in struct
	memset((char*)& address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(PORT);
	inet_pton(AF_INET, ADDRESS, &address.sin_addr);


	// Opening the video File as read-only
	file = open(videoFile, O_RDONLY);

	// Verifying the file is readable
	if (file < 0) {
		perror("There was a problem reading the video file.\n");
		return 0;
	}

	// Calculating the size of the video file
	fstat(file, &fileStat);
	fileSize = fileStat.st_size;
	fprintf(stdout, "Size of Video File: %d bytes\n",(int) fileSize);
	

	// Sending the size of the file to the server
	FILESIZE:
	sendlen = sendto(_socket, &fileSize, sizeof(off_t), 0, (struct sockaddr*) & address, addr_length);
	// Resend the file size to the server
	if (sendlen < 0) {
		goto FILESIZE;
	}


	len = 1;
	// while there is still data to be read in the file
	while (len > 0) {


		// Putting file data into packets
		_seqNum = 0;
		for (int i = 0; i < length; i++) {
            // Reading data 
			len = read(file, packets[i].data, BUFSIZE);
            // Allocating sequence numbers
			packets[i].seqNum = _seqNum;
            // Specifying Size
			packets[i].size = len;
			_seqNum++;
			// The last packet to be sent
            if (len == 0){ 
                      printf("End of file reached.\n");
                      // Setting a condition for last packet
                      packets[i].size = -999; 
                      // Decrementing the remaining loops 
                      length = i + 1; 
                      break; 
            }



		}

		// Sending Packets 
		for (int i = 0; i < length; i++) {
			fprintf(stdout, "Sending packet %d\n", packets[i].seqNum);
			sendlen = sendto(_socket, &packets[i], sizeof(struct packet), 0, (struct sockaddr*) & address, addr_length);
		}


		// Setting array of Acks to zero 
        // Reinitializing the array
		memset(acks, 0, sizeof(acks));
        for (int i = 0; i < length; i++){ acks[i].size = 0;}



		_acks = 0;
		// receiving Acknowledgments
		for (int i = 0; i < length; i++) {
			recvlen = recvfrom(_socket, &ack, sizeof(struct packet), 0, (struct sockaddr*) & address, &addr_length);
            // Checking the specified condition 
			if (ack.size == 1) {
				fprintf(stdout, "Ack Received: %d\n", ack.seqNum);
				// Reorder acknowlegements according to their packet's sequence number
				acks[ack.seqNum] = ack;
				_acks++;
			}
                     
		}

                // Stop and Wait after sending 5 packets
		// code is sleeping for 5000000ns
		nanosleep(&time1, &time2);

		// Selective Repeat 
		// Sending those packets whose acks have not been received
		RESEND:
		for (int i = 0; i < length; i++) {
			// checking which acks have not been received
			if (acks[i].size == 0) {
				// Sending the missing packets
                fprintf(stdout,"sending missing packet: %d",packets[i].seqNum);
				sendlen = sendto(_socket, &packets[i], sizeof(struct packet), 0, (struct sockaddr*) & address, addr_length);
				// Receiving acknowlegements again
				if (recvfrom(_socket, &ack, sizeof(struct packet), 0, (struct sockaddr*) & address, &addr_length)) {
					fprintf(stdout, "Ack Received: %d\n", ack.seqNum);
					acks[ack.seqNum] = ack;
					_acks++;

				}
			}
		}

		// Resend the packets again whose acknowlegements have not been received
		if (_acks != length) {
			fprintf(stdout, "Resending\n");
			goto RESEND;
		     
		}

	}

	fprintf(stdout,"\n\nFile sent successfully!\n\n");
	close(_socket);

	return 0;
}

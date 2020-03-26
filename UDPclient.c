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
# define videoFile "earth.mov"
# define BUFSIZE 500 // Restricting payload 

struct packet {
	int seqNum;
	char data[BUFSIZE];
	int size;
};

int main(int argc, char* argv[]) {

	// Two arguments should be provided
	if (argc < 2) {
		perror("Port Number not specified.\nTry ./cli.out 9898\n");
                return 0;

	}

	// Converting string to integer (atoi)
	int PORT = atoi(argv[1]);
	int _socket;
	struct sockaddr_in address;
	socklen_t addr_length = sizeof(struct sockaddr_in);

	struct timespec time1, time2;
	time1.tv_sec = 0;
	time1.tv_nsec = 500000000L;

	int sendlen;
	int len;
	int recvlen;
	int file;
        int length = 5;
	struct stat fileStat;
	off_t fileSize;

	struct packet packets[5]; // Restricting to 5 UDP segments 
	int _seqNum = 1;
	int _acks;
	int totalPackets = 0;
	struct packet ack;
	struct packet acks[5];
        char end[BUFSIZE] = "END OF FILE";



	// Socket Created
	_socket = socket(AF_INET, SOCK_DGRAM, 0);


	// Error Checking 
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

	// Checking for errors

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

	while (len > 0) {


		// Putting file data into packets
		_seqNum = 0;
		for (int i = 0; i < length; i++) {

			len = read(file, packets[i].data, BUFSIZE);
			packets[i].seqNum = _seqNum;
			packets[i].size = len;
			_seqNum++;
			// The last packet to be sent
                        if (len == 0){ printf("End of file reached.\n");packets[i].size = -999; length = i + 1; break; }



		}

		// Sending Packets 
		for (int i = 0; i < length; i++) {
			fprintf(stdout, "Sending packet %d\n", packets[i].seqNum);
			sendlen = sendto(_socket, &packets[i], sizeof(struct packet), 0, (struct sockaddr*) & address, addr_length);
		}


		// Setting array of Acks to zero 
		memset(acks, 0, sizeof(acks));
                for (int i = 0; i < length; i++){ acks[i].size = 0;}

		// stop and wait after sending 5 packets
		// code is sleeping for 1s + 500ns
		//nanosleep(&time1, &time2);
		_acks = 0;
		// receiving Acknowledgments
		for (int i = 0; i < length; i++) {
			recvlen = recvfrom(_socket, &ack, sizeof(struct packet), 0, (struct sockaddr*) & address, &addr_length);
			if (ack.size == -99) {
				fprintf(stdout, "Ack Received: %d\n", ack.seqNum);
				acks[ack.seqNum] = ack;
				_acks++;
			}
                     
		}

		// selective Repeat 
		// sending those packets whose acks have not been received
		for (int i = 0; i < length; i++) {
			if (acks[i].size == 0) {
				sendlen = sendto(_socket, &packets[i], sizeof(struct packet), 0, (struct sockaddr*) & address, addr_length);
			}
		}




		



	}


	
















	close(_socket);

	return 0;
}

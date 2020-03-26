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

# define PORT 15000
# define ADDRESS "127.0.0.1"
# define videoFile "earth.mov"
# define BUFSIZE 500 // Restricting payload 

struct packet {
	int seqNum;
	char data[BUFSIZE];
	int size;
};

int main(int argc, char** argv) {
	int _socket;
	struct sockaddr_in address;
	socklen_t addr_length = sizeof(address);


	int sendlen;
	int file;
	struct stat fileStat;
	off_t fileSize;




	//char* message = "taddaaaa, it worked";


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
	fileSize = file_stat.st_size;
	fprintf(stdout, "Size of Video File: %d bytes\n", fileSize);
	

	// Sending the size of the file to the server
	FILESIZE:
	sendlen = sendto(_socket, &fileSize, sizeof(off_t), 0, (struct sockaddr*) & address, addr_length);
	// Resend the file size to the server
	if (sendlen < 0) {
		goto FILESIZE;
	}
















	return 0;
}

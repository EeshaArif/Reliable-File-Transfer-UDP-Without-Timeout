# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <stdlib.h>
# include <errno.h>
# include <string.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <netinet/in.h>

# define PORT 15000
# define recVideoFile "received-video.mov"
# define BUFSIZE 500 // Restricting payload 

struct packet {
	int seqNum;
	char data[BUFSIZE];
	int size;
};

int main(int argc, char** argv) {
	int _bind;
	int _socket;
	struct sockaddr_in address;
	socklen_t addr_length = sizeof(address);

	int recvlen;

	unsigned char buf[BUFSIZE]; // receive buffer

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

	// binding the socket
	_bind = bind(_socket, (struct sockaddr*) & address, sizeof(address));

	// Error Checking
	if (_bind < 0) {
		perror("Could not bind\n");
		return 0;
	}

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
      
       return 0;


}
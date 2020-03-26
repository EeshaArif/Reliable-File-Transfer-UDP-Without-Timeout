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
# define ADDRESS "127.0.0.1"
# define VideoFile "earth.mov"
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

	char* message = "taddaaaa, it worked";

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

	// Sending message

	sendlen = sendto(_socket, message, strlen(message), 0, (struct sockaddr*) &address, sizeof(address));

	// Checking for errors
	if (sendlen < 0) {
		perror("Failed to send message");
		return 0;
	}

	return 0;
 




}

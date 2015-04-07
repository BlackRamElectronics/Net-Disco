#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <iostream>

#include <stdlib.h>
#include <unistd.h>

#include "tinyxml2.h"

#define TCP_RESPONSE_PORT 2413

#define MULTICAST_PORT 12344
#define MULTICAST_GROUP "225.0.0.36"

using namespace tinyxml2;
using namespace std;

void GetResponse(void);
void ProcessResponse(int sock, struct sockaddr_in cli_addr);

//============================================================================
void error(const char *msg)
{
	perror(msg);
	exit(1);
}

//============================================================================
int main(int argc, char *argv[])
{
	struct sockaddr_in addr;
	int fd, cnt;
	struct ip_mreq mreq;
	char *message="Net Disco Broadcast";

	cout << "=========================================================" << endl;
	cout << "=                     NET DISCO                         =" << endl;
	cout << "=========================================================" << endl << endl;

	// Create a socket
	if((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		error("Unable to open transmit socket");
	}

	// Set up destination address
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
	addr.sin_port = htons(MULTICAST_PORT);
     
	// Send multicast packet to all listening devices
	if(sendto(fd, message, sizeof(message), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		error("Failed to send multicast packet\r\n");
	}

	// Get the responses from the drvices	
	GetResponse();

	return(0);
}

//============================================================================
void ProcessResponse(int sock, struct sockaddr_in cli_addr)
{
	int n;
	char receive_buffer[4096];
	char output_buffer[512];
	char *conn_addr;
	std::string output_string;

	output_string = "---------------------------------------------------------\r\n";

	bzero(receive_buffer,4096);

	n = read(sock,receive_buffer,4096);
	if(n < 0)
	{
		error("ERROR reading from scoket");
	}
	// Get the IP address from the received packet
	conn_addr = inet_ntoa(cli_addr.sin_addr);

	// Create a XML object
	XMLDocument doc;
	doc.Parse(receive_buffer);	// Pass the recived data to be parsed

	// Get the hostname
	const char* host = doc.FirstChildElement("SYSTEM")->FirstChildElement("HOSTNAME")->GetText();
	sprintf(output_buffer, "\tHostname: \t%s\r\n", host);
	output_string += output_buffer;

	// Get the MAC address
	const char* mac = doc.FirstChildElement("SYSTEM")->FirstChildElement("MAC")->GetText();
	sprintf(output_buffer, "\tMAC Address: \t%s\r\n", mac);
	output_string += output_buffer;

	// Get the IP address
	const char* ip = doc.FirstChildElement("SYSTEM")->FirstChildElement("IP")->GetText();
	sprintf(output_buffer, "\tIP Address: \t%s\r\n", conn_addr);
	output_string += output_buffer;

	output_string += "---------------------------------------------------------\r\n";
	output_string += "\r\n";	// Added here to avoid concurrency problems
	cout << output_string;
}

//============================================================================
void GetResponse(void)
{
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	// Fisrt call to socket() function
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		error("ERROR failed to open listen socket");
	}

	// Init socket structure
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = TCP_RESPONSE_PORT;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	// Now bind to the address using bind() call
	if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		error("ERROR on binding");
	}

	// Now start listening for clients, here process will go into sleep mode
	// and will wait for the incoming connection
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	while(1)
	{
		newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
		if(newsockfd < 0)
		{
			error("ERROR on accept");
		}

		// Create child process
		pid_t pid = fork();
		if(pid < 0)
		{
			error("ERROR on fork");
		}

		if(pid == 0)
		{
			// This is the client process
			close(sockfd);
			ProcessResponse(newsockfd, cli_addr);
			exit(0);
		}
		else
		{
			close(newsockfd);
		}
	}
}


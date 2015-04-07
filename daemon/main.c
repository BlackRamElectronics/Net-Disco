#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <string>

#include <stdlib.h>
#include <unistd.h>
#include <netdb.h> 

#define TCP_RESPONSE_PORT 2413

#define MULTICAST_PORT 12344
#define MULTICAST_GROUP "225.0.0.36"
#define MSGBUFSIZE 256

void SendResponse(struct sockaddr_in addr);

//====================================================================
void error(const char *msg)
{
	perror(msg);
	exit(1);
}

//====================================================================
int main(int argc, char *argv[])
{
	struct sockaddr_in addr;
	int fd, nbytes,addrlen;
	struct ip_mreq mreq;
	char msgbuf[MSGBUFSIZE];
	char *conn_addr;

	//int i;

	u_int yes=1;

	// Create what looks like an ordinary UDP socket
	if((fd=socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		perror("socket");
		exit(1);
	}

	// Allow multiple sockets to use the same PORT number
	if(setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0)
	{
		perror("Reusing ADDR failed");
		exit(1);
	}

	// Set up destination address
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY); // N.B.: differs from sender
	addr.sin_port=htons(MULTICAST_PORT);
     
	// Bind to receive address
	if(bind(fd,(struct sockaddr *) &addr,sizeof(addr)) < 0)
	{
		perror("bind");
		exit(1);
	}
     
	// Use setsockopt() to request that the kernel join a multicast group
	mreq.imr_multiaddr.s_addr=inet_addr(MULTICAST_GROUP);
	mreq.imr_interface.s_addr=htonl(INADDR_ANY);
	if(setsockopt(fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,sizeof(mreq)) < 0)
	{
		perror("setsockopt");
		exit(1);
	}

	printf("Waiting for packets!\r\n");

	// Now just enter a read-print loop
	while(1)
	{
		addrlen=sizeof(addr);
		if((nbytes = recvfrom(fd, msgbuf,MSGBUFSIZE, 0, (struct sockaddr *)&addr, (socklen_t *)&addrlen)) < 0)
		{
			perror("recvfrom");
			exit(1);
		}
		
		conn_addr = inet_ntoa(addr.sin_addr);
		printf("Multicast from IP address: %s\r\n",conn_addr);

		SendResponse(addr);
	}

	return(0);
}

//====================================================================
void SendResponse(struct sockaddr_in addr)
{
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[256];
	char res_buffer[256];
	char host_name_buff[256];
	std::string xml_res;
	
	// Create the socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{ 
		error("ERROR opening socket");
	}

	// === Build the XML response ===
	xml_res = "<?xml version=\"1.0\"?>";	// Add the XML header

	// Open system tag
	sprintf(res_buffer, "<SYSTEM>");
	xml_res += res_buffer;

	// Add host name
	gethostname(host_name_buff, sizeof(host_name_buff));
	sprintf(res_buffer, "<HOSTNAME>%s</HOSTNAME>", host_name_buff);
	xml_res += res_buffer;

//#################################################################
	struct ifaddrs *ifaddr, *ifa;
	int family, s;
	char ip_address[NI_MAXHOST];
	char macaddrstr[18];

	if (getifaddrs(&ifaddr) == -1)
	{
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}

	// Walk through linked list, maintaining head pointer so we can free list later

	for(ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++)
	{
		if(ifa->ifa_addr == NULL)
		{
			continue;
		}

		family = ifa->ifa_addr->sa_family;

		// For an AF_INET* interface address, display the address
		if(strncmp("eth0", ifa->ifa_name, 4) == 0)
		{
			if(family == AF_INET || family == AF_INET6)
			{
				s = getnameinfo(ifa->ifa_addr,
					(family == AF_INET) ? sizeof(struct sockaddr_in) :
						sizeof(struct sockaddr_in6),
						ip_address, NI_MAXHOST,
						NULL, 0, NI_NUMERICHOST);
				if (s != 0)
				{
					printf("getnameinfo() failed: %s\n", gai_strerror(s));
					exit(EXIT_FAILURE);
				}

				struct ifreq req;
				strcpy(req.ifr_name, ifa->ifa_name);
				if(ioctl(sockfd, SIOCGIFHWADDR, &req) != -1)
				{
					unsigned char *ptr = (unsigned char*)req.ifr_ifru.ifru_hwaddr.sa_data;
					sprintf(macaddrstr, "%02x:%02x:%02x:%02x:%02x:%02x", *ptr, *(ptr+1), *(ptr+2), *(ptr+3), *(ptr+4), *(ptr+5));
				}
			}
		}
	}

	freeifaddrs(ifaddr);
//#################################################################



	// Add IP address
	sprintf(res_buffer, "<IP>%s</IP>", ip_address);
	xml_res += res_buffer;

	// Add mac address
	sprintf(res_buffer, "<MAC>%s</MAC>", macaddrstr);
	xml_res += res_buffer;

	// Open system tag
	sprintf(res_buffer, "</SYSTEM>");
	xml_res += res_buffer;

	// === End XML build ===

	// Get the port number
	portno = TCP_RESPONSE_PORT;
	
	server = gethostbyname(inet_ntoa(addr.sin_addr));
	if(server == NULL)
	{
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	
	serv_addr.sin_port = htons(portno);
	
	if(connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	{
		error("ERROR connecting");
	}
	
	n = write(sockfd, xml_res.c_str(),strlen(xml_res.c_str()));
	if(n < 0)
	{ 
		error("ERROR writing to socket");
	}

	bzero(buffer,256);

	printf("Response sent!\r\n");

	close(sockfd);
}


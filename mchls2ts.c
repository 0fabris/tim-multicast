#include <stdio.h>	
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/socket.h>	 
#include <errno.h>		
#include <netinet/in.h>	
#include <arpa/inet.h>
#include <unistd.h>	
#include <sys/utsname.h> 
#include <sys/stat.h> 
#include <string.h>

#define MAXLEN 2048
#define PATTERN_8021 0x8021
#define PATTERN_80A1 0x80a1
#define PACKET_8021_HEADER_LENG 20
#define PACKET_80A1_HEADER_LENG 16
#define MULTICAST_HEADER 0x01005e48

char filename[301] = "";

void printBytes(uint8_t *buff, int n)
{
    for (int c = 0; c < n; c++)
    {
        printf("%02x ", *(buff + c));
    }
    printf("\n");
}

void printRawBytes(uint8_t *buff, int n)
{
    for (int c = 0; c < n; c++)
    {
        printf("%c", *(buff + c));
    }
    //printf("\n");
}

int compareBytes(uint8_t *buff, int nb, uint32_t bytes)
{
    //max 4 bytes
    int cmp = 1;

    for (int x = 0; cmp && x < nb; ++x)
    {
        cmp &= ((bytes >> (8 * x) & 0xFF) == *(buff + nb - x - 1));
    }
    return cmp;
}

void demodPacket(char* data, int nrd){
	int shift = 0;
	if(nrd >= 16){
		if(compareBytes(data,2,PATTERN_8021)){
			shift = PACKET_8021_HEADER_LENG;
		}
		else if (compareBytes(data,2,PATTERN_80A1)){
			shift = PACKET_80A1_HEADER_LENG;
			
			if(
				   (compareBytes(data+13,1,0x01) && compareBytes(data+15,1,0x01)) 
				|| (compareBytes(data+13,1,0x01) && compareBytes(data+15,1,0x00))
				|| (compareBytes(data+13,1,0x01) && compareBytes(data+15,1,0x02))
				|| (compareBytes(data+13,1,0x02))
			)
				memcpy(&filename, data+shift,301);
			return;
		}
		nrd -= shift;
		memmove(data, data+shift, nrd);
	}
	printRawBytes(data,nrd);
}

int main(int argc, char *argv[])
{
	u_char no = 0;
	u_int yes = 1;		
	int send_s, recv_s;
	u_char ttl;
	struct sockaddr_in mcast_group;
	struct ip_mreq mreq;
	struct utsname name;
	int n;
	int len;
	struct sockaddr_in from;
	char message[MAXLEN + 1];

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s mcast_group port\n", argv[0]);
		exit(1);
	}

	memset(&mcast_group, 0, sizeof(mcast_group));
	mcast_group.sin_family = AF_INET;
	mcast_group.sin_port = htons((unsigned short int)strtol(argv[2], NULL, 0));
	mcast_group.sin_addr.s_addr = inet_addr(argv[1]);

	if ((recv_s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("recv socket");
		exit(1);
	}

	if (setsockopt(recv_s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
	{
		perror("reuseaddr setsockopt");
		exit(1);
	}

	if (bind(recv_s, (struct sockaddr *)&mcast_group, sizeof(mcast_group)) < 0)
	{
		perror("bind");
		exit(1);
	}

	
	mreq.imr_multiaddr = mcast_group.sin_addr;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(recv_s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
	{
		perror("add_membership setsockopt");
		exit(1);
	}

	for (;;)
	{
		len = sizeof(from);
		if ((n = recvfrom(recv_s, message, MAXLEN, 0,
						  (struct sockaddr *)&from, &len)) < 0)
		{
			perror("recv");
			exit(1);
		}
		
		demodPacket(message, n);
	}
}

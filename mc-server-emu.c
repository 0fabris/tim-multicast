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
#include <dirent.h>

#define MAXLEN 2048
#define PATTERN_8021 0x8021
#define PATTERN_80A1 0x80a1
#define PACKET_8021_HEADER_LENG 20
#define PACKET_80A1_HEADER_LENG 16
#define MULTICAST_HEADER 0x01005e48

#define NAME_MAXLEN 300

char filename[NAME_MAXLEN + 1] = "";
int filecounter = 0;

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
        cmp &= ((bytes >> (8 * x) & 0xFF) == *(buff + nb - x - 1));
    return cmp;
}

struct dirent *de;
DIR *dp;
int nfile = 100; // + 5 file di tmp
int reverseListing = 1;

void fileRemover(char* path){
	
	dp = opendir(path);
	if(dp == NULL)
		return;
	
	if(filecounter > nfile){
		//remove files
		//rewinddir(dp);

		if(reverseListing)
			seekdir(dp, nfile);
		
		while ((de = readdir(dp)) != NULL && filecounter > nfile){
			if((strstr(de->d_name,".m4") || strstr(de->d_name,".ts")) && /*not a dazn manifest*/1){
				char *command = malloc(NAME_MAXLEN + 100 + 1);
				sprintf(command, "rm \"%s/%s\"", path, de->d_name);
				printf("command: %s\n",command);
				system(command);
				free(command);
				--filecounter;
			}
		}
	}
	closedir(dp);
}

char* buff;
int curPos,lastPos;
int checkPath(char* fname){
	buff = malloc(NAME_MAXLEN + 1);
	curPos = lastPos = 0;
	int pathLimit = -1;
	for(curPos = lastPos = 0; curPos < NAME_MAXLEN && fname[curPos] != '\0'; ++curPos){
		buff[curPos] = '\0';
		if(fname[curPos] == '/')
		{
			memcpy(buff, fname,curPos);
			pathLimit = curPos-1;
			*(buff + lastPos-curPos) = '\0';
			
			lastPos = curPos+1;
			if (access( buff, F_OK ) != 0){
				mkdir(buff,S_IRWXU);
			}
		}
	}
	fileRemover(buff);
	free(buff);
	
	return pathLimit;
}

int pathLimit;
FILE* fp;
void writeToFile(char* fname, char* content, int len, int start_pos){
	pathLimit = checkPath(fname);
	if(start_pos == 0){ 
		fp = fopen(fname,"w+");
	}
	else
	{
		fp = fopen(fname,"r+");
		fseek(fp, start_pos, SEEK_SET);
	}
	fwrite(content, 1, len, fp);
	fclose(fp);
}

uint32_t Reverse32(uint32_t value) 
{
    return (((value & 0x000000FF) << 24) |
            ((value & 0x0000FF00) <<  8) |
            ((value & 0x00FF0000) >>  8) |
            ((value & 0xFF000000) >> 24));
}

uint32_t start_addr;
int shift = 0;
void demodPacket(char* data, int nrd){	
	shift = 0;
	if(nrd >= 16){
		if(compareBytes(data,2,PATTERN_8021)){
			shift = PACKET_8021_HEADER_LENG;
			//printBytes(data,shift);
			if(compareBytes(data+13,1,0x05))
				return;
			else if(compareBytes(data+13,1,0x03) && filename[0] != '\0'){
				//Segment part
				//Reading start address
				start_addr = 0x00000000;
				memcpy(&start_addr, data+16, 4);
				start_addr = Reverse32(start_addr);
				
				writeToFile(filename,data+shift,nrd-shift, start_addr);
			}
		}
		else if (compareBytes(data,2,PATTERN_80A1)){
			shift = PACKET_80A1_HEADER_LENG;
			//printBytes(data,shift);
			if(compareBytes(data+13,1,0x01) && compareBytes(data+15,1,0x01)){
				//Refresh index.m3u8 / manifest
				memcpy(&filename, data+shift,NAME_MAXLEN + 1);
				printf("filename 01 01 %s\n",filename);
			}else if((compareBytes(data+13,1,0x01))){
				//First part of segment
				memcpy(&filename, data+shift,NAME_MAXLEN + 1);
				filecounter++;
				printf("filename 01 0x %s\n",filename);
			}
			else if(compareBytes(data+13,1,0x02)){
				//Refresh seg name == following parts of segment
				//ignore
				if(filename[0] != '\0')
					memcpy(&filename, data+shift,NAME_MAXLEN + 1);
				//printf("filename 02 xx %s\n",filename);
			}
			return;
		}
		nrd -= shift;
		memmove(data, data+shift, nrd);
	}
	//printRawBytes(data,nrd);
}

int createSocket(char* mcasthost, char* port){
	u_char no = 0;
	u_int yes = 1;	 
	int send_s, recv_s; 
	u_char ttl;
	struct sockaddr_in mcast_group;
	struct ip_mreq mreq;
	struct utsname name;

	memset(&mcast_group, 0, sizeof(mcast_group));
	mcast_group.sin_family = AF_INET;
	mcast_group.sin_port = htons((unsigned short int)strtol(port, NULL, 0));
	mcast_group.sin_addr.s_addr = inet_addr(mcasthost);

	if ((recv_s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("recv socket");
		return -1;
	}

	if (setsockopt(recv_s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
	{
		perror("reuseaddr setsockopt");
		return -1;
	}

	if (bind(recv_s, (struct sockaddr *)&mcast_group, sizeof(mcast_group)) < 0)
	{
		perror("bind");
		return -1;
	}

	mreq.imr_multiaddr = mcast_group.sin_addr;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	if (setsockopt(recv_s, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
	{
		perror("add_membership setsockopt");
		return -1;
	}

	return recv_s;
}

int main(int argc, char *argv[])
{
	int socket_1, socket_2;
	struct sockaddr_in from;
	int n;
	int len;
	char message[MAXLEN + 1];

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s mcast_group port\n", argv[0]);
		exit(1);
	}

	socket_1 = createSocket(argv[1], argv[2]);

	socket_2 = 0; //predisposizione per 2 flussi (audio + video per DASH) da implementare

	printf("waiting for packets...\n");

	for (;socket_1 > -1 && socket_2 > -1;)
	{
		len = sizeof(from);
		if ((n = recvfrom(socket_1, message, MAXLEN, 0,
						  (struct sockaddr *)&from, &len)) < 0)
		{
			perror("recv");
			exit(1);
		}
		demodPacket(message, n);
	}
}

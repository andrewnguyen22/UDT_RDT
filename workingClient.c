//*****************************************************************************
//
//
//
//*****************************************************************************
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>

#define BUFFER 256
// #define IP_ADDRESS "172.16.1.165"
#define IP_ADDRESS "192.168.0.15"
#define FILENAME "test_send.txt"
#define PORT_NUM 1050


struct sockaddr_in server_addr;//CHANGE

int main (int argc, char**argv)
{
	int retcode;
	int client_s;//Client socket descriptor
	int fh;//File handle
	char out_buf[BUFFER];
	long long count = 0, m,buff_size;//long
	long int n;
	int l = sizeof(struct sockaddr_in);
	struct stat buffer;

	//Server variables
	int server_s;
	char in_buf[3];
	long long count2=0, n2; // long type
  unsigned int l2=sizeof(struct sockaddr_in);
	struct sockaddr_in sock_serv, client;//CHANGE

	server_s = socket(AF_INET,SOCK_DGRAM,0);
	if (server_s < 0)
	{
		printf("*** ERROR - socket() failed \n");
		exit(-1);

	}
	l2 = sizeof(struct sockaddr_in);
	bzero(&sock_serv,l2);

	sock_serv.sin_family=AF_INET;
	sock_serv.sin_port=htons(PORT_NUM);
	sock_serv.sin_addr.s_addr=htonl(INADDR_ANY);

	struct timeval tv;		//timeout struct
	tv.tv_sec = 3;        // 3 Secs Timeout
	tv.tv_usec = 0;       // Not init'ing this can cause strange errors
	setsockopt(server_s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

	if(bind(server_s,(struct sockaddr*)&sock_serv,l2)==-1){
		printf("*** ERROR - bind() failed \n");
    exit(-1);
	}

//////////////////////////////////////////////////////////////////////////////
	//char * ipAddr = IP_ADDRESS;
	//int port = atoi(argv[2]);

	client_s = socket(AF_INET,SOCK_DGRAM,0);
	if (client_s < 0)
  {
    printf("*** ERROR - socket() failed \n");
    exit(-1);
  }

	//preparation de l'adresse de la socket destination
	l = sizeof(struct sockaddr_in);
	bzero(&server_addr,l);

	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(PORT_NUM);
	if (inet_pton(AF_INET,IP_ADDRESS,&server_addr.sin_addr)==0){
	 	printf("Invalid IP adress\n");
	 	return EXIT_FAILURE;
	}
	////////////////////////////////////////////////////////////////////////////

	//File open.
	fh = open(FILENAME,O_RDONLY);
	if (fh < 0){
		printf("File open fail");
		exit(-1);
	}
	//File size.
	if (stat(FILENAME,&buffer)==-1){
		perror("stat fail");
		exit(-1);
	}
	else//////////////////////////////////////////////////////////////////
	{
		buff_size = buffer.st_size;
	}

	//CLIENT CODE BELOW
	bzero(&out_buf,BUFFER);
	int resend=0;
	n = read(fh,out_buf,BUFFER-1);
	out_buf[n-1]='0';
	while(n)
	{
		if(n == -1)//reading initial file to send (to server)
		{
			printf("read fails");
			return EXIT_FAILURE;
		}
		m = sendto(client_s,out_buf,n,0,(struct sockaddr*)&server_addr,l);
		if(m == -1)
		{
			printf("send error");//needed to troubleshoot sending (to server)
			return EXIT_FAILURE;
		}
		printf("Transfered %lld bytes\n",m);//bytes received
		//CLIENT END


		//BELOW IS SERVER CODE
		//halts code and waits for ack for 3 seconds
		bzero(&in_buf, 3);
	  n2 = recvfrom(server_s,&in_buf,3,0,(struct sockaddr *)&client,&l2);
		while(n2)
		{
			if(n2 < 0){
				printf("Ack not received... Resending packets!");//from server
				break;
			}
			else{
				printf("Message from server: %s \n",in_buf);//from server
			}
			bzero(in_buf,3);
	    n2 = recvfrom(server_s,&in_buf,3,0,(struct sockaddr *)&client,&l2);
		}
		if(n2 < 0){
			continue;//skips the code below and resends the packet
		}
		//END OF SERVER
		//CLIENT BELOW
		count += m;
		bzero(out_buf,BUFFER);
		n = read(fh,out_buf,BUFFER-1);
		resend++;
		resend=resend%2;
		out_buf[n]=resend + '0';//converts to a character
	}

	m = sendto(client_s,out_buf,0,0,(struct sockaddr*)&server_addr,l);

	printf("Transfer total: %lld\n",count);
	printf("Total File size: %lld \n",buff_size);

//Close the file and socket.
	retcode = close(client_s);
	if (retcode < 0)
	{
		printf("*** ERROR - close() failed \n");
		exit(-1);
	}
	close(fh);
	return (0);
}

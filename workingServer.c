//source: https://github.com/amineamanzou/UDP-TCP-File-Transfer

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
// Buffer size
#define BUFFER_SIZE 256
#define PORT_NUM 1050
#define FILENAME "test.txt"
struct sockaddr_in sock_serv;

int main (){
	//server variables ************
	int file, server_s;
	char in_buf[BUFFER_SIZE];
	long long count=0, n; // long type
  unsigned int l=sizeof(struct sockaddr_in);

	//client variables ********
  struct sockaddr_in   server_addr;     // Server Internet address
  struct sockaddr_in   client;     			// Client Internet address
  struct in_addr       client_ip_addr;  // Client IP address
  int                  addr_len;        // Internet address length
  char                 out_buf[3];   		// Output buffer for data
  int                  retcode;         // Return code

  server_s = socket(AF_INET,SOCK_DGRAM,0);
	if (server_s < 0)
	{
		printf("*** ERROR - socket() failed \n");
		exit(-1);

	}

	l = sizeof(struct sockaddr_in);
	bzero(&sock_serv,l);

	sock_serv.sin_family=AF_INET;
	sock_serv.sin_port=htons(PORT_NUM);
	sock_serv.sin_addr.s_addr=htonl(INADDR_ANY);

	if(bind(server_s,(struct sockaddr*)&sock_serv,l)==-1){
		printf("*** ERROR - bind() failed \n");
    exit(-1);
	}

	//bzero(FILENAME,256);
	//sprintf(FILENAME,"new_file");
	printf("Creating the output file : %s\n",FILENAME);

	//open file
	if((file=open(FILENAME,O_CREAT|O_WRONLY|O_TRUNC,0600))==-1){
		perror("open fail");
		return EXIT_FAILURE;
	}

	bzero(&in_buf, BUFFER_SIZE);
  n = recvfrom(server_s,&in_buf,BUFFER_SIZE,0,(struct sockaddr *)&client,&l);

	// Copy the four-byte client IP address into an IP address structure
  memcpy(&client_ip_addr, &client.sin_addr.s_addr, 4);

  // Print an informational message of IP address and port of the client
	client.sin_port=htons(1050);
  printf("IP address of client = %s  port = %d \n", inet_ntoa(client_ip_addr),
    ntohs(client.sin_port));
	int resend = 0;
	int increment = 0;
	//Sending
	while(n){
		printf("%lld of data received \n",n);
		if(n < 0){
			perror("read fails");
			return EXIT_FAILURE;
		}
		printf("CLIENT RESEND BYTE IS... %c \n", in_buf[n-1]);
		printf("SERVER RESEND BYTE IS... %c \n", resend+'0');
		if(resend+'0' == in_buf[n-1]){//not a resend
			printf("Not a resend...\n");
			out_buf[0]='A';
			out_buf[1]='C';
			out_buf[2]='K';
			retcode = sendto(server_s, out_buf, 3, 0,
				(struct sockaddr *)&client, sizeof(client));
			if (retcode < 0)
			{
				printf("*** ERROR - sendto() failed \n");
				exit(-1);
			}
			else{
				printf("Sending ACK (NOT A RESEND)... retcode is... %i \n", retcode);
			}
			resend++;
			resend = resend%2;
			count += n;
			increment++;
			write(file,in_buf,n);
		}
		else{
			//its a resend..
			printf("It's A Resend... \n");
			out_buf[0]='A';
			out_buf[1]='C';
			out_buf[2]='K';
			retcode = sendto(server_s, out_buf, 3, 0,
				(struct sockaddr *)&client, sizeof(client));
			if (retcode < 0)
			{
				printf("*** ERROR - sendto() failed \n");
				exit(-1);
			}
			printf("Sending ACK (ITS A RESEND)... retcode is... %i \n", retcode);
		}
		bzero(in_buf,BUFFER_SIZE);
		bzero(out_buf,3);
		printf("waiting for new packets...");
    n = recvfrom(server_s,&in_buf,BUFFER_SIZE,0,(struct sockaddr *)&client,&l);
		client.sin_port=htons(1050);
	}
	printf("Transfer total: %lld\n",count);
	printf("Total file size = : %lld\n",(count-increment));

	//Close the sockets
	retcode = close(server_s);
	if (retcode < 0)
	{
		printf("*** ERROR - close() failed \n");
		exit(-1);
	}
	return (0);
}

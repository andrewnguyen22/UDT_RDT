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
#define BUFFER_SIZE 4096
#define PORT_NUM 1050
#define FILENAME "test.txt"
#define DISCARD_RATE        0.00 // Discard rate (from 0.0 to 1.0)


double rand_val(void)
{
  const long  a =      16807;  // Multiplier
  const long  y = 2147483647;  // Modulus
  const long  q =     127773;  // y div a
  const long  r =       2836;  // y mod a
  static long x = 1;           // Random int value (seed is set to 1)
  long        x_div_q;         // x divided by q
  long        x_mod_q;         // x modulo q
  long        x_new;           // New x value

  // RNG using integer arithmetic
  x_div_q = x / q;
  x_mod_q = x % q;
  x_new = (a * x_mod_q) - (r * x_div_q);
  if (x_new > 0)
    x = x_new;
  else
    x = x_new + y;

  // Return a random value between 0.0 and 1.0
  return((double) x / y);
}




int main (){
	double z;                   // Uniform random value 0 to 1
	int									file;
	int                 server_s;
	int                 retcode;         // Return code
	int                 addr_len;        // Internet address length
	char 								in_buf[BUFFER_SIZE];
	char                out_buf[3];   		// Output buffer for data
	long long 					count=0;
	long long 					n;
  unsigned int 				l;
  struct sockaddr_in  client;     			// Client Internet address
	struct sockaddr_in  sock_serv;
  struct in_addr      client_ip_addr;  // Client IP address

  server_s = socket(AF_INET,SOCK_DGRAM,0);	//Server Socket Setup UDT
	if (server_s < 0)
	{
		printf("The server socket was unable to initialize \n");
		exit(-1);
	}

	l = sizeof(struct sockaddr_in);
	bzero(&sock_serv,l);

	sock_serv.sin_family=AF_INET;
	sock_serv.sin_port=htons(PORT_NUM);
	sock_serv.sin_addr.s_addr=htonl(INADDR_ANY);

	if(bind(server_s,(struct sockaddr*)&sock_serv,l)==-1){
		printf("Unable to bind the server socket \n");
    exit(-1);
	}

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
	printf("STARTING PACKET... \n");
	int resend = 0;
	int increment = 0;
	int resend_count = 0;
	//Sending
	while(n){
		printf("\n\n\n\n");
		printf("%lld of data received \n",n);
		if(n < 0){
			perror("read fails");
			return EXIT_FAILURE;
		}
		printf("CLIENT RESEND BYTE IS... %c \n", in_buf[n-1]);
		printf("SERVER RESEND BYTE IS... %c \n", resend+'0');

		z = rand_val();

	  if (z > DISCARD_RATE)
	  {
			out_buf[0]='A';
			out_buf[1]='C';
			out_buf[2]='K';
			client.sin_port=htons(1050);
			printf("IP address of client = %s  port = %d \n", inet_ntoa(client_ip_addr),
		    htons(client.sin_port));
			retcode = sendto(server_s, out_buf, 3, 0,
				(struct sockaddr *)&client, sizeof(client));
	  }
		if(resend+'0' == in_buf[n-1]){//not a resend
			if (retcode < 0)
			{
				printf("*** ERROR - sendto() failed %i \n", retcode);
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
			//its a resend..r
			resend_count++;
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
	}
	printf("Transfer total: %lld\n",count);
	printf("Total file size = : %lld\n",(count-increment));
	printf("Total num of resends = : %i\n", resend_count);
	//Close the sockets
	retcode = close(server_s);
	if (retcode < 0)
	{
		printf("*** ERROR - close() failed \n");
		exit(-1);
	}
	return (0);
}

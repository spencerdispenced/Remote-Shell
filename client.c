//Spencer Thomason
//client.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>


int fd; // socket descriptor for signal handler


void signal_handler(int signo)
{
   close(fd);
   exit(0);
}


void *read_server(void *args)
{
   char message[512];
   char cmd[100];
   memset(message,0,sizeof(message));			

   int server_fd;
   server_fd = *(int *)args;
   free(args);
   int status;

   while((status = read(server_fd,message,sizeof(message))) > 0) // read message from the server
   {
      write(1,message,sizeof(message)); // write to stdout
      write(1, cmd, sprintf(cmd, "\x1b[32mEnter command to remote shell: \x1b[0m"));
      memset(message,0,sizeof(message));
      memset(cmd,0,sizeof(cmd));
   }

   printf("The Remote Host has closed connection\n");

   close(server_fd);
   exit(1);	
}


int main(int argc, char *argv[])
{
   if(argc < 2) 
   {
      fprintf(stderr, "ERROR: USAGE: <SERVER MACHINE> <PORT NUMBER>\n");
      exit(1);       
   }

   int server_fd;
   char message[256];
   pthread_t server_user;
   struct addrinfo addrinfo;
   struct addrinfo *result;

   char port[10];
   strcpy(port, argv[2]);

   
   /*setup signal handler*/
   struct sigaction sa;
   sa.sa_handler = &signal_handler;
   sigaction(SIGINT,&sa,NULL);

   
   addrinfo.ai_flags = 0;
   addrinfo.ai_family = AF_INET;
   addrinfo.ai_socktype = SOCK_STREAM;
   addrinfo.ai_protocol = 0;
   addrinfo.ai_addrlen = 0;
   addrinfo.ai_addr = NULL;
   addrinfo.ai_canonname = NULL;
   addrinfo.ai_next = NULL;


   if(gethostbyname(argv[1]) == NULL)
   {
      printf("ERROR: Invalid Host\n");
      exit(1);
   }

   getaddrinfo(argv[1], argv[2], &addrinfo, &result );

   server_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
   

   void *socket_arg2 = malloc(sizeof(server_fd));
   memcpy(socket_arg2, &server_fd, sizeof(int));
   int status = connect(server_fd, result->ai_addr, result->ai_addrlen);


   while(status < 0)
   {
      close(server_fd);
      server_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

      if(errno != ECONNREFUSED)
      {
         printf("ERROR: %s\n", strerror(errno));
         
         exit(1);
      }

      printf("Could not connect to server. Attempting to reconnect...\n");
      sleep(3);
      status = connect(server_fd, result->ai_addr, result->ai_addrlen);
   }

	
   fd = server_fd; // assignment for signal handler
		
   if(pthread_create(&server_user, NULL, &read_server, socket_arg2) != 0)
   {
      fprintf(stderr, "Error creating thread");
	   exit(1);;
   }

   pthread_detach(server_user);


   while(read(0,message,sizeof(message))) // read from stdin
   {
	   write(server_fd,message,sizeof(message)); // write to server
      memset(message, 0, sizeof(message)); //clear message
      sleep(1);	  
   }

   close(server_fd);
   return 0;	
} 

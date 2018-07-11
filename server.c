// Spencer Thomason
// server.c


#define _GNU_SOURCE

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
#include <sys/wait.h>


pthread_mutex_t lock_unlock; 

int fd; //socket descriptor for signal handler


void signal_handler(int signo)
{
   close(fd);
   exit(0);
}

void *client_session(void *args)
{
   int client_sd;
   client_sd = *(int *)args;
   free(args);

   char message[500];
   char response[500];

   pthread_detach(pthread_self()); 

   printf("Client connected\n");  

   int current_user = client_sd;

   write(current_user, response, sprintf(response, "\nFirst press Enter, then you may start giving remote Linux commands.\n\n"));

   while((read(client_sd,message,sizeof(message))) > 0) // read message from the server
   {
      pid_t pid = fork(); // fork the process

      if (pid == -1)
      {
         write(current_user, response, sprintf(response, "ERROR: could not fork()\n"));
         exit(1);
      } 

      else if (pid > 0)
      {
         int status;
         waitpid(pid, &status, 0);
      }

      else // in child process
      {
         /*redirect stdin and stdout*/
         dup2(current_user, 0);
         dup2(current_user, 1);
    
         /*open shell in child process*/
         execl("/bin/sh", "", NULL);
      }
   }

   return NULL;
}


int main(int argc, char *argv[])
{
   int sd;
   struct addrinfo addrinfo;
   struct addrinfo *result;
    
   char message[256];
   int on = 1;

   addrinfo.ai_flags = AI_PASSIVE;
   addrinfo.ai_family = AF_INET;
   addrinfo.ai_socktype = SOCK_STREAM;
   addrinfo.ai_protocol = 0;
   addrinfo.ai_addrlen = 0;
   addrinfo.ai_addr = 0;
   addrinfo.ai_canonname = NULL;
   addrinfo.ai_next = NULL;

   char port[10];
   strcpy(port, argv[1]);

   /*setup signal handler*/
   struct sigaction sa;
   sa.sa_handler = &signal_handler;
   sigaction(SIGINT,&sa,NULL);

   getaddrinfo(0, port, &addrinfo, &result);

   if((sd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) < 0)
   {
      fprintf(stderr,"ERROR: SOCKET CANNOT BE CREATED\n");
      exit(1);
   }

   else if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR , &on, sizeof(on)) == -1)
   {
      write(1, message, sprintf(message, "setsockopt() failed.  File %s line %d.\n", __FILE__, __LINE__ ));
      freeaddrinfo(result);
      close(sd);
      exit(1);
   }

   else if(bind(sd, result->ai_addr, result->ai_addrlen) < 0)
   {
      fprintf(stderr, "ERROR: COULD NOT BIND TO PORT\n");
      freeaddrinfo(result);
      close(sd);
      exit(1);
   }
     
   write(1, message, sprintf(message, "\x1B[1;35mSUCCESS : Bind to port %s\n" "\x1b[0m", port));

   pthread_mutex_init(&lock_unlock, 0);// initiate mutex lock

   listen(sd,20); 
   void *csa;   // socket argument for every client thread created
   pthread_t client;

   while(1)
   {
      if((fd = accept(sd, 0, 0)) < 0)
      {
         fprintf(stderr, "ERROR: FAILED TO ACCEPT CLIENT\n");
         exit(1);
      }

      csa = malloc(sizeof(int));
      memcpy(csa, &fd, sizeof(on));

      pthread_create(&client, NULL, &client_session, csa);// create thread for client - service
      pthread_detach(client);

   }

   freeaddrinfo(result);

   return 0;
}

/**
 * webserver.c -- A webserver written in C
 * 
 * Test with curl (if you don't have it, install it):
 * 
 *    curl -D - http://localhost:3490/
 *    curl -D - http://localhost:3490/d20
 *    curl -D - http://localhost:3490/date
 * 
 * You can also test the above URLs in your browser! They should work!
 * 
 * Posting Data:
 * 
 *    curl -D - -X POST -H 'Content-Type: text/plain' -d 'Hello, sample data!' http://localhost:3490/save
 * 
 * (Posting data is harder to test from a browser.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>

#define PORT "3490" // the port users will be connecting to 

#define BACKLOG 10 // how many pending connections queue will hold

/**
 * Handle SIGCHILD signal
 *
 * We get this signal when a child process dies. This function wait()s for
 * Zombie processes.
 *
 * This is only necessary if we've implemented a multiprocessed version with
 * fork().
 */
void sigchld_handler(int s) {
  (void)s; // quiet unused variable warning

  // waitpid() might overwrite errno, so we save and restore it:
  int saved_errno = errno;

  // Wait for all children that have died, discard the exit status
  while(waitpid(-1, NULL, WNOHANG) > 0);

  errno = saved_errno;
}
/**
 * Set up a signal handler that listens for child processes to die so
 * they can be reaped with wait()
 *
 * Whenever a child process dies, the parent process gets signal
 * SIGCHLD; the handler sigchld_handler() takes care of wait()ing.
 * 
 * This is only necessary if we've implemented a multiprocessed version with
 * fork().
 */
void start_reaper(void)
{
    struct sigaction sa;

    sa.sa_handler = sigchld_handler; // Reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; //Restart signal handler if interrupted
    if (sigaction(SIGCHLD, &sa, NULL)== -1){
        perror("sigaction");
        exit(1);
    } 
 /** This gets an Internet address, either IPv4 or IPv6
 *
 * Helper function to make printing easier.
 */
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)-> sin6_addr);
}

//**
 * Return the main listening socket
 * 
 * Returns -1 or error
 * /

int get_listener_socket(char *port)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int yes=1;
    int rv;
} 
// This block of code looks at the local network interfaces and

  // tries to find some that match our requirements (namely either

  // IPv4 or IPv6 (AF_UNSPEC) and TCP (SOCK_STREAM) and use any IP on

  // this machine (AI_PASSIVE).

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, port, &hints, &servinfo))! = 0){
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return -1;
  }
  // Once we have a list of potential interfaces, loop through them

  // and try to set up a socket on each. Quit looping the first time

  // we have success.
  for(p = servinfo; p != NULL; p = p->ai_next){
      //try to make a socket based on this candidate interface

   if ((sockfd = socket(p->ai_family, p->ai_socktype,
      p->ai_protocol  ))== -1){
          //perror("server:socket");
          continue;
      }   
      // SO_REUSEADDR prevents the "address already in use" errors

    // that commonly come up when testing servers.

     if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
         sizeof(int))== -1){
         perrpr("setsockopt");
         close(sockfd);
         freeaddrinfo(servinfo);// all done wth this
         
     }
  }
 
 // See if we can bind this socket to this local IP address. 
 //asccociates the file descriptor(the socket descriptor) that
 //we will read and write on with a specific IP address.

 if (bind(sockfd, p->ai_addr, p->ai_addrlen)){
     close(sockfd);
     //perror("server: bind");
     continue;
 }
 //If we got here, we got a bound socket and we're done
 break;
}

freeaddrinfo(servinfo); all done with this structure
// If p is NUll, it means we didnt break break out of loop, above,
//and we dont have a good socket.
 if (p == NULL){
     fprint(stderr, "webserver: failed to find local address\n");
     return -3;
 
 }
 //start listening. this is what allows remote computer to connect
 //to this socket/IP
 if (listen(sockfd, BACKLOG)== -1){
     //perror("listen");
     close(sockfd);
     return -4;
 }
 return sockfd;

 //**
  * send an HTTP response
  * 
  * header: "HTTP/1.1 404 Not Found" or "HTTP/1.1 200 ok, etc. "
  * content_type:"text/plain", etc.
  * body: the data to send.
  * 
  * Return the vlue from the send()function.
  * /
   int send_response(int fd, char *header, char* *content_type, char *content_type, char *body){

   }

    const int max_response_size = 65536;
    char response[max_response_size];
    //int response_length; // total length of header plus body

    //get current time for the HTTP
    time_t t1 time(NULL);
    struct tm *ltime = localtime(&t1);

    //how many bytes in the body

    int content_lenght = sprintf(response,
    "%s\n"
    "Date: %s" //asctime adds it own newlind)
    "connection: close\n"
    "content-Lenght:%d\n"
    "content-Type:%s\n"
    "\n"//end of HTTP header
    "%s\n",

    header,
    asctime(ltime),
    content_length,
    content_type,
    body);
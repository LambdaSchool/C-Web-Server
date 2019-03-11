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
#include <time.h>
#include <sys/file.h>
#include <fcntl.h>
#include "net.h"
#include "file.h"
#include "mime.h"
#include "cache.h"

#define PORT "3490"  // the port users will be connecting to

#define SERVER_FILES "./serverfiles"
#define SERVER_ROOT "./serverroot"

/**
 * Send an HTTP response
 *
 * header:       "HTTP/1.1 404 NOT FOUND" or "HTTP/1.1 200 OK", etc.
 * content_type: "text/plain", etc.
 * body:         the data to send.
 * 
 * Return the value from the send() function.
 */
int send_response(int fd, char *header, char *content_type, void *body, int content_length)
{
    const int max_response_size = 256000; // Previously: 65536
    char response[max_response_size]; // char buffer
    int response_length = 0; // length of header, the total number of bytes returned from sprintf()
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Build HTTP response and store it in response
    // sprintf stands for “String print”. Instead of printing on console, it store output on char buffer which is specified in sprintf
    response_length= sprintf(response,
      "%s\n"
      "Date: %s"
      "Connection: %s\n"
      "Content-Length: %d\n"
      "Content-Type: %s\n\n",  // The end of the header on both the request and response is marked by a blank line i.e. two newlines in a row)

      header,
      asctime(timeinfo),
      "close",
      content_length,
      content_type
    );

    // The total length of the header and body should be stored in the response_length variable
    // so that the send() call knows how many bytes to send out over the wire.
    // Send it all!

    // ATTEMPT 2: Working
    memcpy(response+response_length, body, content_length); // start copying after the length of the header
    int rv = send(fd, response, response_length+content_length, 0);

    // ATTEMPT 1: Error -> GET http://localhost:3490/cat.jpg net::ERR_CONTENT_LENGTH_MISMATCH 200 (OK)
    // response_length += strlen(body);
    // int rv = send(fd, response, response_length, 0);

    if (rv < 0) {
        perror("send");
    }

    return rv;
}


/**
 * Send a /d20 endpoint response
 */
void get_d20(int fd)
{
    // Generate a random number between 1 and 20 inclusive
    srand(time(NULL)); // Seeds the pseudo-random number generator used by rand() with the value seed.
    char body[20]; // stores total number of bytes from sprintf
    int random_number = rand()%20 + 1; // +1 for inclusive 20
    int length = sprintf(body, "%d", random_number);
    printf("Random Number: %d, Length: %d\n", random_number, length);

    // Use send_response() to send it back as text/plain data
    // send_response(int fd, char *header, char *content_type, void *body, int content_length)
    send_response(fd, "HTTP/1.1 200 OK", "text/plain", body, length);
}

/**
 * Send a 404 response
 */
void resp_404(int fd)
{
    char filepath[4096];
    struct file_data *filedata; 
    char *mime_type;

    // Fetch the 404.html file
    snprintf(filepath, sizeof filepath, "%s/404.html", SERVER_FILES);
    filedata = file_load(filepath);

    if (filedata == NULL) {
        // TODO: make this non-fatal
        fprintf(stderr, "cannot find system 404 file\n");
        exit(3);
    }

    mime_type = mime_type_get(filepath);

    send_response(fd, "HTTP/1.1 404 NOT FOUND", mime_type, filedata->data, filedata->size);

    file_free(filedata);
}

/**
 * Read and return a file from disk or cache
 */
void get_file(int fd, struct cache *cache, char *request_path)
{
  char path[1024];
  struct file_data *filedata; // buffer, type from file.c
  char *mime_type; // type from from mime.c

  sprintf(path, "%s%s", SERVER_ROOT, request_path); // parse the file path

  filedata = file_load(path); // load the file into struct buffer
  if (filedata == NULL) { // check if file was loaded properly
      resp_404(fd);
      printf("get_file() -> Cannot find file.\n");
      return;
  }

  mime_type = mime_type_get(path);

  printf("get_file() -> %d, %s, %s, %s,%d", fd, "HTTP/1.1 200 OK", mime_type, filedata->data, filedata->size);
  send_response(fd, "HTTP/1.1 200 OK", mime_type, filedata->data, filedata->size); // send the buffer
  file_free(filedata); // file_load had built in malloc
}

/**
 * Search for the end of the HTTP header
 * 
 * "Newlines" in HTTP can be \r\n (carriage return followed by newline) or \n
 * (newline) or \r (carriage return).
 */
char *find_start_of_body(char *header)
{
  (void)header;
    ///////////////////
    // IMPLEMENT ME! // (Stretch)
    ///////////////////
    return 0;
}

/**
 * Handle HTTP request and send response
 */
void handle_http_request(int fd, struct cache *cache)
{
    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];
    char method[10], path[20], protocol[20];

    // Read request
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0) {
        perror("recv");
        return;
    }

    // Read the three components of the first request line
    // The variable request in handle_http_request() holds the entire HTTP request once the recv() call returns.
    printf("request: %s\n", request);
    sscanf(request, "%s %s %s", method, path, protocol);
    printf("handle_http_request() -> \n"
      "method: %s,\n"
      "path: %s,\n"
      "protocol: %s\n",
      
      method,
      path,
      protocol
    );

    // If GET, handle the get endpoints
    // Check if it's /d20 and handle that special case
    // Otherwise serve the requested file by calling get_file()
    // Hint: strcmp() for matching the request method and path. Another hint: strcmp() returns 0 if the strings are the same!
    if (strcmp(method, "GET") == 0) {
      if (strcmp(path, "/d20") == 0) {
        get_d20(fd);
      }
      else if (strcmp(path, "/") == 0) {
        get_file(fd, cache, "/index.html");
      }
      else {
        get_file(fd, cache, path);
      }
    }
    else {
      resp_404(fd);
    }

    // (Stretch) If POST, handle the post request
}

/**
 * Main
 */
int main(void)
{
    int newfd;  // listen on sock_fd, new connection on newfd
    struct sockaddr_storage their_addr; // connector's address information
    char s[INET6_ADDRSTRLEN];

    struct cache *cache = cache_create(10, 0);

    // Get a listening socket
    int listenfd = get_listener_socket(PORT);

    if (listenfd < 0) {
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }

    printf("webserver: waiting for connections on port %s...\n", PORT);

    // This is the main loop that accepts incoming connections and
    // forks a handler process to take care of it. The main parent
    // process then goes back to waiting for new connections.
    
    while(1) {
        socklen_t sin_size = sizeof their_addr;

        // Parent process will block on the accept() call until someone
        // makes a new connection:
        newfd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);
        if (newfd == -1) {
            perror("accept");
            continue;
        }

        // Print out a message that we got the connection
        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);
        
        // newfd is a new socket descriptor for the new connection.
        // listenfd is still listening for new connections.

        handle_http_request(newfd, cache);

        close(newfd);
    }

    // Unreachable code

    return 0;
}


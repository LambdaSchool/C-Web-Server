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
    const int max_response_size = 65536;
    char response[max_response_size];
    int response_length = 0; //total length of both header and body

    //get time for HTTP header
    time_t t1 = time(NULL);
    struct tm *ltime = localtime(&t1);

    // Build HTTP response and store it in response

    /*
    use sprintf() for creating the HTTP response
    https://www.tutorialspoint.com/c_standard_library/c_function_sprintf.htm
    Composes a string with the same text that would be printed if format was 
    used on printf, but instead of being printed, the content is stored as a 
    C string in the buffer pointed to by str.  Return value is number of bytes

    use strlen() for computing the content length

    use sprintf() to return the total number of bytes in the resulting string

    use time() and localtime() functions are included in time.h
     */

    response_length = sprintf(response,  
        // construct the response
        // formatting for subsequent arguments
        "%s\n" //header
        "Date: %s"
        "Connection: close\n"
        "Content-Length: %d\n"
        "Content-Type: %s\n"
        "\n",

        header,
        asctime(ltime), 
        content_length, 
        content_type
        );

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    memcpy(response + response_length, body, content_length);

    // Send it all!
    int rv = send(fd, response, response_length + content_length, 0);
    
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
    
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    // use srand(getpid()), s "seeding"
    srand(getpid() + time(NULL));

    // initialize the response body 
    char response_body[8];
    // inject an integer into the response body
    sprintf(response_body, "%d\n", (rand() % 20) + 1); // %20 gives 0-19, + 1 shifts upwards by 1

    // Use send_response() to send it back as text/plain data

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    send_response(fd, "HTTP/1.1 200 OK", "text/plain", response_body, strlen(response_body));
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
    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
}

/**
 * Search for the end of the HTTP header
 * 
 * "Newlines" in HTTP can be \r\n (carriage return followed by newline) or \n
 * (newline) or \r (carriage return).
 */
char *find_start_of_body(char *header)
{
    ///////////////////
    // IMPLEMENT ME! // (Stretch)
    ///////////////////
}

/**
 * Handle HTTP request and send response
 */
void handle_http_request(int fd, struct cache *cache)
{
    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];

    //--initialize request type length ~8
    char request_type[8];
    //--initialize request path length ~1024
    char request_path[1024];
    //--initialize request protocol length ~16
    char request_protocol[16];

    // Read request
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0) {
        perror("recv");
        return;
    }


    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    // Read the three components of the first request line

    /*use sscanf() https://www.tutorialspoint.com/c_standard_library/c_function_sscanf.htm
     to take from the request buffer and break the first line into its component pieces, 
     specify format %s %s %s, type, path, protocol */

    sscanf(request, "%s %s %s", request_type, request_path, request_protocol);

    printf("get request: %s %s %s", request_type, request_path, request_protocol);

    // If GET, handle the get endpoints

    //    Check if it's /d20 and handle that special case (strcmp == 0 means strings are the same)
    //    Otherwise serve the requested file by calling get_file()

    //check if the request is a GET type
    if (strcmp(request_type, "GET") == 0){
        // check if the file path is /d20
        if (strcmp(request_path, "/d20") == 0){
            get_d20(fd); //fd will be a randomly generated number between 0 and 20
        } else {
            get_file(fd, cache, request_path);
        }
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


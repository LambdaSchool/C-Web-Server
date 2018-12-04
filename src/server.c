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

    // const int response_length = content_length + strlen(fd) + strlen(header);
    // Build HTTP response 
    
    //and store it in response
    const int response_length = sprintf(response, "%s\n Connection: close\nContent-Length: %d\nContent-Type: %s\n\n%s", header, content_length, content_type, body);

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    // Send it all!
    int rv = send(fd, response, response_length, 0);

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
    srand(time(NULL));
    int random  = rand() % 21;
    printf("get_d20_1 %d\n", random);
    // printf("get_d20_2 %ld\n", sizeof(random));
    char randomstr[3];

    sprintf(randomstr, "%d\n", random);
    // Use send_response() to send it back as text/plain data
    
    // printf("get_d20_3 %d\n", size);

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////
    send_response(fd, "HTTP/1.1 200 OK", "text/plain", randomstr, strlen(randomstr));
}

/**
 * Send a 404 response
 */
void resp_404(int fd)
{
    printf("resp_404\n");
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
    printf("get file%s\n", request_path);
    char filepath[4096];
    struct file_data *filedata; 
    char *mime_type;

    snprintf(filepath, sizeof filepath, "%s/%s", SERVER_ROOT, request_path);
    filedata = file_load(filepath);
    printf("%s\n", filepath);
    if (filedata == NULL) {
        // TODO: make this non-fatal
        fprintf(stderr, "cannot find path requested\n");
        exit(3);
    }

    mime_type = mime_type_get(filepath);

    send_response(fd, "HTTP/1.1 200 OK", mime_type, filedata->data, filedata->size);

    file_free(filedata);

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
    printf("handle_http_request\n");
    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];

    // Read request
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);

    if (bytes_recvd < 0) {
        perror("recv");
        return;
    }

    ///////////////////
    // IMPLEMENT ME! //
    ///////////////////

    printf("\nhandle_http_request2 %s\n", request);
    // Read the three components of the first request line
    // char type[8];
    // char file[4];
    // char protocal[8];

    char type[8];
    char file[75];
    char protocal[9];

    sscanf(request, "%s %s %s", type, file, protocal);
    printf("handle_http_request3. type: %s\nfile: %s\nprotocal: %s\n\n", type, file, protocal);

    // If GET, handle the get endpoints
    if(0 == strcmp(type, "GET"))
    {
        if(0 == strcmp(file, "/d20"))
        {
            get_d20(fd);
        } else {
            printf("\nfile is not /d20\n");
            get_file(fd, cache, file);
        }
    } 
    else
    {
        resp_404(fd);
    }


    //    Check if it's /d20 and handle that special case
    //    Otherwise serve the requested file by calling get_file()

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
        // resp_404(newfd);
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


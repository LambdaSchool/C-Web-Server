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

#define PORT "3490" // the port users will be connecting to

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
    const int max_response_size = 262144;
    char response[max_response_size];

    time_t rawtime;
    struct tm *timestamp;
    char buffer[100];

    time(&rawtime);
    timestamp = localtime(&rawtime);
    strftime(buffer, 100, "%a %b %d %T %Z %Y", timestamp);
    // Build HTTP response and store it in response
    int response_length = snprintf(response, max_response_size,
                                   "%s\n"
                                   "Date: %s\n"
                                   "Connection: close\n"
                                   "Content-Length: %d\n"
                                   "Content-Type: %s\n"
                                   "\n",
                                   header, buffer, content_length, content_type);

    memcpy(response + response_length, body, content_length);
    response_length += content_length;
    // Send it all!
    // To get binary working memcopy or two sends (one response, two body)
    // Talked about in Tuesday lecture
    int rv = send(fd, response, response_length, 0);

    if (rv < 0)
    {
        perror("send");
    }

    return rv;
}

/**
 * Send a /d20 endpoint response
 */
void get_d20(int fd)
{
    char str[5];
    // Generate a random number between 1 and 20 inclusive
    int num = (rand() % (20 - 1 + 1) + 1);
    // Conver number to string
    sprintf(str, "%d", num);
    // Use send_response() to send it back as text/plain data
    send_response(fd, "HTTP/1.1 200 OK", "text/plain", str, strlen(str));
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

    if (filedata == NULL)
    {
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
    char filepath[4096];
    struct file_data *filedata;
    char *mime_type;
    struct cache_entry *ce;
    snprintf(filepath, sizeof filepath, "%s%s", SERVER_ROOT, request_path);
    ce = cache_get(cache, filepath);
    if (ce)
    {
        send_response(fd, "HTTP/1.1 200 OK", ce->content_type, ce->content, ce->content_length);
    }
    else
    {
        filedata = file_load(filepath);

        if (filedata == NULL)
        {
            if (strcmp(filepath, "./serverroot/") == 0)
            {
                get_file(fd, cache, "/index.html");
                return;
            }
            else
            {

                resp_404(fd);
                return;
            }
        }

        mime_type = mime_type_get(filepath);
        send_response(fd, "HTTP/1.1 200 OK", mime_type, filedata->data, filedata->size);

        cache_put(cache, filepath, mime_type, filedata->data, filedata->size);

        file_free(filedata);
    }
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
    return NULL;
}

/**
 * Handle HTTP request and send response
 */
void handle_http_request(int fd, struct cache *cache)
{
    const int request_buffer_size = 65536; // 64K
    char request[request_buffer_size];

    // Read request
    int bytes_recvd = recv(fd, request, request_buffer_size - 1, 0);
    if (bytes_recvd < 0)
    {
        perror("recv");
        return;
    }

    char method[50];
    char path[200];
    char HTTP[50];

    // Read the three components of the first request line
    sscanf(request, "%s %s %s", method, path, HTTP);
    // If GET, handle the get endpoints
    if (strcmp(method, "GET") == 0)
    {
        //    Check if it's /d20 and handle that special case
        if (strncmp(path, "/d20", 4) == 0)
        {
            get_d20(fd);
        }
        else
        {
            //    Otherwise serve the requested file by calling get_file()
            get_file(fd, cache, path);
        }
    }

    // (Stretch) If POST, handle the post request
}

/**
 * Main
 */
int main(void)
{
    int newfd;                          // listen on sock_fd, new connection on newfd
    struct sockaddr_storage their_addr; // connector's address information
    char s[INET6_ADDRSTRLEN];

    struct cache *cache = cache_create(10, 0);

    // Get a listening socket
    int listenfd = get_listener_socket(PORT);
    if (listenfd < 0)
    {
        fprintf(stderr, "webserver: fatal error getting listening socket\n");
        exit(1);
    }

    printf("webserver: waiting for connections on port %s...\n", PORT);

    // This is the main loop that accepts incoming connections and
    // forks a handler process to take care of it. The main parent
    // process then goes back to waiting for new connections.
    while (1)
    {
        socklen_t sin_size = sizeof their_addr;
        // Parent process will block on the accept() call until someone
        // makes a new connection:
        newfd = accept(listenfd, (struct sockaddr *)&their_addr, &sin_size);

        if (newfd == -1)
        {
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

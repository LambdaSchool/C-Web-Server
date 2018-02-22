# A Simple Web Server in C

## Networking Protocols

In networking, each layer adds a wrapper around the data with more information
about data integrity or routing or another information.

At the highest level, you have your data that you want to transmit. As it is
prepared for transmission on the network, the data is _encapsulated_ in other
data to help it arrive at its destination. Any particular piece of data will be
wrapped, partially unwrapped, and re-wrapped as it moves from wire to wire
across the Internet to its destination.

The act of wrapping data puts a new _header_ on the data. This header
encapsulates the original data, _and all the headers that have been added before
it_.

Here is an example of a fully-encapsulated HTTP data packet.

```
+-----------------+
| Ethernet Header |  Deals with routing on the LAN
+-----------------+
| IP Header       |  Deals with routing on the Internet
+-----------------+
| TCP Header      |  Deals with data integrity
+-----------------+
| HTTP Header     |  Deals with web data
+-----------------+
| Your Data       |  Whatever you need to send
|                 |
|                 |
+-----------------+
```

As the data leaves your LAN and heads out in the world, the Ethernet header will
be stripped off, the IP header will be examined to see how the data should be
router, and another header for potentially a different protocol will be put on
to send the traffic over DSL, a cable modem, or fiber.

The Ethernet header is created and managed by the network drivers in the OS.

## Sockets

Under Unix-like operating systems, the _sockets API_ is the one used to send
Internet traffic. It supports both the TCP and UDP protocols, and IPv4 and IPv6.

The sockets API gives access to the IP and TCP layers in the diagram above.

A _socket descriptor_ is a number used by the OS to keep track of open
connections.

You can create a new socket (socket descriptor) with the `socket()` system call.

Once created you still have to _bind_ it to a particular IP address (which the
OS associates with a particular network card). This is done with the `bind()`
system call.

Once bound, you can read and write data to the socket using the `recv()` and
`send()` system calls.

* See also [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

## HTTP

The final piece of information needed for web traffic is the _HyperText
Transport Protocol_ (HTTP). While TCP deals with general data integrity and IP
deals with routine, HTTP is concerned with `GET` and `POST` requests of web
data.

Like the other stages of networking, HTTP adds a header before the data it wants
to send with the packet. Like IP and TCP, This header has a well-defined
specification for exactly what needs to be sent.

Though the specification is complex, a fortunately small amount of information
is needed to implement a barebones version.

For each _HTTP request_ from a client, the server, sends back an _HTTP
response_.

Here is an example HTTP `GET` request and response using version 1.1 of the HTTP
protocol getting the page `http://lambdaschool.com/example`:

```
GET /example HTTP/1.1
Host: lambdaschool.com

```

And here is a sample HTTP response:

```
HTTP/1.1 200 OK
Date: Wed Dec 20 13:05:11 PST 2017
Connection: close
Content-Length: 41749
Content-Type: text/html

<!DOCTYPE html><html><head><title>Lambda School ...
```

The end of the header on both the request and response is marked by a blank line
(i.e. two newlines in a row).

If the file is not found, a `404` response is generated and returned by the
server:

```
HTTP/1.1 404 NOT FOUND
Date: Wed Dec 20 13:05:11 PST 2017
Connection: close
Content-Length: 13
Content-Type: text/plain

404 Not Found
```

Important things to note:

* For HTTP/1.1, the request **must** include the `Host` header.
* The second word of the first line of the response gives you a success or
  failure indicator.
* `Content-Length` gives the length of the request or response body, not
  counting the blank line between the header and the body.
* `Content-Type` gives you the MIME type of the content in the body. This is how
  your web browser knows to display a page as plain text, as HTML, as a GIF
  image, or anything else. They all have their own MIME types.
* Even if your request has no body, a blank line still **must** appear after the
  header.

## What is a Web Server?

A web server is a piece of software that accepts HTTP requests (commonly GET
requests for HTML pages), and returns responses (commonly HTML pages). Other
common uses are GET requests for images within web pages, and POST requests to
upload data to the server (e.g. a form submission or file upload).

## Assignment

We will write a simple web server that returns data on three `GET` endpoints:

* `http://localhost:3490/` should contain some HTML, e.g. `<h1>Hello, world!</h1>`.
* `http://localhost:3490/d20` should return a random number between 1 and 20
  inclusive as `text/plain` data.
* `http://localhost:3490/date` should print the current date and time in GMT as
  `text/plain` data.

Examine the skeleton source code for which pieces you'll need to implement.

For the portions that are already written, study the well-commented code to see
how it works.

Don't worry: the networking code is already written.

### Main Goals

1. Add parsing of the first line of the HTTP request header that arrives. It
   will be in the `request` array.

   Read the three components from the first line of the HTTP header. Hint:
   `sscanf()`.

   Decide which handler to call based on the request type (`GET`, `POST`) and
   the path (`/`, `/d20`, etc.)

2. Implement the `get_root()` handler. This will call `send_response()`.

3. Implement `send_response()`. Hint: `sprintf()`, `strlen()` for computing
   content length.

4. Implement the `get_d20()` handler. Hint: `srand()` with `time(NULL)`,
   `rand()`.

5. Implement the `get_date()` handler. Hint: `time(NULL)`, `gmtime()`.

### Stretch Goals

Post a file:

1. Implement `find_end_of_header()` to locate the end of the HTTP request header
   (and start of the body).

2. Implement the `post_save()` handler. Modify the main loop to pass the body
   into it. Have this handler write the file to disk. Hint: `fopen()`,
   `fwrite()`, `fclose()`.

   The response from `post_save()` should be of type `application/json` and
   should be `{"status":"ok"}`.

Concurrency:

Convert the web server to be multiprocessed by using the `fork()` system call.

1. Examine and understand the signal handler on `SIGCHLD` that watches for when
   child processes exit. (This is already written for you.)

2. Modify the main `while` loop to `fork()` a new child process to handle each
   request.

   _Be careful not to fork-bomb your system to its knees!_

   _Your child process **must** call `exit()` or you will risk having piles of
   extra processes at work!_

3. Modify the `post_save()` function to get an exclusive lock on the file using
   `flock()`. The lock should be unlocked once the file has been written.

   What happens if multiple processes try to write to the POSTed file at the
   same time without locking the file?

# A Simple Web Server in C 

In this project, we'll finish the implementation of a web server in C.

What's already there:

* Skeleton code that handles all the network communication
* The main loop in `main()`
* Skeleton endpoint handler calls functions

What you need to write:

* Code that parses HTTP requests
* Code that builds HTTP responses
* Your code will interface with the existing code

What you don't need to write:

* Any system calls, including `send()` and `recv()`
* Any new functions from scratch--there's a skeleton for all functions you'll
  need

## What is a Web Server?

A web server is a piece of software that accepts HTTP requests (e.g. GET
requests for HTML pages), and returns responses (e.g. HTML pages). Other common
uses are GET requests for getting data from RESTful API endpoints, images within
web pages, and POST requests to upload data to the server (e.g. a form
submission or file upload).

## General Information about Networking

Before learning about the web server, let's take a look at some general
information about how networking works. Some of these terms will be familiar to
you, and we'll expand their definitions a bit.

### Networking Protocols

_This is background information. You will not need to use this directly in the
web server._

A _protocol_ is an agreement between two programs about how they will
communicate. For the Internet, most protocols take the form of "If you send me
_x_, I'll send you back _y_." Internet-related protocols are clearly written
down in specifications, known as an _RFC_.

When you send some data out on the network, that data is wrapped up in several
layers of additional data that provide information about data integrity,
routing, and so on.

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
| <h1>Hello, worl |  Whatever you need to send
| d!</h1>         |
|                 |
+-----------------+
```

The details of what data exists in each header type is beyond the scope of what
most people need to know. It is enough to know the short description of what
each does.

As the data leaves your LAN and heads out in the world, the Ethernet header will
be stripped off, the IP header will be examined to see how the data should be
routed, and another header for potentially a different protocol will be put on
to send the traffic over DSL, a cable modem, or fiber.

The Ethernet header is created and managed by the network drivers in the OS.

### Sockets

_This is background information. You will not need to use this directly in the
web server. This code is written for you._

Under Unix-like operating systems, the _sockets API_ is the one used to send
Internet traffic. It supports both the TCP and UDP protocols, and IPv4 and IPv6.

The sockets API gives access to the IP and TCP layers in the diagram above.

A _socket descriptor_ is a number used by the OS to keep track of open
connections. It is used to send and receive data. In our web server, this
variable is called `fd`.

You can create a new socket (socket descriptor) with the `socket()` system call.

Once created you still have to _bind_ it to a particular IP address (which the
OS associates with a particular network card). This is done with the `bind()`
system call.

Once bound, you can read and write data to the socket using the `recv()` and
`send()` system calls.

* See also [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

## HTTP

_In the webserver, you will be writing code that parses down strings that hold
HTTP requests, and builds strings that hold HTTP responses. Study what an HTTP
request and response look like._

The final piece of information needed for web traffic is the _HyperText
Transport Protocol_ (HTTP). While TCP deals with general data integrity and IP
deals with routine, HTTP is concerned with `GET` and `POST` requests of web
data.

Like the other stages of networking, HTTP adds a header before the data it wants
to send with the packet. Like IP and TCP, this header has a well-defined
specification for exactly what needs to be sent.

Though the specification is complex, fortunately only a small amount of information
is needed to implement a barebones version.

For each _HTTP request_ from a client, the server sends back an _HTTP
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

If you've ever looked in the Network panel of your web browser's debugger, some
of these headers might look familiar.

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
* `Connection: close` tells the web browser that the TCP connection will be
  closed after this response. This should be included.
* The `Date` should be the date right now, but this field is optional.

## Assignment

We will write a simple web server that returns data on three `GET` endpoints:

* `http://localhost:3490/` should contain some HTML, e.g. `<h1>Hello, world!</h1>`.
* `http://localhost:3490/d20` should return a random number between 1 and 20
  inclusive as `text/plain` data.
* `http://localhost:3490/date` should print the current date and time in GMT as
  `text/plain` data.

Examine the skeleton source code for which pieces you'll need to implement.

_Spend some time inventorying the code to see what is where. Write down notes.
Write an outline. Note which functions call which other functions. Time spent up
front doing this will reduce overall time spent down the road._

For the portions that are already written, study the well-commented code to see
how it works.

Don't worry: the networking code is already written.

There is a `Makefile` provided. On the command line, type `make` to build the
server.

Type `./server` to run the server.

### Main Goals

_Read through all the main and stretch goals before writing any code to get an overall view,
then come back to goal #1 and dig in._

1. Examine `handle_http_request()` in the file `server.c`.

   You'll want to parse the first line of the HTTP request header to see if this
   is a `GET` or `POST` request, and to see what the path is. You'll use this
   information to decide which handler function to call.

   The variable `request` in `handle_http_request()` holds the entire HTTP
   request once the `recv()` call returns.

   Read the three components from the first line of the HTTP header. Hint:
   `sscanf()`.

   Right after that, call the appropriate handler based on the request type
   (`GET`, `POST`) and the path (`/`, `/d20`, etc.) You can start by just
   checking for `/` and add the others later as you get to them.

   The handler for `GET /` is `get_root()` (search for the skeleton code). The
   handler for `GET /d20` is `get_d20()`, etc.
   
   Hint: `strcmp()` for matching the request method and path. Another hint:
   `strcmp()` returns `0` if the strings are the _same_!

   > Note: you can't `switch()` on strings in C since it will compare the string
   > pointer values instead of the string contents. You have to use an
   > `if`-`else` block with `strcmp()` to get the job done.

   If you can't find an appropriate handler, call `resp_404()` instead to give
   them a "404 Not Found" response.

2. Implement the `get_root()` handler. This will call `send_response()`.

   See above at the beginning of the assignment for what `get_root()` should
   pass to `send_response()`.

   If you need a hint as to what the `send_response()` call should look like,
   check out the usage of it in `resp_404()`, just above there.

   > The `fd` variable that is passed widely around to all the functions holds a
   > _file descriptor_. It's just a number use to represent an open
   > communications path. Usually they point to regular files on disk, but in
   > the case it points to an open _socket_ network connection. All of the code
   > to create and use `fd` has been written already, but we still need to pass
   > it around to the points it is used.

3. Implement `send_response()`.

   This needs to build a complete HTTP response with the given parameters. It
   should write the response to the string in the `response` variable.
   
   The total length of the header **and** body should be stored in the
   `response_length` variable so that the `send()` call knows how many bytes to
   send out over the wire.

   See the [HTTP](#http) section above for an example of an HTTP response and
   use that to build your own.

   Hint: `sprintf()` for creating the HTTP response. `strlen()` for computing
   content length. `sprintf()` also returns the total number of bytes in the
   result string, which might be helpful.

   > The HTTP `Content-Length` header only includes the length of the body, not
   > the header. But the `response_length` variable used by `send()` is the
   > total length of both header and body.

4. Implement the `get_d20()` handler. Hint: `srand()` with `time(NULL)`,
   `rand()`.

5. Implement the `get_date()` handler. Hint: `time(NULL)`, `gmtime()`.

### Stretch Goals

Post a file:

1. Implement `find_start_of_body()` to locate the start of the HTTP request body
   (just after the header).

2. Implement the `post_save()` handler. Modify the main loop to pass the body
   into it. Have this handler write the file to disk. Hint: `open()`, `write()`,
   `close()`. `fopen()`, `fwrite()`, and `fclose()` variants can also be used,
   but the former three functions will be slightly more straightforward to use
   in this case.

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

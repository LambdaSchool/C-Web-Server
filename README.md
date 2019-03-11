# A Simple Web Server in C

In this project, we'll finish the implementation of a web server in C.

What you need to write:

* HTTP request parser
* HTTP response builder
* LRU cache
  * Doubly linked list (some functionality provided)
  * Use existing hashtable functionality (below)

* Your code will interface with the existing code. Understanding the existing
  code is an expected part of this challenge.

What's already here:



* `net.h` and `net.c` contain low-level networking code
- `BACKLOG` defines how many pending connections queue will hold
- `void *get_in_addr(struct sockaddr *sa)` gets an internet address, IPv4 or IPv6
- `int get_listener_socket(char *port)` returns the main listening socket, returns -1 or error
1) Looks at local network interfaces and gather list of potential interfaces
2) Loop through list and try to set up a socket on each, break if we have success
3) Error handler for if we have a bad socket
4) Sets up a listener for remote computers to connect to this socket/IP



* `mime.h` and `mime.c` contains functionality for determining the MIME type of a file
- `char *strlower(char *s)` lower cases a string
- `char *mime_type_get(char *filename)` return a MIME type for a given filename



* `file.h` and `file.c` contains handy file-reading code that you may want to utilize, namely the `file_load()` and `file_free()` functions for reading file data and deallocating file data, respectively (or you could just perform these operations manually as well)
- `struct file_data *file_load(char *filename)` loads a file into memory and returns a pointer to the data
1) Gets file size
2) Make sure it's a regular file
3) open the file for reading
4) Allocate that many bytes
5) Read in the entire file
6) Allocate the file data struct
- `void file_free(struct file_data *filedata)` frees memory allocated by file_load()



* `hashtable.h` and `hashtable.c` contain an implementation of a hashtable (this one is a bit more complicated than what you built in the Hashtables sprint)
- Create a hash table of 128 elements and use the default hash function
- `hashtable_put(ht, "some data", &data1);` stores pointers to data in the hash table
- data can be pointers to any type of data
- you can also store a struct
- `hashtable_get(ht, "other data");` retrieves data
- Hash table entry:
```
struct htent {
    void *key;
    int key_size;
    int hashed_key;
    void *data;
};
```
- `void add_entry_count(struct hashtable *ht, int d)` change the entry count, maintain load metrics
- `int default_hashf(void *data, int data_size, int bucket_count)` is default modulo hashing function
- `struct hashtable *hashtable_create(int size, int (*hashf)(void *, int, int))` creates a new hashtable
- `void htent_free(void *htent, void *arg)` frees an htent (struct)
- `void hashtable_destroy(struct hashtable *ht)` destroys a hashtable, does NOT free the data pointer
- `int htcmp(void *a, void *b)` is comparison function for hashtable entries

- `void *hashtable_put(struct hashtable *ht, char *key, void *data)` puts to hash table with a string key
- `void *hashtable_get(struct hashtable *ht, char *key)` gets from the hash table a string key
- `void *hashtable_delete(struct hashtable *ht, char *key)` delete from the hashtable by string key

- `void *hashtable_put_bin(struct hashtable *ht, void *key, int key_size, void *data)` puts to hash table with a binary key
- `void *hashtable_get_bin(struct hashtable *ht, void *key, int key_size)` gets from the hash table a binary key
- `void *hashtable_delete_bin(struct hashtable *ht, void *key, int key_size)` delete from the hashtable by binary key

- `void hashtable_foreach(struct hashtable *ht, void (*f)(void *, void *), void *arg)` for each element in hashtable
- elements are returned in effectively random order
- `void foreach_callback(void *vent, void *vpayload)` forEach callback function



* `llist.h` and `llist.c` contain an implementation of a doubly-linked list (used solely by the hashable--you don't need it)
- `struct llist *llist_create(void)` allocates a new linked list
- `void llist_destroy(struct llist *llist)` destroys a linked list, does not deallocate the data in each node

- `int llist_count(struct llist *llist)` return the number of elements in the list
- `void llist_foreach(struct llist *llist, void (*f)(void *, void *), void *arg)` for each item in the list run a function

- `void *llist_insert(struct llist *llist, void *data)` insert at the head of a linked list
- `void *llist_append(struct llist *llist, void *data)` appends to the end of a list
- cmpfn should return 0 if the comparison to this node's data is equal
- `void *llist_delete(struct llist *llist, void *data, int (*cmpfn)(void *, void *))` delete an element in the list

- `void *llist_find(struct llist *llist, void *data, int (*cmpfn)(void *, void *))` find an element in the list
- `void *llist_head(struct llist *llist)` returns the first element in a list
- `void *llist_tail(struct llist *llist)` returns the last element in a list
- cmpfn should return 0 if the comparison to this node's data is equal
- does *not* free the data--it merely returns a pointer to it

- `void **llist_array_get(struct llist *llist)` allocates and returns a new NULL-terminated array of pointers to data elements in the list.
- `void llist_array_free(void **a)` Free an array allocated with llist_array_get()
- this does not modify the linked list or its data in any way



* `cache.h` and `cache.c` are where you will implement the LRU cache functionality for days 3 and 4
- `struct cache_entry *alloc_entry(char *path, char *content_type, void *content, int content_length)` allocates a cache entry
- `void free_entry(struct cache_entry *entry)` deallocates a cache entry

- `void dllist_insert_head(struct cache *cache, struct cache_entry *ce)` insert a cache entry at the head of the linked list
- `void dllist_move_to_head(struct cache *cache, struct cache_entry *ce)` moves a cache entry to the head of the list
- `struct cache_entry *dllist_remove_tail(struct cache *cache)` removes the tail from the list and returns it
- does not deallocate the tail

- `struct cache *cache_create(int max_size, int hashsize)` create a new cache
- max_size: maximum number of entries in the cache
- hashsize: hashtable size (0 for default)
- `void cache_free(struct cache *cache)`

- `void cache_put(struct cache *cache, char *path, char *content_type, void *content, int content_length)` stores an entry in the cache
- `struct cache_entry *cache_get(struct cache *cache, char *path)` retrieves an entry in the cache



## What is a Web Server?

A web server is a piece of software that accepts HTTP requests (e.g. GET
requests for HTML pages), and returns responses (e.g. HTML pages). Other common uses are GET requests for getting data from RESTful API endpoints, images within web pages, and POST requests to upload data to the server (e.g. a form submission or file upload).

## Reading

* [Networking Background](guides/net.md)
* [Doubly-Linked Lists](guides/dllist.md)
* [LRU Caches](guides/lrucache.md)
* [MIME types](guides/mime.md)

## Assignment

We will write a simple web server that returns files and some specialized data on a certain endpoint.

* `http://localhost:3490/d20` should return a random number between 1 and 20 inclusive as `text/plain` data.
* Any other URL should map to the `serverroot` directory and files that lie within. For example:

  ```
  http://localhost:3490/index.html
  ```

  should serve the file

  ```
  ./serverroot/index.html
  ```

Examine the skeleton source code in `server.c` and `cache.c` for which pieces you'll need to implement.

**IMPORTANT** _Spend some time inventorying the code to see what is where. Write down notes. Write an outline. Note which functions call which other functions. Time spent up front doing this will reduce overall time spent down the road._

_The existing code is all one big hint on how to attack the problem._

For the portions that are already written, study the moderately-well-commented code to see how it works.

There is a `Makefile` provided. On the command line, type `make` to build the server.

Type `./server` to run the server.

### Main Goals

_Read through all the main and stretch goals before writing any code to get an overall view, then come back to goal #1 and dig in._

#### Days 1 and 2

1. Implement `send_response()`.

   This function is responsible for formatting all the pieces that make up an HTTP response into the proper format that clients expect. In other words, it needs to build a complete HTTP response with the given parameters. It should write the response to the string in the `response` variable.
   
   The total length of the header **and** body should be stored in the `response_length` variable so that the `send()` call knows how many bytes to
   send out over the wire.

   See the [HTTP](#http) section above for an example of an HTTP response and use that to build your own.

   Hint: `sprintf()` for creating the HTTP response. `strlen()` for computing
   content length. `sprintf()` also returns the total number of bytes in the
   result string, which might be helpful. For getting the current time for the Date field of the response, you'll want to look at the `time()` and `localtime()` functions, both of which are already included in the `time.h` header file.

   > The HTTP `Content-Length` header only includes the length of the body, not
   > the header. But the `response_length` variable used by `send()` is the
   > total length of both header and body.

   You can test whether you've gotten `send_response` working by calling the `resp_404` function from somewhere inside the `main` function, and seeing if the client receives the 404 response. 

2. Examine `handle_http_request()` in the file `server.c`.

   You'll want to parse the first line of the HTTP request header to see if this is a `GET` or `POST` request, and to see what the path is. You'll use this information to decide which handler function to call.

   The variable `request` in `handle_http_request()` holds the entire HTTP request once the `recv()` call returns.

   Read the three components from the first line of the HTTP header. Hint: `sscanf()`.

   Right after that, call the appropriate handler based on the request type
   (`GET`, `POST`) and the path (`/d20` or other file path.) You can start by
   just checking for `/d20` and then add arbitrary files later.

   Hint: `strcmp()` for matching the request method and path. Another hint:
   `strcmp()` returns `0` if the strings are the _same_!

   > Note: you can't `switch()` on strings in C since it will compare the string pointer values instead of the string contents. You have to use an
   > `if`-`else` block with `strcmp()` to get the job done.

   If you can't find an appropriate handler, call `resp_404()` instead to give
   them a "404 Not Found" response.

3. Implement the `get_d20()` handler. This will call `send_response()`.

   See above at the beginning of the assignment for what `get_d20()` should pass to `send_response()`.

   If you need a hint as to what the `send_response()` call should look like, check out the usage of it in `resp_404()`, just above there.

   Note that unlike the other responses that send back file contents, the `d20` endpoint will simply compute a random number and send it back. It does not read the number from a file.

   > The `fd` variable that is passed widely around to all the functions holds a _file descriptor_. It's just a number use to represent an open
   > communications path. Usually they point to regular files on disk, but in
   > this case it points to an open _socket_ network connection. All of the code to create and use `fd` has been written already, but we still need to pass it around to the points it is used.

4. Implement arbitrary file serving.

   Any other URL should map to the `serverroot` directory and files that lie within. For example:

   `http://localhost:3490/index.html` serves file `./serverroot/index.html`.

   `http://localhost:3490/foo/bar/baz.html` serves file
   `./serverroot/foo/bar/baz.html`.

   You might make use of the functionality in `file.c` to make this happen.

   You also need to set the `Content-Type` header depending on what data is in
   the file. `mime.c` has useful functionality for this.

#### Days 3 and 4

Implement an LRU cache. This will be used to cache files in RAM so you don't have to load them through the OS.

When a file is requested, the cache should be checked to see if it is there. If so, the file is served from the cache. If not, the file is loaded from disk, served, and saved to the cache.

The cache has a maximum number of entries. If it has more entries than the max, the least-recently used entries are discarded.

The cache consists of a [doubly-linked
list](https://en.wikipedia.org/wiki/Doubly_linked_list) and a
[hash table](https://en.wikipedia.org/wiki/Hash_table).

The hashtable code is already written and can be found in `hashtable.c`.

1. Implement `cache_put()` in `cache.c`.

   Algorithm:

   * Allocate a new cache entry with the passed parameters.
   * Insert the entry at the head of the doubly-linked list.
   * Store the entry in the hashtable as well, indexed by the entry's `path`.
   * Increment the current size of the cache.
   * If the cache size is greater than the max size:
     * Remove the cache entry at the tail of the linked list.
     * Remove that same entry from the hashtable, using the entry's `path` and the `hashtable_delete` function.
     * Free the cache entry.
     * Ensure the size counter for the number of entries in the cache is correct.

2. Implement `cache_get()` in `cache.c`.

   Algorithm:

   * Attempt to find the cache entry pointer by `path` in the hash table.
   * If not found, return `NULL`.
   * Move the cache entry to the head of the doubly-linked list.
   * Return the cache entry pointer.

3. Add caching functionality to `server.c`.

   When a file is requested, first check to see if the path to the file is in
   the cache (use the file path as the key).

   If it's there, serve it back.

   If it's not there:

   * Load the file from disk (see `file.c`)
   * Store it in the cache
   * Serve it

There's a set of unit tests included to ensure that your cache implementation is functioning correctly. From the `src` directory, run `make tests` in order to run the unit tests against your implementation. 

### Stretch Goals

#### 1. Post a file

1. Implement `find_start_of_body()` to locate the start of the HTTP request body
   (just after the header).

2. Implement the `post_save()` handler. Modify the main loop to pass the body
   into it. Have this handler write the file to disk. Hint: `open()`, `write()`,
   `close()`. `fopen()`, `fwrite()`, and `fclose()` variants can also be used,
   but the former three functions will be slightly more straightforward to use
   in this case.

   The response from `post_save()` should be of type `application/json` and
   should be `{"status":"ok"}`.

#### 2. Automatic `index.html` serving

We know that if the user hits `http://localhost:3490/index.html` it should
return the file at `./serverroot/index.html`.

Make it so that if the user hits `http://localhost:3490/` (which is endpoint
`/`, on disk `./serverroot/`), if no file is found there, try adding an
`index.html` to the end of the path and trying again.

So `http://localhost:3490/` would first try:

```
./serverroot/
```

fail to find a file there, then try:

```
./serverroot/index.html
```

and succeed.

#### 3. Expire cache entries

It doesn't make sense to cache things forever--what if the file changes on disk?

Add a `created_at` timestamp to cache entries.

If an item is found in the cache, check to see if it is more than 1 minute old. If it is, delete it from the cache, then load the new one from disk as if it weren't found.

You'll have to add a `cache_delete` function to your cache code that does the work of actually removing entries that are too old from the cache.

#### 4. Concurrency

_Difficulty: Pretty Dang Tough_

Research the pthreads library.

When a new connection comes in, launch a thread to handle it.

Be sure to lock the cache when a thread accesses it so the threads don't step on each other's toes and corrupt the cache.

Also have thread cleanup handlers to handle threads that have died.

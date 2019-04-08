# Notes about Project

## Send response

-   build complete response with the params into the response variable
-   total length of the header and body is stored in response_length, so send() knows, this is not the same as the content-length
-   sprintf() for creating response, also returns the total number of bytes is string
-   strlen for content length
-   look at time() & localtime() for the date (already included in time.h)
-   to make sure working, call resp_404 in the main

## handle_http_request

-   Parse first line to check for get or post request
-   request holds the entire HTTP request
-   sscanf help with the three components
-   strcmp() for matching the method and path (Return 0 if the same)
-   Have to do if-else statements
-   If not handler call resp_404()

## arbitrary file serving

-   Will need to add more to handle_http_request
-   Will be changing get_file
-   Will use resp_404 for help
-   Fetch file = snprintf(filepath, sizeof filepath, "%s/index.html", SERVER_ROOT);

# Cache

## Plan

-   Create the cache - max-size 10
-   create cache_entry
-   Implememnt the cache in get_file
-   Create put and counter parts

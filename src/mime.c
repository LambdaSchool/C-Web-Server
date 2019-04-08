#include <string.h>
#include <ctype.h>
#include "mime.h"

#define DEFAULT_MIME_TYPE "application/octet-stream"

/**
 * Lowercase a string
 */
char *strlower(char *s)
{
    for (char *p = s; *p != '\0'; p++)
    {
        *p = tolower(*p);
    }

    return s;
}

/**
 * Return a MIME type for a given filename
 */
char *mime_type_get(char *filename)
{
    char *ext = strrchr(filename, '.');
    if (ext == NULL)
    {
        return DEFAULT_MIME_TYPE;
    }

    ext++;

    strlower(ext);

    // TODO: this is O(n) and it should be O(1)

    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0)
    {
        return "text/html";
    }
    if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0)
    {
        return "image/jpg";
    }
    if (strcmp(ext, "css") == 0)
    {
        return "text/css";
    }
    if (strcmp(ext, "js") == 0)
    {
        return "application/javascript";
    }
    if (strcmp(ext, "json") == 0)
    {
        return "application/json";
    }
    if (strcmp(ext, "txt") == 0)
    {
        return "text/plain";
    }
    if (strcmp(ext, "gif") == 0)
    {
        return "image/gif";
    }
    if (strcmp(ext, "png") == 0)
    {
        return "image/png";
    }

    return DEFAULT_MIME_TYPE;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"
#include "cache.h"

void cache_free(struct cache *cache)
{
    struct cache_entry *cur_entry = cache->head;

    hashtable_destroy(cache->index);

    while (cur_entry != NULL) {
        struct cache_entry *next_entry = cur_entry->next;

        free_entry(cur_entry);

        cur_entry = next_entry;
    }
}
/**
 * Allocate a cache entry
 */
struct cache_entry *alloc_entry(char *path, char *content_type, void *content, int content_length)
{
    struct cache_entry *ce = malloc(sizeof(struct cache_entry));
    ce->path = path;
    ce->content_length = content_length;
    ce->content_type = content_type;
    ce->content = content;
    return ce;
}

/**
 * Deallocate a cache entry
 */
void free_entry(struct cache_entry *entry)
{
    if(entry != NULL) {

        free(entry);
    }
}

/**
 * Insert a cache entry at the head of the linked list
 */
void dllist_insert_head(struct cache *cache, struct cache_entry *ce)
{
    // Insert at the head of the list
    if (cache->head == NULL) {
        cache->head = cache->tail = ce;
        ce->prev = ce->next = NULL;
    } else {
        cache->head->prev = ce;
        ce->next = cache->head;
        ce->prev = NULL;
        cache->head = ce;
    }
}

/**
 * Move a cache entry to the head of the list
 */
void dllist_move_to_head(struct cache *cache, struct cache_entry *ce)
{
    if (ce != cache->head) {
        if (ce == cache->tail) {
            // We're the tail
            cache->tail = ce->prev;
            cache->tail->next = NULL;

        } else {
            // We're neither the head nor the tail
            ce->prev->next = ce->next;
            ce->next->prev = ce->prev;
        }

        ce->next = cache->head;
        cache->head->prev = ce;
        ce->prev = NULL;
        cache->head = ce;
    }
}


/**
 * Removes the tail from the list and returns it
 * 
 * NOTE: does not deallocate the tail
 */
struct cache_entry *dllist_remove_tail(struct cache *cache)
{
    struct cache_entry *oldtail = cache->tail;
    cache->tail = oldtail->prev;
    cache->tail->next = NULL;

    cache->cur_size--;

    return oldtail;
}

/**
 * Create a new cache
 * 
 * max_size: maximum number of entries in the cache
 * hashsize: hashtable size (0 for default)
 */
struct cache *cache_create(int max_size, int hashsize)
{
    struct hashtable *ht = hashtable_create(hashsize, NULL);
    struct cache *cache_inst = malloc(sizeof(struct cache));
    cache_inst->head = NULL;
    cache_inst->tail = NULL;
    cache_inst->cur_size = 0;
    cache_inst->max_size = max_size;
    cache_inst->index = ht;
    return cache_inst;
}

void cache_free(struct cache *cache)
{
    struct cache_entry *cur_entry = cache->head;

    hashtable_destroy(cache->index);

    while (cur_entry != NULL) {
        struct cache_entry *next_entry = cur_entry->next;

        free_entry(cur_entry);

        cur_entry = next_entry;
    }

    free(cache);
}

/**
 * Store an entry in the cache
 *
 * This will also remove the least-recently-used items as necessary.
 * 
 * NOTE: doesn't check for duplicate cache entries
 */
void cache_put(struct cache *cache, char *path, char *content_type, void *content, int content_length)
{
   struct cache_entry *entry = alloc_entry(path, content_type, content, content_length);

   dllist_insert_head(cache, entry);

   hashtable_put(cache->index, entry->path, entry);

   // Increment the current size of the cache
   cache->cur_size++;

    // if the cache size is greater than the max size

    if (cache->cur_size > cache->max_size) {
        // Remove the entry from the hashtable, using the entry's path and the hashtable_delete function.

        hashtable_delete(cache->index, cache->tail->path);
        // Remove the cache entry at the tail of the linked list
        // Free the cache entry
        free_entry(dllist_remove_tail(cache));
        
        // Ensure the size counter for the number of entries in the cache is correct.
        if (cache->cur_size > cache->max_size) {
            printf("Cache's current size is bigger than its capacity.");
        }
    }
}

/**
 * Retrieve an entry from the cache
 */
struct cache_entry *cache_get(struct cache *cache, char *path)
{
    struct cache_entry *entry = hashtable_get(cache->index, path);
    if (entry != NULL) {
        dllist_move_to_head(cache, entry);
    }
    else {
        return NULL;
    }
}

#include <stdlib.h>
#include "hash_table.h"
#include <string.h>

HashTable *ht_create(int capacity)
{

    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));

    // init int fields
    ht->capacity = capacity;
    ht->count = 0;

    // init array
    ht->entries = (Entry *)malloc(sizeof(Entry) * capacity);

    for (int i = 0; i < capacity; i++)
    {
        ht->entries[i].key = NULL;
        ht->entries[i].value = NULL;
    }

    return ht;
}

void ht_free(HashTable *ht)
{
    for (int i = 0; i < ht->capacity; i++)
    {
        if (ht->entries[i].key != NULL)
        {
            free(ht->entries[i].key);
            free(ht->entries[i].value);
        }
    }
    free(ht->entries);
    free(ht);
}

int ht_hash(char *key, int capacity)
{
    int hash = 0;
    for (int i = 0; key[i] != '\0'; i++)
    {
        hash += key[i]; // add ASCII value of each character
    }
    return hash % capacity; // mod by capacity to get index
}

void ht_resize(HashTable *ht)
{
    int old_capacity = ht->capacity;
    Entry *old_entries = ht->entries;

    int new_capacity = old_capacity * 2;
    Entry *new_entries = (Entry *)malloc(sizeof(Entry) * new_capacity);

    for (int i = 0; i < new_capacity; i++)
    {
        new_entries[i].key = NULL;
        new_entries[i].value = NULL;
    }

    ht->entries = new_entries;
    ht->capacity = new_capacity;
    ht->count = 0;

    // TODO: loop through old_entries, re-insert each non-empty, non-tombstone entry using ht_insert
    for (int i = 0; i < old_capacity; i++)
    {
        if (old_entries[i].key != NULL && strcmp(old_entries[i].key, "") != 0)
        {
            ht_insert(ht, old_entries[i].key, old_entries[i].value);
        }
    }
    for (int i = 0; i < old_capacity; i++)
    {
        free(old_entries[i].key);
        free(old_entries[i].value);
    }
    free(old_entries);
}
int get_index_from_key(HashTable *ht, char *key)
{
    int index = ht_hash(key, ht->capacity);
    int initial = index;
    while (ht->entries[index].key != NULL && strcmp(ht->entries[index].key, key) != 0)
    {
        if (index < ht->capacity - 1)
        {
            index++;
        }
        else
        {
            index = 0;
        }
        if (index == initial || ht->entries[index].key == NULL)
        {
            break;
        }
    }
    if (ht->entries[index].key == NULL || strcmp(ht->entries[index].key, key) != 0)
    {
        return -1;
    }
    return index;
}

void ht_insert(HashTable *ht, char *key, char *value)
{
    int index = get_index_from_key(ht, key);

    if (index != -1)
    {
        // key already exists — update value only
        free(ht->entries[index].value);
        ht->entries[index].value = strdup(value);
        return;
    }

    // check load factor before inserting a NEW key
    if ((float)(ht->count + 1) / ht->capacity > 0.7)
    {
        ht_resize(ht);
    }

    // key doesn't exist — find empty/tombstone slot
    index = ht_hash(key, ht->capacity);
    while (ht->entries[index].key != NULL && strcmp(ht->entries[index].key, "") != 0)
    {
        if (index == ht->capacity - 1)
        {
            index = 0;
        }
        else
        {
            index++;
        }
    }

    if (ht->entries[index].key != NULL)
    {
        free(ht->entries[index].key);
    }
    ht->entries[index].key = strdup(key);
    ht->entries[index].value = strdup(value);
    ht->count++;
}

char *ht_get(HashTable *ht, char *key)
{
    int index = get_index_from_key(ht, key);
    if (index != -1)
    {
        return ht->entries[index].value;
    }
    else
    {
        return NULL;
    }
}

void ht_delete(HashTable *ht, char *key)
{
    int index = get_index_from_key(ht, key);
    if (index != -1)
    {

        free(ht->entries[index].key);
        free(ht->entries[index].value);
        ht->entries[index].key = strdup("");
        ht->entries[index].value = NULL;
        ht->count--;
    }
}

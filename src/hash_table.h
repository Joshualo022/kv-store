#ifndef HASH_TABLE_H
#define HASH_TABLE_H

typedef struct
{
    char *key;
    char *value;
} Entry;

typedef struct
{
    Entry *entries;
    int capacity;
    int count;
} HashTable;

HashTable *ht_create(int capacity);
void ht_insert(HashTable *ht, char *key, char *value);
char *ht_get(HashTable *ht, char *key);
void ht_delete(HashTable *ht, char *key);
void ht_free(HashTable *ht);

#endif
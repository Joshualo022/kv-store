#include <stdio.h>
#include "hash_table.h"

int main()
{
    HashTable *ht = ht_create(10);

    ht_insert(ht, "name", "Josh");
    ht_insert(ht, "language", "C");

    printf("name: %s\n", ht_get(ht, "name"));
    printf("language: %s\n", ht_get(ht, "language"));
    printf("missing key: %s\n", ht_get(ht, "nothere"));

    ht_delete(ht, "name");
    printf("after delete: %s\n", ht_get(ht, "name"));

    ht_free(ht);
    return 0;
}

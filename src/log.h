#ifndef LOG_H
#define LOG_H

#include "hash_table.h"

void log_command(char *command, char *key, char *value);
void replay_log(HashTable *ht);

#endif
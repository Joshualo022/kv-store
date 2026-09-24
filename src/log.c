#include "hash_table.h"
#include <stdio.h>
#include <string.h>
#include "protocol.h"

static FILE *log_file = NULL;

void log_command(char *command, char *key, char *value)
{
    if (log_file == NULL)
    {
        log_file = fopen("kvstore.log", "a");
        if (log_file == NULL)
            return;
    }

    if (value)
    {
        fprintf(log_file, "%s %s %s\n", command, key, value);
    }
    else
    {
        fprintf(log_file, "%s %s\n", command, key);
    }

    fflush(log_file); // force write to disk without closing
}
void replay_log(HashTable *ht)
{
    char line[1024];
    FILE *file = fopen("kvstore.log", "r");
    if (file == NULL)
    {
        return;
    }
    while (fgets(line, sizeof(line), file) != NULL)
    {
        line[strcspn(line, "\n")] = '\0';
        Command *cmd = parse_command(line);
        if (strcmp(cmd->command, "SET") == 0)
        {
            ht_insert(ht, cmd->key, cmd->value);
        }
        else if (strcmp(cmd->command, "DEL") == 0)
        {
            ht_delete(ht, cmd->key);
        }

        free_command(cmd);
    }

    fclose(file);
}
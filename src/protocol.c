#include <stdlib.h>
#include <string.h>
#include "protocol.h"

Command *parse_command(char *input)
{
    char *copy = strdup(input);

    char *header = strtok(copy, " ");
    char *key = strtok(NULL, " ");
    char *value = strtok(NULL, " ");

    Command *new_command = malloc(sizeof(Command));
    new_command->command = strdup(header);
    new_command->key = strdup(key);
    new_command->value = value ? strdup(value) : NULL;

    free(copy);

    return new_command;
}

void free_command(Command *cmd)
{
    if (cmd)
    {
        free(cmd->command);
        free(cmd->key);
        free(cmd->value);
        free(cmd);
    }
}
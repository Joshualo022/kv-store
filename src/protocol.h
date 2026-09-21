#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef struct
{
    char *command; // "SET", "GET", "DEL"
    char *key;
    char *value;
} Command;

Command *parse_command(char *input);
void free_command(Command *cmd);

#endif
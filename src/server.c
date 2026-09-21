#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "protocol.h"
#include "hash_table.h"
#include <string.h>
#include "log.h"
#pragma comment(lib, "Ws2_32.lib")

void start_server(int port)
{
    // Initialize Winsock
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    // Create hash table
    HashTable *ht = ht_create(16);
    replay_log(ht);

    // Create socket
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Bind to port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    // Listen for connections
    listen(server_socket, 1);
    printf("Server listening on port %d\n", port);

    // Accept connection
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);
    SOCKET client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

    printf("Client connected\n");

    // read / write

    while (1)
    {

        char buffer[1024];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0)
        {
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Raw buffer: [%s]\n", buffer);

        Command *cmd = parse_command(buffer);
        printf("Parsed command: [%s] key=[%s] value=[%s]\n", cmd->command, cmd->key, cmd->value ? cmd->value : "NULL");

        if (strcmp(cmd->command, "SET") == 0)
        {
            ht_insert(ht, cmd->key, cmd->value);
            log_command("SET", cmd->key, cmd->value);
            send(client_socket, "SET operation OK\n", strlen("SET operation OK\n"), 0);
        }

        else if (strcmp(cmd->command, "GET") == 0)
        {
            char *value = ht_get(ht, cmd->key);
            if (value)
            {
                char response[1024];
                snprintf(response, sizeof(response), "%s\n", value);

                send(client_socket, response, strlen(response), 0);
            }
            else
            {
                send(client_socket, "Not found\n", 10, 0);
            }
        }
        else if (strcmp(cmd->command, "DEL") == 0)
        {
            ht_delete(ht, cmd->key);
            log_command("DEL", cmd->key, cmd->value);
            send(client_socket, "DEL operation OK\n", strlen("DEL operation OK\n"), 0);
        }

        free_command(cmd);
    }

    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
}
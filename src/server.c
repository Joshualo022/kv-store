#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "protocol.h"
#include "hash_table.h"
#include <string.h>
#include "log.h"
#pragma comment(lib, "Ws2_32.lib")

void send_response(SOCKET client_socket, char *message)
{
    int length = strlen(message);
    unsigned char length_bytes[4];
    length_bytes[0] = (length >> 24) & 0xFF;
    length_bytes[1] = (length >> 16) & 0xFF;
    length_bytes[2] = (length >> 8) & 0xFF;
    length_bytes[3] = length & 0xFF;

    send(client_socket, (char *)length_bytes, 4, 0);
    send(client_socket, message, length, 0);
}

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

        unsigned char length_bytes[4];
        int received = recv(client_socket, (char *)length_bytes, 4, 0);
        if (received <= 0)
            break; // client disconnected
        int message_length = (length_bytes[0] << 24) | (length_bytes[1] << 16) | (length_bytes[2] << 8) | length_bytes[3];

        char buffer[1024];
        int total_read = 0;
        while (total_read < message_length)
        {
            int n = recv(client_socket, buffer + total_read, message_length - total_read, 0);
            if (n <= 0)
            {
                break;
            }
            total_read += n;
        }

        buffer[message_length] = '\0';
        printf("Raw buffer: [%s]\n", buffer);

        Command *cmd = parse_command(buffer);
        printf("Parsed command: [%s] key=[%s] value=[%s]\n", cmd->command, cmd->key, cmd->value ? cmd->value : "NULL");

        if (strcmp(cmd->command, "SET") == 0)
        {
            ht_insert(ht, cmd->key, cmd->value);
            log_command("SET", cmd->key, cmd->value);
            send_response(client_socket, "SET operation OK");
        }

        else if (strcmp(cmd->command, "GET") == 0)
        {
            char *value = ht_get(ht, cmd->key);
            if (value)
            {
                send_response(client_socket, value);
            }
            else
            {
                send_response(client_socket, "Not found");
            }
        }
        else if (strcmp(cmd->command, "DEL") == 0)
        {
            ht_delete(ht, cmd->key);
            log_command("DEL", cmd->key, cmd->value);
            send_response(client_socket, "DEL operation OK");
        }

        free_command(cmd);
    }

    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
}
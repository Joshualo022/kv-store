#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include "protocol.h"
#include "hash_table.h"
#include <string.h>
#include "log.h"

typedef struct
{
    SOCKET client_socket;
    HashTable *ht;
} ClientArgs;

CRITICAL_SECTION ht_lock;

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

DWORD WINAPI handle_client(LPVOID arg)
{
    ClientArgs *args = (ClientArgs *)arg;
    SOCKET client_socket = args->client_socket;
    HashTable *ht = args->ht;
    free(args);

    while (1)
    {

        unsigned char length_bytes[4];
        int received = recv(client_socket, (char *)length_bytes, 4, 0);
        if (received <= 0)
            break; // client disconnected
        int message_length = (length_bytes[0] << 24) | (length_bytes[1] << 16) | (length_bytes[2] << 8) | length_bytes[3];
        char buffer[1024];
        if (message_length <= 0 || message_length >= sizeof(buffer))
        {
            break;
        }

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
        if (cmd->command == NULL)
        {
            send_response(client_socket, "ERROR: Empty command");
            free(cmd);
            continue;
        }
        printf("Parsed command: [%s] key=[%s] value=[%s]\n", cmd->command, cmd->key ? cmd->key : "NULL", cmd->value ? cmd->value : "NULL");

        if (strcmp(cmd->command, "SET") == 0)
        {

            if (cmd->value == NULL || cmd->key == NULL)
            {
                send_response(client_socket, "ERROR: SET requires a key and value");
            }
            else
            {
                EnterCriticalSection(&ht_lock);
                ht_insert(ht, cmd->key, cmd->value);
                log_command("SET", cmd->key, cmd->value);
                LeaveCriticalSection(&ht_lock);
                send_response(client_socket, "SET operation OK");
            }
        }

        else if (strcmp(cmd->command, "GET") == 0)
        {
            if (cmd->key == NULL)
            {
                send_response(client_socket, "ERROR: GET requires key");
            }
            else
            {
                EnterCriticalSection(&ht_lock);
                char *value = ht_get(ht, cmd->key);
                char *copy = value ? strdup(value) : NULL;
                LeaveCriticalSection(&ht_lock);
                if (copy)
                {

                    send_response(client_socket, copy);
                }
                else
                {
                    send_response(client_socket, "Not found");
                }
                free(copy);
            }
        }
        else if (strcmp(cmd->command, "DEL") == 0)
        {

            if (cmd->key == NULL)
            {
                send_response(client_socket, "ERROR: DEL requires key");
            }

            else
            {
                EnterCriticalSection(&ht_lock);
                ht_delete(ht, cmd->key);
                log_command("DEL", cmd->key, cmd->value);
                LeaveCriticalSection(&ht_lock);
                send_response(client_socket, "DEL operation OK");
            }
        }
        else
        {
            send_response(client_socket, "ERROR: Unknown Command");
        }

        free_command(cmd);
    }

    closesocket(client_socket);
    return 0;
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
    listen(server_socket, 5);
    printf("Server listening on port %d\n", port);

    InitializeCriticalSection(&ht_lock);

    while (1) // outer loop: keep accepting new clients forever
    {
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);

        printf("Client connected\n");

        ClientArgs *args = (ClientArgs *)malloc(sizeof(ClientArgs));
        args->client_socket = client_socket;
        args->ht = ht;

        HANDLE thread = CreateThread(NULL, 0, handle_client, args, 0, NULL);
        CloseHandle(thread); // we don't need to track the thread handle, let it run independently
    }

    closesocket(server_socket);
    WSACleanup();
}
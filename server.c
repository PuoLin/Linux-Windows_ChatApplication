#include "chat_header.h"

#define MAX_CLIENTS 10

typedef struct {
    SOCKET socketID;
    char username[32];
} ClientNode;

ClientNode activeClients[MAX_CLIENTS] = {0}; 

int main() {
    initSockets();
    
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);
    
    printf("--- WhatsChat Server ---\n");
    printf("Listening for connections on port %d...\n", PORT);

    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(serverSocket, &readfds);
        SOCKET max_sd = serverSocket;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            SOCKET sd = activeClients[i].socketID;
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0) printf("select() error\n");

        if (FD_ISSET(serverSocket, &readfds)) {
            struct sockaddr_in clientAddr;
            socklen_t clientSize = sizeof(clientAddr);
            SOCKET new_socket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientSize);
            
            ChatPacket regPacket;
            int bytesRecv = recv(new_socket, (char*)&regPacket, sizeof(ChatPacket), 0);
            
            if (bytesRecv > 0 && regPacket.type == 0) {
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (activeClients[i].socketID == 0) {
                        activeClients[i].socketID = new_socket;
                        strcpy(activeClients[i].username, regPacket.sender);
                        printf("Registered user '%s' at index %d\n", activeClients[i].username, i);
                        break;
                    }
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            SOCKET sd = activeClients[i].socketID;
            
            if (sd > 0 && FD_ISSET(sd, &readfds)) {
                ChatPacket incomingPacket;
                int bytesReceived = recv(sd, (char*)&incomingPacket, sizeof(ChatPacket), 0);

                if (bytesReceived <= 0) {
                    printf("User '%s' disconnected.\n", activeClients[i].username);
                    closeSocket(sd);
                    activeClients[i].socketID = 0;
                    memset(activeClients[i].username, 0, 32); 
                } else {
                    
                    // Work for texts and file
                    if (strcmp(incomingPacket.destination, "ALL") == 0) {
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (activeClients[j].socketID != 0 && activeClients[j].socketID != sd) {
                                send(activeClients[j].socketID, (char*)&incomingPacket, sizeof(ChatPacket), 0);
                            }
                        }
                    } else {
                        int targetFound = 0;
                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (activeClients[j].socketID != 0 && strcmp(activeClients[j].username, incomingPacket.destination) == 0) {
                                send(activeClients[j].socketID, (char*)&incomingPacket, sizeof(ChatPacket), 0);
                                targetFound = 1;
                                break;
                            }
                        }
                        
                        // Only throw an error if a text message that failed to find a user
                        if (!targetFound && incomingPacket.type == 1) {
                            ChatPacket errorNotification;
                            errorNotification.type = 4; 
                            strcpy(errorNotification.sender, "SERVER");
                            sprintf(errorNotification.message, "User '%s' is offline.", incomingPacket.destination);
                            send(sd, (char*)&errorNotification, sizeof(ChatPacket), 0);
                        }
                    }
                }
            }
        }
    }
    closeSocket(serverSocket);
    return 0;
}

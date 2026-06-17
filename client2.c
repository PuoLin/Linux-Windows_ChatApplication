#include "chat_header.h"

char myUsername[32];

void sendFile(SOCKET s, const char* targetUser, const char* filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Cannot open file %s\n> ", filename);
        return;
    }

    ChatPacket fp;
    memset(&fp, 0, sizeof(ChatPacket));
    strcpy(fp.sender, myUsername);
    strcpy(fp.destination, targetUser);

    //end File Name
    fp.type = 5; 
    strcpy(fp.message, filename);
    send(s, (char*)&fp, sizeof(ChatPacket), 0);
    delay_ms(100);

    //Send the Data
    fp.type = 6; 
    while ((fp.dataLength = fread(fp.message, 1, 512, file)) > 0) {
        send(s, (char*)&fp, sizeof(ChatPacket), 0);
        delay_ms(10);
    }

    //Send EOF Marker
    fp.type = 7; 
    fp.dataLength = 0;
    send(s, (char*)&fp, sizeof(ChatPacket), 0);

    printf("File '%s' sent to %s successfully.\n> ", filename, targetUser);
    fclose(file);
}

#ifdef _WIN32
DWORD WINAPI receiveData(LPVOID arg) {
#else
void* receiveData(void* arg) {
#endif
    SOCKET s = *(SOCKET*)arg;
    ChatPacket inPacket;
    FILE *outFile = NULL;

    while (1) {
        int bytesReceived = recv(s, (char*)&inPacket, sizeof(ChatPacket), 0);
        
        if (bytesReceived <= 0) {
            printf("\nDisconnected from server.\n");
            exit(0);
        }

        // Handle based on its type
        if (inPacket.type == 5) {
            char outname[1024];
            snprintf(outname, sizeof(outname), "recv_%s", inPacket.message);
            outFile = fopen(outname, "wb");
            printf("\n[Incoming file from %s: %s]\n> ", inPacket.sender, inPacket.message);
        } 
        else if (inPacket.type == 6 && outFile != NULL) {
            fwrite(inPacket.message, 1, inPacket.dataLength, outFile);
        } 
        else if (inPacket.type == 7 && outFile != NULL) {
            fclose(outFile);
            outFile = NULL;
            printf("\n[File saved successfully!]\n> ");
        }
        else if (inPacket.type == 4) {
            printf("\a\n\033[33m*** [SERVER]: %s ***\033[0m\n> ", inPacket.message);
        } 
        else if (inPacket.type == 1) {
            printf("\n[Private from %s]: %s\n> ", inPacket.sender, inPacket.message);
        } 
        else if (inPacket.type == 3) {
            printf("\n[%s]: %s\n> ", inPacket.sender, inPacket.message);
        }
        fflush(stdout);
    }
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

int main() {
    initSockets();
    printf("--- Welcome to WhatsChat ---\n");
    
    printf("Enter your username: ");
    fgets(myUsername, sizeof(myUsername), stdin);
    myUsername[strcspn(myUsername, "\n")] = 0;

    char ip[100];
    printf("Enter Server IP Address: ");
    fgets(ip, sizeof(ip), stdin);
    ip[strcspn(ip, "\n")] = 0; 

    SOCKET peerSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);
    serverAddr.sin_port = htons(PORT);

    if (connect(peerSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("Connection failed.\n");
        return 1;
    }
    
    ChatPacket regPacket;
    memset(&regPacket, 0, sizeof(ChatPacket));
    regPacket.type = 0; 
    strcpy(regPacket.sender, myUsername);
    send(peerSocket, (char*)&regPacket, sizeof(ChatPacket), 0);
    
    printf("Connected to server!\n");

#ifdef _WIN32
    HANDLE thread = CreateThread(NULL, 0, receiveData, &peerSocket, 0, NULL);
    CloseHandle(thread);
#else
    pthread_t thread;
    pthread_create(&thread, NULL, receiveData, &peerSocket);
    pthread_detach(thread);
#endif

    char input[512];
    printf("Commands:\n - '/msg <user> <text>' for private chat\n - '/file <user> <filename>' to send a file\n - '/file ALL <filename>' to broadcast a file\n - Or just type to broadcast.\n>");
    
    while (1) {
        if (fgets(input, sizeof(input), stdin) != NULL) {
            input[strcspn(input, "\n")] = 0; 
            
            ChatPacket outPacket;
            memset(&outPacket, 0, sizeof(ChatPacket));
            strcpy(outPacket.sender, myUsername);
            
            if (strncmp(input, "/file ", 6) == 0) {
                char targetUser[32], filename[256];
                if (sscanf(input + 6, "%s %s", targetUser, filename) == 2) {
                    sendFile(peerSocket, targetUser, filename);
                } else {
                    printf("Usage: /file <username> <filename>\n> ");
                }
            }
            else if (strncmp(input, "/msg ", 5) == 0) {
                outPacket.type = 1; 
                sscanf(input + 5, "%s %[^\n]", outPacket.destination, outPacket.message);
                send(peerSocket, (char*)&outPacket, sizeof(ChatPacket), 0);
                printf("> ");
            } 
            else if (strlen(input) > 0) {
                outPacket.type = 3; 
                strcpy(outPacket.destination, "ALL");
                strcpy(outPacket.message, input);
                send(peerSocket, (char*)&outPacket, sizeof(ChatPacket), 0);
                printf("> ");
            }
        }
    }
    closeSocket(peerSocket);
    return 0;
}

#ifndef CHAT_HEADER_H
#define CHAT_HEADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


#if defined(_WIN32)
    #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0600
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h> // Needed for multithreading
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>
    #include <pthread.h> // Needed for multithreading
#endif


#if defined(_WIN32)
    #define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
    #define CLOSESOCKET(s) closesocket(s)
    #define GETSOCKETERRNO() (WSAGetLastError())
#else
    #define ISVALIDSOCKET(s) ((s) >= 0)
    #define CLOSESOCKET(s) close(s)
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define GETSOCKETERRNO() (errno)
#endif

// to onnects the lab's code to my code
#define closeSocket(s) CLOSESOCKET(s)

#define PORT 8080
#define BUFFER_SIZE 4096

typedef struct {
    int type; // 0=Reg, 1=Private, 3=Broadcast, 4=Notif, 5=FileStart, 6=FileData, 7=FileEnd
    char sender[32]; 
    char destination[32];
    char message[512]; 
    int dataLength;    
} ChatPacket;

static inline void delay_ms(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

static inline void initSockets() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        exit(1);
    }
#endif
}

#endif

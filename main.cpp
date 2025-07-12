#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8080"


#pragma comment(lib, "ws2_32.lib")

enum class EServerState {
    Running,
    Stopped
};


DWORD WINAPI ClientHandler(LPVOID lpParam) {
    SOCKET ClientSocket = (SOCKET)lpParam;
    char ReceiveBuffer[DEFAULT_BUFLEN];
    int SendResult{};
    int BytesReceived{};

    //printf ("Client Connected.\n");

    char Nickname[DEFAULT_BUFLEN] = "Anonymous";
    BytesReceived = recv(ClientSocket, Nickname, DEFAULT_BUFLEN, 0);
    if (BytesReceived>0) {
        Nickname[BytesReceived] = '\0';
        printf("%s has joined the chat!\n", Nickname);
    } else {
        printf("Failed to receive nickname. Disconnecting client.");
        closesocket(ClientSocket);
    }

    do {
        BytesReceived = recv(ClientSocket, ReceiveBuffer, DEFAULT_BUFLEN, 0);
        if (BytesReceived > 0) {
            printf("Message received: %.*s\n", BytesReceived, ReceiveBuffer);
            char ResponseMessage[DEFAULT_BUFLEN];
            snprintf(ResponseMessage, DEFAULT_BUFLEN, "%s: %.*s\n",Nickname, BytesReceived, ReceiveBuffer);

            if (strncmp(ReceiveBuffer, "/help", 6) == 0) {
                snprintf(ResponseMessage, DEFAULT_BUFLEN, "Write /quit to disconnect.\n");
            }
            SendResult = send(ClientSocket, ResponseMessage, static_cast<int>(strlen(ResponseMessage)), 0);
            if (SendResult == SOCKET_ERROR) {
                printf("Send failed! %d\n", WSAGetLastError());
                break;
            }
        } else if (BytesReceived == 0) {
            printf("Closing connection...\n");
        } else {
            printf("Error in receiving message: %d\n", WSAGetLastError());
            break;
        }
    } while (BytesReceived > 0);

    closesocket(ClientSocket);
    printf("Client disconnected!");
    return 0;
}

int main(int argc, char* argv[]) {
    WSAData WSAData{};
    SOCKET ListenSocket = INVALID_SOCKET;
    struct addrinfo *result = nullptr, *ptr = nullptr, hints;
    int Result;

    char ReceiveBuffer[DEFAULT_BUFLEN];
    int ReceiveBufferLen = DEFAULT_BUFLEN;
    int SendResult;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    Result = WSAStartup(MAKEWORD(2, 2), &WSAData);
    if (Result == SOCKET_ERROR) {
        printf("Couldn't startup WSA");
        return 1;
    }
    printf("WSA Started up!\n");

    Result = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (Result == SOCKET_ERROR) {
        printf("Error in getaddrinfo %d", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    printf("Resolved local address for server!\n");
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("Error in socket %d", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    printf("Listen socket successfully created!\n");

    Result = bind(ListenSocket, result->ai_addr, result->ai_addrlen);
    if (Result == SOCKET_ERROR) {
        printf("Error in bind %d", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    printf("Successfully bind to socket!\n");

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed: %d", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    printf("Listening setup!\n");

    EServerState ServerState = EServerState::Running;
    while (ServerState == EServerState::Running) {
        //printf("Listening...\n");
        SOCKET ClientSocket = accept(ListenSocket, nullptr, nullptr);
            if (ClientSocket == INVALID_SOCKET) {
                printf("Error in accept %d\n", WSAGetLastError());
                break;
            }
        CreateThread(
            nullptr,
            0,
            ClientHandler,
            (LPVOID)ClientSocket,
            0,
            nullptr);
    }

    closesocket(ListenSocket);
    WSACleanup();
    printf("Server shut down.\n");

    return 0;
}
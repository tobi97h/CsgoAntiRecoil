#include "Listener.h"



void Listener::loop() {

    while (keep_running) {

        SOCKET client_socket = INVALID_SOCKET;
        client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(listen_socket);
            WSACleanup();
        }
        int res;

        // recieve loop
        do {
            // number of message bytes will be send first from state_recognition
            INT16 num_bytes;
            res = recv(client_socket, (char*)&num_bytes, sizeof(INT16), 0);
            if (res > 0) {

                char* recvbuf = new char[num_bytes]();
                res = recv(client_socket, recvbuf, num_bytes, 0);

                if (res > 0) {
                    std::cout << recvbuf << std::endl;

                    this->recoil->set_current(recvbuf);

                    delete[] recvbuf;
                }
                else if (res == 0)
                    printf("Connection closing...\n");
                else {
                    printf("recv failed with error: %d\n", WSAGetLastError());
                    closesocket(client_socket);
                    WSACleanup();
                }
            }
            else if (res == 0)
                printf("Connection closing...\n");
            else {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(client_socket);
                WSACleanup();
            }
        } while (res > 0);
        // shutdown the connection since we're done
        res = shutdown(client_socket, SD_SEND);
        if (res == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(client_socket);
            WSACleanup();
        }

        // cleanup
        closesocket(client_socket);

    }

    // No longer need server socket
    closesocket(listen_socket);


    WSACleanup();

}

Listener::Listener(Recoil* recoil) {

    this->recoil = recoil;

    WSADATA wsaData;
    int iResult;

    struct addrinfo* result = NULL;
    struct addrinfo hints;


    const char* default_port = "5000";
    int iSendResult;


    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, default_port, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
    }

    // Create a SOCKET for connecting to server
    listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listen_socket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
    }

    // Setup the TCP listening socket
    iResult = bind(listen_socket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listen_socket);
        WSACleanup();
    }

    freeaddrinfo(result);

    iResult = listen(listen_socket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();

    }
    thread = std::thread(&Listener::loop, this);

}

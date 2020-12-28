#include "Listener.h"


void handle_res(int res, SOCKET& client_socket) {
    if (res == 0)
        printf("Connection closing...\n");
    else if (res < 0) {
        printf("recv failed with error: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
    }
}

// loop for only one client at a time
void Listener::loop() {

    while (keep_running) {

        // accept the client
        SOCKET client_socket = INVALID_SOCKET;
        client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(listen_socket);
            WSACleanup();
        }

        int res;

        // recieve loop for the currently selected stuff
        do {

            // number of message bytes will be send first from state_recognition
            INT16 num_bytes;
            res = recv(client_socket, (char*)&num_bytes, sizeof(INT16), 0);
            if (res > 0) {

                char* selected = new char[num_bytes]();
                res = recv(client_socket, selected, num_bytes, 0);

                if (res > 0) {
                    this->recoil->set_current(selected);

                    // deallocate after processesing
                    delete[] selected;
                }
                handle_res(res, client_socket);
            }
            handle_res(res, client_socket);
          
        } while (res > 0);

        // shutdown the current connection - listen for new state_recognition
        res = shutdown(client_socket, SD_SEND);
        if (res == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(client_socket);
            WSACleanup();
        }

        // cleanup
        closesocket(client_socket);

    }

    // progmram shutdown, stop listening all together
    closesocket(listen_socket);
    WSACleanup();

}

Listener::Listener(Recoil* recoil) {

    this->recoil = recoil;

    int res;

    // init winsock dll to be used
    WSADATA wsaData;
    res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        fprintf(stderr, "WSAStartup failed with error: %d\n", res);
    }

    // settings
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // get the address
    struct addrinfo* result = NULL;
    res = getaddrinfo(NULL, default_port, &hints, &result);
    if (res != 0) {
        fprintf(stderr, "getaddrinfo failed with error: %d\n", res);
        WSACleanup();
    }

    // create server socket
    listen_socket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listen_socket == INVALID_SOCKET) {
        fprintf(stderr, "socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
    }

    // create the listener
    res = bind(listen_socket, result->ai_addr, (int)result->ai_addrlen);
    if (res == SOCKET_ERROR) {
        fprintf(stderr, "bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listen_socket);
        WSACleanup();
    }

    freeaddrinfo(result);

    // start listing
    res = listen(listen_socket, SOMAXCONN);
    if (res == SOCKET_ERROR) {
        fprintf(stderr, "listen failed with error: %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();

    }

    // start the client handler loop
    thread = std::thread(&Listener::loop, this);

}

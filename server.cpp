#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "27015"
#define BUFF_LEN 512

int main(){

    WSADATA wsaData; // must be initialized in order to call winsock functions 
    int res = 0; // check if there an error occured 

    // Winsock initialization (called to initiate use of WS2_32.dll)
    res = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (res != 0) {
        std::cout << "WSAStartup fail" << std::endl; 
        return 1;
    }

    // Creation of SOCKET object
    struct addrinfo hints; // addrinfo contains sockaddr structure 
    ZeroMemory(&hints, sizeof(hints)); // set all fields to zero 
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // because we use TCP otherwise SOCK_DGRAM for UDP
    hints.ai_protocol = IPPROTO_TCP; // TCP 
    hints.ai_flags = AI_PASSIVE; 

    // define IP and port 
    struct addrinfo *addrResult = nullptr; // pointer to a linked list of addrinfo structs
    res = getaddrinfo(NULL, DEFAULT_PORT, &hints, &addrResult); 
    if (res != 0) {
        std::cout << "getaddrinfo fail" << std::endl;
        WSACleanup();
        return 1;
    }

    // create socket 
    SOCKET listenSock = INVALID_SOCKET;
    listenSock = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (listenSock == INVALID_SOCKET){
        std::cout << "socket error" << std::endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // Setting up listening socket
    res = bind(listenSock, addrResult->ai_addr, addrResult->ai_addrlen);
    if (res == SOCKET_ERROR) {
        std::cout << "socket bind error" << std::endl;
        freeaddrinfo(addrResult);
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(addrResult); // no longer needed 

    // listening for connections 
    if (listen( listenSock, SOMAXCONN ) == SOCKET_ERROR) {
        std::cout << "listen failed" << std::endl;
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    // accepting connection
    SOCKET clientSock = INVALID_SOCKET;
    clientSock = accept(listenSock, NULL, NULL);
     if (clientSock == INVALID_SOCKET) {
        std::cout << "accept failed" << std::endl;
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    // send and receive info 
    char recvBuf[BUFF_LEN];
    int sendRes; 

    // receive and send data until the connection is closed 
    do{
        res = recv(clientSock, recvBuf, BUFF_LEN, 0);
        if (res > 0) {
            std::cout << "Bytes received: " << res << std::endl;

            sendRes = send(clientSock, recvBuf, res, 0); // if no error, return number of bytes sent
            if (sendRes == SOCKET_ERROR) {
                std::cout << "send failed" << std::endl; 
                closesocket(listenSock);
                WSACleanup();
                return 1;
            }
            std::cout << "Bytes sent: " << sendRes << std::endl;
        } else if (res == 0) {
            std::cout << "connection closed" << res << std::endl;
        } else {
            std::cout << "recv failed" << res << std::endl;
            closesocket(clientSock);
            WSACleanup();
            return 1;
        }
    } while (res > 0);

    res = shutdown(clientSock, SD_SEND);
    if (res == SOCKET_ERROR) {
        std::cout << "shutdown failed" << std::endl;
        closesocket(clientSock);
        WSACleanup();
        return 1;
    }

    closesocket(clientSock);
    WSACleanup();

    return 0;
}
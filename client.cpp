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

    // define IP and port 
    struct addrinfo *addrResult = nullptr; // pointer to a linked list of addrinfo structs
    res = getaddrinfo("localhost", DEFAULT_PORT, &hints, &addrResult); 
    if (res != 0) {
        std::cout << "getaddrinfo fail" << std::endl;
        WSACleanup();
        return 1;
    }

    // create socket 
    SOCKET sock = INVALID_SOCKET;
    sock = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (sock == INVALID_SOCKET){
        std::cout << "socket error" << std::endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    // connect to server 
    res = connect(sock, addrResult->ai_addr, (int)addrResult->ai_addrlen);
    if (res == SOCKET_ERROR) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }
    freeaddrinfo(addrResult);
    if (sock == INVALID_SOCKET) {
        std::cout << "Unable to connect" << std::endl;
        WSACleanup();
        return 1;   
    }

    // send and receive info 
    const char *sendBuf = "Test program run";
    char recvBuf[BUFF_LEN];

    // send 
    res = send(sock, sendBuf, (int)strlen(sendBuf), 0); // if no error, return number of bytes sent
    if (res == SOCKET_ERROR) {
        std::cout << "send failed" << std::endl; 
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Bytes sent: " << res << std::endl;

    // shutting down the connection for sending data. it still can be used for receiving data 
    res = shutdown(sock, SD_SEND);
    if (res == SOCKET_ERROR) {
           std::cout << "shutdown failed" << std::endl; 
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    // receive data until the connection is closed 
    do{
        res = recv(sock, recvBuf, BUFF_LEN, 0);
        if (res > 0) {
            std::cout << "Bytes received: " << res << std::endl;
        } else if (res == 0) {
            std::cout << "connection closed" << res << std::endl;
        } else {
            std::cout << "recv failed" << res << std::endl;
        }
    } while (res > 0);

    closesocket(sock);
    WSACleanup();

    return 0;
}
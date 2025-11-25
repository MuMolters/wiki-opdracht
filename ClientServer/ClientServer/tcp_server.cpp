#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#pragma comment(lib, "Ws2_32.lib")

#define PORT 12345

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 1);

    std::cout << "Server wacht op een verbinding..." << std::endl;
    client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);

    std::ofstream outfile("received_data.jpg", std::ios::binary); // Je ontvangt hier image.jpg
    char buffer[4096];
    int bytes_received;
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        outfile.write(buffer, bytes_received);
    }

    outfile.close();
    closesocket(client_socket);
    closesocket(server_fd);
    WSACleanup();
    std::cout << "Bestand ontvangen!" << std::endl;
    return 0;
}

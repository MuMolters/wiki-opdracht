#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#pragma comment(lib, "Ws2_32.lib")

#define PORT 12345

std::string clean_filename(const char* fname, int len) {
    int realLen = 0;
    while (realLen < len && fname[realLen] != '\0') realLen++;
    return std::string(fname, realLen);
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, SOMAXCONN);

    std::cout << "Server wacht op een verbinding..." << std::endl;

    while (true) { // Hoofdloop: steeds nieuwe client accepteren
        SOCKET client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        std::cout << "Nieuwe client verbonden!" << std::endl;
        // Je kunt hier ook per client een extra thread maken indien gewenst

        // Bestanden blijven ontvangen tot verbinding sluit
        while (true) {
            char filename[256] = { 0 };
            int name_len = recv(client_socket, filename, sizeof(filename), 0);
            if (name_len <= 0) break;
            std::string fname = clean_filename(filename, name_len);

            uint32_t filesize;
            int size_recv = recv(client_socket, (char*)&filesize, sizeof(filesize), 0);
            if (size_recv <= 0) break;

            std::ofstream outfile(fname.c_str(), std::ios::binary);
            char buffer[4096];
            uint32_t bytes_received = 0;

            auto start = std::chrono::high_resolution_clock::now();

            while (bytes_received < filesize) {
                int to_read = ((int)sizeof(buffer) < (int)(filesize - bytes_received)) ? (int)sizeof(buffer) : (int)(filesize - bytes_received);
                int chunk = recv(client_socket, buffer, to_read, 0);
                if (chunk <= 0) break;
                outfile.write(buffer, chunk);
                bytes_received += chunk;
            }
            outfile.close();

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> sec = end - start;
            double time_ms = sec.count() * 1000.0;
            double speed_mbps = sec.count() > 0.0001 ? ((double)filesize * 8.0 / 1e6) / sec.count() : 0.0;

            std::cout << std::left << std::setw(30) << fname
                << " time = " << std::setw(8) << std::fixed << std::setprecision(2) << time_ms << " ms"
                << " speed = " << std::setw(8) << std::fixed << std::setprecision(2) << speed_mbps << " Mbps"
                << std::endl;
        }
        std::cout << "Client is losgekoppeld." << std::endl;
        closesocket(client_socket);
    }
    closesocket(server_fd);
    WSACleanup();
    return 0;
}

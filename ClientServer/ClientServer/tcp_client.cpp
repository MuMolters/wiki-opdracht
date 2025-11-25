#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#pragma comment(lib, "Ws2_32.lib")

#define PORT 12345

void send_file(const std::string& filename, SOCKET sock) {
    // Bestandsnaam als 256 bytes sturen
    char fname_msg[256] = { 0 };
    strncpy_s(fname_msg, sizeof(fname_msg), filename.c_str(), _TRUNCATE);
    send(sock, fname_msg, sizeof(fname_msg), 0);
    // Filesize als 4 bytes sturen
    std::ifstream infile(filename.c_str(), std::ios::binary | std::ios::ate);
    if (!infile) {
        std::cerr << "Kan bestand niet openen: " << filename << std::endl;
        return;
    }
    uint32_t filesize = static_cast<uint32_t>(infile.tellg());
    infile.seekg(0);

    send(sock, (char*)&filesize, sizeof(filesize), 0);

    char buffer[4096];
    uint32_t bytes_sent = 0;

    // TIMER START
    auto start = std::chrono::high_resolution_clock::now();

    while (bytes_sent < filesize) {
        int to_send = ((int)sizeof(buffer) < (int)(filesize - bytes_sent)) ? (int)sizeof(buffer) : (int)(filesize - bytes_sent);
        infile.read(buffer, to_send);
        int sent = send(sock, buffer, to_send, 0);
        bytes_sent += sent;
    }

    // TIMER STOP
    auto end = std::chrono::high_resolution_clock::now();
    infile.close();

    std::chrono::duration<double> sec = end - start;
    double time_ms = sec.count() * 1000.0;
    double speed_mbps = 0;
    if (sec.count() > 0.0001)
        speed_mbps = ((double)filesize * 8.0 / 1e6) / sec.count(); // bits naar mbps

    // Netjes afdrukken
    std::cout << std::left << std::setw(30) << filename
        << " time = " << std::setw(8) << std::fixed << std::setprecision(2) << time_ms << " ms"
        << " speed = " << std::setw(8) << std::fixed << std::setprecision(2) << speed_mbps << " Mbps"
        << std::endl;
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "192.168.0.151", &serv_addr.sin_addr); // Server IP hier invullen!

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    // Folder iteratie met Windows API
    std::string folder_path = "C:/Users/janse/Documents/wiki opdracht";
    std::string filter = folder_path + "/*";
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA(filter.c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string filepath = folder_path + "/" + findFileData.cFileName;
                send_file(filepath, sock);
            }
        } while (FindNextFileA(hFind, &findFileData));
        FindClose(hFind);
    }
    else {
        std::cerr << "Kan folder niet openen!" << std::endl;
    }

    closesocket(sock);
    WSACleanup();
    std::cout << "Alle bestanden verstuurd." << std::endl;
    return 0;
}


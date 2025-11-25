#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#pragma comment(lib, "Ws2_32.lib")

#define PORT 12345

void send_file(const std::string& filename, SOCKET sock) {
    std::ifstream infile(filename.c_str(), std::ios::binary); // C++14 compatibel!
    if (!infile) {
        std::cerr << "Kan bestand niet openen: " << filename << std::endl;
        return;
    }
    char buffer[4096];
    while (infile.read(buffer, sizeof(buffer))) {
        send(sock, buffer, sizeof(buffer), 0);
    }
    send(sock, buffer, infile.gcount(), 0);
    infile.close();
    std::cout << "Bestand verstuurd: " << filename << std::endl;
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr); // Andere IP indien nodig

    connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    std::string folder_path = "./testdata"; // Controleer jouw pad!
    std::string filter = folder_path + "/*"; // WinAPI wildcard

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

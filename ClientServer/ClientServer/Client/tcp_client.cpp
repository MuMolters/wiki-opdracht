//tcp_client.cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include hrono>
#include <iomanip>
#pragma comment(lib, "Ws2_32.lib")

#define POORTNUMMER 12345

// Functie: stuurt een enkel bestand naar de server
void bestand_verzenden(const std::string& bestandsnaam, SOCKET socket) {
    // Bestandsnaam als 256 bytes sturen (buffer met nullen)
    char naam_voor_overdracht[256] = { 0 };
    strncpy_s(naam_voor_overdracht, sizeof(naam_voor_overdracht), bestandsnaam.c_str(), _TRUNCATE);
    send(socket, naam_voor_overdracht, sizeof(naam_voor_overdracht), 0);

    // Bepaal grootte van het bestand
    std::ifstream invoer(bestandsnaam.c_str(), std::ios::binary | std::ios::ate);
    if (!invoer) {
        std::cerr << "Kon bestand niet openen: " << bestandsnaam << std::endl;
        return;
    }
    uint32_t bestandsgrootte = static_cast<uint32_t>(invoer.tellg());
    invoer.seekg(0);

    // Verstuur de bestandsgrootte (4 bytes)
    send(socket, (char*)&bestandsgrootte, sizeof(bestandsgrootte), 0);

    char buffer[4096];
    uint32_t totaal_verzonden = 0;

    // Tijdmeting start
    auto tijd_start = std::chrono::high_resolution_clock::now();

    // Versturen in delen zolang er data is
    while (totaal_verzonden < bestandsgrootte) {
        int te_verzenden = ((int)sizeof(buffer) < (int)(bestandsgrootte - totaal_verzonden)) ? (int)sizeof(buffer) : (int)(bestandsgrootte - totaal_verzonden);
        invoer.read(buffer, te_verzenden);
        int verstuurd = send(socket, buffer, te_verzenden, 0);
        totaal_verzonden += verstuurd;
    }

    // Tijdmeting einde
    auto tijd_einde = std::chrono::high_resolution_clock::now();
    invoer.close();

    std::chrono::duration<double> duur = tijd_einde - tijd_start;
    double duur_ms = duur.count() * 1000.0;
    double snelheid_mbps = (duur.count() > 0.0001) ? ((double)bestandsgrootte * 8.0 / 1e6) / duur.count() : 0;

    // Resultaat printen
    std::cout << std::left << std::setw(30) << bestandsnaam
        << " tijd = " << std::setw(8) << std::fixed << std::setprecision(2) << duur_ms << " ms"
        << " snelheid = " << std::setw(8) << std::fixed << std::setprecision(2) << snelheid_mbps << " Mbps"
        << std::endl;
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // Socket aanmaken en verbinden met server
    SOCKET socket_client = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in adres_server;
    adres_server.sin_family = AF_INET;
    adres_server.sin_port = htons(POORTNUMMER);

    inet_pton(AF_INET, "192.168.0.151", &adres_server.sin_addr); // Pas IP naar jouw server aan!
    connect(socket_client, (struct sockaddr*)&adres_server, sizeof(adres_server));

    // Alle bestanden in gekozen map verzenden
    std::string map_pad = "C:/Users/janse/Documents/wiki opdracht";
    std::string wildcard = map_pad + "/*";
    WIN32_FIND_DATAA vindBestandData;
    HANDLE hFind = FindFirstFileA(wildcard.c_str(), &vindBestandData);

    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(vindBestandData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                std::string volledig_pad = map_pad + "/" + vindBestandData.cFileName;
                bestand_verzenden(volledig_pad, socket_client);
            }
        } while (FindNextFileA(hFind, &vindBestandData));
        FindClose(hFind);
    }
    else {
        std::cerr << "Kon map niet openen!" << std::endl;
    }

    closesocket(socket_client);
    WSACleanup();
    std::cout << "Alle bestanden verstuurd." << std::endl;
    return 0;
}

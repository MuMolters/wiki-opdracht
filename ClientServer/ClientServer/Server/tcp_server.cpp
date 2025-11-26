#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#pragma comment(lib, "Ws2_32.lib")

#define POORTNUMMER 12345

// Functie: haalt echte bestandsnaam uit char-array
std::string bestandsnaam_opschonen(const char* naam, int lengte) {
    int echteLengte = 0;
    while (echteLengte < lengte && naam[echteLengte] != '\0') echteLengte++;
    return std::string(naam, echteLengte);
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET luisterSocket;
    struct sockaddr_in serverAdres;
    int adresLengte = sizeof(serverAdres);

    // Socket aanmaken en voorbereiden
    luisterSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverAdres.sin_family = AF_INET;
    serverAdres.sin_addr.s_addr = INADDR_ANY;
    serverAdres.sin_port = htons(POORTNUMMER);

    bind(luisterSocket, (struct sockaddr*)&serverAdres, sizeof(serverAdres));
    listen(luisterSocket, SOMAXCONN);

    std::cout << "Server staat klaar voor verbindingen..." << std::endl;

    while (true) { // Hoofdloop: wacht steeds op een nieuwe client
        SOCKET clientSocket = accept(luisterSocket, (struct sockaddr*)&serverAdres, &adresLengte);
        std::cout << "Nieuwe client is verbonden!" << std::endl;

        // Blijft bestanden ontvangen tot de client afsluit
        while (true) {
            char naam[256] = { 0 };
            int ontvangenNaam = recv(clientSocket, naam, sizeof(naam), 0);
            if (ontvangenNaam <= 0) break;
            std::string echteNaam = bestandsnaam_opschonen(naam, ontvangenNaam);

            uint32_t bestandsgrootte;
            int ontvangenGrootte = recv(clientSocket, (char*)&bestandsgrootte, sizeof(bestandsgrootte), 0);
            if (ontvangenGrootte <= 0) break;

            std::ofstream uitvoerBestand(echteNaam.c_str(), std::ios::binary);
            char buffer[4096];
            uint32_t totaalOntvangen = 0;

            // Tijd bijhouden vanaf het begin van het ontvangen bestand
            auto tijdStart = std::chrono::high_resolution_clock::now();

            while (totaalOntvangen < bestandsgrootte) {
                int maxTeLezen = ((int)sizeof(buffer) < (int)(bestandsgrootte - totaalOntvangen)) ? (int)sizeof(buffer) : (int)(bestandsgrootte - totaalOntvangen);
                int ontvangen = recv(clientSocket, buffer, maxTeLezen, 0);
                if (ontvangen <= 0) break;
                uitvoerBestand.write(buffer, ontvangen);
                totaalOntvangen += ontvangen;
            }
            uitvoerBestand.close();

            // Tijd gemeten einde
            auto tijdStop = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duur = tijdStop - tijdStart;
            double duurMs = duur.count() * 1000.0;
            double snelheidMbps = duur.count() > 0.0001 ? ((double)bestandsgrootte * 8.0 / 1e6) / duur.count() : 0.0;

            // Overzichtelijke en nette uitprint van resultaat
            std::cout << std::left << std::setw(30) << echteNaam
                << " tijd = " << std::setw(8) << std::fixed << std::setprecision(2) << duurMs << " ms"
                << " snelheid = " << std::setw(8) << std::fixed << std::setprecision(2) << snelheidMbps << " Mbps"
                << std::endl;
        }
        std::cout << "Client heeft de verbinding verbroken." << std::endl;
        closesocket(clientSocket);
    }

    closesocket(luisterSocket);
    WSACleanup();
    return 0;
}

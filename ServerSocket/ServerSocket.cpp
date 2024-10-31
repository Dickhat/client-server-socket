#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <ctime>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")


int main()
{
    // Структура WSADATA содержит сведения о реализации сокетов Windows
    WSADATA wsaData;                            
    int WSAError;                            // Код возврата
    WORD wVersionRequested = MAKEWORD(2, 2); // Версия socket 2.2

    // Инициирует использование Winsock DLL процессом.
    WSAError = WSAStartup(wVersionRequested, &wsaData);
    
    // Проверка, что dll подключилась
    if (WSAError != 0 )
    {
        std::cout << " Error: WSA startup failed\n";
        return WSAError;
    }

    // Создание сокета
    SOCKET serverSocket;
    serverSocket = INVALID_SOCKET;
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Проверка создания сокета
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Error at socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    std::cout << "Socket is OK!" << std::endl;

    // Bind data
    sockaddr_in service;                                    // IPv4 Socket address
    service.sin_family = AF_INET;                           // internetwork: UDP, TCP, etc.
    service.sin_addr.s_addr = inet_addr("192.168.0.130");   // IP address
    service.sin_port = htons(30411);                        // Choose a port number

    // Подвязка данных
    if (bind(serverSocket, (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR) {
        std::cout << "bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "bind is OK!" << std::endl;

    std::fstream logFile;
    logFile.open(".\\log.txt", std::ios::app);

    // Время старта сервера
    time_t timestamp;
    time(&timestamp);
    logFile << "\n\nServer start " << ctime(&timestamp);

    while (true)
    {
        // Прослушивание входящих подключений
        if (listen(serverSocket, 5) == SOCKET_ERROR){
            std::cout << "listen: Error listening on socket: " << WSAGetLastError() << std::endl;
            continue;
        }

        std::cout << "listen is OK! I'm waiting for new connections..." << std::endl;

        // Сокет для входящих подключений
        SOCKET clientSocket;
        clientSocket = accept(serverSocket, nullptr, nullptr);

        if (clientSocket == INVALID_SOCKET) {
            std::cout << "accept failed: " << WSAGetLastError() << std::endl;
        }
        else {
            std::cout << "accept is OK!" << std::endl;

            // Время подключения
            time(&timestamp);
            logFile << "\n\nConnection time " << ctime(&timestamp);

            char recvbuf[1024];

            for (int i = 0; i < 1024; ++i)
            {
                recvbuf[i] = '\0';
            }

            int byteRecv = 0;
            int byteSend = 0;
            // Время получения сообщения
            time(&timestamp);
            logFile << "Message recieved time " << ctime(&timestamp) << "Recieved message:";

            std::string response;

            // Получение данных
            while (true) {
                byteRecv = recv(clientSocket, recvbuf, 1024, 0);

                response.append(std::string(recvbuf, byteRecv));
                logFile << recvbuf;

                if (byteRecv < 1024)
                    break;
            }
            
            // Отзеркалирование сообщения
            std::reverse(response.begin(), response.end());
            response.append(" Server written by Bogdanov Daniil Alekseevich M30-411B-21");

            logFile << "\nMessage send time " << ctime(&timestamp) << "Send message:";

            byteSend = send(clientSocket, response.c_str(), strlen(response.c_str()), 0);
            logFile << response;
        }
        
        logFile << "\n Disconnect time " << ctime(&timestamp);
        logFile.flush();            // Обнуление буфера для отображения данных в файле
        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

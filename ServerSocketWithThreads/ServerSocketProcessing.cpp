#define _WINSOCK_DEPRECATED_NO_WARNINGS 
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <list>
#include <ctime>
#include <semaphore>
//#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")


DWORD WINAPI ClientThreadProc(LPVOID lpParam);

HANDLE checkThreadSem = CreateSemaphore(NULL, 1, 1, NULL);
HANDLE WriteFileSem = CreateSemaphore(NULL, 1, 1, NULL);
std::fstream logFile;

int id = 0;

int main()
{
    setlocale(LC_ALL, "RUSSIAN");

    // Структура WSADATA содержит сведения о реализации сокетов Windows
    WSADATA wsaData;
    int WSAError;                            // Код возврата
    WORD wVersionRequested = MAKEWORD(2, 2); // Версия socket 2.2

    // Инициирует использование Winsock DLL процессом.
    WSAError = WSAStartup(wVersionRequested, &wsaData);

    // Проверка, что dll подключилась
    if (WSAError != 0)
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
    if (bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cout << "bind failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "bind is OK!" << std::endl;

    // Открытие лог файла
    logFile.open(".\\log.txt", std::ios::app);

    // Время старта сервера
    time_t timestamp;
    time(&timestamp);

    WaitForSingleObject(WriteFileSem, INFINITE);
    logFile << "\n\nServer start " << ctime(&timestamp);
    ReleaseSemaphore(WriteFileSem, 1, NULL);

    SOCKET* clientSocket = new SOCKET{};// Сокет для клиентов
    std::list<HANDLE> ThreadL;          // Список работающих потоков
    int TotalThreads = 10;              // Max работающих потоков
    DWORD semRet;                       // Итог ситуации захвата семафора
    
    while (true)
    {
        // Прослушивание входящих подключений
        if (listen(serverSocket, 5) == SOCKET_ERROR) {
            std::cout << "listen: Error listening on socket: " << WSAGetLastError() << std::endl;
            continue;
        }

        std::cout << "listen is OK! I'm waiting for new connections..." << std::endl;

        //  Захват семафора
        semRet = WaitForSingleObject(checkThreadSem, 100);

        switch (semRet)
        {
            // Семафор захвачен
            case WAIT_OBJECT_0:
                // Пока потоки не освободятся, ничего не делать
                if (ThreadL.size() == 10)
                    ReleaseSemaphore(checkThreadSem, 1, NULL);
                else
                {
                    ReleaseSemaphore(checkThreadSem, 1, NULL);
                    
                    // Сокет для входящих подключений
                    *clientSocket = accept(serverSocket, nullptr, nullptr);

                    if (*clientSocket == INVALID_SOCKET)
                        std::cout << "accept failed: " << WSAGetLastError() << std::endl;
                    else
                    {
                        ThreadL.push_back(CreateThread(NULL,  // default security attributes
                                                       0,     // default stack size
                                                       ClientThreadProc,
                                                       clientSocket,  // no thread function arguments
                                                       0,     // default creation flags
                                                       NULL)  // receive thread identifier);
                        );
                    }
                }
            default:
                break;
        }

        // Добавить проверку что поток отработал
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

DWORD WINAPI ClientThreadProc(LPVOID lpParam)
{
    SOCKET clientSocket = *reinterpret_cast<SOCKET*>(lpParam);

    time_t timestamp;

    int SessionId = -1;

    // Время подключения
    time(&timestamp);
    WaitForSingleObject(WriteFileSem, INFINITE);
    id++;
    std::cout << "\n Клиент " << id << " подключился\n";
    logFile << "\n\nConnection time " << ctime(&timestamp);
    ReleaseSemaphore(WriteFileSem, 1, NULL);

    char recvbuf[1024];

    for (int i = 0; i < 1024; ++i)
    {
        recvbuf[i] = '\0';
    }

    int byteRecv = 0;
    int byteSend = 0;
    // Время получения сообщения
    time(&timestamp);
    WaitForSingleObject(WriteFileSem, INFINITE);
    logFile << "Message recieved time " << ctime(&timestamp) << "Recieved message:";
    ReleaseSemaphore(WriteFileSem, 1, NULL);

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

    Sleep(5000);  // миллисекунды

    WaitForSingleObject(WriteFileSem, INFINITE);
    logFile << "\nMessage send time " << ctime(&timestamp) << "Send message:";
    ReleaseSemaphore(WriteFileSem, 1, NULL);

    byteSend = send(clientSocket, response.c_str(), strlen(response.c_str()), 0);
    
    WaitForSingleObject(WriteFileSem, INFINITE);
    logFile << response;
    ReleaseSemaphore(WriteFileSem, 1, NULL);

    WaitForSingleObject(WriteFileSem, INFINITE);
    logFile << "\n Disconnect time " << ctime(&timestamp);
    logFile.flush();            // Обнуление буфера для отображения данных в файле
    ReleaseSemaphore(WriteFileSem, 1, NULL);

    closesocket(clientSocket);
    ExitThread(0);
}

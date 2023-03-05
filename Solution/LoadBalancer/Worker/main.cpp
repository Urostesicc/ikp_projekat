#include "../LoadBalancer/common.h" //kad se rijesi worker message ubaciti 'common.h'
#include "stockList.h"

#define DEFAULT_PORT 4000



int main()
{
    // socket used to communicate with server
    SOCKET socket = connectSock(DEFAULT_PORT);
    if (socket == INVALID_SOCKET)
    {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    MESSAGEW buffer;
    FD_SET read;
    timeval time = { 1, 0 };

    printf("Press q to exit...\n");
    while (true) {
        if (_kbhit()) {
            if (_getch() == 'q') {
                shutdownProp(socket);
                break;
            }
        }

        FD_ZERO(&read);
        FD_SET(socket, &read);
        
        int sResult = select(0, &read, NULL, NULL, &time);
        if (sResult == SOCKET_ERROR) 
            break;
        else if (sResult == 0)
            Sleep(100);
        else if(sResult > 0) {
            if (FD_ISSET(socket, &read)) {
                int iResult = recv(socket, (char*)&buffer, sizeof(MESSAGEW), 0);
                if (iResult > 0) {
                    buffer.data[buffer.messageSize] = '\0';
                    printf("Received %d %s\n", buffer.returnData, buffer.data);
                    if (!buffer.returnData) {
                        strcpy(buffer.data, append(buffer.data) ? "success" : "failed");
                        iResult = send(socket, (char*)&buffer, sizeof(MESSAGEW), 0);
                        if (iResult == SOCKET_ERROR) 
                            printf("Error send: %d\n", WSAGetLastError());
                        continue;
                    }

                    int numToReturn = buffer.messageSize;
                    printf("Need to return %d data\n", numToReturn);
                    if (numToReturn < 0) break;
                    sendBack(socket, numToReturn);
                } 
                else if (iResult == 0) {
                    closesocket(socket);
                    WSACleanup();
                    return 0;
                }
                else {
                    closesocket(socket);
                    WSACleanup();
                    return 1;
                }
            }
        }
    }

    // cleanup
    shutdown(socket, SD_BOTH);
    closesocket(socket);
    WSACleanup();
    _getch();
    return 0;
}





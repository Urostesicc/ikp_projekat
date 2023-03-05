#include "../LoadBalancer/common.h"
#define MESSAGES 200

void StressTest(SOCKET socket) {
	char message[BUFFER_SIZE];
	int numOfChars = 50;
	for (int i = 0; i < numOfChars; i++)
		message[i] = 'A' + abs(rand()) % 26;
	message[numOfChars] = '\0';

	timeval interval = { 1, 0 };
	FD_SET write;
	int i = 0;
	while (true) {
		FD_ZERO(&write);
		FD_SET(socket, &write);
		int iResult = select(0, NULL, &write, NULL, &interval);
		if (iResult != SOCKET_ERROR && FD_ISSET(socket, &write)) {
			iResult = send(socket, message, strlen(message) + 1, 0);
			if (iResult == SOCKET_ERROR) {
				printf("Error send: %d", WSAGetLastError());
				shutdown(socket, SD_BOTH);
				closesocket(socket);
				WSACleanup();
				return;
			}
			Sleep(10);
			if (++i >= MESSAGES) break;
		}
	}
	// Zatvaramo socket
	shutdown(socket, SD_BOTH);
	closesocket(socket);
	WSACleanup();
}

int main()
{
	SOCKET socket = connectSock(5059);
	if (socket == INVALID_SOCKET) {
		printf("Connection doesnt work %d", WSAGetLastError());
		closesocket(socket);
		WSACleanup();
		return 1;
	}

	printf("Press s now to execute stress test, or any key to continue client\n");
	if (_getch() == 's') {
		StressTest(socket);
		printf("Finished");
		_getch();
		return 0;
	}

	char buffer[BUFFER_SIZE];

	FD_SET write;
	timeval time = { 1, 0 };
	printf("Press q to exit...\n\n");
	Sleep(1000);

	while (true) {
		if (_kbhit()) 
			if(_getch() == 'q')
				break;

		FD_ZERO(&write);
		FD_SET(socket, &write);

		int sResult = select(0, NULL, &write, NULL, &time);
		if (sResult == SOCKET_ERROR) {
			printf("ERROR select: %d\n", WSAGetLastError());
			shutdown(socket, SD_BOTH);
			closesocket(socket);
			WSACleanup();
			return 1;
		}
		else if (sResult == 0) {
			continue;
		}
		else if (sResult > 0) {
			if (FD_ISSET(socket, &write)) {
				for (int i = 0; i < 50; i++) 
					buffer[i] = abs(rand()) % 25 + 'a';
				buffer[50] = '\0';
			
				printf("Sending: %s\n", buffer);
				int iResult = send(socket, buffer, strlen(buffer), 0);
				if (iResult == SOCKET_ERROR) {
					printf("Error: %d\n", WSAGetLastError());
					shutdown(socket, SD_BOTH);
					closesocket(socket);
					WSACleanup();
					return 1;
				}
			}
		}
		Sleep(2000);
	}

	printf("Safely returned");
	shutdown(socket, SD_BOTH);
	closesocket(socket);
	WSACleanup();
	_getch();
	return 0;
}

#pragma once

#ifndef HANDLECLIENT_H
#define HANDLECLIENT_H

#include "queue.h"

typedef struct client_st {
	HANDLE handle;
	SOCKET socket;
	bool isUsed;
} CLIENT;

CLIENT clients[MAX_CLIENTS];

void initClients() {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		clients[i].socket = INVALID_SOCKET;
		clients[i].isUsed = false;
	}
}

void closeClient(SOCKET socket) {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i].socket == socket && clients[i].isUsed) {
			clients[i].isUsed = false;
			SafeCloseHandle(clients[i].handle);
			closesocket(socket);
			clients[i].socket = INVALID_SOCKET;
			return;
		}
	}
}

void closeAllClients() {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (clients[i].isUsed) {
			clients[i].isUsed = false;
			SafeCloseHandle(clients[i].handle);
			shutdown(clients[i].socket, SD_BOTH);
			closesocket(clients[i].socket);
		}
	}
}

//Handles clients socketing
DWORD WINAPI clientThread(LPVOID param) {
	SOCKET socket = *(SOCKET*)param;
	print("Client %d connected", socket);
	FD_SET set;
	char buffer[BUFFER_SIZE];
	timeval time = { 1, 0 };

	while (true)
	{
		if (shuttingDown) {
			Sleep(10);
			continue;
		}
		FD_ZERO(&set);
		FD_SET(socket, &set);

		int result = select(0, &set, NULL, NULL, &time);
		if (result == SOCKET_ERROR) {
			closeClient(socket);
			break;
		}
		else if (result > 0) {
			int iResult = recv(socket, buffer, BUFFER_SIZE, 0);
			if (iResult > 0) {
				if (iResult < BUFFER_SIZE) {
					buffer[iResult] = '\0';
				}
				print("Received from client %d: %s", socket, buffer);
				pushQ1(buffer);
			}
			else if (iResult == 0) {
				print("Client %d disconnected", socket);
				closeClient(socket);
				break;
			}
			else {
				break;
			}
		}
	}
	return 0;
}

#endif // !HANDLECLIENT_H
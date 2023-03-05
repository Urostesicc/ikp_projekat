#pragma once
#ifndef HANDLEWORKER_H
#define HANDLEWORKER_H

#include "list.h"
#include "queue.h"
#define _CRT_SECURE_NO_WARNINGS
//Handles clients socketing
DWORD WINAPI WorkerHandler(LPVOID param) {
	WORKER* worker = (WORKER*)param;
	if (worker == NULL) {
		print("Cant thread worker - worker null");
		return 0;
	}

	FD_SET read, write;
	bool shuttingdown = false;
	MESSAGEW buffer;
	timeval time = { 1, 0 };
	strcpy_s(worker->buffer.data, BUFFER_SIZE, "");
	worker->buffer.messageSize = 0;
	while (true)
	{
		if (shuttingDown) {
			Sleep(10);
			continue;
		}
		FD_ZERO(&read);
		FD_ZERO(&write);
		FD_SET(worker->socket, &read);
		FD_SET(worker->socket, &write);

		int result = select(0, &read, &write, NULL, &time);
		if (result == SOCKET_ERROR) {
			print("Error: %d", WSAGetLastError());
			CloseWorker(worker->socket);

			break;
		}
		else if (result > 0) {
			if (FD_ISSET(worker->socket, &read)) {
				int iResult = recv(worker->socket, (char*)&buffer, sizeof(MESSAGEW), 0);
				if (iResult > 0) {
					
					if (buffer.returnData) {
						print("Received data from worker %d", worker->socket);
						pushQ2(buffer.data);
						continue;
					}

					if (!strcmp(buffer.data, "success")) {
						print("Worker %d saved data");
						continue;
					}
					else if (!strcmp(buffer.data, "failed")) {
						print("Worker %d couldn't save data");
						continue;
					}

					//if returnData is false and buffer has something in it, it is because worker is closing
					if(!shuttingdown) {
						shuttingdown = true;
						shutdownW(worker->socket);
					}

					pushQ2(buffer.data);
				}
				else if (iResult == 0) {	// Check if shutdown command is received
					print("Connection with worker closed.\n");
					break;
				}
				else {	// There was an error during recv
					print("recv failed with error: %d\n", WSAGetLastError());
					CloseWorker(worker->socket);
				}
			}
			else if (FD_ISSET(worker->socket, &write)) {
				if (!strcmp(worker->buffer.data, "") && worker->buffer.messageSize == 0) {
					continue;
				}

				int iResult = send(worker->socket, (char*)&worker->buffer, sizeof(MESSAGEW), 0);
				if (iResult == SOCKET_ERROR) {
					print("Send failed with error: %d\n", WSAGetLastError());
					CloseWorker(worker->socket);
					break;
				}
				worker->numOfData++;
				strcpy_s(worker->buffer.data, BUFFER_SIZE, "");
				worker->buffer.messageSize = 0;
			}
		}
	}

	print("Worker %d closed successfully", worker->socket);
	free(worker);
	return 0;
}

#endif // !HANDLEWORKER_H
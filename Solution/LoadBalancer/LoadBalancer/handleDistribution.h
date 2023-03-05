#pragma once
#ifndef HANDLEDISTRIBUTION_H
#define HANDLEDISTRIBUTION_H

#include "common.h"
#include "list.h"
#include "queue.h"

bool reorganize = false;
int numOfStoredData = 0;

void AddMessage(WORKER* worker, char* message, bool needsToReturn, int numToReturn = 0) {
	worker->buffer.returnData = needsToReturn;
	if (needsToReturn) {
		worker->buffer.messageSize = numToReturn;
		return;
	}
	worker->buffer.messageSize = strlen(message);
	strcpy_s(worker->buffer.data, BUFFER_SIZE, message);
}

bool Reorganize(SOCKET socket) {
	print("Started reorganization...");
	int numToLeave = numOfStoredData / (numOfWorkers + 1);
	WORKER* temp = headL;
	int allToSend = 0;
	while (temp != NULL) {
		int toSend = temp->numOfData - numToLeave;
		if (toSend < 0) {
			print("REOR NOT WORKING CORRECTLY");
			return false;
		}

		if (toSend > 0) {
			allToSend += toSend;
			print("Worker %d send %d data", temp->socket, toSend);
			AddMessage(temp, (char*)&toSend, true, toSend);
			temp->numOfData = numToLeave;
		}
		temp = temp->next;
	}

	print("New worker needs to receive: %d", allToSend);
	if (allToSend > 0) {
		FD_SET set;
		timeval time = { 0, 0 };
		char* data = NULL;
		MESSAGEW buffer;
		buffer.returnData = false;
		for (int i = 0; i < allToSend; i++) {
			FD_ZERO(&set);
			FD_SET(socket, &set);
			
			while (emptyQ2()) {
				Sleep(10);
			}

			data = popQ2();
			buffer.messageSize = strlen(data);
			strcpy_s(buffer.data, BUFFER_SIZE, data);
			free(data);

			int sResult = select(0, NULL, &set, NULL, &time);
			if (sResult == SOCKET_ERROR) {
				print("Error select: %d", WSAGetLastError());
				continue;
			}
			if (FD_ISSET(socket, &set)) {
				int iResult = send(socket, (char*)&buffer, sizeof(MESSAGEW), 0);
				if (iResult == SOCKET_ERROR)
					return false;
			}
		}
		print("Sent all data to new worker");
	}
	
	WORKER* newWorker = appendL(0, socket, allToSend);
	if (!newWorker) {
		print("Cant add new worker!");
		return false;
	}
	newWorker->numOfData = allToSend;
	newWorker->handle = CreateThread(NULL, 0, &WorkerHandler, newWorker, 0, NULL);
	if (newWorker->handle == NULL) {
		print("Cant make thread worker");
		return false;
	}
	numOfWorkers++;
	reorganize = false;
	return true;
}

DWORD WINAPI DistributionHandler(LPVOID param) {
	while (true) {
		Sleep(10);
		if (reorganize) {
			SOCKET* s = (SOCKET*)param;
			if (!Reorganize(*s)) {
				print("Couldn't reorganize");
				continue;
			}
		}

		if (!numOfWorkers) {
			Sleep(100);
			continue;
		}

		char* data;
		
		if (!emptyQ2()) {
			//When one queue shuts down it stores to Q2
			data = popQ2();
			if (!headL) {
				free(data);
				continue;
			}

			AddMessage(headL, data, false);
			free(data);
			moveFirstToLast();
			continue;
		}

		if (emptyQ1()) {
			continue;
		}

		data = popQ1();
		if (!headL) {
			print("Can't find not shutting down");
			free(data);
			continue;
		}

		AddMessage(headL, data, false);
		numOfStoredData++;
		free(data);
		moveFirstToLast();
	}
	return 0;
}
#endif // !HANDLEDISTRIBUTION_H
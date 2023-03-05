#pragma once
#ifndef LIST_H
#define LIST_H

#include "common.h"

typedef struct worker_st {
	int numOfData = 0;
	HANDLE handle;
	SOCKET socket;
	struct worker_st* next;
	MESSAGEW buffer;
} WORKER;


WORKER* headL = NULL, * tailL = NULL;
CRITICAL_SECTION csL;

WORKER* appendL(HANDLE handle, SOCKET socket, int numOfData) {
	WORKER* temp = (WORKER*)malloc(sizeof(WORKER));
	if (temp != NULL)
	{
		temp->numOfData = numOfData;
		temp->handle = handle;
		temp->socket = socket;

		temp->next = NULL;
		EnterCriticalSection(&csL);
		if (tailL == NULL) {
			headL = tailL = temp;
		}
		else {
			if (headL->numOfData > temp->numOfData) {
				temp->next = headL;
				headL = temp;
			}
			else {
				tailL->next = temp;
				tailL = tailL->next;
			}
		}
		LeaveCriticalSection(&csL);
	}
	
	return temp;
}

void moveFirstToLast() {
	EnterCriticalSection(&csL);
	if (headL != tailL) {
		WORKER* temp = headL;
		headL = (headL)->next;
		temp->next = NULL;
		(tailL)->next = temp;
		tailL = temp;
	}
	LeaveCriticalSection(&csL);
}

void shutdownW(SOCKET socket) {
	EnterCriticalSection(&csL);
	if (headL->socket == socket) {
		headL = headL->next;
	}
	else {
		WORKER* temp1 = headL, * temp2 = (headL)->next;
		while (temp2 != NULL) {
			if (temp2->socket == socket) {
				if (temp2 == tailL)
					tailL = temp1;

				temp1->next = temp2->next;
				numOfWorkers--;
				break;
			}

			temp1 = temp2;
			temp2 = temp2->next;
		}
	}
	LeaveCriticalSection(&csL);
}

//Closes Worker for given socket
void CloseWorker(SOCKET socket) {
	EnterCriticalSection(&csL);
	if (headL != NULL)
	{
		if (headL->socket == socket) {
			WORKER* temp = headL;
			headL = headL->next;
			closesocket(temp->socket);
			SafeCloseHandle(temp->handle);
			free(temp);
		}
		else {
			WORKER* temp1 = headL, * temp2 = headL->next;
			while (temp2 != NULL) {
				if (temp2->socket == socket) {
					if (temp2 == tailL)
						tailL = temp1;

					temp1->next = temp2->next;
					closesocket(temp2->socket);
					SafeCloseHandle(temp2->handle);
					free(temp2);
					break;
				}

				temp1 = temp2;
				temp2 = temp2->next;
			}
		}
	}
	LeaveCriticalSection(&csL);
}

//Clears list
void CloseAllWorkers() {
	EnterCriticalSection(&csL);
	while (headL != NULL) {
		WORKER* temp = headL;
		headL = headL->next;
		SafeCloseHandle(temp->handle);
		shutdown(temp->socket, SD_BOTH);
		closesocket(temp->socket);
		free(temp);
	}
	tailL = NULL;
	LeaveCriticalSection(&csL);
}
#endif // !LIST_H
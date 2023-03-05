#pragma once
#ifndef STOCK_H
#define STOCK_H
#include <stdlib.h>
#include <string.h>
#include "../LoadBalancer/common.h"


DATA* head = NULL;

void clear() {
	while (head != NULL) {
		DATA* temp = head;
		head = head->next;
		free(temp);
	}
}

char* get() {
	if (head == NULL)
		return NULL;

	char* ret = (char*)malloc(BUFFER_SIZE);
	if (ret == NULL)
		return NULL;
	memcpy(ret, head->data, BUFFER_SIZE);
	DATA* temp = head;
	head = head->next;
	free(temp);
	return ret;
}

bool append(char* data) {
	DATA* temp = (DATA*)malloc(sizeof(DATA));
	if (temp == NULL)
		return false;

	memcpy(temp->data, data, BUFFER_SIZE);
	temp->next = head;
	head = temp;
	return true;
}

void shutdownProp(SOCKET socket) {
    FD_SET set;
    timeval time = { 1, 0 };
    char buffer[BUFFER_SIZE + 1];
    buffer[0] = (char)false;
    while (head != NULL) {
        FD_ZERO(&set);
        FD_SET(socket, &set);

        char* data = get();
        if (data == NULL) break;
        strcpy(buffer + 1, data);
        free(data);

        int sSelect = select(0, NULL, &set, NULL, &time);
        if (sSelect > 0 && FD_ISSET(socket, &set)) {
            int iResult = send(socket, buffer, strlen(buffer + 1) + 1, 0);
            if (iResult == SOCKET_ERROR)
                printf("Error send: %d\n", WSAGetLastError());
        }
    }
}

void sendBack(SOCKET socket, int num) {
    FD_SET write;
    timeval time = { 1, 0 };
    
    MESSAGEW buffer;
    buffer.returnData = true;
    int sent = 0;
    while (sent < num) {
        FD_ZERO(&write);
        FD_SET(socket, &write);

        int sSelect = select(0, NULL, &write, NULL, &time);
        if (sSelect == SOCKET_ERROR)
            return;
        else if (sSelect == 0)
            Sleep(100);
        else if (sSelect > 0) {
            if (FD_ISSET(socket, &write)) {
                char* data = get();
                if (data == NULL)
                    break;

                printf("Sending back: %s\n", data);
                strcpy(buffer.data, data);
                free(data);

                int iResult = send(socket, (char*)&buffer, sizeof(MESSAGEW), 0);
                if (iResult == SOCKET_ERROR)
                    printf("Error send: %d\n", WSAGetLastError());
                ++sent;
            }
        }
    }
}

#endif // !STOCK_H
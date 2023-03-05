#pragma once
#ifndef QUEUE_H
#define QUEUE_H
#include <stdlib.h>
#include <string.h>
#include "common.h"

CRITICAL_SECTION csQ1, csQ2;
DATA* headQ1 = NULL, * tailQ1 = NULL;
DATA* headQ2 = NULL, * tailQ2 = NULL;

bool emptyQ1() {
	return headQ1 == NULL;
}

bool emptyQ2() {
	return headQ2 == NULL;
}

char* popQ1() {
	EnterCriticalSection(&csQ1);
	char* ret = (char*)NULL;
	if (headQ1 != NULL)
	{
		ret = (char*)malloc(BUFFER_SIZE);
		if (ret != NULL) {
			memcpy(ret, headQ1->data, BUFFER_SIZE);
			DATA* temp = headQ1;
			if (tailQ1 == headQ1)
				tailQ1 = NULL;
			headQ1 = headQ1->next;
			free(temp);
		}
	}
	LeaveCriticalSection(&csQ1);
	return ret;
}

char* popQ2() {
	EnterCriticalSection(&csQ2);
	char* ret = NULL;
	if (headQ2 != NULL)
	{
		ret = (char*)malloc(BUFFER_SIZE);
		if (ret != NULL) {
			memcpy(ret, headQ2->data, BUFFER_SIZE);
			DATA* temp = headQ2;
			if (tailQ2 == headQ2)
				tailQ2 = NULL;
			headQ2 = headQ2->next;
			free(temp);
		}
	}
	LeaveCriticalSection(&csQ2);
	return ret;
}

void pushQ1(char* data) {
	EnterCriticalSection(&csQ1);
	DATA* temp = (DATA*)malloc(sizeof(DATA));
	if (temp != NULL)
	{
		memcpy(temp->data, data, BUFFER_SIZE);
		temp->next = NULL;

		if (tailQ1 == NULL) 
			headQ1 = tailQ1 = temp;
		else {
			tailQ1->next = temp;
			tailQ1 = temp;
		}
		
	}
	LeaveCriticalSection(&csQ1);
}

void pushQ2(char* data) {
	EnterCriticalSection(&csQ2);
	DATA* temp = (DATA*)malloc(sizeof(DATA));
	if (temp != NULL)
	{
		memcpy(temp->data, data, BUFFER_SIZE);
		temp->next = NULL;
		if (tailQ2 == NULL)
			headQ2 = tailQ2 = temp;
		else {
			tailQ2->next = temp;
			tailQ2 = temp;
		}
	}
	LeaveCriticalSection(&csQ2);
}

void clearQ1() {
	EnterCriticalSection(&csQ1);
	while (headQ1 != NULL)
	{
		DATA* temp = headQ1;
		headQ1 = headQ1->next;
		free(temp);
	}
	tailQ1 = NULL;
	LeaveCriticalSection(&csQ1);
}

void clearQ2() {
	EnterCriticalSection(&csQ2);
	while (headQ2 != NULL)
	{
		DATA* temp = headQ2;
		headQ2 = headQ2->next;
		free(temp);
	}
	tailQ2 = NULL;
	LeaveCriticalSection(&csQ2);
}
#endif // !QUEUE_H
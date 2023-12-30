#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>

#ifndef FILE_H
#define FILE_H

#ifdef    __cplusplus
extern "C" {
#endif

bool loadFileData(char *fileName, char **fileData, int *fileSize);

bool writeFileData(char *fileName, const char *fileData, int fileSize);

void *sendData(void *data);

void *receiveData(void *data);

void data_writeData(int socket, void *data, size_t dataSize);

void data_readData(int socket, void *data, size_t dataSize);

#ifdef    __cplusplus
}
#endif

#endif

#include <stdbool.h>
#include <stdio.h>

#ifndef FILE_H
#define FILE_H

#define INPUT_BUFFER_CELL_SIZE 256
#define INPUT_BUFFER_CELL_COUNT 5

bool loadFileData(char *fileName, char **fileData, int *fileSize);
bool writeFileData(char *fileName, const char *fileData, int fileSize);
void initInputBuffer(char ***buf);
void destroyInputBuffer(char ***buf);
  void readInput(char **buf);

  void *sendData(void *data);
  void *receiveData(void *data);

  void data_writeData(int socket, void *data, size_t dataSize);
  void data_readData(int socket, void *data, size_t dataSize);

#endif

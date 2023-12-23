#include "file.h"
#include "definitions.h"
#include "helper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

void initInputBuffer(char ***buf) {
  *buf = calloc(INPUT_BUFFER_CELL_COUNT, sizeof(char *));
  for (int i = 0; i < INPUT_BUFFER_CELL_COUNT; i++) {
    (*buf)[i] = calloc(INPUT_BUFFER_CELL_SIZE, sizeof(char));
  }
}

void destroyInputBuffer(char ***buf) {
  for (int i = 0; i < INPUT_BUFFER_CELL_COUNT; i++) {
    free((*buf)[i]);
  }
  free(*buf);
  *buf = NULL;
}

void readInput(char **buf) {
  int tempSize = INPUT_BUFFER_CELL_COUNT * INPUT_BUFFER_CELL_SIZE + 1;
  char temp[tempSize + 1];
  int bufIndex = 0;
  fgets(temp, tempSize, stdin);

  char *strtokPtr;
  char delimeter = ' ';
  char *splitStr = strtok_r(temp, &delimeter, &strtokPtr);
  while (splitStr != NULL && bufIndex < INPUT_BUFFER_CELL_COUNT) {
    memcpy(*buf, splitStr, strlen(splitStr) * sizeof(char));
    if (*(*buf + strlen(splitStr) - 1) == '\n') {
      *(*buf + strlen(splitStr) - 1) = '\0';
    }
    splitStr = strtok_r(NULL, &delimeter, &strtokPtr);
    buf++;
    bufIndex++;
  }
}

bool loadFileData(char * fileName, char ** fileData, int * fileSize) {
  FILE * f = fopen(fileName, "rb");
  if (!f) {
    return false;
  }

  //get file size
  fseek(f, 0, SEEK_END);
  *fileSize = ftell(f);
  rewind(f);

  *fileData = calloc(*fileSize, sizeof(char));
  if (!fileData) {
    printf("[system] Error allocating memory on file %s.\n", fileName);
    fclose(f);
    return false;
  }

  if (fread(*fileData, 1, *fileSize, f) != *fileSize) {
    fclose(f);
    free(*fileData);
    return false;
  }

  fclose(f);

  return true;
}

bool writeFileData(char *fileName, const char *fileData, int fileSize) {
  FILE * f = fopen(fileName, "wb");
  if (!f) {
    return false;
  }

  if (fwrite(fileData, 1, fileSize, f) != fileSize) {
    fclose(f);
    return false;
  }

  fclose(f);

  return true;
}

void data_writeData(int socket, void * data, size_t dataSize) {
  size_t sentBytesTotal = 0;

  while (sentBytesTotal < dataSize) {
    size_t sentBytes = write(socket, (unsigned char *)data + sentBytesTotal, dataSize - sentBytesTotal);
    sentBytesTotal += sentBytes;
  }
}

void *sendData(void *data) {
  DATA * pdata = (DATA *)data;
  // send username
  data_writeData(pdata->socket, pdata->userName, USER_LENGTH);

  char **inputBuffer = NULL;
  while (!data_isStopped(pdata)) {
    // while (1) {
    initInputBuffer(&inputBuffer);
    readInput(inputBuffer);

    // check again if connection has ended while waiting for user input
    if (data_isStopped(pdata)) {
      continue;
    }

    if (strcmp(inputBuffer[0], "quit") == 0) {
      // send quit to peer
      printf("[system] Ending connection... Notifying peer.\n");
      data_writeData(pdata->socket, inputBuffer[0], INPUT_BUFFER_CELL_SIZE);
      data_stop(pdata);
      continue;
    }

    if (strcmp(inputBuffer[0], "alg1") != 0 &&
        strcmp(inputBuffer[0], "alg2") != 0 &&
        strcmp(inputBuffer[0], "plain") != 0) {
      printf("[system] Wrong input. Try again with correct mode - "
             "plain/alg1/alg2.\n");
      continue;
    }

    // for every specified file
    for (int i = 1; i < INPUT_BUFFER_CELL_COUNT; i++) {
      if (strcmp(inputBuffer[i], "") == 0) {
        break;
      }

      // load file into variable
      char *fileData;
      int fileSize;
      bool result = loadFileData(inputBuffer[i], &fileData, &fileSize);
      if (!result) {
        printf("[system] Could not read the file \"%s\". Skpping...\n",
               inputBuffer[i]);
        continue;
      } else {
        printf("[system] Successfuly read file.\n");
      }

      //if compress then compress
      //if compress2 then compress2
      //send compression method
      data_writeData(pdata->socket, inputBuffer[0], INPUT_BUFFER_CELL_SIZE);
      //send file size
      data_writeData(pdata->socket, &fileSize, sizeof(int));
      //send file name
      data_writeData(pdata->socket, inputBuffer[i], INPUT_BUFFER_CELL_SIZE);
      //send file over socket
      printf("[system] Sending file %s of size %d.\n", inputBuffer[i], fileSize);
      data_writeData(pdata->socket, fileData, fileSize);
      printf("[system] Successfully sent file %s to peer.\n", inputBuffer[i]);

      free(fileData);
    }

    destroyInputBuffer(&inputBuffer);
  }

  if (inputBuffer != NULL) {
    destroyInputBuffer(&inputBuffer);
  }

  return NULL;
}

void data_readData(int socket, void *data, size_t dataSize) {
  size_t receivedBytesTotal = 0;

  while (receivedBytesTotal < dataSize) {
    size_t receivedBytes = read(socket, (unsigned char *)data + receivedBytesTotal, dataSize - receivedBytesTotal);
    receivedBytesTotal += receivedBytes;
  }
}

void *receiveData(void *data) {
  DATA *pdata = (DATA *)data;

  //read and save username
  char userName[USER_LENGTH + 1];
  userName[USER_LENGTH] = '\0';
  bzero(userName, USER_LENGTH);
  data_readData(pdata->socket, userName, USER_LENGTH);
  printf("[%s] Peer joined.\n", userName);

  while (!data_isStopped(pdata)) {
    // while (1) {
    // read compression method / or end of messaging
    char compressionMethod[INPUT_BUFFER_CELL_SIZE];
    data_readData(pdata->socket, compressionMethod, INPUT_BUFFER_CELL_SIZE);

    // if user sends quit, then stop
    if (strcmp(compressionMethod, "quit") == 0) {
      //if quit was initiated from this application and is just receiving response to quit
      if (data_isStopped(pdata)) {
        continue;
      }

      //also send quit to peer LISTENING thread
      char * quitMessage = "quit";
      data_writeData(pdata->socket, quitMessage, INPUT_BUFFER_CELL_SIZE);
      //print statement that connection has ended
      printf("[%s] Peer ended connection... Press any key to continue.\n", userName);
      data_stop(pdata);
      continue;
    }

    // read file size
    int fileSize;
    data_readData(pdata->socket, &fileSize, sizeof(int));
    // read file name
    char fileName[INPUT_BUFFER_CELL_SIZE];
    data_readData(pdata->socket, fileName, INPUT_BUFFER_CELL_SIZE);
    // read file
    printf("[%s] Incoming file %s with size %d.\n", userName, fileName, fileSize);
    char * fileData = calloc(fileSize, sizeof(char));
    data_readData(pdata->socket, fileData, fileSize);
    // maybe decompress?
    // save file
    bool result = writeFileData(fileName, fileData, fileSize);
    if (!result) {
      printf("[%s] Could not write the file \"%s\". Skpping...\n",
             userName, fileName);
    } else {
      printf("[%s] Successfuly written file \"%s\".\n", userName, fileName);
    }

    //free allocated memory
    free(fileData);
  }

  return NULL;
}
